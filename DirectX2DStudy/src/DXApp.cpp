#include "DXApp.h"

namespace
{
	//to forward msgs to user defined proc function
	DXApp* g_pApp = nullptr;
}
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (g_pApp) return g_pApp->MsgProc(hwnd, msg, wParam, lParam);
	else return DefWindowProc(hwnd, msg, wParam, lParam);
}


DXApp::DXApp(HINSTANCE hInstance) :
	m_hAppInstance(hInstance), m_hAppWnd(NULL), m_ClientWidth(800), m_ClientHeight(600),
	m_AppTitle("Tutorial"), m_WndStyle(WS_OVERLAPPEDWINDOW),
	m_pDevice(nullptr), m_pImmediateContext(nullptr), m_pRenderTargetView(nullptr), m_pSwapChain(nullptr)
{
	g_pApp = this;
}


DXApp::~DXApp()
{
	//cleanup direct3d
	if (m_pImmediateContext) m_pImmediateContext->ClearState();
	Memory::SafeRelease(m_pRenderTargetView);
	Memory::SafeRelease(m_pSwapChain);
	Memory::SafeRelease(m_pImmediateContext);
	Memory::SafeRelease(m_pDevice);
}

int DXApp::Run()
{
	//main message loop
	MSG msg = { 0 };	//everything to zero
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update(0.0f);
			Render(0.0f);
		}
	}
	return static_cast<int>(msg.wParam);
}

bool DXApp::Init()
{
	if (!InitWindow())
		return false;
	if (!InitDirect3D())
		return false;
	return true;
}
LRESULT DXApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:	//when close
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}
bool DXApp::InitWindow()
{
	//Wnd class
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hInstance = m_hAppInstance;
	wcex.lpfnWndProc = MainWndProc;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION); //own : makeEntrySource?
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "DXAPPWNDCLASS";

	if (!RegisterClassEx(&wcex))
	{
		OutputDebugString("FAILED TO CREATE WINDOW CLASS\n");
		return false;
	}
	//for borders, titlebar
	RECT r = { 0,0,m_ClientWidth,m_ClientHeight };
	AdjustWindowRect(&r, m_WndStyle, FALSE); //false : no menu
	UINT width = r.right - r.left;
	UINT height = r.bottom - r.top;

	UINT x = GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2;
	UINT y = GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2;
	m_hAppWnd = CreateWindow("DXAPPWNDCLASS", m_AppTitle.c_str(), m_WndStyle, x, y, width, height, NULL, NULL, m_hAppInstance, NULL);
	if(!m_hAppWnd)
	{
		OutputDebugString("FAILED TO CREATE WINDOW\n");
		return false;
	}
	ShowWindow(m_hAppWnd, SW_SHOW);

	return true;
}

bool DXApp::InitDirect3D()
{
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif	//_DEBUG
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	//swapchange
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 1; //double buffered
	swapDesc.BufferDesc.Width = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;	//60fps
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hAppWnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = true;
	swapDesc.SampleDesc.Count = 1;	//샘플링은 많으면 렉걸림
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //allow alt+enter

	HRESULT result;
	for (int i = 0; i < numDriverTypes; ++i)
	{
		result = D3D11CreateDeviceAndSwapChain(
			NULL,
			driverTypes[i],
			NULL,
			createDeviceFlags,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&swapDesc,
			&m_pSwapChain,
			&m_pDevice,
			&m_FeatureLevel,
			&m_pImmediateContext);
		if (SUCCEEDED(result))
		{
			m_DriverType = driverTypes[i];
			break;
		}
	}
	if (FAILED(result))
	{
		OutputDebugString("FAILED TO CREATE DEVICE AND SWAP CHAIN");
		return false;
	}


	//create render target view
	ID3D11Texture2D* m_pBackBufferTex;
	m_pSwapChain->GetBuffer(NULL, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pBackBufferTex));
	m_pDevice->CreateRenderTargetView(m_pBackBufferTex, nullptr, &m_pRenderTargetView);

	//bind render target view
	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr); //third is for shadowmap etc

	//viewport creation
	m_Viewport.Width = static_cast<float>(m_ClientWidth);
	m_Viewport.Height = static_cast<float>(m_ClientHeight);
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	//bind viewport
	m_pImmediateContext->RSSetViewports(1, &m_Viewport);

	return true;
}