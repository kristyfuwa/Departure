#pragma once
#include "D3DApp.h"
class InitDirect3DApp :
	public D3DApp
{
public:
	InitDirect3DApp(HINSTANCE hInstance);
	~InitDirect3DApp();

	virtual			bool	Init();
	virtual			void	OnResize();
	virtual			void	UpdateScene(float dt);
	virtual			void	DrawScene();

};

