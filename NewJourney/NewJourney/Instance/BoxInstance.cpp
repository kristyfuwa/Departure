#include "BoxInstance.h"
#include "Common/D3DMathHelper.h"
#include <d3dcompiler.h>
#include <sstream>

struct Vertex
{
	XMFLOAT3		Position;
	XMFLOAT4		Color;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};


BoxInstance::BoxInstance(HINSTANCE hInstance):D3DApp(hInstance),
	m_pVertexBuffer(nullptr),
	m_pIndexBuffer(nullptr),
	m_pConstantBuffer(nullptr),
	m_pVertexLayout(nullptr),
	m_pVertexShader(nullptr),
	m_pPixelShader(nullptr),
	m_pRasterizerState(nullptr),
	m_fTheta(1.5f*D3DMathHelper::PI),
	m_fPhi(0.25f*D3DMathHelper::PI),
	m_fRadius(5.0f)
{

	m_sMainWndCaption = L"Box";

	m_LastMousePos.x = 0;
	m_LastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_World, I);
	XMStoreFloat4x4(&m_View, I);
	XMStoreFloat4x4(&m_Proj, I);
}

BoxInstance::~BoxInstance()
{
	ReleaseCom(m_pVertexBuffer);
	ReleaseCom(m_pIndexBuffer);
	ReleaseCom(m_pConstantBuffer);
	ReleaseCom(m_pVertexLayout);
	ReleaseCom(m_pVertexShader);
	ReleaseCom(m_pPixelShader);
	ReleaseCom(m_pRasterizerState);
}

bool BoxInstance::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildVertexLayout();

	return true;
}

void BoxInstance::OnResize()
{
	D3DApp::OnResize();
	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f*D3DMathHelper::PI, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, p);
}

void BoxInstance::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = m_fRadius * sinf(m_fPhi)*cosf(m_fTheta);
	float z = m_fRadius * sinf(m_fPhi)*sinf(m_fTheta);
	float y = m_fRadius * cosf(m_fPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, V);

	static float t = 0.0f;

	static DWORD dwTimeStart = 0;
	DWORD dwTimeCur = GetTickCount();
	if (dwTimeStart == 0)
		dwTimeStart = dwTimeCur;
	t = (dwTimeCur - dwTimeStart) / 1000.0f;

	//
	// Animate the cube
	//
	//XMMATRIX V2 = XMMatrixRotationY(t);
	//XMStoreFloat4x4(&m_World, V2);
}

void BoxInstance::DrawScene()
{
	m_pD3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&Colors::Black));
	m_pD3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pD3dImmediateContext->IASetInputLayout(m_pVertexLayout);
	m_pD3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pD3dImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	m_pD3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	m_pD3dImmediateContext->RSSetState(m_pRasterizerState);

	// Set constants
	ConstantBuffer cb;
	cb.mWorld = XMLoadFloat4x4(&m_World);
	cb.mView = XMLoadFloat4x4(&m_View);
	cb.mProjection = XMLoadFloat4x4(&m_Proj);//不转置矩阵时，则需要再shader里边左乘矩阵。否则此处需要转置矩阵
	m_pD3dImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);
	
	m_pD3dImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
	m_pD3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pD3dImmediateContext->PSSetShader(m_pPixelShader, NULL, 0);

	m_pD3dImmediateContext->DrawIndexed(36, 0, 0);

	HR(m_pSwapChain->Present(0, 0));
}

void BoxInstance::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
	SetCapture(m_hMainWnd);
}

void BoxInstance::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxInstance::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - m_LastMousePos.y));

		// Update angles based on input to orbit camera around box.
		m_fTheta += dx;
		m_fPhi += dy;

		// Restrict the angle mPhi.
		m_fPhi = D3DMathHelper::Clamp(m_fPhi, 0.1f, D3DMathHelper::PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.005f*static_cast<float>(x - m_LastMousePos.x);
		float dy = 0.005f*static_cast<float>(y - m_LastMousePos.y);

		// Update the camera radius based on input.
		m_fRadius += dx - dy;

		// Restrict the radius.
		m_fRadius = D3DMathHelper::Clamp(m_fRadius, 3.0f, 15.0f);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

}

void BoxInstance::OnMouseWheel(int degree)
{
	//static float s = 0.0f;
	//float scale = degree / 2400.0f;
	//s += scale;
	//XMMATRIX V = XMMatrixScaling(s, s, s);
	//XMStoreFloat4x4(&m_World, V);
	//std::wostringstream outs;
	//outs.precision(6);
	//outs << L"  scale= " << s << L" degree:   " << degree;
	//SetWindowText(m_hMainWnd, outs.str().c_str());
}

void BoxInstance::OnKeyDown(WPARAM keyState)
{

}

void BoxInstance::OnKeyUp(WPARAM keyState)
{
	static float x = 0.0f;
	static float y = 0.0f; 
	if (keyState == VK_UP)
		y += 1;
	else if (keyState == VK_DOWN)
		y -= 1;
	else if (keyState == VK_LEFT)
		x -= 1;
	else if (keyState == VK_RIGHT)
		x += 1;
	XMMATRIX V = XMMatrixTranslation(x, 0, y);
	XMStoreFloat4x4(&m_World, V);
}

void BoxInstance::BuildVertexLayout()
{
	ID3DBlob* pVSBlob = nullptr;
	WCHAR fxName[] = L"Instance/FX/Box.fx";
	HR(CompileShaderFromFile(fxName, "VS", "vs_5_0", &pVSBlob));

	HR(m_pD3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader));

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	UINT numElemets = ARRAYSIZE(layout);
	HR(m_pD3dDevice->CreateInputLayout(layout, numElemets, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout));
	ReleaseCom(pVSBlob);

	ID3DBlob* pPSBlob = nullptr;
	HR(CompileShaderFromFile(fxName, "PS", "ps_5_0", &pPSBlob));

	HR(m_pD3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(),NULL, &m_pPixelShader));
	ReleaseCom(pPSBlob);

}

void BoxInstance::BuildGeometryBuffers()
{
	Vertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), (const float*)&Colors::White   },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), (const float*)&Colors::Black   },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), (const float*)&Colors::Red     },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), (const float*)&Colors::Green   },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), (const float*)&Colors::Blue    },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), (const float*)&Colors::Yellow  },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), (const float*)&Colors::Cyan    },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), (const float*)&Colors::Magenta }
	};

	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
	vertexDesc.ByteWidth = sizeof(Vertex)*8;
	vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.CPUAccessFlags = 0;
	vertexDesc.MiscFlags = 0;
	vertexDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subResourceData;
	ZeroMemory(&subResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	subResourceData.pSysMem = vertices;

	HR(m_pD3dDevice->CreateBuffer(&vertexDesc, &subResourceData, &m_pVertexBuffer));

	UINT indices[36] =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC indexDesc;
	ZeroMemory(&indexDesc, sizeof(D3D11_BUFFER_DESC));
	indexDesc.ByteWidth = sizeof(UINT) * 36;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexDesc.CPUAccessFlags = 0;
	indexDesc.MiscFlags = 0;
	indexDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	initData.pSysMem = indices;

	HR(m_pD3dDevice->CreateBuffer(&indexDesc, &initData, &m_pIndexBuffer));

	D3D11_BUFFER_DESC constantDesc;
	ZeroMemory(&constantDesc, sizeof(D3D11_BUFFER_DESC));
	constantDesc.Usage = D3D11_USAGE_DEFAULT;
	constantDesc.ByteWidth = sizeof(ConstantBuffer);
	constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantDesc.CPUAccessFlags = 0;

	HR(m_pD3dDevice->CreateBuffer(&constantDesc, NULL, &m_pConstantBuffer));

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthClipEnable = TRUE;
	HR(m_pD3dDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerState));

}

HRESULT BoxInstance::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined (DEBUG)|defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	ID3DBlob* pErrorBob = nullptr;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBob != nullptr)
			OutputDebugString((WCHAR*)pErrorBob->GetBufferPointer());
		ReleaseCom(pErrorBob);
		return hr;
	}
	ReleaseCom(pErrorBob);
	return S_OK;
}

