#define VTX_IMPL
#include "Graphics/Vertex.h"

namespace Vtx {

template<VertexLayout::ElementType Type>
struct SysSizeLookup {
	static constexpr auto Exec() noexcept {
		return sizeof(VertexLayout::Info<Type>::SysType);
	}
};

template<VertexLayout::ElementType Type>
struct DescGenerate {
	static constexpr D3D11_INPUT_ELEMENT_DESC Exec(usize offset) noexcept {
		return {
			VertexLayout::Info<Type>::semantic, 0,
			VertexLayout::Info<Type>::dxgiFormat, 0,
			(u32)offset, D3D11_INPUT_PER_VERTEX_DATA, 0
		};
	}
};

template<VertexLayout::ElementType Type>
struct CodeLookup {
	static constexpr auto Exec() noexcept {
		return VertexLayout::Info<Type>::code;
	}
};

constexpr usize VertexLayout::Element::SizeOf(VertexLayout::ElementType type) dbgexcept {
	return Bridge<SysSizeLookup>(type);
}

VertexLayout::Element::Element(VertexLayout::ElementType type, usize offset) : type(type), offset(offset) {}

usize VertexLayout::Element::GetOffsetAfter() const dbgexcept { return offset+Size(); }

usize VertexLayout::Element::GetOffset() const dbgexcept { return offset; }

usize VertexLayout::Element::Size() const dbgexcept { return SizeOf(type); }

VertexLayout::ElementType VertexLayout::Element::GetType() const noexcept { return type; }

D3D11_INPUT_ELEMENT_DESC VertexLayout::Element::GetDesc() const dbgexcept {
	return Bridge<DescGenerate>(type, GetOffset());
}

const char* VertexLayout::Element::GetCode() const noexcept {
	return Bridge<CodeLookup>(type);
}

bool VertexLayout::Element::operator==(const Element& other) {
	return type == other.type && offset == other.offset;
}



const VertexLayout::Element& VertexLayout::ResolveByIndex(usize i) const dbgexcept { return elements[i]; }

usize VertexLayout::Size() const dbgexcept { return elements.empty()? 0 : elements.back().GetOffsetAfter(); }

VertexLayout& VertexLayout::Append(ElementType type) dbgexcept { elements.emplace_back(type, Size()); return *this; }

usize VertexLayout::GetElementCount() const noexcept { return elements.size(); }

std::vector<D3D11_INPUT_ELEMENT_DESC> VertexLayout::GetD3DLayout() const dbgexcept {
	std::vector<D3D11_INPUT_ELEMENT_DESC> ret;
	ret.reserve(GetElementCount());
	for (auto const& e : elements) { ret.push_back(e.GetDesc()); }
	return ret;
}

std::string VertexLayout::GetCode() const dbgexcept {
	std::string code;
	for (const auto& e : elements) code += e.GetCode();
	return code;
}

bool VertexLayout::Has(ElementType type) const noexcept {
	for (auto& e : elements) if (e.GetType() == type) return true;
	return false;
}

bool VertexLayout::operator==(const VertexLayout& other) {
	if (elements.size() != other.elements.size()) return false;
	for (usize i = 0; i < elements.size(); i++)
		if (elements[i] != other.elements[i]) return false;
	return true;
}



Vertex::Vertex(u8* pData, const VertexLayout& layout) dbgexcept : pData(pData), layout(layout) { assert(pData); }

ConstVertex::ConstVertex(const Vertex& v) dbgexcept : vertex(v) {}



VertexBuffer::VertexBuffer(VertexLayout layout, usize size) dbgexcept : layout(std::move(layout)) {
	Resize(size);
}

const u8* VertexBuffer::GetData() const dbgexcept { return buffer.data(); }

const VertexLayout& VertexBuffer::GetLayout() const noexcept { return layout; }

usize VertexBuffer::Size() const dbgexcept { return buffer.size()/layout.Size(); }

void VertexBuffer::Resize(usize newSize) dbgexcept {
	const usize size = Size();
	if (size < newSize)
		buffer.resize(buffer.size() + layout.Size() * (newSize - size));
}

void VertexBuffer::Reserve(usize newCapacity) {
	buffer.reserve(layout.Size() * newCapacity);
}

void VertexBuffer::Clear() {
	buffer.clear();
}

Vertex VertexBuffer::Back() dbgexcept {
	assert(buffer.size() != 0);
	return Vertex{buffer.data() + buffer.size() - layout.Size(), layout};
}

Vertex VertexBuffer::Front() dbgexcept {
	assert(buffer.size() != 0);
	return Vertex{buffer.data(), layout};
}

Vertex VertexBuffer::operator[](usize i) dbgexcept {
	assert(i < Size());
	return Vertex{buffer.data() + layout.Size()*i, layout};
}

ConstVertex VertexBuffer::Back() const dbgexcept {
	return const_cast<VertexBuffer*>(this)->Back();
}

ConstVertex VertexBuffer::Front() const dbgexcept {
	return const_cast<VertexBuffer*>(this)->Front();
}

ConstVertex VertexBuffer::operator[](usize i) const dbgexcept {
	return const_cast<VertexBuffer&>(*this)[i];
}

}
