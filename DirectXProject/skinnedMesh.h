#pragma once 

#include "mesh.h"
#include <vector>

struct Bone : D3DXFRAME
{
public:
	D3DXMATRIX CombinedTransformationMatrix;

};

struct BoneMesh : public D3DXMESHCONTAINER
{
	ID3DXMesh *originalMesh;
	std::vector<D3DMATERIAL9> materials;
	std::vector<IDirect3DTexture9*> pTextures;

	DWORD numAttributeGroups;
	D3DXATTRIBUTERANGE* attributeTable;
	D3DXMATRIX **boneMatrixPtrs;
	D3DXMATRIX  *boneOffsetMatrices;
	D3DXMATRIX  *currentBoneMatrices;

};


class SkinnedMesh 
{
public: 
	SkinnedMesh();
	~SkinnedMesh();

	void Load(char* fName) ;
	void Render(Bone *bone);
	void RenderSkeleton(Bone* bone, Bone *parent, D3DXMATRIX world);



private:
	void UpdateMatrices(Bone* bone, D3DXMATRIX *parentMatrix);
	void SetupBoneMatrixPointers(Bone *bone);


protected:
	D3DXFRAME *m_pRootBone;
	LPD3DXMESH m_pSphereMesh;

};





