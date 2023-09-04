#define CBD_IMPL
#include "Graphics/ConstantBufferDyn.h"
#include <algorithm>
#include <cctype>
#include <string>

namespace CBD {

struct ExtraData {
	struct Struct : public LayoutElement::ExtraDataBase {
		std::vector<std::pair<std::string, LayoutElement>> layoutElements;
	};
	struct Array : public LayoutElement::ExtraDataBase {
		std::optional<LayoutElement> layoutElement;
		usize size;
	};
};

#pragma region LayoutElement
std::string LayoutElement::GetSignature() const dbgexcept {
	switch (type) {
	#define X(t) case t: return Info<t>::code;
		BET
			#undef X
	case Struct: return GetSignatureForStruct();
	case Array: return GetSignatureForArray();
	}
	assert("Bad type in signature generation" && false);
	return "???";
}

bool LayoutElement::Exists() const noexcept {
	return type != Empty;
}

std::pair<usize, const LayoutElement*> LayoutElement::CalculateIndexingOffset(usize offset_, usize index) const dbgexcept {
	assert("Indexing into non-array" && type == Array);
	const auto& data = static_cast<ExtraData::Array&>(*pExtraData);
	assert(index < data.size);
	return {offset_ + data.layoutElement->GetSizeInBytes() * index, &*data.layoutElement};
}

LayoutElement& LayoutElement::operator[](const std::string& key) dbgexcept {
	assert("Keying into non-struct" && type == Struct);
	for (auto& mem : static_cast<ExtraData::Struct&>(*pExtraData).layoutElements) {
		if (mem.first == key) return mem.second;
	}
	return GetEmptyElement();
}

const LayoutElement& LayoutElement::operator[](const std::string& key) const dbgexcept {
	return const_cast<LayoutElement&>(*this)[key];
}

LayoutElement& LayoutElement::T() dbgexcept {
	assert("Accessing T of non-array" && type == Array);
	return *static_cast<ExtraData::Array&>(*pExtraData).layoutElement;
}

const LayoutElement& LayoutElement::T() const dbgexcept {
	return const_cast<LayoutElement&>(*this).T();
}

usize LayoutElement::GetOffsetBegin() const dbgexcept { return *offset; }

usize LayoutElement::GetOffsetEnd() const dbgexcept {
	switch (type) {
	#define X(t) case t: return *offset + Info<t>::hlslSize;
	BET
	#undef X
	case Struct: {
		const auto& data = static_cast<ExtraData::Struct&>(*pExtraData);
		return AdvanceToBoundary(data.layoutElements.back().second.GetOffsetEnd());
	}
	case Array: {
		const auto& data = static_cast<ExtraData::Array&>(*pExtraData);
		return *offset + AdvanceToBoundary(data.layoutElement->GetSizeInBytes()) * data.size;
	}
	}
	assert("Tried to get offset of empty or invalid element" && false);
	return 0;
}

usize LayoutElement::GetSizeInBytes() const dbgexcept {
	return GetOffsetEnd() - GetOffsetBegin();
}

LayoutElement& LayoutElement::Add(Type type_, const std::string& name) {
	assert("Add to non-struct in layout" && type == Struct);
	assert("invalid symbol name in struct" && ValidateSymbolName(name));
	auto& structData = static_cast<ExtraData::Struct&>(*pExtraData);
	for (auto& mem : structData.layoutElements) 
		if (mem.first == name) assert("Adding duplicate name to struct" && false);

	structData.layoutElements.emplace_back(name, LayoutElement{type_});
	return *this;
}

LayoutElement& LayoutElement::Set(Type type_, usize size) dbgexcept {
	assert("Set on non-array in layout" && type == Array);
	assert(size != 0);
	auto& arrayData = static_cast<ExtraData::Array&>(*pExtraData);
	arrayData.layoutElement ={type_};
	arrayData.size = size;
	return *this;
}

LayoutElement::LayoutElement(Type type_) dbgexcept : type(type_) {
	assert(type_ != Empty);
	if (type_ == Struct) {
		pExtraData = std::unique_ptr<ExtraData::Struct>{new ExtraData::Struct()};
	} else if (type_ == Array) {
		pExtraData = std::unique_ptr<ExtraData::Array>{new ExtraData::Array()};
	}
}

std::string LayoutElement::GetSignatureForStruct() const dbgexcept {
	std::string sig{"St{"};
	for (const auto& el : static_cast<ExtraData::Struct&>(*pExtraData).layoutElements)
		sig += el.first + ":" + el.second.GetSignature() + ";";
	sig += "}";
	return sig;
}

std::string LayoutElement::GetSignatureForArray() const dbgexcept {
	const auto& data = static_cast<ExtraData::Array&>(*pExtraData);
	return "Ar:" + std::to_string(data.size) + "{" + data.layoutElement->GetSignature() + "}";
}

usize LayoutElement::Finalize(usize offset_) dbgexcept {
	switch (type) {
	#define X(t) case t: offset = AdvanceIfCrossesBoundary(offset_, Info<t>::hlslSize); return *offset + Info<t>::hlslSize;
		BET
			#undef X
	case Struct: return FinalizeForStruct(offset_);
	case Array: return FinalizeForArray(offset_);
	}
	assert("Bad type in size computation");
	return 0;
}

usize LayoutElement::FinalizeForStruct(usize offset_) dbgexcept {
	auto& data = static_cast<ExtraData::Struct&>(*pExtraData);
	assert(data.layoutElements.size() != 0);
	offset = AdvanceToBoundary(offset_);
	usize offsetNext = *offset;
	for (auto& el : data.layoutElements)
		offsetNext = el.second.Finalize(offsetNext);
	return offsetNext;
}

usize LayoutElement::FinalizeForArray(usize offset_) dbgexcept {
	auto& data = static_cast<ExtraData::Array&>(*pExtraData);
	assert(data.size != 0);
	offset_ = AdvanceToBoundary(offset_);
	data.layoutElement->Finalize(*offset);
	return GetOffsetEnd();
}

LayoutElement& LayoutElement::GetEmptyElement() noexcept {
	static LayoutElement empty;
	return empty;
}

bool LayoutElement::CrossesBoundary(usize offset, usize size) noexcept {
	const usize end = offset + size;
	const usize pageStart = offset/16;
	const usize pageEnd = end/16;
	return (pageStart != pageEnd && end%16 != 0) || size > 16;
}

usize LayoutElement::AdvanceIfCrossesBoundary(usize offset, usize size) noexcept {
	return CrossesBoundary(offset, size) ? AdvanceToBoundary(offset) : offset;
}

usize LayoutElement::AdvanceToBoundary(usize offset) noexcept {
	return offset + (16 - offset%16) % 16;
}

bool LayoutElement::ValidateSymbolName(const std::string& name) noexcept {
	return !name.empty() && !std::isdigit(name.front()) &&
		std::all_of(name.begin(), name.end(), [](char c) {
			return std::isalnum(c) || c == '_';
		});
}

#pragma endregion

#pragma region Layouts
usize Layout::GetSizeInBytes() const noexcept {
	return pRoot->GetSizeInBytes();
}

std::string Layout::GetSignature() const dbgexcept {
	return pRoot->GetSignature();
}

Layout::Layout(std::shared_ptr<LayoutElement> pRoot) noexcept : pRoot(std::move(pRoot)) {}

RawLayout::RawLayout() noexcept : Layout(std::shared_ptr<LayoutElement>{new LayoutElement(Struct)}) {}

LayoutElement& RawLayout::operator[](const std::string& key) dbgexcept {
	return (*pRoot)[key];
}

LayoutElement& RawLayout::Add(Type type, const std::string& key) dbgexcept {
	return pRoot->Add(type, key);
}

void RawLayout::ClearRoot() noexcept {
	*this = RawLayout();
}

std::shared_ptr<LayoutElement> RawLayout::DeliverRoot() noexcept {
	auto temp = std::move(pRoot);
	temp->Finalize(0);
	ClearRoot();
	return std::move(temp);
}

CookedLayout::CookedLayout(std::shared_ptr<LayoutElement> pRoot) noexcept : Layout(std::move(pRoot)) {}

const LayoutElement& CookedLayout::operator[](const std::string& key) const dbgexcept {
	return (*pRoot)[key];
}

std::shared_ptr<LayoutElement> CookedLayout::ShareRoot() const noexcept {
	return pRoot;
}

std::shared_ptr<LayoutElement> CookedLayout::DeliverRoot() const noexcept {
	return std::move(pRoot);
}
#pragma endregion

CookedLayout LayoutCodex::Resolve(RawLayout&& layout) dbgexcept {
	std::string sig = layout.GetSignature();
	auto& map = Get_().map;
	const auto i = map.find(sig);

	if (i != map.end()) {
		layout.ClearRoot();
		return {i->second};
	}

	auto result = map.insert({std::move(sig), layout.DeliverRoot()});
	return {result.first->second};
}

LayoutCodex& LayoutCodex::Get_() noexcept {
	static LayoutCodex codex;
	return codex;
}

#pragma region ElementRefs
ConstElementRef::Ptr::Ptr(const ConstElementRef* ref) noexcept : ref(ref) {}

ConstElementRef::ConstElementRef(const LayoutElement* pLayout, const u8* pBytes, usize offset) noexcept 
	: offset(offset), pLayout(pLayout), pBytes(pBytes) {}

bool ConstElementRef::Exists() const noexcept {
	return pLayout->Exists();
}

ConstElementRef ConstElementRef::operator[](const std::string& key) const dbgexcept {
	return {&(*pLayout)[key], pBytes, offset};
}

ConstElementRef ConstElementRef::operator[](usize index) const dbgexcept {
	const auto indexingData = pLayout->CalculateIndexingOffset(offset, index);
	return {indexingData.second, pBytes, indexingData.first};
}

ConstElementRef::Ptr ConstElementRef::operator&() const dbgexcept {
	return Ptr{this};
}

ElementRef::Ptr::Ptr(ElementRef* ref) noexcept : ref(ref) {}

ElementRef::ElementRef(const LayoutElement* pLayout, u8* pBytes, usize offset) noexcept 
	: offset(offset), pLayout(pLayout), pBytes(pBytes) {}

ElementRef::operator ConstElementRef() const noexcept {
	return {pLayout, pBytes, offset};
}

bool ElementRef::Exists() const noexcept {
	return pLayout->Exists();
}

ElementRef ElementRef::operator[](const std::string& key) const dbgexcept {
	return {&(*pLayout)[key], pBytes, offset};
}

ElementRef ElementRef::operator[](usize index) const dbgexcept {
	const auto indexingData = pLayout->CalculateIndexingOffset(offset, index);
	return {indexingData.second, pBytes, indexingData.first};
}

ElementRef::Ptr ElementRef::operator&() const dbgexcept {
	return Ptr{const_cast<ElementRef*>(this)};
}
#pragma endregion

#pragma region Buffer
Buffer::Buffer(RawLayout&& layout) dbgexcept : Buffer(LayoutCodex::Resolve(std::move(layout))) {}
Buffer::Buffer(const CookedLayout& layout) dbgexcept : pLayoutRoot(layout.ShareRoot()), bytes(pLayoutRoot->GetOffsetEnd()) {}
Buffer::Buffer(CookedLayout&& layout) dbgexcept : pLayoutRoot(layout.DeliverRoot()), bytes(pLayoutRoot->GetOffsetEnd()) {}
Buffer::Buffer(const Buffer& other) noexcept : pLayoutRoot(other.pLayoutRoot), bytes(other.bytes) {}
Buffer::Buffer(Buffer&& other) noexcept : pLayoutRoot(std::move(other.pLayoutRoot)), bytes(std::move(other.bytes)) {}

ElementRef Buffer::operator[](const std::string& key) dbgexcept {
	return {&(*pLayoutRoot)[key], bytes.data(), 0};
}

ConstElementRef Buffer::operator[](const std::string& key) const dbgexcept {
	return const_cast<Buffer&>(*this)[key];
}

const u8* Buffer::GetData() const noexcept { return bytes.data(); }
usize Buffer::GetSizeInBytes() const noexcept { return bytes.size(); }

const LayoutElement& Buffer::GetRootLayoutElement() const noexcept { return *pLayoutRoot; }
std::shared_ptr<LayoutElement> Buffer::ShareLayoutRoot() const noexcept { return pLayoutRoot; }

void Buffer::CopyFrom(const Buffer& other) dbgexcept {
	assert("Buffer layouts must match to copy data." && &GetRootLayoutElement() == &other.GetRootLayoutElement());
	std::copy(other.bytes.begin(), other.bytes.end(), bytes.begin());
}
#pragma endregion

}