#pragma once 

#include "mesh.h"

struct Bone : D3DXFRAME
{
public:
	D3DXMATRIX CombinedTransformationMatrix;

};

struct BoneMesh : public D3DXMESHCONTAINER
{
	//More to come here later...
};


class SkinnedMesh 
{
public: 
	SkinnedMesh();
	~SkinnedMesh();

	void Load(char* fName) ;
	void RenderSkeleton(Bone* bone, Bone *parent, D3DXMATRIX world);



private:
	void UpdateMatrices(Bone* bone, D3DXMATRIX *parentMatrix);

protected:
	D3DXFRAME *m_pRootBone;
	LPD3DXMESH m_pSphereMesh;

};





