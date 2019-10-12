#include "D3DApp.h"
#include <sstream>
#include <WindowsX.h>
#include "D3DUtil.h"
#include <assert.h>

namespace
{
	D3DApp* g_d3dApp = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return g_d3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance)
	:m_hAppInst(hInstance),
	m_sMainWndCaption(L"D3D Application"),
	m_D3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	m_iClientWidth(800),
	m_iClientHeight(600),
	m_bEnable4xMsaa(false),
	m_hMainWnd(0),
	m_bAppPaused(false),
	m_bMaximized(false),
	m_bMinimized(false),
	m_bResizing(false),
	m_ui4xMsaaQuality(0),
	m_pD3dDevice(0),
	m_pD3dImmediateContext(0),
	m_pDepthStencilView(0),
	m_pRenderTargetView(0),
	m_pSwapChain(0),
	m_pDepthStencilBuffer(0)
{
	ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));
	g_d3dApp = this;
}

D3DApp::~D3DApp()
{
	ReleaseCom(m_pDepthStencilView);
	ReleaseCom(m_pRenderTargetView);
	ReleaseCom(m_pSwapChain);
	ReleaseCom(m_pDepthStencilBuffer);
	// Restore all default settings.
	if (m_pD3dImmediateContext)
		m_pD3dImmediateContext->ClearState();
	ReleaseCom(m_pD3dImmediateContext);
	ReleaseCom(m_pD3dDevice);

}

HINSTANCE D3DApp::AppInst() const
{
	return m_hAppInst;
}

HWND D3DApp::MainWnd() const
{
	return m_hMainWnd;
}

float D3DApp::AspectRatio() const
{
	return static_cast<float>(m_iClientWidth) / m_iClientHeight;
}

int D3DApp::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	m_Timer.Reset();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_Timer.Tick();

			if (!m_bAppPaused)
			{
				CalculateFrameStats();
				UpdateScene(m_Timer.DeltaTime());
				DrawScene();
			}
			else
				m_Timer.Stop();
		}
	}
	return (int)msg.wParam;
}

bool D3DApp::Init()
{
	if (!InitMainWindow())
		return false;
	if (!InitDirect3D())
		return false;
	return true;
}

void D3DApp::OnResize()
{
	assert(m_pD3dDevice);
	assert(m_pD3dImmediateContext);
	assert(m_pSwapChain);

	ReleaseCom(m_pRenderTargetView);
	ReleaseCom(m_pDepthStencilView);
	ReleaseCom(m_pDepthStencilBuffer);

	HR(m_pSwapChain->ResizeBuffers(1, m_iClientWidth, m_iClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer));//每次调用一次GetBuffer,后台缓冲区COM引用计数会+1；
	HR(m_pD3dDevice->CreateRenderTargetView(backBuffer, 0, &m_pRenderTargetView));
	ReleaseCom(backBuffer);

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = m_iClientWidth;
	depthStencilDesc.Height = m_iClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (m_bEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_ui4xMsaaQuality-1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_pD3dDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer));

	HR(m_pD3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer, 0, &m_pDepthStencilView));
	//第二个参数，这个结构体描述了资源中这个元素数据 类型（格式）。如果资源是一个有类型的格式（非 typeless），这个参数可以为空值，表示创 建一个资源的第一个 mipmap 等级的视图（深度/模板缓冲也只能使用一个 mipmap 等级）。 因为我们指定了深度/模板缓冲的格式，所以将这个参数设置为空值。

	m_pD3dImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	D3D11_VIEWPORT viewPort;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = (float)m_iClientWidth;
	viewPort.Height = (float)m_iClientHeight;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;

	m_pD3dImmediateContext->RSSetViewports(1, &viewPort);

}

LRESULT D3DApp::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				m_bAppPaused = true;
				m_Timer.Stop();
			}
			else
			{
				m_bAppPaused = false; 
				m_Timer.Start();
			}
			return 0;
		}
		case WM_KEYDOWN:
		{
			OnKeyDown(wParam);
			return 0;
		}
		case WM_KEYUP:
		{
			OnKeyUp(wParam);
			return 0;
		}
		case WM_ENTERSIZEMOVE:
		{
			m_bAppPaused = true;
			m_bResizing = true;
			m_Timer.Stop();
			return 0;
		}
		case WM_EXITSIZEMOVE:
		{
			m_bAppPaused = false;
			m_bResizing = false;
			m_Timer.Start();
			OnResize();
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			OnMouseWheel((short)HIWORD(wParam));
			return 0;
		}
		case WM_MENUCHAR:
		{
			return MAKELRESULT(0,MNC_CLOSE);
		}
		case WM_GETMINMAXINFO:
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;
		}
		case WM_SIZE:
		{
			m_iClientWidth = LOWORD(lParam);
			m_iClientHeight = HIWORD(lParam);
			if (m_pD3dDevice)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					m_bMinimized = true;
					m_bMaximized = false;
					m_bAppPaused = true;
				}
				else if (wParam == SIZE_MAXIMIZED)
				{
					m_bMinimized = false;
					m_bMaximized = true;
					m_bAppPaused = false;
					OnResize();
				}
				else if (wParam == SIZE_RESTORED)
				{
					if (m_bMinimized)
					{
						m_bAppPaused = false;
						m_bMinimized = false;
						OnResize();
					}
					else if (m_bMaximized)
					{
						m_bMaximized = false;
						m_bAppPaused = false;
						OnResize();
					}
					else if (m_bResizing)
					{
						// If user is dragging the resize bars, we do not resize 
						// the buffers here because as the user continuously 
						// drags the resize bars, a stream of WM_SIZE messages are
						// sent to the window, and it would be pointless (and slow)
						// to resize for each WM_SIZE message received from dragging
						// the resize bars.  So instead, we reset after the user is 
						// done resizing the window and releases the resize bars, which 
						// sends a WM_EXITSIZEMOVE message.
					}
					else
					{
						OnResize();
					}
				}
			}
			return 0;
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_MOUSEMOVE:
			OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, m_iClientWidth, m_iClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_hMainWnd = CreateWindow(L"D3DWndClassName", m_sMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hAppInst, 0);
	if (!m_hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}

bool D3DApp::InitDirect3D()
{
	HRESULT hr = S_OK;
	UINT	createDeviceFlags = 0;
#if defined(DEBUG) | defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HR(D3D11CreateDevice(0, m_D3dDriverType, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pD3dDevice, &featureLevel, &m_pD3dImmediateContext));
	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D FeatureLevel 11 unsupported.", 0, 0);
		return false;
	}

	HR(m_pD3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_ui4xMsaaQuality));
	assert(m_ui4xMsaaQuality > 0);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = m_hMainWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;//用于描述前台缓存在绘制之后D3D应该作何选择
	swapChainDesc.Flags = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Height = m_iClientHeight;
	swapChainDesc.BufferDesc.Width = m_iClientWidth;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	if (m_bEnable4xMsaa)
	{
		swapChainDesc.SampleDesc.Quality = m_ui4xMsaaQuality - 1;
		swapChainDesc.SampleDesc.Count = 4;
	}
	else
	{
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.SampleDesc.Count = 1;
	}
	IDXGIDevice* dxgiDevice = 0;
	HR(m_pD3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFatory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFatory));

	//Create SwapChain;
	HR(dxgiFatory->CreateSwapChain(m_pD3dDevice, &swapChainDesc, &m_pSwapChain));

	ReleaseCom(dxgiDevice);
	ReleaseCom(dxgiAdapter);
	ReleaseCom(dxgiFatory);

	OnResize();

	return true;
}

void D3DApp::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << m_sMainWndCaption << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(m_hMainWnd, outs.str().c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}
