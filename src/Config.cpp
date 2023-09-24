#include "Config.h"
#include "Globe.h"
#include "Windows\Window.h"
#include <fstream>

Config::Config() {
}

std::map<std::string, std::vector<u8>>& Config::operator()() noexcept {
	return cfgMap;
}

const std::map<std::string, std::vector<u8>>& Config::operator()() const noexcept {
	return cfgMap;
}

void Config::Clear() {
	cfgMap.clear();
}

void Config::Open(Globe& gb) {
	static std::vector<std::pair<const wchar_t*, const wchar_t*>> fileTypes = { {L"Config files", L"*.omvcfg"}, {L"All files", L"*.*"} };
	currPath = gb.Wnd().OpenFile(fileTypes, 1, L".omvcfg");
	Read(currPath);
}

void Config::Save(Globe& gb) {
	if (currPath.empty()) { SaveAs(gb); return; }
	Write(currPath);
}

void Config::SaveAs(Globe& gb) {
	static std::vector<std::pair<const wchar_t*, const wchar_t*>> fileTypes = { {L"Config files", L"*.omvcfg"}, {L"All files", L"*.*"} };
	currPath = gb.Wnd().SaveFile(fileTypes, 1, L".omvcfg");
	Save(gb);
}

static bool checkStream(std::ifstream& file) {
	return file.good();
	/*std::string str("000");
	auto state = file.rdstate();
	if (state & std::ios::badbit) str[1] = '1';
	if (state & std::ios::failbit) str[2] = '1';
	if (state & std::ios::eofbit) str[3] = '1';
	DEBUG_LOG(("Stream state: " + str + "\n").c_str());
	return !strncmp(str.c_str(), "000", 3);*/
}

#define READ(var) file.read(reinterpret_cast<char*>(&var), sizeof(var)); if (!checkStream(file)) return

void Config::Read(const std::wstring& path) {
	std::ifstream file(path, std::ios::binary);
	if (!file.good()) return;

	u8 hLength = 0; READ(hLength);
	auto pos = file.tellg();
	u8 version = 0; READ(version);
	file.seekg(pos + static_cast<std::streamoff>(hLength));

	while (true) {
		std::string id;
		std::getline(file, id, '\0');
		u32 dLength = 0; READ(dLength);
		std::vector<u8> data(dLength);
		file.read(reinterpret_cast<char*>(data.data()), dLength);
		cfgMap[id] = std::move(data);
		if (!checkStream(file)) return;
	}
}

#undef READ

void Config::Write(const std::wstring& path) {
	std::ofstream file(path, std::ios::binary);

	file.put(1); // header size
	file.put(1); // config version

	for (const auto& p : cfgMap) {
		file.write(p.first.c_str(), p.first.size());
		file.put(0); // null terminator
		u32 dataSize = static_cast<u32>(p.second.size());
		file.write((char*)&dataSize, sizeof(dataSize));
		file.write((char*)p.second.data(), dataSize);
	}
}

