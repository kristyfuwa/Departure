#include "InitDirect3DApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,	PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	InitDirect3DApp application(hInstance);

	if (!application.Init())
		return 0;

	return application.Run();
}