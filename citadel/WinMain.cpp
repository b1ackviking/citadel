#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <array>
#include <cstdlib>
#include <ctime>
#include <cwchar>
#include <stdexcept>

#ifndef NDEBUG
#include <crtdbg.h>

int main()
{
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);

  HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  _CrtSetBreakAlloc(0);

  return wWinMain(GetModuleHandleW(nullptr), nullptr, GetCommandLineW(),
                  SW_SHOW);
}
#endif // !NDEBUG

class App
{
private:
  HINSTANCE m_hInstance;
  HWND m_hWnd;
  ATOM m_WindowClass;

  // NOTE: clang-format doesn't recognize attributes [[likely]/[[unlikely]]
  // clang-format off
  static LRESULT CALLBACK m_fnStaticWndProc(HWND hWnd, UINT message,
                                            WPARAM wParam, LPARAM lParam)
  {
    if (message == WM_CREATE) [[unlikely]]
    {
      auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
      SetWindowLongPtrW(hWnd, GWLP_USERDATA,
                        reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
    }
    else [[likely]]
    {
      App* app = reinterpret_cast<App*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
      if (app)
        return app->m_fnWndProc(hWnd, message, wParam, lParam);
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
  }
  // clang-format on

  LRESULT m_fnWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    switch (message)
    {
    case WM_CLOSE:
    case WM_DESTROY: PostQuitMessage(0); return 0;
    default: return DefWindowProcW(hWnd, message, wParam, lParam);
    }
  }

public:
  App(const App&) = delete;
  App& operator=(const App&) = delete;

  App(const wchar_t* Title = L"Citadel", int Width = 800, int Height = 600)
  {
    m_hInstance = GetModuleHandleW(nullptr);

    std::array<wchar_t, 30> windowClassName{};
    std::swprintf(windowClassName.data(), windowClassName.size(), L"Citadel_%u",
                  std::rand());

    WNDCLASSW wc{};
    wc.lpfnWndProc   = m_fnStaticWndProc;
    wc.hInstance     = m_hInstance;
    wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = windowClassName.data();

    if (m_WindowClass = RegisterClassW(&wc); !m_WindowClass)
      throw std::runtime_error{ "Failed to register a window class\n" };

    RECT windowRect{ .left = 0, .top = 0, .right = Width, .bottom = Height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
    int w = windowRect.right - windowRect.left;
    int h = windowRect.bottom - windowRect.top;

    m_hWnd = CreateWindowExW(0, reinterpret_cast<LPCWSTR>(m_WindowClass), Title,
                             WS_OVERLAPPEDWINDOW,
                             (GetSystemMetrics(SM_CXSCREEN) - w) / 2,
                             (GetSystemMetrics(SM_CYSCREEN) - h) / 2, w, h,
                             nullptr, nullptr, m_hInstance, this);

    if (!m_hWnd)
      throw std::runtime_error{ "Failed to create a window\n" };

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
  }

  ~App()
  {
    DestroyWindow(m_hWnd);
    UnregisterClassW(reinterpret_cast<LPCWSTR>(m_WindowClass), m_hInstance);
  }

  int Run()
  {
    MSG msg{};
    bool running = true;
    while (running)
    {
      while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
      {
        if (msg.message == WM_QUIT)
          running = false;

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
      }

      Sleep(15);
    }

    return static_cast<int>(msg.wParam);
  }
};

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
  std::srand(static_cast<unsigned int>(std::time(nullptr)));

  App app{ L"Citadel", 1280, 720 };

  return app.Run();
}
