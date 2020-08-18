#include <Windows.h>

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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_CLOSE:
  case WM_DESTROY: PostQuitMessage(0); return 0;
  default: return DefWindowProcW(hWnd, message, wParam, lParam);
  }
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR,
                    _In_ int nCmdShow)
{
  constexpr auto Title  = L"Citadel";
  constexpr auto Width  = 800;
  constexpr auto Height = 600;

  WNDCLASSW wc{};
  wc.lpfnWndProc   = WndProc;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
  wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  wc.lpszClassName = Title;

  if (!RegisterClassW(&wc))
    return -1;

  RECT windowRect{ .left = 0, .top = 0, .right = Width, .bottom = Height };
  AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
  int w = windowRect.right - windowRect.left;
  int h = windowRect.bottom - windowRect.top;

  HWND hWnd = CreateWindowExW(0, Title, Title, WS_OVERLAPPEDWINDOW,
                              (GetSystemMetrics(SM_CXSCREEN) - w) / 2,
                              (GetSystemMetrics(SM_CYSCREEN) - h) / 2, w, h,
                              nullptr, nullptr, hInstance, nullptr);

  if (!hWnd)
    return -2;

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

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
