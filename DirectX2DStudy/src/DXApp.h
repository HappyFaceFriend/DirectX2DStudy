#pragma once
#define WIN32_LEAN_AND_MEAN	//include windows.h with only basics
#include <Windows.h>
#include <string>
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

	bool InitWindow();
};

