#include "stdafx.h"
#include "BlurDemo.h"


void BlurDemo::Initialize()
{
	Context::Get()->GetCamera()->RotationDegree(20, 0, 0);
	Context::Get()->GetCamera()->Position(0, 36, -85);
	((Freedom*)Context::Get()->GetCamera())->Speed(50, 2);
	
	shader = new Shader(L"27_GeometryBillboard.fxo");
	sky = new CubeSky(L"Environment/Mountain1024.dds");

	float w = D3D::Width(), h = D3D::Height();
	renderTarget = new RenderTarget(w, h);

	
	depthStencil = new DepthStencil(w, h) ;
	viewport = new Viewport(w, h);

	render2D = new Render2D();
	render2D->GetTransform()->Scale(355, 200, 1);
	render2D->GetTransform()->Position(200, 120, 0);
	render2D->SRV(renderTarget->SRV());

	postProcess = new PostProcess(L"30_Blur.fxo");
	postProcess->SRV(renderTarget->SRV());

	Mesh();
	Airplane();
	Kachujin();
	Weapon();

	PointLights();
	SpotLights();

	Billboards();

	//Gizmo::Get()->SetTransform(kachujin->GetTransform(0));
}

void BlurDemo::Destroy()
{
	SafeDelete(shader);
	SafeDelete(sky);
	
	//Material
	SafeDelete(floor);
	SafeDelete(stone);
	SafeDelete(brick);
	SafeDelete(wall);
	
	//Mesh
	SafeDelete(cube);
	SafeDelete(plane);
	SafeDelete(cylinder);
	SafeDelete(sphere);

	//Model
	SafeDelete(airplane);
	SafeDelete(kachujin);
	SafeDelete(weapon);

	//Billboard
	SafeDelete(billboard);

	//RenderTarget
	SafeDelete(renderTarget);
	SafeDelete(depthStencil);
	SafeDelete(viewport);
	SafeDelete(postProcess);
	SafeDelete(render2D);

}

void BlurDemo::Update()
{
	ImGui::SliderFloat3("LightDirection", Lighting::Get()->Direction(), -1, 1);

	static UINT pass = postProcess->GetShader()->PassCount() - 1;
	ImGui::InputInt("BlurPass", (int*)&pass);
	pass %= postProcess->GetShader()->PassCount();
	postProcess->Pass(pass);

	Vector2 pixelSize = Vector2(1 / D3D::Width(), 1 / D3D::Height());
	postProcess->GetShader()->AsVector("PixelSize")->SetFloatVector(pixelSize);
	
	//Blur
	{
		static UINT blurCount = 1;
		ImGui::InputInt("BlurCount", (int*)&blurCount, 1, 32);
		postProcess->GetShader()->AsScalar("BlurCount")->SetInt(blurCount);
	}

	//RadialBlur
	{
		static UINT blurCount = 8;
		ImGui::SliderInt("RadialBlurCount", (int*)&blurCount, 1, 32);
		postProcess->GetShader()->AsScalar("RadialBlurCount")->SetInt(blurCount);

		static float radius = 0.6f;
		ImGui::InputFloat("RadialBlurRadius", &radius, 0.01f);
		postProcess->GetShader()->AsScalar("RadialBlurRadius")->SetFloat(radius);

		static float amount = 0.04f;
		ImGui::InputFloat("RadialBlurAmout", &amount, 0.001f);
		postProcess->GetShader()->AsScalar("RadialBlurAmout")->SetFloat(amount);

		static Vector2 center = Vector2(0.5f, 0.5f);
		ImGui::SliderFloat2("Center", (float*)&center, 0.0f, 1.0f);
		postProcess->GetShader()->AsVector("RadialCenter")->SetFloatVector(center);
	}


	sky->Update();

	cube->Update();
	plane->Update();
	cylinder->Update();
	sphere->Update();

	airplane->Update();
	kachujin->Update();

	Matrix worlds[MAX_MODEL_TRANSFORMS];
	for (UINT i = 0; i < kachujin->TransformCount(); i++)
	{
		kachujin->GetAttachBones(i, worlds);
		weapon->GetTransform(i)->World(weaponTransform->World() * worlds[40]);
	}
	
	weapon->UpdateTransforms();
	weapon->Update();

	billboard->Update();

	postProcess->Update();
	render2D->Update();

}

void BlurDemo::PreRender()
{
	renderTarget->PreRender(depthStencil);
	viewport->RSSetViewport();
	{
		sky->Render();

		Pass(0);

		wall->Render();
		sphere->Render();

		brick->Render();
		cylinder->Render();

		stone->Render();
		cube->Render();

		floor->Render();
		plane->Render();

		airplane->Render();
		kachujin->Render();
		weapon->Render();

		billboard->Pass(6);
		billboard->Render();
	}

}

void BlurDemo::Render()
{ 
	postProcess->Render();
	render2D->Render();
}

void BlurDemo::Mesh()
{
	//Create Material
	{
		floor = new Material(shader);
		floor->DiffuseMap("Floor.png");
		floor->Specular(1, 1, 1, 20);
		floor->NormalMap("Floor_Normal.png");
		floor->SpecularMap("Floor_Specular.png");

		stone = new Material(shader);
		stone->DiffuseMap("Stones.png");
		stone->Specular(1, 1, 1, 20);
		stone->NormalMap("Stones_Normal.png");
		stone->SpecularMap("Stones_Specular.png");
		stone->Emissive(0.15f, 0.15f, 0.15f, 0.3f);

		brick = new Material(shader);
		brick->DiffuseMap("Bricks.png");
		brick->Specular(1, 1, 1, 20);
		brick->NormalMap("Bricks_Normal.png");
		brick->SpecularMap("Bricks_Specular.png");
		brick->Emissive(0.15f, 0.15f, 0.15f, 0.3f);

		wall = new Material(shader);
		wall->DiffuseMap("Wall.png");
		wall->Specular(1, 1, 1, 20);
		wall->NormalMap("Wall_Normal.png");
		wall->SpecularMap("Wall_Specular.png");
		wall->Emissive(0.15f, 0.15f, 0.15f, 0.3f);
	}

	//Create Mesh
	{
		Transform* transform = nullptr;

		cube = new MeshRender(shader, new MeshCube());
		transform = cube->AddTransform();
		transform->Position(0, 5, 0);
		transform->Scale(20, 10, 20);

		plane = new MeshRender(shader, new MeshPlane(2.5f, 2.5f));
		transform = plane->AddTransform();
		transform->Scale(12, 1, 12);

		cylinder = new MeshRender(shader, new MeshCylinder(0.3f, 0.5f, 3.0f, 20, 20));
		sphere = new MeshRender(shader, new MeshSphere(0.5f, 20, 20));
		for (UINT i = 0 ; i < 5; i++)
		{
			transform = cylinder->AddTransform();
			transform->Position(-30, 6, (float)i * 15.0f - 15.0f);
			transform->Scale(5, 5, 5);

			transform = cylinder->AddTransform();
			transform->Position(+30, 6, (float)i * 15.0f - 15.0f);
			transform->Scale(5, 5, 5);

			transform = sphere->AddTransform();
			transform->Position(-30, 15.5f, (float)i * 15.0f - 15.0f);
			transform->Scale(5, 5, 5);

			transform = sphere->AddTransform();
			transform->Position(+30, 15.5f, (float)i * 15.0f - 15.0f);
			transform->Scale(5, 5, 5);
		}
	}

	cube->UpdateTransforms();
	plane->UpdateTransforms();
	cylinder->UpdateTransforms();
	sphere->UpdateTransforms();

	meshes.push_back(cube);
	meshes.push_back(plane);
	meshes.push_back(cylinder);
	meshes.push_back(sphere);
}

void BlurDemo::Airplane()
{
	airplane = new ModelRender(shader);
	airplane->ReadMesh(L"B787/Airplane");
	airplane->ReadMaterial(L"B787/Airplane"); //TODO. ?????? ???????? ????????
	
	Transform* transform = airplane->AddTransform();
	transform->Scale(0.004f, 0.004f, 0.004f);
	transform->Position(2.0f, 9.91f, 2.0f);
	airplane->UpdateTransforms();

	models.push_back(airplane);
}

void BlurDemo::Kachujin()
{
	kachujin = new ModelAnimator(shader);
	kachujin->ReadMesh(L"Kachujin/Mesh");
	kachujin->ReadMaterial(L"Kachujin/Mesh");
	kachujin->ReadClip(L"Kachujin/Idle");
	kachujin->ReadClip(L"Kachujin/Walk");
	kachujin->ReadClip(L"Kachujin/Run");
	kachujin->ReadClip(L"Kachujin/Slash");
	kachujin->ReadClip(L"Kachujin/Uprock");

	Transform* transform = nullptr;
	
	transform = kachujin->AddTransform();
	transform->Position(0, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayTweenMode(0, 0, 1.0f);

	transform = kachujin->AddTransform();
	transform->Position(-15, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayTweenMode(1, 1, 1.0f);

	transform = kachujin->AddTransform();
	transform->Position(-30, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayTweenMode(2, 2, 0.75f);

	transform = kachujin->AddTransform();
	transform->Position(15, 0, -30);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayBlendMode(3, 0, 1, 2);
	kachujin->SetBlendAlpha(3, 1.75f);

	transform = kachujin->AddTransform();
	transform->Position(30, 0, -32.5f);
	transform->Scale(0.075f, 0.075f, 0.075f);
	kachujin->PlayTweenMode(4, 4, 0.75f);

	kachujin->UpdateTransforms();

	animators.push_back(kachujin);
}

void BlurDemo::Weapon()
{
	weapon = new ModelRender(shader);
	weapon->ReadMesh(L"Weapon/Sword");
	weapon->ReadMaterial(L"Weapon/Sword");

	UINT count = kachujin->TransformCount();
	for (UINT i = 0; i < count; i++)
		weapon->AddTransform();

	weapon->UpdateTransforms();
	models.push_back(weapon);


	weaponTransform = new Transform();
	weaponTransform->Position(-2.9f, 1.45f, -6.45f);
	weaponTransform->Scale(0.5f, 0.5f, 0.75f);
	weaponTransform->Rotation(0, 0, 1);
}

void BlurDemo::PointLights()
{
	PointLight light;
	light =
	{
		Color(0.0f, 0.0f, 0.0f, 1.0f), //Ambient;
		Color(0.0f, 0.3f, 1.0f, 1.0f),//Diffuse;
		Color(0.0f, 0.0f, 0.7f, 1.0f),//Specular;
		Color(0.0f, 0.0f, 0.7f, 1.0f),//Emissive;
		Vector3(-30, 10, -30),//Position;
		15.0f, //Range;
		0.9f //Intensity;
	};
	Lighting::Get()->AddPointLight(light);

	light =
	{
		Color(0.0f, 0.0f, 0.0f, 1.0f), //Ambient;
		Color(1.0f, 0.0f, 0.0f, 1.0f),//Diffuse;
		Color(0.6f, 0.2f, 0.0f, 1.0f),//Specular;
		Color(0.6f, 0.3f, 0.0f, 1.0f),//Emissive;
		Vector3(15, 10, -30),//Position;
		10.0f, //Range;
		1.0f //Intensity;
	};
	Lighting::Get()->AddPointLight(light);

	light =
	{
		Color(0.0f, 0.0f, 0.0f, 1.0f), //Ambient;
		Color(0.0f, 1.0f, 0.0f, 1.0f),//Diffuse;
		Color(0.0f, 0.7f, 0.0f, 1.0f),//Specular;
		Color(0.0f, 0.7f, 0.0f, 1.0f),//Emissive;
		Vector3(-5, 1, -17.0f),//Position;
		5.0f, //Range;
		0.9f //Intensity;
	};
	Lighting::Get()->AddPointLight(light);

	light =
	{
		Color(0.0f, 0.0f, 0.0f, 1.0f), //Ambient;
		Color(0.0f, 0.0f, 1.0f, 1.0f),//Diffuse;
		Color(0.0f, 0.0f, 0.7f, 1.0f),//Specular;
		Color(0.0f, 0.0f, 0.7f, 1.0f),//Emissive;
		Vector3(-10, 1, -17.0f),//Position;
		5.0f, //Range;
		0.9f //Intensity;
	};
	Lighting::Get()->AddPointLight(light);
}

void BlurDemo::SpotLights()
{
	SpotLight light;
	light =
	{
		Color(0.3f, 1.0f, 0.0f, 1.0f), //Ambient
		Color(0.7f, 1.0f, 0.0f, 1.0f), //Diffuse
		Color(0.3f, 1.0f, 0.0f, 1.0f), //Specular
		Color(0.3f, 1.0f, 0.0f, 1.0f), //Emissive
		Vector3(-15, 20, -30), //Position
		25.0f, //Range
		Vector3(0, -1, 0), //Direction
		30.0f, //Angle
		0.9f //Intensity
	};
	Lighting::Get()->AddSpotLight(light);


	light =
	{
		Color(1.0f, 0.2f, 0.9f, 1.0f), //Ambient
		Color(1.0f, 0.2f, 0.9f, 1.0f), //Diffuse
		Color(1.0f, 0.2f, 0.9f, 1.0f), //Specular
		Color(1.0f, 0.2f, 0.9f, 1.0f), //Emissive
		Vector3(0, 20, -30), //Position
		30.0f, //Range
		Vector3(0, -1, 0), //Direction
		40.0f, //Angle
		0.9f //Intensity
	};
	Lighting::Get()->AddSpotLight(light);



}

void BlurDemo::Billboards()
{
	billboard = new Billboard(shader);
	billboard->Pass(3);

	billboard->AddTexture(L"Terrain/grass_14.tga");
	billboard->AddTexture(L"Terrain/grass_07.tga");
	billboard->AddTexture(L"Terrain/grass_11.tga");

	for (UINT i = 0; i < 1200; i++)
	{
		Vector2 position = Math::RandomVec2(-60, 60);
		Vector2 scale = Math::RandomVec2(1, 3);

		billboard->Add(Vector3(position.x, scale.y * 0.5f, position.y), scale, 0);
	}

	for (UINT i = 0; i < 1200; i++)
	{
		Vector2 position = Math::RandomVec2(-60, 60);
		Vector2 scale = Math::RandomVec2(2, 4);

		billboard->Add(Vector3(position.x, scale.y * 0.5f, position.y), scale, 1);
	}

	for (UINT i = 0; i < 1200; i++)
	{
		Vector2 position = Math::RandomVec2(-60, 60);
		Vector2 scale = Math::RandomVec2(3, 5);

		billboard->Add(Vector3(position.x, scale.y * 0.5f, position.y), scale, 2);
	}
}

void BlurDemo::Pass(UINT val)
{
	for (MeshRender* mesh : meshes)
		mesh->Pass(val);

	val++;
	for (ModelRender* model : models)
		model->Pass(val);

	val++;
	for (ModelAnimator* animator : animators)
		animator->Pass(val);
}
