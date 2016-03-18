
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


	BoneMesh* boneMesh = new BoneMesh;
	memset(boneMesh, 0, sizeof(BoneMesh));


	// get mesh data
	boneMesh->originalMesh = pMeshData->pMesh;
	boneMesh->MeshData.pMesh = pMeshData->pMesh;
	boneMesh->MeshData.Type = pMeshData->Type;

	pMeshData->pMesh->AddRef(); 

	IDirect3DDevice9 *device = NULL;	
	pMeshData->pMesh->GetDevice(&device);

	for (int i=0; i<(int)NumMaterials; ++i) 
	{
		D3DXMATERIAL mtrl;
		memcpy(&mtrl, &pMaterials[i], sizeof(D3DXMATERIAL));
		boneMesh->materials.push_back(mtrl.MatD3D);

		IDirect3DTexture9* newTexture = NULL;

		if (mtrl.pTextureFilename!=NULL)
		{
			char textureName[200];
			strcpy_s(textureName, "resources/meshes/");
			strcat_s(textureName, mtrl.pTextureFilename);

			D3DXCreateTextureFromFile(device, textureName, &newTexture);
		}

		boneMesh->pTextures.push_back(newTexture);
	}

	if (pSkinInfo != NULL) 
	{
		boneMesh->pSkinInfo = pSkinInfo;
		pSkinInfo->AddRef();


		DWORD maxVertInfluences = 0;
		DWORD numBoneComboEntries = 0;
		ID3DXBuffer *boneComboTable = NULL;

		pSkinInfo->ConvertToIndexedBlendedMesh(
			pMeshData->pMesh, D3DXMESH_MANAGED|D3DXMESH_WRITEONLY,
			30, NULL, NULL, NULL, NULL, 
			&maxVertInfluences, &numBoneComboEntries, 
			&boneComboTable, &boneMesh->MeshData.pMesh);


		// bone combination table not used 
		if (boneComboTable) 
		{
			boneComboTable->Release();
		}

		// get attribute table 
		boneMesh->MeshData.pMesh->GetAttributeTable(NULL, &boneMesh->numAttributeGroups); // 获取attribute table的大小
		boneMesh->attributeTable = new D3DXATTRIBUTERANGE[boneMesh->numAttributeGroups];  // 创建空间
		boneMesh->MeshData.pMesh->GetAttributeTable(boneMesh->attributeTable, NULL);      // 向attribute table填充数据


		int numBones = pSkinInfo->GetNumBones();
		boneMesh->boneOffsetMatrices  = new D3DXMATRIX[numBones];
		boneMesh->currentBoneMatrices = new D3DXMATRIX[numBones];

		for (int i=0; i<numBones; ++i) 
		{
			boneMesh->boneOffsetMatrices[i] = *(boneMesh->pSkinInfo->GetBoneOffsetMatrix(i));
		}
	}

	//Set ppNewMeshContainer to the newly created boneMesh container
	*ppNewMeshContainer = boneMesh;

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
	BoneMesh *boneMesh = (BoneMesh*)pMeshContainerBase;
	if (boneMesh) 
	{
		int numTexture = (int)boneMesh->pTextures.size();
		for (int i=0; i<numTexture; ++i) 
		{
			if (boneMesh->pTextures[i]) 
			{
				boneMesh->pTextures[i]->Release();
			}
		}

		if(boneMesh->MeshData.pMesh)   boneMesh->MeshData.pMesh->Release();
		if(boneMesh->pSkinInfo)        boneMesh->pSkinInfo->Release();
		if(boneMesh->originalMesh)     boneMesh->originalMesh->Release();
		delete boneMesh;
	}
	return S_OK;
}

void SkinnedMesh::Load(char* fName)
{
	BoneHierarchyLoader boneHierarchy;
	D3DXLoadMeshHierarchyFromX(
		fName, D3DXMESH_MANAGED, 
		g_pDevice, &boneHierarchy,
		NULL, &m_pRootBone, NULL);

	SetupBoneMatrixPointers((Bone*)m_pRootBone);

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
	D3DXMatrixMultiply(
		&bone->CombinedTransformationMatrix,
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

void SkinnedMesh::Render(Bone *bone)
{
	if (bone==NULL) 
	{
		bone = (Bone*)m_pRootBone;
	}

	if ( bone->pMeshContainer) 
	{
		BoneMesh *boneMesh = (BoneMesh*) bone->pMeshContainer;

		if (boneMesh->pSkinInfo!=NULL) 
		{
			// set up bone transform
			int numBones = boneMesh->pSkinInfo->GetNumBones();
			for (int i=0; i<numBones; ++i) 
			{
				D3DXMatrixMultiply(&boneMesh->currentBoneMatrices[i],
					&boneMesh->boneOffsetMatrices[i],
					boneMesh->boneMatrixPtrs[i]);
			}

			// set hw matrix palette
			D3DXMATRIX identity;	
			g_pEffect->SetMatrixArray("FinalTransforms", boneMesh->currentBoneMatrices, boneMesh->pSkinInfo->GetNumBones());
			D3DXMatrixIdentity(&identity);

			// render the mesh 
			for (int i=0; i<(int)boneMesh->numAttributeGroups; ++i) 
			{
				int mtrlIndex = boneMesh->attributeTable[i].AttribId;
				g_pDevice->SetMaterial(&(boneMesh->materials[mtrlIndex]));
				g_pDevice->SetTexture(0, boneMesh->pTextures[mtrlIndex]);
				g_pEffect->SetTexture("texDiffuse", boneMesh->pTextures[mtrlIndex]);
				g_pEffect->SetMatrix("matW", &identity);
				D3DXHANDLE hTech = g_pEffect->GetTechniqueByName("Skinning");

				g_pEffect->SetTechnique(hTech);
				g_pEffect->Begin(NULL, NULL);
				g_pEffect->BeginPass(0);

				boneMesh->MeshData.pMesh->DrawSubset(mtrlIndex);

				g_pEffect->EndPass();
				g_pEffect->End();
			}
		}
		else 
		{
			// normal static mesh
			g_pEffect->SetMatrix("matW", &bone->CombinedTransformationMatrix);
			D3DXHANDLE hTech = g_pEffect->GetTechniqueByName("Lighting");
			g_pEffect->SetTechnique(hTech);

			// render the mesh 
			int numMaterial = (int)boneMesh->materials.size();
			for (int i=0; i<numMaterial; ++i) 
			{
				g_pDevice->SetMaterial(&boneMesh->materials[i]);
				g_pEffect->SetTexture("texDiffuse", boneMesh->pTextures[i] );

				g_pEffect->Begin(NULL, NULL);
				g_pEffect->BeginPass(0);

				boneMesh->originalMesh->DrawSubset(i);

				g_pEffect->EndPass();
				g_pEffect->End();
			}
		}
	}

	if(bone->pFrameSibling) Render((Bone*)bone->pFrameSibling);
	if(bone->pFrameFirstChild) Render((Bone*)bone->pFrameFirstChild);
}

void SkinnedMesh::SetupBoneMatrixPointers(Bone *bone)
{
	if (bone->pMeshContainer)
	{
		BoneMesh *boneMesh = (BoneMesh*) bone->pMeshContainer;

		if (boneMesh->pSkinInfo) 
		{
			int numBones = boneMesh->pSkinInfo->GetNumBones();
			boneMesh->boneMatrixPtrs = new D3DXMATRIX *[numBones];

			for (int i=0; i<numBones; ++i) 
			{
				Bone*b = (Bone*)D3DXFrameFind(m_pRootBone, boneMesh->pSkinInfo->GetBoneName(i));
				if (b) 
				{
					boneMesh->boneMatrixPtrs[i] = &b->CombinedTransformationMatrix;
				}
				else 
				{
					boneMesh->boneMatrixPtrs[i] = NULL;
				}  
			}
		}
	}

	if(bone->pFrameSibling)  SetupBoneMatrixPointers((Bone*)bone->pFrameSibling);
	if(bone->pFrameFirstChild)  SetupBoneMatrixPointers((Bone*)bone->pFrameFirstChild);

}

