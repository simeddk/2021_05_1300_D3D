#pragma once
#include "Systems/IExecute.h"

class DynamicCubeMapDemo : public IExecute
{
public:
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	void Mesh();
	void Airplane();
	void Kachujin();
	void Kachujin2();
	void Weapon();

	void PointLights();
	void SpotLights();

	void Pass(UINT val);
	
private:
	Shader* shader;

	DynamicCube* dynamicCube;
	MeshRender* sphere2;

	CubeSky* sky;

	Material* floor;
	Material* stone;
	Material* brick;
	Material* wall;
	
	MeshRender* cube;
	MeshRender* plane;
	MeshRender* cylinder;
	MeshRender* sphere;

	ModelRender* airplane;

	ModelAnimator* kachujin;
	ModelAnimator* kachujin2;
	ModelRender* weapon;
	Transform* weaponTransform;
	
	vector<MeshRender*> meshes;
	vector<ModelRender*> models;
	vector<ModelAnimator*> animators;

};