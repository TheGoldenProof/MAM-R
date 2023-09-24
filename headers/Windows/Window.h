#pragma once

#include "Graphics/Graphics.h"
#include "MyException.h"
#include "TGLib/TGLib.h"
#include "Windows/framework.h"
#include "Windows/Keyboard.h"
#include "Windows/Mouse.h"
#include <memory>
#include <vector>

class Window {
public:
	class Exception : public MyException {
		using MyException::MyException;
	public:
		static std::wstring TranslateErrorCode(HRESULT hr) noexcept;
	};
	class HrException : public Exception {
	public:
		HrException(i32 line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		virtual const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
	private:
		HRESULT hr;
	};
	class NoGfxException : public Exception {
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;
	};
private: 
	class WinAPIClass {
	public:
		static LPCWSTR GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
		static HACCEL GetAccelTable() noexcept;
	private:
		WinAPIClass() noexcept;
		WinAPIClass(const WinAPIClass&) = delete;
		~WinAPIClass();
		WinAPIClass& operator=(const WinAPIClass&) = delete;
		static constexpr LPCWSTR wndClassName = L"MAMR";
		static WinAPIClass wndClass;
		HINSTANCE hInstance;
		HACCEL hAccelTable;
	};
public:
	Window(i32 width, i32 height, LPCWSTR name);
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	HWND GetHWND() const noexcept;
	void SetTitle(LPCWSTR title);
	static std::optional<i32> ProcessMessages();
	Graphics& Gfx();
	
	void OnResize(u32 width, u32 height);

	void SetBorderlessFullscreen(bool val);
	bool IsFullscreen() const noexcept;

	std::wstring OpenFile(const std::vector<std::pair<const wchar_t*, const wchar_t*>>& saveTypes, u32 fileTypeIndex, const wchar_t* defaultExtension);
	std::wstring SaveFile(const std::vector<std::pair<const wchar_t*, const wchar_t*>>& saveTypes, u32 fileTypeIndex, const wchar_t* defaultExtension);
private:
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	std::wstring FileDialog(const CLSID& clsid, const std::vector<std::pair<const wchar_t*, const wchar_t*>>& saveTypes, u32 fileTypeIndex, const wchar_t* defaultExtension);
public:
	Keyboard kbd;
	Mouse mouse;
private:
	i32 width;
	i32 height;
	HWND hWnd;
	std::unique_ptr<Graphics> pGfx;

	bool isFullscreen = false;
	RECT windowedRect;
	static constexpr LONG windowedStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;
	static constexpr LONG borderlessStyle = WS_POPUP;
};
