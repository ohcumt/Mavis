
#include "mesh.h"
#include "utilityMacroDefine.h"
#include "utility.h"

Mesh::Mesh()
:m_pMesh(NULL)
{
	m_WhiteMaterial.Ambient = m_WhiteMaterial.Specular = m_WhiteMaterial.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_WhiteMaterial.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	m_WhiteMaterial.Power = 1.0f;
}

Mesh::Mesh(char* fName)
{
	m_WhiteMaterial.Ambient = m_WhiteMaterial.Specular = m_WhiteMaterial.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_WhiteMaterial.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	m_WhiteMaterial.Power = 1.0f;

	m_pMesh = NULL;
	Load(fName);
}

HRESULT Mesh::Load(char* fName)
{
	// release old resource 
	Release();

	ID3DXBuffer* adjacencyBuffer = NULL;
	ID3DXBuffer* materialBuffer = NULL;

	DWORD materialNum = NULL;
	//HRESULT result = D3DXLoadMeshFromX((LPCWSTR)fName, D3DXMESH_MANAGED, g_pDevice, &adjacencyBuffer, &materialBuffer, NULL, &materialNum, &m_pMesh);
	HRESULT result = D3DXLoadMeshFromX(L"resources/meshes/soldier.x", D3DXMESH_MANAGED, g_pDevice, &adjacencyBuffer, &materialBuffer, NULL, &materialNum, &m_pMesh);

	if(FAILED(result))
	{
		return E_FAIL;
	}

	D3DXMATERIAL* materials = (D3DXMATERIAL*) materialBuffer->GetBufferPointer();

	for (int i=0; i<(int)materialNum; ++i)
	{
		m_Materials.push_back(materials[i].MatD3D);
		
		// load texture for each material
		if( materials[i].pTextureFilename != NULL ) 
		{
			char textureFileName[90];

#define PATH_TO_TEXTURES "resources/meshes/"
			strcpy_s(textureFileName, PATH_TO_TEXTURES);
			strcat_s(textureFileName, materials[i].pTextureFilename);
			IDirect3DTexture9* newTexture = NULL;
			D3DXCreateTextureFromFile(g_pDevice, (LPCWSTR)textureFileName, &newTexture);
			m_pTextures.push_back(newTexture);
#undef PATH_TO_TEXTURES
		}
		else 
		{
			m_pTextures.push_back(NULL);
		}
	}

	// 对网格数据进行优化
	m_pMesh->OptimizeInplace(
		D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_COMPACT | D3DXMESHOPT_VERTEXCACHE,
		(DWORD*)adjacencyBuffer->GetBufferPointer(), NULL, NULL, NULL);

	adjacencyBuffer->Release();
	materialBuffer->Release();

	return S_OK;	
}

void Mesh::Render()
{
	int MtrlNum = (int)m_Materials.size();

	for(int i=0; i<MtrlNum; ++i)
	{
		if(m_pTextures[i] != NULL)
		{
			g_pDevice->SetMaterial(&m_WhiteMaterial);
		}
		else
		{
			g_pDevice->SetMaterial(&m_Materials[i]);
		}

		g_pDevice->SetTexture(0, m_pTextures[i]);

		m_pMesh->DrawSubset(i);
	}

}

void Mesh::Release()
{
	if(m_pMesh!=NULL)
	{
		m_pMesh->Release();
		m_pMesh = NULL;
	}

	// clear texture and material
	int materialNum = (int)m_Materials.size();

	for(int i=0; i<materialNum; ++i)
	{
		if(m_pTextures[i]!=NULL)
		{
			m_pTextures[i]->Release();
		}
	}

	m_pTextures.clear();
	m_Materials.clear();
}

Mesh::~Mesh()
{
	Release();
}




