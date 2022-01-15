#include <windows.h>
#include <D2d1.h>
#include <assert.h>
#include <atlbase.h>
#pragma comment(lib, "d2d1")

#include "math/types.hpp"

template <class DERIVED_TYPE> class BaseWindow {
  public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                       LPARAM lParam) {
        DERIVED_TYPE * pThis = NULL;
        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT * pCreate = (CREATESTRUCT *)lParam;
            pThis                  = (DERIVED_TYPE *)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
            pThis->m_hwnd = hwnd;
        } else {
            pThis = (DERIVED_TYPE *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        if (pThis) {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        } else {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    BaseWindow() : m_hwnd(NULL) {
    }
    BOOL Create(const char * lpWindowName, DWORD dwStyle, DWORD dwExStyle = 0,
                i32 x = CW_USEDEFAULT, i32 y = CW_USEDEFAULT, i32 nWidth = CW_USEDEFAULT,
                i32 nHeight = CW_USEDEFAULT, HWND hWndParent = 0, HMENU hMenu = 0) {
        WNDCLASS wc      = {0};
        wc.lpfnWndProc   = DERIVED_TYPE::WindowProc;
        wc.hInstance     = GetModuleHandle(NULL);
        wc.lpszClassName = ClassName();
        RegisterClass(&wc);
        m_hwnd =
            CreateWindowEx(dwExStyle, ClassName(), lpWindowName, dwStyle, x, y, nWidth,
                           nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this);
        return (m_hwnd ? TRUE : FALSE);
    }
    HWND Window() const {
        return m_hwnd;
    }

  protected:
    virtual const char * ClassName() const                                      = 0;
    virtual LRESULT      HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    HWND                 m_hwnd;
};

class GraphicsScene {
  protected:
    CComPtr<ID2D1Factory>          m_pFactory;
    CComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;
    f32                          m_fScaleX;
    f32                          m_fScaleY;

  protected:
    virtual HRESULT CreateDeviceIndependentResources()  = 0;
    virtual void    DiscardDeviceIndependentResources() = 0;
    virtual HRESULT CreateDeviceDependentResources()    = 0;
    virtual void    DiscardDeviceDependentResources()   = 0;
    virtual void    CalculateLayout()                   = 0;
    virtual void    RenderScene()                       = 0;

  protected:
    HRESULT CreateGraphicsResources(HWND hwnd) {
        HRESULT hr = S_OK;
        if (m_pRenderTarget == NULL) {
            RECT rc;
            GetClientRect(hwnd, &rc);
            D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
            hr               = m_pFactory->CreateHwndRenderTarget(
                              D2D1::RenderTargetProperties(),
                              D2D1::HwndRenderTargetProperties(hwnd, size), &m_pRenderTarget);
            if (SUCCEEDED(hr)) { hr = CreateDeviceDependentResources(); }
            if (SUCCEEDED(hr)) { CalculateLayout(); }
        }
        return hr;
    }
    template <class T> T PixelToDipX(T pixels) const {
        return static_cast<T>(pixels / m_fScaleX);
    }
    template <class T> T PixelToDipY(T pixels) const {
        return static_cast<T>(pixels / m_fScaleY);
    }

  public:
    GraphicsScene() : m_fScaleX(1.0f), m_fScaleY(1.0f) {
    }
    virtual ~GraphicsScene() {
    }
    HRESULT Initialize() {
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
        if (SUCCEEDED(hr)) { CreateDeviceIndependentResources(); }
        return hr;
    }
    void Render(HWND hwnd) {
        HRESULT hr = CreateGraphicsResources(hwnd);
        if (FAILED(hr)) { return; }
        assert(m_pRenderTarget != NULL);
        m_pRenderTarget->BeginDraw();
        RenderScene();
        hr = m_pRenderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET) {
            DiscardDeviceDependentResources();
            m_pRenderTarget.Release();
        }
    }
    HRESULT Resize(i32 x, i32 y) {
        HRESULT hr = S_OK;
        if (m_pRenderTarget) {
            hr = m_pRenderTarget->Resize(D2D1::SizeU(x, y));
            if (SUCCEEDED(hr)) { CalculateLayout(); }
        }
        return hr;
    }
    void CleanUp() {
        DiscardDeviceDependentResources();
        DiscardDeviceIndependentResources();
    }
};

class Scene : public GraphicsScene {
    CComPtr<ID2D1SolidColorBrush> m_pFill;
    CComPtr<ID2D1SolidColorBrush> m_pStroke;
    D2D1_ELLIPSE                  m_ellipse;
    D2D_POINT_2F                  m_Ticks[24];
    HRESULT                       CreateDeviceIndependentResources() {
        return S_OK;
    }
    void DiscardDeviceIndependentResources() {
    }
    HRESULT CreateDeviceDependentResources();
    void    DiscardDeviceDependentResources();
    void    CalculateLayout();
    void    RenderScene();
    void    DrawClockHand(f32 fHandLength, f32 fAngle, f32 fStrokeWidth);
};
HRESULT Scene::CreateDeviceDependentResources() {
    HRESULT hr = m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(1.0f, 1.0f, 0), D2D1::BrushProperties(), &m_pFill);
    if (SUCCEEDED(hr)) {
        hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0),
                                                    D2D1::BrushProperties(), &m_pStroke);
    }
    return hr;
}
void Scene::DrawClockHand(f32 fHandLength, f32 fAngle, f32 fStrokeWidth) {
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(fAngle, m_ellipse.point));
    D2D_POINT_2F endPoint = D2D1::Point2F(
        m_ellipse.point.x, m_ellipse.point.y - (m_ellipse.radiusY * fHandLength));
    m_pRenderTarget->DrawLine(m_ellipse.point, endPoint, m_pStroke, fStrokeWidth);
}
void Scene::RenderScene() {
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
    m_pRenderTarget->FillEllipse(m_ellipse, m_pFill);
    m_pRenderTarget->DrawEllipse(m_ellipse, m_pStroke);
    for (DWORD i = 0; i < 12; i++)
        m_pRenderTarget->DrawLine(m_Ticks[i * 2], m_Ticks[i * 2 + 1], m_pStroke, 2.0f);
    SYSTEMTIME time;
    GetLocalTime(&time);
    const f32 fHourAngle   = (360.0f / 12) * (time.wHour) + (time.wMinute * 0.5f);
    const f32 fMinuteAngle = (360.0f / 60) * (time.wMinute);
    const f32 fSecondAngle =
        (360.0f / 60) * (time.wSecond) + (360.0f / 60000) * (time.wMilliseconds);
    DrawClockHand(0.6f, fHourAngle, 6);
    DrawClockHand(0.85f, fMinuteAngle, 4);
    DrawClockHand(0.85f, fSecondAngle, 1);
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}
void Scene::CalculateLayout() {
    D2D1_SIZE_F fSize  = m_pRenderTarget->GetSize();
    const f32 x      = fSize.width / 2.0f;
    const f32 y      = fSize.height / 2.0f;
    const f32 radius = min(x, y);
    m_ellipse          = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
    D2D_POINT_2F pt1 =
        D2D1::Point2F(m_ellipse.point.x, m_ellipse.point.y - (m_ellipse.radiusY * 0.9f));
    D2D_POINT_2F pt2 =
        D2D1::Point2F(m_ellipse.point.x, m_ellipse.point.y - m_ellipse.radiusY);
    for (DWORD i = 0; i < 12; i++) {
        D2D1::Matrix3x2F mat =
            D2D1::Matrix3x2F::Rotation((360.0f / 12) * i, m_ellipse.point);
        m_Ticks[i * 2]     = mat.TransformPoint(pt1);
        m_Ticks[i * 2 + 1] = mat.TransformPoint(pt2);
    }
}
void Scene::DiscardDeviceDependentResources() {
    m_pFill.Release();
    m_pStroke.Release();
}

class MainWindow : public BaseWindow<MainWindow> {
    HANDLE m_hTimer;
    Scene  m_scene;
    BOOL   InitializeTimer();

  public:
    MainWindow() : m_hTimer(NULL) {
    }
    void         WaitTimer();
    const char * ClassName() const {
        return "Clock Window Class";
    }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

int main() {
    if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
        return 0;
    MainWindow win;
    if (!win.Create("Analog Clock", WS_OVERLAPPEDWINDOW)) return 0;
    ShowWindow(win.Window(), SW_SHOW);
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        win.WaitTimer();
    }
    CoUninitialize();
    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HWND hwnd = m_hwnd;
    switch (uMsg) {
    case WM_CREATE:
        if (FAILED(m_scene.Initialize()) || !InitializeTimer()) { return -1; }
        return 0;
    case WM_DESTROY:
        CloseHandle(m_hTimer);
        m_scene.CleanUp();
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
    case WM_DISPLAYCHANGE: {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        m_scene.Render(m_hwnd);
        EndPaint(m_hwnd, &ps);
        return 0;
    }
    case WM_SIZE: {
        i32 x = (i32)(short)LOWORD(lParam);
        i32 y = (i32)(short)HIWORD(lParam);
        m_scene.Resize(x, y);
        InvalidateRect(m_hwnd, NULL, FALSE);
        return 0;
    }
    case WM_ERASEBKGND: return 1;
    default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
BOOL MainWindow::InitializeTimer() {
    m_hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
    if (m_hTimer == NULL) { return FALSE; }
    LARGE_INTEGER li = {0};
    if (!SetWaitableTimer(m_hTimer, &li, (1000 / 60), NULL, NULL, FALSE)) {
        CloseHandle(m_hTimer);
        m_hTimer = NULL;
        return FALSE;
    }
    return TRUE;
}
void MainWindow::WaitTimer() {
    // Wait until the timer expires or any message is posted.
    if (MsgWaitForMultipleObjects(1, &m_hTimer, FALSE, INFINITE, QS_ALLINPUT) ==
        WAIT_OBJECT_0) {
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}
