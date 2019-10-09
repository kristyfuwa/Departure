#include "InitDirect3DApp.h"
#include "D3DUtil.h"
#include <assert.h>

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance):D3DApp(hInstance)
{

}

InitDirect3DApp::~InitDirect3DApp()
{
}

bool InitDirect3DApp::Init()
{
	if (!D3DApp::Init())
		return false;
	return  true;
}

void InitDirect3DApp::OnResize()
{
	D3DApp::OnResize();
}

void InitDirect3DApp::UpdateScene(float dt)
{

}

void InitDirect3DApp::DrawScene()
{
	assert(m_pD3dImmediateContext);
	assert(m_pSwapChain);

	//FLOAT color[3] = { 0.0f,0.0f,255.0f };
	m_pD3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	m_pD3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	m_pSwapChain->Present(0, 0);
}
