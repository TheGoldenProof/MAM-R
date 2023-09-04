#include "MyException.h"
#include <sstream>

MyException::MyException(i32 line, const char* file) noexcept : line(line), file(file) {}

MyException::MyException(i32 line, const char* file, const std::string& msg) noexcept : line(line), file(file), msg(msg) {}

const char* MyException::what() const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl;
	if (!msg.empty()) oss << msg << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* MyException::GetType() const noexcept { return "MyException"; }

i32 MyException::GetLine() const noexcept { return line; }

const std::string& MyException::GetFile() const noexcept { return file; }
std::string MyException::GetOriginString() const noexcept {
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
		<< "[Line] " << line;
	return oss.str();
}