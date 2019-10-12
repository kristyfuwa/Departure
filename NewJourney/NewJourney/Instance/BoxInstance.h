#pragma once
#include "Common/D3DApp.h"
#include "Common/D3DUtil.h"
class BoxInstance :
	public D3DApp
{
public:
	BoxInstance(HINSTANCE hInstance);
	virtual ~BoxInstance();

	virtual 	bool	Init();
	virtual 	void	OnResize();
	virtual 	void	UpdateScene(float dt);
	virtual 	void	DrawScene();

	virtual		void	OnMouseDown(WPARAM btnState, int x, int y);
	virtual		void	OnMouseUp(WPARAM btnState, int x, int y);
	virtual		void	OnMouseMove(WPARAM btnState, int x, int y);
	virtual		void	OnMouseWheel(int degree);
	virtual		void	OnKeyDown(WPARAM keyState);
	virtual		void	OnKeyUp(WPARAM keyState);
private:
	void				BuildVertexLayout();
	void				BuildGeometryBuffers();
	HRESULT				CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
private:
	ID3D11Buffer*			m_pVertexBuffer;
	ID3D11Buffer*			m_pIndexBuffer;
	ID3D11Buffer*			m_pConstantBuffer;
	ID3D11InputLayout*		m_pVertexLayout;
	ID3D11VertexShader*		m_pVertexShader;
	ID3D11PixelShader*		m_pPixelShader;
	ID3D11RasterizerState*	m_pRasterizerState;

	XMFLOAT4X4				m_World;
	XMFLOAT4X4				m_View;
	XMFLOAT4X4				m_Proj;

	float					m_fTheta;
	float					m_fPhi;
	float					m_fRadius;

	POINT					m_LastMousePos;
};

