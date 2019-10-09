#pragma once
#include <windows.h>
#include <D3DX11.h>
#include <string>


class GameTimer;

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE		AppInst()const;
	HWND			MainWnd()const;
	float			AspectRatio()const;

	int				Run();

	// 框架方法。派生类需要重载这些方法实现所需的功能。

	virtual			bool	Init();
	virtual			void	OnResize();
	virtual			void	UpdateScene(float dt) = 0;
	virtual			void	DrawScene() = 0;
	virtual			LRESULT	MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// 处理鼠标输入事件的便捷重载函数
	virtual			void	OnMouseDown(WPARAM btnState, int x, int y) {}
	virtual			void	OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual			void	OnMouseMove(WPARAM btnState, int x, int y) {}

protected:
	bool			InitMainWindow();
	bool			InitDirect3D();

	void			CalculateFrameStats();

protected:
	HINSTANCE						m_hAppInst; // 应用程序实例句柄
	HWND							m_hMainWnd; // 主窗口句柄 
	bool							m_bAppPaused; // 程序是否处在暂停状态
	bool							m_bMinimized; // 程序是否最小化 
	bool							m_bMaximized; // 程序是否最大化 
	bool							m_bResizing; // 程序是否处在改变大小的状态
	UINT							m_ui4xMsaaQuality;// 4X MSAA 质量等级

	// 用于记录"delta-time"和游戏时间
	GameTimer						m_Timer;

	// 渲染目标和深度/模板视图和视口。 
	ID3D11Device*					m_pD3dDevice;
	ID3D11DeviceContext*			m_pD3dImmediateContext;
	IDXGISwapChain*					m_pSwapChain;
	ID3D11Texture2D*				m_pDepthStencilBuffer;
	ID3D11RenderTargetView*			m_pRenderTargetView;
	ID3D11DepthStencilView*			m_pDepthStencilView;
	D3D11_VIEWPORT					m_ScreenViewport;

	std::wstring					m_sMainWndCaption;

	D3D_DRIVER_TYPE					m_D3dDriverType;
	int								m_iClientWidth;
	int								m_iClientHeight;

	// 设置为 true 则使用 4XMSAA，默认为 false。 
	bool							m_bEnable4xMsaa;

};

