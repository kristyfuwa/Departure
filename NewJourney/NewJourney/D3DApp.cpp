#include "D3DApp.h"



D3DApp::D3DApp(HINSTANCE hInstance)
{

}

D3DApp::~D3DApp()
{
}

float D3DApp::AspectRatio() const
{
	return static_cast<float>(m_iClientWidth) / m_iClientHeight;
}
