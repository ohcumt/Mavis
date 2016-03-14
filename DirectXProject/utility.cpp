

#include "utility.h"


IDirect3DDevice9* g_pDevice = NULL;
ID3DXSprite*      g_pSprite = NULL;
ID3DXFont*        g_pFont   = NULL;
ID3DXEffect*      g_pEffect = NULL;
std::ofstream     g_debug("debug.txt");

bool KeyDown(int vk_code){ return (GetAsyncKeyState(vk_code) & 0x8000) ? true : false; }
bool KeyUp(int vk_code){ return (GetAsyncKeyState(vk_code) & 0x8000) ? false : true; }



