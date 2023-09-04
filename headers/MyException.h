#pragma once

#include <exception>
#include <string>
#include "TGLib/TGLib.h"

class MyException : public std::exception {
public:
	MyException(i32 line, const char* file) noexcept;
	MyException(i32 line, const char* file, const std::string& msg) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	i32 GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;
private:
	i32 line;
	std::string file;
	std::string msg;
protected:
	mutable std::string whatBuffer;
};

#define MYDEF_EXCEPT() MyException(__LINE__, __FILE__)