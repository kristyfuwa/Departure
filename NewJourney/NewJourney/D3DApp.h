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

	// ��ܷ�������������Ҫ������Щ����ʵ������Ĺ��ܡ�

	virtual			bool	Init();
	virtual			void	OnResize();
	virtual			void	UpdateScene(float dt) = 0;
	virtual			void	DrawScene() = 0;
	virtual			LRESULT	MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// ������������¼��ı�����غ���
	virtual			void	OnMouseDown(WPARAM btnState, int x, int y) {}
	virtual			void	OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual			void	OnMouseMove(WPARAM btnState, int x, int y) {}

protected:
	bool			InitMainWindow();
	bool			InitDirect3D();

	void			CalculateFrameStats();

protected:
	HINSTANCE						m_hAppInst; // Ӧ�ó���ʵ�����
	HWND							m_hMainWnd; // �����ھ�� 
	bool							m_bAppPaused; // �����Ƿ�����ͣ״̬
	bool							m_bMinimized; // �����Ƿ���С�� 
	bool							m_bMaximized; // �����Ƿ���� 
	bool							m_bResizing; // �����Ƿ��ڸı��С��״̬
	UINT							m_ui4xMsaaQuality;// 4X MSAA �����ȼ�

	// ���ڼ�¼"delta-time"����Ϸʱ��
	GameTimer						m_Timer;

	// ��ȾĿ������/ģ����ͼ���ӿڡ� 
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

	// ����Ϊ true ��ʹ�� 4XMSAA��Ĭ��Ϊ false�� 
	bool							m_bEnable4xMsaa;

};

