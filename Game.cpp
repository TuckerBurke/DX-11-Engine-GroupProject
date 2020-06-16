#include "Game.h"
#include "Vertex.h"
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Material.h"
#include "SimpleShader.h"
#include "SimpleAI.h"
#include "WICTextureLoader.h"
#include "PlayerInterface.h"
#include <algorithm>
#include <ppl.h>
#include <iostream>

using namespace Concurrency;
using namespace std;

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	parallel_for
	(
		size_t(0), entities.size(), [&](size_t i)
		{
			delete entities[i];
		},
		static_partitioner()
	);

	parallel_for 
	(
		size_t(0), materials.size(), [&](size_t i)
		{
			delete materials[i];
		},
		static_partitioner()
	);

	parallel_for 
	(
		size_t(0), meshes.size(), [&](size_t i) 
		{
			delete meshes[i];
		}, 
		static_partitioner()
	);

	parallel_for
	(
		size_t(0), ghostEntities.size(), [&](size_t i)
		{
			delete ghostEntities[i];
		},
		static_partitioner()
	);

	parallel_for
	(
		size_t(0), route1.size(), [&](size_t i)
		{
			delete route1[i];
		},
		static_partitioner()
	);

	parallel_for
	(
		size_t(0), route2.size(), [&](size_t i)
		{
			delete route2[i];
		},
		static_partitioner()
	);

	parallel_for
	(
		size_t(0), aiGhosts.size(), [&](size_t i)
		{
			delete aiGhosts[i];
		},
		static_partitioner()
	);

	srvBrick->Release();
	srvMetal->Release();
	srvRock->Release();
	srvRockNormal->Release();
	textureSampler->Release();
	srvCushion->Release();
	srvCushionNormal->Release();

	srvBlueprintBlue->Release();
	srvBlueprintGray->Release();
	srvBlueprintOrange->Release();
	srvBlueprintDefault->Release();
	srvBlueprintGreen->Release();

	blendState->Release();

	delete playerCamera;

	delete pixelShader;
	delete vertexShader;

	delete normalVS;
	delete normalPS;

	delete solidColorTransparentPS;

	delete[] lights;

	delete ppVS;
	
	delete ppPS;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	LoadShaders();

	CreateBasicGeometry();

	lights = new Light[MAX_LIGHTS_IN_SCENE];

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/**
	 * Scene Light definitions
	 */

	lights[lightsInScene].color = XMFLOAT3(1.0f, 0.2f, 0.2f);
	lights[lightsInScene].type = LIGHT_TYPE_SPOT;
	lights[lightsInScene].direction = XMFLOAT3(0.f, 0.f, -1.f);
	lights[lightsInScene].range = 15.f;
	lights[lightsInScene].intensity = 5.f;
	lights[lightsInScene].spotFalloff = 25.f;
	lights[lightsInScene++].position = XMFLOAT3(-4.5f, 3.f, -34.5f);

	lights[lightsInScene].color = XMFLOAT3(1.0f, 0.2f, 0.2f);
	lights[lightsInScene].type = LIGHT_TYPE_SPOT;
	lights[lightsInScene].direction = XMFLOAT3(0.f, 0.f, -1.f);
	lights[lightsInScene].range = 15.f;
	lights[lightsInScene].intensity = 5.f;
	lights[lightsInScene].spotFalloff = 25.f;
	lights[lightsInScene++].position = XMFLOAT3(-3.f, .5f, -24.f);

	lights[lightsInScene].color = XMFLOAT3(.65f, .2f, .3f);
	lights[lightsInScene].type = LIGHT_TYPE_POINT;
	lights[lightsInScene].range = 5.f;
	lights[lightsInScene].intensity = 2.f;
	lights[lightsInScene++].position = XMFLOAT3(0, 0, 0);

	lights[lightsInScene].color = XMFLOAT3(1.f, 1.f, 1.f);
	lights[lightsInScene].type = LIGHT_TYPE_POINT;
	lights[lightsInScene].range = 4.f;
	lights[lightsInScene].intensity = 2.f;
	lights[lightsInScene++].position = XMFLOAT3(-7.5f, 3.f, 7.5f);

	lights[lightsInScene].color = XMFLOAT3(1.f, 1.f, 1.f);
	lights[lightsInScene].type = LIGHT_TYPE_POINT;
	lights[lightsInScene].range = 2.5f;
	lights[lightsInScene].intensity = 2.f;
	lights[lightsInScene++].position = XMFLOAT3(-5.f, 1.85f, 1.f);

	lights[lightsInScene].color = XMFLOAT3(1.f, 1.f, 1.f);
	lights[lightsInScene].type = LIGHT_TYPE_POINT;
	lights[lightsInScene].range = 3.f;
	lights[lightsInScene].intensity = 1.5f;
	lights[lightsInScene++].position = XMFLOAT3(-5.f, 1.85f, -11.f);

	lights[lightsInScene].color = XMFLOAT3(1.f, 1.f, 1.f);
	lights[lightsInScene].type = LIGHT_TYPE_POINT;
	lights[lightsInScene].range = 4.5f;
	lights[lightsInScene].intensity = 1.f;
	lights[lightsInScene++].position = XMFLOAT3(5.f, 2.5f, -20.f);

	lights[lightsInScene].color = XMFLOAT3(1.f, 1.f, 1.f);
	lights[lightsInScene].type = LIGHT_TYPE_POINT;
	lights[lightsInScene].range = 4.f;
	lights[lightsInScene].intensity = 1.f;
	lights[lightsInScene++].position = XMFLOAT3(5.f, 2.5f, -33.f);

	lights[lightsInScene].color = XMFLOAT3(.5f, 1.f, .9f);
	lights[lightsInScene].type = LIGHT_TYPE_POINT;
	lights[lightsInScene].range = 4.f;
	lights[lightsInScene].intensity = 1.f;
	lights[lightsInScene++].position = XMFLOAT3(-4.5f, 2.5f, -34.5f);

	lights[lightsInScene].color = XMFLOAT3(.98f, .85f, .85f);
	lights[lightsInScene].type = LIGHT_TYPE_POINT;
	lights[lightsInScene].range = 4.5f;
	lights[lightsInScene].intensity = 1.f;
	lights[lightsInScene++].position = XMFLOAT3(-11.5f, 2.5f, -26.5f);

	lights[lightsInScene].color = XMFLOAT3(1.f, 1.f, 1.f);
	lights[lightsInScene].type = LIGHT_TYPE_AMBIENT;
	lights[lightsInScene++].intensity = .1f;

	ResizePostProcessResources();

	ppData.opacity = .95f;
	ppData.innerRadius = 0.2f;
	ppData.outerRadius = .6f;
	
	// all the initialization for the engine has to be done prior to this. Now the game specific stuff needs to initialize
	BeginPlay();
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	playerCamera = new Camera(XMFLOAT3(-5.1f, 2.1f, 5.0f), XMFLOAT3(0, XM_PI, 0), (float)this->width / this->height);

	vertexShader = new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShader.cso").c_str());
	pixelShader = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShader.cso").c_str());

	normalVS = new SimpleVertexShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"NormalMapVS.cso").c_str());
	normalPS = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"NormalMapPS.cso").c_str());

	solidColorTransparentPS = new SimplePixelShader(device.Get(), context.Get(), GetFullPathTo_Wide(L"SolidColorTransparentShader.cso").c_str());

	ppVS = new SimpleVertexShader(
		device.Get(),
		context.Get(),
		GetFullPathTo_Wide(L"PostProcessVS.cso").c_str());

	ppPS = new SimplePixelShader(
		device.Get(),
		context.Get(),
		GetFullPathTo_Wide(L"VignettePS.cso").c_str());

	// Make the blend state for basic alpha blending
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;

	// Define how to blend RGB components
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	// Define how to blend the alpha channel
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&blendDesc, &blendState);
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// setup models
    meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device.Get()));
	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device.Get()));
	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device.Get()));
	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/torus.obj").c_str(), device.Get()));
	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/cylinder.obj").c_str(), device.Get()));

	// setup game room models
	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/Rooms/BeginRoom.obj").c_str(), device.Get()));
	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/Rooms/MainRoom.obj").c_str(), device.Get()));

	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/RoomAssets/Arch.obj").c_str(), device.Get()));
	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/RoomAssets/Doorway.obj").c_str(), device.Get()));
	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/RoomAssets/Prism.obj").c_str(), device.Get()));
	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/RoomAssets/Pipe.obj").c_str(), device.Get()));

	// ghost model
	meshes.push_back(new Mesh(GetFullPathTo("../../Assets/Models/Enemies/inky.obj").c_str(), device.Get()));

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Tri-linear filtering
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT res = device->CreateSamplerState(&sampDesc, &textureSampler);

	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/brick.png").c_str(), nullptr, &srvBrick);
	if(res != S_OK) 
	{
		assert(false);
	}
	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"/../../Assets/Textures/metal.png").c_str(), nullptr, &srvMetal);
	if (res != S_OK)
	{
		assert(false);
	}
	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"/../../Assets/Textures/rock.png").c_str(), nullptr, &srvRock);
	if (res != S_OK)
	{
		assert(false);
	}
	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"/../../Assets/Textures/rock_normals.png").c_str(), nullptr, &srvRockNormal);
	if (res != S_OK)
	{
		assert(false);
	}
	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"/../../Assets/Textures/cushion.png").c_str(), nullptr, &srvCushion);
	if (res != S_OK)
	{
		assert(false);
	}
	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"/../../Assets/Textures/cushion_normals.png").c_str(), nullptr, &srvCushionNormal);
	if (res != S_OK)
	{
		assert(false);
	}
	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"/../../Assets/Textures/GridBox_Default.png").c_str(), nullptr, &srvBlueprintDefault);
	if (res != S_OK)
	{
		assert(false);
	}
	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"/../../Assets/Textures/prototype_512x512_orange.png").c_str(), nullptr, &srvBlueprintOrange);
	if (res != S_OK)
	{
		assert(false);
	}
	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"/../../Assets/Textures/prototype_512x512_blue2.png").c_str(), nullptr, &srvBlueprintBlue);
	if(res != S_OK) 
	{
		assert(false);
	}
	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"/../../Assets/Textures/prototype_512x512_grey2.png").c_str(), nullptr, &srvBlueprintGray);
	if (res != S_OK)
	{
		assert(false);
	}
	res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"/../../Assets/Textures/prototype_512x512_green1.png").c_str(), nullptr, &srvBlueprintGreen);
	if (res != S_OK)
	{
		assert(false);
	}

	// setup materials
	// sphere gets shininess
	// uses normal maps
	materials.push_back(new Material(XMFLOAT4(1.f, 1.f, 1.f, 1.f), 5.f, srvCushion, srvCushionNormal, textureSampler, normalVS, normalPS));
	// cube gets full shininess
	materials.push_back(new Material(XMFLOAT4(.8f, .86f, .8f, 1), 1.f, srvBrick, textureSampler, vertexShader, pixelShader));
	// helix slightly less shiny
	materials.push_back(new Material(XMFLOAT4(.88f, 0.1f, .68f, 1), .75f, srvMetal, textureSampler, vertexShader, pixelShader));
	// torus barely shiny
	materials.push_back(new Material(XMFLOAT4(.75f, .75f, .8f, 1), .45f, srvRock, srvRockNormal, textureSampler, normalVS, normalPS));
	// cylinder is not going to have any shininess
	materials.push_back(new Material(XMFLOAT4(0.2f, 0.8f, .28f, 1), 0, srvMetal, textureSampler, vertexShader, pixelShader));

	/*
	Stealth Game materials go here
	*/
	materials.push_back(new Material(XMFLOAT4(1.f, 1.f, 1.f, 1.f), 0.f, srvBlueprintBlue, textureSampler, vertexShader, pixelShader));
	materials.push_back(new Material(XMFLOAT4(1.f, 1.f, 1.f, 1.f), 0.f, srvBlueprintGray, textureSampler, vertexShader, pixelShader));

	materials.push_back(new Material(XMFLOAT4(1.f, 1.f, 1.f, 1.f), 0.f, srvBlueprintDefault, textureSampler, vertexShader, pixelShader));
	materials.push_back(new Material(XMFLOAT4(1.f, 1.f, 1.f, 1.f), 0.f, srvBlueprintOrange, textureSampler, vertexShader, pixelShader));
	materials.push_back(new Material(XMFLOAT4(1.f, 1.f, 1.f, 1.f), 0.f, srvBlueprintGreen, textureSampler, vertexShader, pixelShader));

	// transparent material
	materials.push_back(new Material(XMFLOAT4(.1f, .1f, 1.f, .5f), 0.f, vertexShader, solidColorTransparentPS));

	materials.push_back(new Material(XMFLOAT4(1.f, 1.f, 0.f, 1.f), 0.f, vertexShader, solidColorTransparentPS));

	// setup entities
	entities.push_back(new Entity(meshes[0], materials[0]));
	entities.push_back(new Entity(meshes[1], materials[1]));
	entities.push_back(new Entity(meshes[2], materials[2]));
	entities.push_back(new Entity(meshes[3],  materials[3]));
	entities.push_back(new Entity(meshes[4],  materials[4]));

	/*entities for the stealth game*/

	// blue building
	entities.push_back(new Entity(meshes[5], materials[5]));
	// gray building
	entities.push_back(new Entity(meshes[6], materials[6]));

	// room assets
	
	//arch
	entities.push_back(new Entity(meshes[7], materials[7]));
	//doorway
	entities.push_back(new Entity(meshes[8], materials[8]));
	//prism
	entities.push_back(new Entity(meshes[9], materials[8]));
	//pipe
	entities.push_back(new Entity(meshes[10], materials[9]));

	// Ghost
	ghostEntities.push_back(new Entity(meshes[11], materials[10]));
	ghostEntities.push_back(new Entity(meshes[11], materials[10]));

	/**
	 * The first route for the right side of the room
	 */
	
	route1.push_back(new Entity(meshes[0], materials[11]));
	route1.push_back(new Entity(meshes[0], materials[11]));
	route1.push_back(new Entity(meshes[0], materials[11]));
	route1.push_back(new Entity(meshes[0], materials[11]));
	route1.push_back(new Entity(meshes[0], materials[11]));

	route2.push_back(new Entity(meshes[0], materials[11]));
	route2.push_back(new Entity(meshes[0], materials[11]));
	route2.push_back(new Entity(meshes[0], materials[11]));
	route2.push_back(new Entity(meshes[0], materials[11]));
	route2.push_back(new Entity(meshes[0], materials[11]));

	aiGhosts.push_back(new SimpleAI(playerCamera, &route1[0], ghostEntities[0]));
	aiGhosts.push_back(new SimpleAI(playerCamera, &route2[0], ghostEntities[1]));

	
	bDrawWaypoints = true;
}


void Game::BeginPlay()
{
	if(entities.size() <= 0)
		return;

	entities[0]->GetTransform()->MoveAbsolute(3, 0, 1);

	entities[1]->GetTransform()->SetPosition(.2f, 1, .5f);

	//helix
	entities[2]->GetTransform()->SetPosition(-1.5, 0, -1);
	entities[2]->GetTransform()->SetScale(.5f, .5f, .5f);

	entities[4]->GetTransform()->SetPosition(1, -1.5, -.05f);

	// stealth game related begin play
	entities[6]->GetTransform()->MoveAbsolute(0, 0, -10);

	entities[7]->GetTransform()->SetRotation(0, 35.5f, 0.f);
	entities[7]->GetTransform()->MoveAbsolute(0,0,-24);

	entities[8]->GetTransform()->MoveAbsolute(8,.5f,-27);

	entities[9]->GetTransform()->MoveAbsolute(-9,.5f,-22);

	entities[10]->GetTransform()->MoveAbsolute(-6,.5f,-34);

	ghostEntities[0]->GetTransform()->MoveAbsolute(-6.f, .5f, -30.f);
	ghostEntities[1]->GetTransform()->MoveAbsolute(-3.f,.5f, -24.f);
	
	route1[0]->GetTransform()->MoveAbsolute(-8.5f, 1.5f, -35.f);
	route1[0]->GetTransform()->SetScale(.25f, .25f, .25f);

	route1[1]->GetTransform()->MoveAbsolute(-15.f, 1.5f, -35.f);
	route1[1]->GetTransform()->SetScale(.25f, .25f, .25f);

	route1[2]->GetTransform()->MoveAbsolute(-16.f, 1.5f, -30.f);
	route1[2]->GetTransform()->SetScale(.25f, .25f, .25f);

	route1[3]->GetTransform()->MoveAbsolute(-8.f, 1.5f, -27.f);
	route1[3]->GetTransform()->SetScale(.25f, .25f, .25f);

	route1[4]->GetTransform()->MoveAbsolute(-2.f, 1.5f, -29.f);
	route1[4]->GetTransform()->SetScale(.25f, .25f, .25f);

	route2[0]->GetTransform()->MoveAbsolute(-2.f, 1.5f, -20.f);
	route2[0]->GetTransform()->SetScale(.25f, .25f, .25f);

	route2[1]->GetTransform()->MoveAbsolute(0.f, 1.5f, -13.5f);
	route2[1]->GetTransform()->SetScale(.25f, .25f, .25f);

	route2[2]->GetTransform()->MoveAbsolute(-8.f, 1.5f, -12.f);
	route2[2]->GetTransform()->SetScale(.25f, .25f, .25f);

	route2[3]->GetTransform()->MoveAbsolute(-16.3f, 1.5f, -14.f);
	route2[3]->GetTransform()->SetScale(.25f, .25f, .25f);

	route2[4]->GetTransform()->MoveAbsolute(-13.5f, 1.5f, -20.f);
	route2[4]->GetTransform()->SetScale(.25f, .25f, .25f);
}

// ghostEntities are all transparent
void Game::SortAndRenderTransparentEntities()
{
	ghostEntities[0]->GetMaterial()->GetVertexShader()->SetShader();
	ghostEntities[0]->GetMaterial()->GetPixelShader()->SetShader();
	// Turn on the blend state
	context->OMSetBlendState(blendState, 0, UINT_MAX);

	auto camTransform = playerCamera->GetTransform();
	std::sort(ghostEntities.begin(), ghostEntities.end(), [&](const auto& lhs, const auto& rhs)
		{
			return (camTransform->DistanceSquaredTo(lhs->GetTransform()->GetPosition()) > camTransform->DistanceSquaredTo(rhs->GetTransform()->GetPosition()));
		});

	for (auto& ghost : ghostEntities)
	{
		ghost->DrawTransparent(context.Get(), playerCamera);
	}

	context->OMSetBlendState(nullptr, 0, UINT_MAX);
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	if(!playerCamera)
		return;
	playerCamera->UpdateProjectionMatrix((float)this->width / this->height);
	ResizePostProcessResources();
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	// Handle input
	inputSystem->Frame(deltaTime, playerCamera);

	if(entities.size() == 0) 
	{
		return;
	}

	float sinTime = (float)sin(totalTime);
	float offset = (sinTime*deltaTime);

	entities[0]->GetTransform()->MoveAbsolute(-offset/3.f, offset/5.f, 0);
	entities[0]->GetTransform()->SetPosition(
		entities[0]->GetTransform()->GetPosition().x,
		entities[0]->GetTransform()->GetPosition().y, -.01f
	);

	entities[1]->GetTransform()->MoveAbsolute(0, offset, 0);

	entities[2]->GetTransform()->Rotate(0,  1.f * deltaTime, 0);
	
	entities[3]->GetTransform()->MoveAbsolute(0,0, offset*2.f);
	entities[3]->GetTransform()->MoveAbsolute(offset/2.f, -offset/2.f, 0);
	entities[3]->GetTransform()->Rotate(-1.5f * deltaTime, 0, 0);

	entities[4]->GetTransform()->Rotate(0, 0,  offset*2.f);
	
	// Vignette Calculation
	float	distToLight;
	int	lightType;
	float	lightRange;
	bool	inLight = PlayerInLight(&distToLight, &lightType, &lightRange);
	CalculateVignette(inLight, distToLight, lightType, lightRange);
	
	for (SimpleAI* ai : aiGhosts)
	{
		ai->Update(inLight, deltaTime);
	}

	lights[0].position = aiGhosts[0]->self->GetTransform()->GetPosition();
	lights[0].position.y = 1.5f;
	
	lights[1].position = aiGhosts[1]->self->GetTransform()->GetPosition();
	lights[1].position.y = 1.5f;	

	playerCamera->UpdateViewMatrix();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(
		depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// Clear post process target too
	context->ClearRenderTargetView(ppRTV.Get(), color);
	
	// --- Post Processing - Pre-Draw ---------------------
	{
		// Change the render target
		context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), depthStencilView.Get());
	}

	// since they are all shared we don't need to individually set it per entity
	normalPS->SetData("lights", (void*)(lights), sizeof(Light) * lightsInScene);
	normalPS->SetInt("lightCount", lightsInScene);
	normalPS->SetFloat3("cameraPosition", playerCamera->GetTransform()->GetPosition());
	normalPS->CopyAllBufferData();

	pixelShader->SetData("lights", (void*)(lights), sizeof(Light) * lightsInScene);
	pixelShader->SetInt("lightCount", lightsInScene);
	pixelShader->SetFloat3("cameraPosition", playerCamera->GetTransform()->GetPosition());
	pixelShader->CopyAllBufferData();

	for (Entity* entity : entities)
	{
		// detect if light affects the material
		Material* entityMat = entity->GetMaterial();
		entityMat->GetVertexShader()->SetShader();
		entityMat->GetPixelShader()->SetShader();

		entity->Draw(context.Get(), playerCamera);
	}


	if(bDrawWaypoints) 
	{
		route1[0]->GetMaterial()->GetPixelShader()->SetShader();
		for(Entity* route : route1) 
		{
			route->DrawTransparent(context.Get(), playerCamera);
		}
		for (Entity* route : route2)
		{
			route->DrawTransparent(context.Get(), playerCamera);
		}
	}

	SortAndRenderTransparentEntities();

	// --- Post processing - Post-Draw -----------------------
	{
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), 0);

		// Set up post process shaders
		ppVS->SetShader();

		ppPS->SetShaderResourceView("pixels", ppSRV.Get());
		ppPS->SetSamplerState("samplerOptions", textureSampler);
		ppPS->SetShader();

		ppData.ppgData.width = 1.f / width;
		ppData.ppgData.height = 1.f / height;
		ppPS->SetData("vignetteData", (void*)&ppData, sizeof(VignetteData));
		ppPS->CopyAllBufferData();

		// Turn OFF vertex and index buffers
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		ID3D11Buffer* nothing = 0;
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
		context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);

		// Draw exactly 3 vertices for our "full screen triangle"
		context->Draw(3, 0);


		// Unbind shader resource views at the end of the frame,
		// since we'll be rendering into one of those textures
		// at the start of the next
		ID3D11ShaderResourceView* nullSRVs[16] = {};
		context->PSSetShaderResources(0, 16, nullSRVs);
	}

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}

void Game::ResizePostProcessResources()
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // Will render to it and sample from it!
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* ppTexture;
	device->CreateTexture2D(&textureDesc, 0, &ppTexture);

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture, &rtvDesc, ppRTV.ReleaseAndGetAddressOf());

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture, &srvDesc, ppSRV.ReleaseAndGetAddressOf());

	// We don't need the texture reference itself no mo'
	ppTexture->Release();
}

// --------------------------------------------------------
// Calculate the vignette opacity and pass to post processing
// --------------------------------------------------------
void Game::CalculateVignette(bool inLight, float sqDist, int lightType, float lightRange)
{
	// Only point lights affect vignette
	if (!inLight || lightType != LIGHT_TYPE_POINT)
	{
		ppData.opacity = .95f;
		return;
	}

	// Calculate vignette based on light range
	ppData.opacity = (sqDist / lightRange) - .05f;
}

// --------------------------------------------------------
// Iterate through all lights, and return information about
// any lights the player is standing in.
// --------------------------------------------------------
bool Game::PlayerInLight(_Out_ float* _sqDist, _Out_ int* _lightType, _Out_ float* _sqLightRange)
{
	// Return true if player is within the range of any light
	for (int i = 0; i < lightsInScene; ++i)
	{
		float SqLightRange = lights[i].range * lights[i].range;
		XMFLOAT3 lightPos = lights[i].position;
		float sqDistToLight = playerCamera->GetTransform()->DistanceSquaredTo(lightPos);

		if (sqDistToLight < SqLightRange)
		{
			// If in light range, return light type and distance to light thru params
			// some processes like vignette need this info
			*_sqDist = sqDistToLight;
			*_lightType = lights[i].type;
			*_sqLightRange = SqLightRange;
			return true;
		}
	}
	// If false, return clearly invalid values to each 
	*_sqDist = -1.0f;
	*_lightType = -1;
	*_sqLightRange = -1.0f;

	return false;
}