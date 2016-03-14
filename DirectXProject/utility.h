#pragma once 
#include "utilityMacroDefine.h"
#include <d3dx9.h>
#include <fstream>


extern IDirect3DDevice9* g_pDevice;
extern ID3DXSprite*      g_pSprite;
extern ID3DXFont*        g_pFont;
extern ID3DXEffect*      g_pEffect;
extern std::ofstream     g_debug;

#define WINDOW_WIDTH    480
#define WINDOW_HEIGHT   680

bool KeyDown(int vk_code);
bool KeyUp(int vk_code);



