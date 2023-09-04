#include "imgui/imgui_impl_win32.h"
#include "mamr_defs.h"
#include "../resource.h"
#include "Windows\Callbacks\Callbacks.h"
#include "Windows/Window.h"
#include "Windows/WindowMacros.h"
#include <sstream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Window::WinAPIClass Window::WinAPIClass::wndClass;

Window::WinAPIClass::WinAPIClass() noexcept : hInstance(GetModuleHandle(nullptr)) {
    WNDCLASSEXW wcex{0};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc    = HandleMsgSetup;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAMR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = nullptr;
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MAMR);
    wcex.lpszClassName  = wndClassName;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAMR));
}

Window::WinAPIClass::~WinAPIClass() { UnregisterClass(wndClassName, hInstance); }

LPCWSTR Window::WinAPIClass::GetName() noexcept { return wndClassName; }
HINSTANCE Window::WinAPIClass::GetInstance() noexcept { return wndClass.hInstance; }
HACCEL Window::WinAPIClass::GetAccelTable() noexcept { return wndClass.hAccelTable; }

Window::Window(i32 width, i32 height, LPCWSTR title) : width(width), height(height) {
    RECT wr{128, 128, 128+width, 128+height};
    LONG wstyle = WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME; 

    if (AdjustWindowRect(&wr, wstyle , FALSE) == 0) throw MYWND_LAST_EXCEPT();

    hWnd = CreateWindowW(WinAPIClass::GetName(), title, wstyle,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right-wr.left, wr.bottom-wr.top, 
        nullptr, nullptr, WinAPIClass::GetInstance(), this);

    if (hWnd == nullptr) throw MYWND_LAST_EXCEPT();

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    ImGui_ImplWin32_Init(hWnd);

    pGfx = std::make_unique<Graphics>(hWnd, width, height);
}

Window::~Window() { 
    ImGui_ImplWin32_Shutdown();

    DestroyWindow(hWnd); 
    /*MSG msg = MSG();
    while (msg.message != WM_QUIT) {
        GetMessage(&msg, 0, 0, 0);
    }*/
}

void Window::SetTitle(LPCWSTR title) {
    if (SetWindowText(hWnd, title) == 0) throw MYWND_LAST_EXCEPT();
}

std::optional<i32> Window::ProcessMessages() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) return (i32)msg.wParam;

        if (!TranslateAccelerator(msg.hwnd, WinAPIClass::GetAccelTable(), &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return {};
}

Graphics& Window::Gfx() { 
    if (!pGfx) throw MYWND_NOGFX_EXCEPT();
    return *pGfx; 
}

void Window::OnResize(u32 width_, u32 height_) {
    width = static_cast<i32>(width_);
    height = static_cast<i32>(height_);
    if (pGfx) Gfx().OnResize(width_, height_);
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        const CREATESTRUCT* const pCreate = (CREATESTRUCTW*)lParam;
        Window* const pWnd = (Window*)pCreate->lpCreateParams;

        SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(pWnd));
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, LONG_PTR(&Window::HandleMsgThunk));

        return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return ((Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA))->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND _hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    ImGuiIO const& imio = ImGui::GetIO();

    switch (msg) {
    case WM_COMMAND: {
        i32 wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDM_ABOUT:
            DialogBox(WinAPIClass::GetInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, WindowCallbacks::About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
    }
    break;

    //case WM_PAINT: {
    //    PAINTSTRUCT ps;
    //    HDC hdc = BeginPaint(_hWnd, &ps);
    //    UNREFERENCED_PARAMETER(hdc);
    //    // TODO: Add any drawing code that uses hdc here...
    //    EndPaint(_hWnd, &ps);
    //}
    //break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KILLFOCUS:
        kbd.ClearState();
        break;
    case WM_SIZE:
        OnResize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (imio.WantCaptureKeyboard) break;
        if (!(lParam & (1 << 30)) || kbd.AutorepeatIsEnabled())
            kbd.OnKeyPressed((u8)wParam);
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (imio.WantCaptureKeyboard) break;
        kbd.OnKeyReleased((u8)wParam);
        break;
    case WM_CHAR:
        if (imio.WantCaptureKeyboard) break;
        kbd.OnChar((char)wParam);
        break;

    case WM_MOUSEMOVE: {
        if (imio.WantCaptureMouse) break;
        const POINTS pt = MAKEPOINTS(lParam);
        if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height) {
            mouse.OnMouseMove(pt.x, pt.y);
            if (!mouse.IsInWindow()) {
                SetCapture(hWnd);
                mouse.OnMouseEnter();
            } else {
                if (wParam & (MK_LBUTTON | MK_RBUTTON)) {
                    mouse.OnMouseMove(pt.x, pt.y);
                } else {
                    ReleaseCapture();
                    mouse.OnMouseLeave();
                }
            }
        }
        break;
    }
    case WM_LBUTTONDOWN: {
        if (imio.WantCaptureMouse) break;
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnLeftPressed(pt.x, pt.y);
        SetForegroundWindow(hWnd);
        break;
    }
    case WM_LBUTTONUP: {
        if (imio.WantCaptureMouse) break;
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnLeftReleased(pt.x, pt.y);
        if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height) {
            ReleaseCapture();
            mouse.OnMouseLeave();
        }
        break;
    }
    case WM_RBUTTONDOWN: {
        if (imio.WantCaptureMouse) break;
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnRightPressed(pt.x, pt.y);
        SetForegroundWindow(hWnd);
        break;
    }
    case WM_RBUTTONUP: {
        if (imio.WantCaptureMouse) break;
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnRightReleased(pt.x, pt.y);
        if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height) {
            ReleaseCapture();
            mouse.OnMouseLeave();
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        if (imio.WantCaptureMouse) break;
        const POINTS pt = MAKEPOINTS(lParam);
        const i32 delta = GET_WHEEL_DELTA_WPARAM(wParam);
        mouse.OnWheelDelta(pt.x, pt.y, delta);
        break;
    }

    default:
        return DefWindowProc(_hWnd, msg, wParam, lParam);
    }
    return 0;
}

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept {
    char* msgBuf;
    DWORD msgLen = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, hr, 0, (LPSTR)(&msgBuf), 0, nullptr);
    if (msgLen == 0) {
        return "Unknown error code";
    }
    std::string errorString(msgBuf);
    LocalFree(msgBuf);
    return errorString;
}

Window::HrException::HrException(i32 line, const char* file, HRESULT hr) noexcept : Exception(line, file), hr(hr) {}

const char* Window::HrException::what() const noexcept {
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "[Error Code]  " << GetErrorCode() << std::endl
        << "[Description] " << GetErrorString() << std::endl
        << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept { return "Windows MyException"; }

HRESULT Window::HrException::GetErrorCode() const noexcept { return hr; }

std::string Window::HrException::GetErrorString() const noexcept { return TranslateErrorCode(hr); }

const char* Window::NoGfxException::GetType() const noexcept { return "Windows NoGfxException"; }
