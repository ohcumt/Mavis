
#include "skinnedMesh.h"
#include "utility.h"
#include <windows.h>


struct VERTEX{
	VERTEX();
	VERTEX(D3DXVECTOR3 pos, D3DCOLOR col){ position = pos; color = col; }
	D3DXVECTOR3 position;
	D3DCOLOR color;
	static const DWORD FVF;
};

const DWORD VERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

// 重写骨骼层级加载器,自定义加载frame
class BoneHierarchyLoader : public ID3DXAllocateHierarchy
{
public:
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
	STDMETHOD(CreateMeshContainer)(THIS_ LPCTSTR Name, CONST D3DXMESHDATA * pMeshData, CONST D3DXMATERIAL * pMaterials, CONST D3DXEFFECTINSTANCE * pEffectInstances, DWORD NumMaterials, CONST DWORD * pAdjacency, LPD3DXSKININFO pSkinInfo, LPD3DXMESHCONTAINER * ppNewMeshContainer);
	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);
};

HRESULT BoneHierarchyLoader::CreateFrame(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame)
{
	Bone *newBone = new Bone;
	memset(newBone, 0, sizeof(Bone));
	
	if ( Name!=NULL ) 
	{
		newBone->Name = new char[strlen(Name)+1];
		for (int i=0; i<(int)strlen(Name); ++i)
		{
			newBone->Name[i] = Name[i];
		}
		newBone->Name[strlen(Name)] = '\0';
	}
	 
	D3DXMatrixIdentity(&newBone->TransformationMatrix);
	D3DXMatrixIdentity(&newBone->CombinedTransformationMatrix);

	*ppNewFrame = (D3DXFRAME*)newBone;

	return S_OK;
}

HRESULT BoneHierarchyLoader::CreateMeshContainer(THIS_ LPCTSTR Name, CONST D3DXMESHDATA * pMeshData, CONST D3DXMATERIAL * pMaterials, CONST D3DXEFFECTINSTANCE * pEffectInstances, DWORD NumMaterials, CONST DWORD * pAdjacency, LPD3DXSKININFO pSkinInfo, LPD3DXMESHCONTAINER * ppNewMeshContainer)
{
	//Just return a temporary mesh for now...
	*ppNewMeshContainer = new BoneMesh;
	memset(*ppNewMeshContainer, 0, sizeof(BoneMesh));
	return S_OK;
}

STDMETHODIMP BoneHierarchyLoader::DestroyFrame(THIS_ LPD3DXFRAME pFrameToFree)
{
	if (pFrameToFree)
	{
		if (pFrameToFree->Name) 
		{
			delete [] pFrameToFree->Name;
		}
		delete pFrameToFree;
	} 

	pFrameToFree = NULL;

	return S_OK;
}

STDMETHODIMP BoneHierarchyLoader::DestroyMeshContainer(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase)
{
	if (pMeshContainerBase) 
	{
		delete pMeshContainerBase;
	}
	return S_OK;
}

void SkinnedMesh::Load(char* fName)
{
	BoneHierarchyLoader boneHierarchy;
	D3DXLoadMeshHierarchyFromX(fName, D3DXMESH_MANAGED, g_pDevice, &boneHierarchy,NULL, &m_pRootBone, NULL);

	D3DXMATRIX i;
	D3DXMatrixIdentity(&i);
	UpdateMatrices((Bone*)m_pRootBone, &i);

	//Create Sphere
	D3DXCreateSphere(g_pDevice, 0.02f, 10, 10, &m_pSphereMesh, NULL);
}

void SkinnedMesh::UpdateMatrices(Bone* bone, D3DXMATRIX *parentMatrix)
{
	if(bone == NULL)return;

	//Calculate the combined transformation matrix
	D3DXMatrixMultiply(&bone->CombinedTransformationMatrix,
		&bone->TransformationMatrix,
		parentMatrix);

	if(bone->pFrameSibling)  UpdateMatrices((Bone*)bone->pFrameSibling, parentMatrix);
	if(bone->pFrameFirstChild)  UpdateMatrices((Bone*)bone->pFrameFirstChild, &bone->CombinedTransformationMatrix);
}

void SkinnedMesh::RenderSkeleton(Bone* bone, Bone *parent, D3DXMATRIX world)
{

	//Temporary function to render the bony hierarchy
	if(bone == NULL)bone = (Bone*)m_pRootBone;


	D3DXMATRIX r, s;

	// 计算绕y轴旋转角度
	D3DXMatrixRotationYawPitchRoll(&r, -D3DX_PI*0.5f, 0.0f, 0.0f);

	if (parent && parent->Name && bone->Name) 
	{
		g_pDevice->SetRenderState(D3DRS_LIGHTING, true); // 打开光照
		g_pDevice->SetTransform(D3DTS_WORLD, &(r*bone->CombinedTransformationMatrix*world));
		m_pSphereMesh->DrawSubset(0);

		D3DXMATRIX w1 = bone->CombinedTransformationMatrix;
		D3DXMATRIX w2 = parent->CombinedTransformationMatrix;

		D3DXVECTOR3 thisBone = D3DXVECTOR3(w1(3, 0), w1(3, 1), w1(3, 2));
		D3DXVECTOR3 ParentBone = D3DXVECTOR3(w2(3, 0), w2(3, 1), w2(3, 2));

		if (D3DXVec3Length(&(thisBone - ParentBone)) < 2.0f) 
		{
			g_pDevice->SetTransform(D3DTS_WORLD, &world);
			g_pDevice->SetRenderState(D3DRS_LIGHTING, false);


			VERTEX vert[] = {VERTEX(ParentBone, 0xffff0000), VERTEX(thisBone, 0xff00ff00)}; 
			g_pDevice->SetFVF(VERTEX::FVF);
			g_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, &vert[0], sizeof(VERTEX));
		}
	}

	if(bone->pFrameSibling)RenderSkeleton((Bone*)bone->pFrameSibling, parent, world);
	if(bone->pFrameFirstChild)RenderSkeleton((Bone*)bone->pFrameFirstChild, bone, world);
}

SkinnedMesh::SkinnedMesh()
:m_pRootBone(NULL)
,m_pSphereMesh(NULL)
{

}

SkinnedMesh::~SkinnedMesh()
{
	BoneHierarchyLoader boneHierarchy;
	boneHierarchy.DestroyFrame(m_pRootBone);
}

