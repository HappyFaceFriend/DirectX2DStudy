#pragma once
#define WIN32_LEAN_AND_MEAN	//include windows.h with only basics
#include <Windows.h>
#include <string>
#include "DXUtil.h"

class DXApp
{
public:
	DXApp(HINSTANCE hInstance);
	virtual ~DXApp();

	//main app loop
	int Run();
	
	//Framework Methods
	virtual bool Init();
	virtual void Update(float dt) = 0;
	virtual void Render(float dt) = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	//Win32 Attributes
	HWND m_hAppWnd;
	HINSTANCE m_hAppInstance;
	UINT m_ClientWidth;
	UINT m_ClientHeight;
	std::string m_AppTitle;
	DWORD m_WndStyle;
	
	//DirectX Attribues
	ID3D11Device* m_pDevice;	//setting states (shaders)
	ID3D11DeviceContext* m_pImmediateContext;	//rendering
	IDXGISwapChain* m_pSwapChain;	//presenting & handling buffers
	ID3D11RenderTargetView* m_pRenderTargetView;
	D3D_DRIVER_TYPE m_DriverType;
	D3D_FEATURE_LEVEL m_FeatureLevel;
	D3D11_VIEWPORT m_Viewport;

	//Initialize Win32 Window
	bool InitWindow();
	//Initialize Direct3D
	bool InitDirect3D();
};

