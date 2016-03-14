
#pragma once 
#include <d3dx9.h>
#include <vector>


class Mesh
{
public:
	Mesh();
	Mesh(char* fName);
	~Mesh();

	HRESULT Load(char* fName);
	void Render();
	void Release();

private:
	ID3DXMesh *m_pMesh;
	std::vector<IDirect3DTexture9*> m_pTextures;
	std::vector<D3DMATERIAL9> m_Materials;
	D3DMATERIAL9 m_WhiteMaterial;
};







