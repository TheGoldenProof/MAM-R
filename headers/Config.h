#pragma once
#include "TGLib\TGLib.h"
#include <map>
#include <vector>

class Config {
private:
	std::map<std::string, std::vector<u8>> cfgMap; // <setting id, data>
	std::wstring currPath;
public:
	Config();

	template <typename T>
	void Set(const std::string& id, const T& val) {
		const u8* data = reinterpret_cast<const u8*>(&val);
		std::vector<u8>& vec = cfgMap[id];
		vec.clear(); vec.resize(sizeof(T));
		memcpy(vec.data(), data, sizeof(T));
	}
	template <typename T>
	void Set(const std::string& id, const T* pVal, usize count = 1) {
		const u8* data = reinterpret_cast<const u8*>(pVal);
		std::vector<u8>& vec = cfgMap[id];
		vec.clear(); vec.resize(count * sizeof(T));
		memcpy(vec.data(), data, count * sizeof(T));
	}

	template <typename T>
	usize Get(const std::string& id, T* dest, usize maxCount = (SIZE_MAX/sizeof(T))) const {
		if (!dest || maxCount == 0) return 0;
		if (const auto it = cfgMap.find(id); it != cfgMap.end()) {
			usize writeCount = std::min(it->second.size(), maxCount * sizeof(T));
			memcpy(dest, it->second.data(), writeCount);
			return writeCount;
		};
		return 0;
	}

	std::map<std::string, std::vector<u8>>& operator()() noexcept;
	const std::map<std::string, std::vector<u8>>& operator()() const noexcept;

	void Clear();
	void Open(class Globe& gb);
	void Save(class Globe& gb);
	void SaveAs(class Globe& gb);
private:
	void Read(const std::wstring& path);
	void Write(const std::wstring& path);
};