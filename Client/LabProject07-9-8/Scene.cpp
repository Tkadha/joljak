//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "GameFramework.h"

#include "NonAtkState.h"
#include "AtkState.h"

bool ChangeAlbedoTexture(
	CGameObject* pParentGameObject,
	int materialIndex,
	UINT textureSlot,
	const wchar_t* textureFilePath,
	ResourceManager* pResourceManager,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12Device* pd3dDevice)
{
	if (!pParentGameObject || !pParentGameObject->m_pChild ||
		!pResourceManager || !pd3dCommandList || !pd3dDevice ||
		!textureFilePath || !*textureFilePath) { 
		return false;
	}

	CMaterial* pTargetMaterial = pParentGameObject->m_pChild->GetMaterial(materialIndex);
	if (!pTargetMaterial) {
		return false;
	}

	std::shared_ptr<CTexture> pTextureToAssign = pResourceManager->GetTexture(textureFilePath, pd3dCommandList);
	if (!pTextureToAssign) {
		return false;
	}

	return pTargetMaterial->AssignTexture(textureSlot, pTextureToAssign, pd3dDevice);
}

CScene::CScene(CGameFramework* pFramework) : m_pGameFramework(pFramework)
{
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 5;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = false;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 300.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(230.0f, 330.0f, 480.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	
	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f); 
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.75f, 0.7f, 1.0f); 
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f); 
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.5f, -0.707f, 0.5f); 


	m_pLights[1].m_bEnable = false;
	{
		m_pLights[1].m_nType = SPOT_LIGHT;
		m_pLights[1].m_fRange = 500.0f;
		m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
		m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
		m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
		m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
		m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
		m_pLights[1].m_fFalloff = 8.0f;
		m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
		m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	}
	m_pLights[3].m_bEnable = false;
	{
		m_pLights[3].m_nType = SPOT_LIGHT;
		m_pLights[3].m_fRange = 600.0f;
		m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
		m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
		m_pLights[3].m_xmf3Position = XMFLOAT3(550.0f, 330.0f, 530.0f);
		m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
		m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
		m_pLights[3].m_fFalloff = 8.0f;
		m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
		m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
	}
	m_pLights[4].m_bEnable = false;
	{
		m_pLights[4].m_nType = POINT_LIGHT;
		m_pLights[4].m_fRange = 200.0f;
		m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.3f, 0.3f, 1.0f);
		m_pLights[4].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
		m_pLights[4].m_xmf3Position = XMFLOAT3(600.0f, 250.0f, 700.0f);
		m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	}
}

void CScene::ServerBuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	assert(m_pGameFramework != nullptr && "GameFramework pointer is needed!");
	ShaderManager* pShaderManager = m_pGameFramework->GetShaderManager();
	assert(pShaderManager != nullptr && "ShaderManager is not available!");
	ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager(); 

	BuildDefaultLightsAndMaterials();

	if (!pResourceManager) {
		OutputDebugString(L"Error: ResourceManager is not available in CScene::BuildObjects.\n");
		return;
	}

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pGameFramework);
	srand((unsigned int)time(NULL));

	XMFLOAT3 xmf3Scale(5.f, 0.2f, 5.f);
	XMFLOAT4 xmf4Color(0.0f, 0.0f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, _T("Terrain/terrain_16.raw"), 2049, 2049, xmf3Scale, xmf4Color, m_pGameFramework);
	m_pTerrain->m_xmf4x4World = Matrix4x4::Identity();
	m_pTerrain->m_xmf4x4ToParent = Matrix4x4::Identity();


	m_pPreviewPine = new CConstructionObject(
		pd3dDevice, pd3dCommandList, m_pGameFramework);
	m_pPreviewPine->SetPosition(XMFLOAT3(0, 0, 0));
	
	m_pPreviewPine->SetScale(10, 10, 10);
	
	m_pPreviewPine->isRender = false;
	m_pPreviewPine->m_id = -1;
	m_pPreviewPine->m_treecount = tree_obj_count;
	m_vGameObjects.emplace_back(m_pPreviewPine);
	auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, m_pPreviewPine->m_worldOBB.Center);
	octree.insert(std::move(t_obj));

	for (auto obj : m_vGameObjects) {
		obj->SetOBB(1.f,1.f,1.f,XMFLOAT3{0.f,0.f,0.f});
		obj->InitializeOBBResources(pd3dDevice, pd3dCommandList);
		if (obj->m_pSkinnedAnimationController) obj->PropagateAnimController(obj->m_pSkinnedAnimationController);
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	
	assert(m_pGameFramework != nullptr && "GameFramework pointer is needed!");
	ShaderManager* pShaderManager = m_pGameFramework->GetShaderManager();
	assert(pShaderManager != nullptr && "ShaderManager is not available!");
	ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager(); 
	BuildDefaultLightsAndMaterials();

	if (!pResourceManager) {
		
		OutputDebugString(L"Error: ResourceManager is not available in CScene::BuildObjects.\n");
		return;
	}

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pGameFramework);
	srand((unsigned int)time(NULL));

	XMFLOAT3 xmf3Scale(5.f, 0.2f, 5.f);
	XMFLOAT4 xmf4Color(0.0f, 0.0f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, _T("Terrain/terrain_16.raw"), 2049, 2049, xmf3Scale, xmf4Color, m_pGameFramework);
	m_pTerrain->m_xmf4x4World = Matrix4x4::Identity();
	m_pTerrain->m_xmf4x4ToParent = Matrix4x4::Identity();

	
	std::random_device rd;
	std::mt19937 gen(rd());


	m_pShadowMap = std::make_unique<ShadowMap>(pd3dDevice, 2048, 2048);

	float spawnMin = 500, spawnMax = 9500;
	float objectMinSize = 15, objectMaxSize = 20;

	int nTreeObjects = 100;
	for (int i = 0; i < nTreeObjects; ++i) {
		CGameObject* gameObj = new CPineObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);

		gameObj->m_treecount = tree_obj_count;
		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nTreeObjects; ++i) {
		CGameObject* gameObj = new CBirchObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		int materialIndexToChange = 1;
		UINT albedoTextureSlot = 0;
		const wchar_t* textureFile = L"Model/Textures/Tree_Bark_Diffuse.dds";

		ChangeAlbedoTexture(gameObj, materialIndexToChange, albedoTextureSlot, textureFile, pResourceManager, pd3dCommandList, pd3dDevice);

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nTreeObjects; ++i) {
		CGameObject* gameObj = new CWillowObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		////XMFLOAT3 position = gameObj->GetPosition();
		//XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		////position.y += 50.0f;
		//XMFLOAT3 size = XMFLOAT3(1.0f, 10.0f, 1.0f);
		//XMFLOAT4 rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		//gameObj->SetOBB(position, size, rotation);
		//gameObj->InitializeOBBResources(pd3dDevice, pd3dCommandList);

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}


	int nRockObjects = 100;	objectMinSize = 10, objectMaxSize = 15;
	for (int i = 0; i < nRockObjects; ++i) {
		CGameObject* gameObj = new CRockClusterAObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		int materialIndexToChange = 0;
		UINT albedoTextureSlot = 0;
		const wchar_t* textureFile = L"Model/Textures/RockClusters_AlbedoRoughness.dds";

		ChangeAlbedoTexture(gameObj, materialIndexToChange, albedoTextureSlot, textureFile, pResourceManager, pd3dCommandList, pd3dDevice);

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nRockObjects; ++i) {
		CGameObject* gameObj = new CRockClusterBObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		int materialIndexToChange = 0;
		UINT albedoTextureSlot = 0;
		const wchar_t* textureFile = L"Model/Textures/RockClusters_AlbedoRoughness.dds";

		ChangeAlbedoTexture(gameObj, materialIndexToChange, albedoTextureSlot, textureFile, pResourceManager, pd3dCommandList, pd3dDevice);

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nRockObjects; ++i) {
		CGameObject* gameObj = new CRockClusterCObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		int materialIndexToChange = 0;
		UINT albedoTextureSlot = 0;
		const wchar_t* textureFile = L"Model/Textures/RockClusters_AlbedoRoughness.dds";

		ChangeAlbedoTexture(gameObj, materialIndexToChange, albedoTextureSlot, textureFile, pResourceManager, pd3dCommandList, pd3dDevice);

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}


	nRockObjects = 30;	objectMinSize = 30, objectMaxSize = 50;
	spawnMin = 1700, spawnMax = 9000;
	for (int i = 0; i < nRockObjects; ++i) {
		CGameObject* gameObj = new CCliffFObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		int materialIndexToChange = 0;
		UINT albedoTextureSlot = 0;
		const wchar_t* textureFile = L"Model/Textures/RockClusters_AlbedoRoughness.dds";

		ChangeAlbedoTexture( gameObj,materialIndexToChange,albedoTextureSlot,textureFile,pResourceManager,pd3dCommandList, pd3dDevice);

		m_vGameObjects.emplace_back(gameObj);

		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}


	m_pPreviewPine = new CConstructionObject(
		pd3dDevice, pd3dCommandList, m_pGameFramework);
	m_pPreviewPine->SetPosition(XMFLOAT3(0, 0, 0));

	//auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
	m_pPreviewPine->SetScale(10, 10, 10);

	m_pPreviewPine->isRender = false;

	m_pPreviewPine->m_treecount = tree_obj_count;
	m_vGameObjects.emplace_back(m_pPreviewPine);
	auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, m_pPreviewPine->m_worldOBB.Center);
	octree.insert(std::move(t_obj));

	//int nCliffFObjectCObjects = 5;
	//for (int i = 0; i < nCliffFObjectCObjects; ++i) {
	//	CGameObject* gameObj = new CCliffFObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
	//	auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
	//	gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
	//	auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
	//	gameObj->SetScale(w, h, w);
	//	m_vGameObjects.emplace_back(gameObj);
	//}



	int nBushObject = 100;
	objectMinSize = 10, objectMaxSize = 15;
	for (int i = 0; i < nBushObject; ++i) {
		CGameObject* gameObj = new CBushAObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	int nVegetationObject = 20;
	for (int i = 0; i < nVegetationObject; ++i) {
		CGameObject* gameObj = new CStaticObject(pd3dDevice, pd3dCommandList, "Model/Vegetation/ChervilCluster.bin", m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nVegetationObject; ++i) {
		CGameObject* gameObj = new CStaticObject(pd3dDevice, pd3dCommandList, "Model/Vegetation/RedPoppyCluster.bin", m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nVegetationObject; ++i) {
		CGameObject* gameObj = new CStaticObject(pd3dDevice, pd3dCommandList, "Model/Vegetation/Speedwell.bin", m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nVegetationObject; ++i) {
		CGameObject* gameObj = new CStaticObject(pd3dDevice, pd3dCommandList, "Model/Vegetation/ElephantEar_A.bin", m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nVegetationObject; ++i) {
		CGameObject* gameObj = new CStaticObject(pd3dDevice, pd3dCommandList, "Model/Vegetation/GrassPatch_LOD0.bin", m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}

	for (int i = 0; i < nVegetationObject; ++i) {
		CGameObject* gameObj = new CStaticObject(pd3dDevice, pd3dCommandList, "Model/Vegetation/Groundcover_Clovers.bin", m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nVegetationObject; ++i) {
		CGameObject* gameObj = new CStaticObject(pd3dDevice, pd3dCommandList, "Model/Vegetation/Groundcover_Daisies.bin", m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nVegetationObject; ++i) {
		CGameObject* gameObj = new CStaticObject(pd3dDevice, pd3dCommandList, "Model/Vegetation/Groundcover_Leaves.bin", m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nVegetationObject; ++i) {
		CGameObject* gameObj = new CStaticObject(pd3dDevice, pd3dCommandList, "Model/Vegetation/Groundcover_Poppies.bin", m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}




	int nCowObjects = 10;
	int animate_count = 13;
	for (int i = 0; i < nCowObjects; ++i)
	{
		CLoadedModelInfo* pCowModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Cow.bin", m_pGameFramework);
		CGameObject* gameobj = new CMonsterObject(pd3dDevice, pd3dCommandList, pCowModel, animate_count, m_pGameFramework);
		gameobj->m_objectType = GameObjectType::Cow;
		gameobj->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		for (int j = 1; j < animate_count; ++j) {
			gameobj->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
			gameobj->m_pSkinnedAnimationController->SetTrackEnable(j, false);
		}
		gameobj->SetOwningScene(this);
		gameobj->FSM_manager->SetCurrentState(std::make_shared<NonAtkNPCStandingState>());
		gameobj->FSM_manager->SetGlobalState(std::make_shared<NonAtkNPCGlobalState>());

		gameobj->Rotate(0.f, 180.f, 0.f);
		auto [x, z] = genRandom::generateRandomXZ(gen, 800, 2500, 800, 2500);
		gameobj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		gameobj->SetScale(12.0f, 12.0f, 12.0f);
		gameobj->SetTerraindata(m_pTerrain);
		gameobj->m_treecount = tree_obj_count;
		m_vGameObjects.emplace_back(gameobj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameobj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
		if (pCowModel) delete pCowModel;
	}
	int nPigObjects = 10;
	for (int i = 0; i < nPigObjects; ++i)
	{
		CLoadedModelInfo* pPigModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Pig.bin", m_pGameFramework);
		CGameObject* gameobj = new CMonsterObject(pd3dDevice, pd3dCommandList, pPigModel, animate_count, m_pGameFramework);
		gameobj->m_objectType = GameObjectType::Pig;
		gameobj->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		for (int j = 1; j < animate_count; ++j) {
			gameobj->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
			gameobj->m_pSkinnedAnimationController->SetTrackEnable(j, false);
		}
		gameobj->SetOwningScene(this);
		gameobj->FSM_manager->SetCurrentState(std::make_shared<NonAtkNPCStandingState>());
		gameobj->FSM_manager->SetGlobalState(std::make_shared<NonAtkNPCGlobalState>());
		gameobj->Rotate(0.f, 180.f, 0.f);
		auto [x, z] = genRandom::generateRandomXZ(gen, 800, 2500, 800, 2500);
		gameobj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		gameobj->SetScale(10.0f, 10.0f, 10.0f);
		gameobj->SetTerraindata(m_pTerrain);
		gameobj->m_treecount = tree_obj_count;

		m_vGameObjects.emplace_back(gameobj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameobj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
		if (pPigModel) delete pPigModel;
	}

	int nSpiderObjects = 10;
	for (int i = 0; i < nSpiderObjects; ++i)
	{
		CLoadedModelInfo* pSpiderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Spider.bin", m_pGameFramework);
		CGameObject* gameobj = new CMonsterObject(pd3dDevice, pd3dCommandList, pSpiderModel, animate_count, m_pGameFramework);
		gameobj->m_objectType = GameObjectType::Spider;
		gameobj->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		for (int j = 1; j < animate_count; ++j) {
			gameobj->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
			gameobj->m_pSkinnedAnimationController->SetTrackEnable(j, false);
		}
		gameobj->SetOwningScene(this);
		gameobj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		gameobj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		gameobj->Rotate(0.f, 180.f, 0.f);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1800, 3500, 1800, 3500);
		gameobj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		gameobj->SetScale(8.f, 8.f, 8.f);
		gameobj->SetTerraindata(m_pTerrain);
		gameobj->m_treecount = tree_obj_count;
		m_vGameObjects.emplace_back(gameobj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameobj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
		if (pSpiderModel) delete pSpiderModel;
	}
	int nToadObjects = 10;
	for (int i = 0; i < nToadObjects; ++i)
	{
		CLoadedModelInfo* pToadModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Toad.bin", m_pGameFramework);
		CGameObject* gameobj = new CMonsterObject(pd3dDevice, pd3dCommandList, pToadModel, animate_count, m_pGameFramework);
		gameobj->m_objectType = GameObjectType::Toad;
		gameobj->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		for (int j = 1; j < animate_count; ++j) {
			gameobj->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
			gameobj->m_pSkinnedAnimationController->SetTrackEnable(j, false);
		}
		gameobj->SetOwningScene(this);
		gameobj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		gameobj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		gameobj->Rotate(0.f, 180.f, 0.f);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1800, 3500, 1800, 3500);
		gameobj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		gameobj->SetScale(8.f, 8.f, 8.f);
		gameobj->SetTerraindata(m_pTerrain);
		gameobj->m_treecount = tree_obj_count;
		m_vGameObjects.emplace_back(gameobj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameobj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
		if (pToadModel) delete pToadModel;
	}
	int nWolfObjects = 10;
	for (int i = 0; i < nWolfObjects; ++i)
	{
		CLoadedModelInfo* pWolfModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Wolf.bin", m_pGameFramework);
		CGameObject* gameobj = new CMonsterObject(pd3dDevice, pd3dCommandList, pWolfModel, animate_count, m_pGameFramework);
		gameobj->m_objectType = GameObjectType::Wolf;
		gameobj->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		for (int j = 1; j < animate_count; ++j) {
			gameobj->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
			gameobj->m_pSkinnedAnimationController->SetTrackEnable(j, false);
		}
		gameobj->SetOwningScene(this);
		gameobj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		gameobj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

		gameobj->Rotate(0.f, 180.f, 0.f);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1800, 3500, 1800, 3500);
		gameobj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		gameobj->SetScale(10.f, 10.f, 10.f);
		gameobj->SetTerraindata(m_pTerrain);
		gameobj->m_treecount = tree_obj_count;
		m_vGameObjects.emplace_back(gameobj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameobj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
		if (pWolfModel) delete pWolfModel;
	}

	// 플레이어 커스터마이징(임시)

	//int materialIndexToChange = 0;
	//UINT albedoTextureSlot = 0;
	//const wchar_t* textureFile = L"Model/Textures/T_HU_M_Body_04_D.dds";
	//CGameObject* gameObj = m_pPlayer->FindFrame("Bracers_Naked");
	//ChangeAlbedoTexture(gameObj, materialIndexToChange, albedoTextureSlot, textureFile, pResourceManager, pd3dCommandList, pd3dDevice);

	//m_pPlayer->SetCollisionTargets(m_vGameObjects);

	for (auto obj : m_vGameObjects) {
		if (obj->m_objectType == GameObjectType::Tree) {
			obj->SetOBB(0.1f, 1.0f, 0.1f,XMFLOAT3(0.0f,0.0f,0.0f));
		}
		else if (obj->m_objectType == GameObjectType::Pig) {
			obj->SetOBB(1.0f, 0.8f, 1.0f, XMFLOAT3(0.0f, 1.0f, -1.0f));
		}
		else {
			obj->SetOBB(1.0f,1.0f,1.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
		}
		
		if (obj->m_pSkinnedAnimationController) obj->PropagateAnimController(obj->m_pSkinnedAnimationController);

		switch (obj->m_objectType)
		{
		case GameObjectType::Wasp:
		case GameObjectType::Snail:
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[5].SetAnimationType(ANIMATION_TYPE_ONCE);
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[6].SetAnimationType(ANIMATION_TYPE_ONCE);
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[7].SetAnimationType(ANIMATION_TYPE_ONCE);
			break;
		case GameObjectType::Snake:
		case GameObjectType::Spider:
		case GameObjectType::Bat:
		case GameObjectType::Turtle:
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[9].SetAnimationType(ANIMATION_TYPE_ONCE);
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[10].SetAnimationType(ANIMATION_TYPE_ONCE);
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[11].SetAnimationType(ANIMATION_TYPE_ONCE);
			break;
		case GameObjectType::Wolf:
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[8].SetAnimationType(ANIMATION_TYPE_ONCE);
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[9].SetAnimationType(ANIMATION_TYPE_ONCE);
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[10].SetAnimationType(ANIMATION_TYPE_ONCE);
			break;
		case GameObjectType::Toad:
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[7].SetAnimationType(ANIMATION_TYPE_ONCE);
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[8].SetAnimationType(ANIMATION_TYPE_ONCE);
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[9].SetAnimationType(ANIMATION_TYPE_ONCE);
			break;
		case GameObjectType::Cow:
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[8].SetAnimationType(ANIMATION_TYPE_ONCE);
			break;
		case GameObjectType::Pig:
			obj->m_pSkinnedAnimationController->m_pAnimationTracks[9].SetAnimationType(ANIMATION_TYPE_ONCE);
			obj->m_localOBB.Center.y += 30.0f;
			break;
		default:	
			break;
		}

		obj->InitializeOBBResources(pd3dDevice, pd3dCommandList);
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void CScene::ReleaseObjects()
{
	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;

	ReleaseShaderVariables();
	//for(auto& obj : m_vGameObjects) {
	//	if (obj) delete obj;
	//}
	m_vGameObjects.clear();
	m_listBranchObjects.clear();

	if (m_pLights) delete[] m_pLights;
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); 
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);


	
	UINT m_nObjects = 100;
	
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL,
		sizeof(VS_VB_INSTANCE) * m_nObjects, D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	
	m_pd3dcbGameObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);
	
	m_d3dInstancingBufferView.BufferLocation =
		m_pd3dcbGameObjects->GetGPUVirtualAddress();
	m_d3dInstancingBufferView.StrideInBytes = sizeof(VS_VB_INSTANCE);
	m_d3dInstancingBufferView.SizeInBytes = sizeof(VS_VB_INSTANCE) * m_nObjects;
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pcbMappedLights && m_pLights) {
		assert(m_nLights >= 0 && m_nLights <= MAX_LIGHTS && "Invalid number of lights!");
		if (m_nLights < 0 || m_nLights > MAX_LIGHTS) {
			OutputDebugStringA("!!!!!!!! ERROR: Invalid m_nLights value detected! Clamping to 0. !!!!!!!!\n");
			m_nLights = 0; 
		}
		::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
		::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
		::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
	}
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}

	if (m_pd3dcbGameObjects) m_pd3dcbGameObjects->Unmap(0, NULL);
	if (m_pd3dcbGameObjects) m_pd3dcbGameObjects->Release();
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	for(auto& obj : m_vGameObjects) {
		if (obj) obj->ReleaseUploadBuffers();
	}
}


bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		break;
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	if (m_pPlayer) m_pPlayer->checkmove = false;

	for (auto& obj : m_listBranchObjects) {
		if (CollisionCheck(m_pPlayer, obj)) {
			auto branch = dynamic_cast<CBranchObject*>(obj);
			if (branch && branch->m_bOnGround && branch->isRender) {
				m_pPlayer->m_pGameFramework->AddItem("wood", 1);
				branch->isRender = false;
			}
		}
	}

	for (auto& obj : m_listRockObjects) {
		if (CollisionCheck(m_pPlayer, obj)) {
			auto rock = dynamic_cast<CRockDropObject*>(obj);
			if (rock && rock->m_bOnGround && rock->isRender) {
				int randValue = rand() % 100; // 0 ~ 99
				if (randValue < 50) {
					m_pGameFramework->AddItem("stone",3);
				}
				else if (randValue < 75) {
					m_pGameFramework->AddItem("coal",1);
				}
				else {
					m_pGameFramework->AddItem("iron_material",1);
				}
				rock->isRender = false;
			}
		}
	}

	/*
	if (m_pPlayer) {
		for (auto obj : m_vGameObjects) {
			if (obj->m_objectType != GameObjectType::Player&& obj->isRender) {
				if (m_pPlayer->CheckCollisionOBB(obj)) {
					m_pPlayer->checkmove = true; // 異⑸룎 諛쒖깮 ???대룞 湲덉?
					break;
				}
			}
		}
	}
	*/
	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}
}


void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, FrameResource* pFrameResource)
{

	assert(m_pGameFramework != nullptr && "GameFramework pointer is needed in CScene!");
	ShaderManager* pShaderManager = m_pGameFramework->GetShaderManager();
	assert(pShaderManager != nullptr && "ShaderManager is not available!");

	// 사용할 상수 버퍼들을 가져옵니다.
	auto passCB = pFrameResource->PassCB->Resource();
	auto objectCB = pFrameResource->ObjectCB->Resource();

	// =================================================================
	// Pass 1: 그림자 맵 생성 (빛의 시점)
	// =================================================================
	{
		// --- 렌더 타겟을 그림자 맵으로 설정 ---
		pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pShadowMap->Resource(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		pd3dCommandList->RSSetViewports(1, &m_pShadowMap->Viewport());
		pd3dCommandList->RSSetScissorRects(1, &m_pShadowMap->ScissorRect());
		pd3dCommandList->ClearDepthStencilView(m_pShadowMap->Dsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		pd3dCommandList->OMSetRenderTargets(0, nullptr, FALSE, &m_pShadowMap->Dsv());

		// --- 그림자 생성을 위한 PSO와 루트 서명 설정 ---
		// (참고: "Shadow" 셰이더와 PSO는 미리 생성되어 있어야 합니다)
		CShader* pShadowShader = pShaderManager->GetShader("Shadow", pd3dCommandList);
		pd3dCommandList->SetPipelineState(pShadowShader->GetPipelineState());
		pd3dCommandList->SetGraphicsRootSignature(pShadowShader->GetRootSignature());

		// Pass 상수 버퍼를 바인딩합니다. (빛의 ViewProj 행렬 등이 필요)
		pd3dCommandList->SetGraphicsRootConstantBufferView(0, passCB->GetGPUVirtualAddress());

		// --- 그림자를 생성하는 모든 오브젝트를 그립니다 ---
		// 여기서는 더 간단한 RenderShadow 함수를 호출합니다.
		for (auto& pObject : m_vGameObjects) {
			if (pObject) pObject->RenderShadow(pd3dCommandList, objectCB);
		}
		for (auto& pObject : PlayerList) {
			if (pObject.second) pObject.second->RenderShadow(pd3dCommandList, objectCB);
		}
		if (m_pPlayer) m_pPlayer->RenderShadow(pd3dCommandList, objectCB);

	}

	// 그림자 맵을 셰이더에서 읽을 수 있도록 상태를 다시 변경합니다.
	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pShadowMap->Resource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));

	// =================================================================
	// Pass 2: 메인 씬 렌더링 (카메라 시점)
	// =================================================================

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pCamera->UpdateShaderVariables(pd3dCommandList);


	UpdateShaderVariables(pd3dCommandList);


	ID3D12DescriptorHeap* ppHeaps[] = { m_pGameFramework->GetCbvSrvHeap() };
	if (ppHeaps[0]) {
		pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	}
	else {
		assert(!"CBV/SRV Descriptor Heap is NULL in CScene::Render!");
		return;
	}


	m_pCurrentRootSignature = nullptr;
	m_pCurrentPSO = nullptr;
	m_pCurrentShader = nullptr;




	if (m_pSkyBox) {
		m_pSkyBox->Render(pd3dCommandList, pCamera);
	}


	if (m_pTerrain) {
		m_pTerrain->Render(pd3dCommandList, pCamera);
	}

	//std::vector<tree_obj*> results;
	//tree_obj player_obj{ -1, m_pPlayer->GetPosition() };
	//octree.query(player_obj, XMFLOAT3{ 2500,1000,2500 }, results);
	//{
	//	std::lock_guard<std::mutex> lock(m_Mutex);
	//	for (auto& obj : results) {
	//		if (m_vGameObjects[obj->u_id]) {
	//			if (m_vGameObjects[obj->u_id]->FSM_manager) m_vGameObjects[obj->u_id]->FSMUpdate();
	//			//if (m_vGameObjects[obj->u_id]->m_pSkinnedAnimationController) m_vGameObjects[obj->u_id]->Animate(m_fElapsedTime);
	//			if (m_vGameObjects[obj->u_id]) m_vGameObjects[obj->u_id]->Animate(m_fElapsedTime);
	//			if (m_vGameObjects[obj->u_id]->isRender) m_vGameObjects[obj->u_id]->Render(pd3dCommandList, pCamera);
	//		}
	//	}
	//}


	// Pass 상수 버퍼와 그림자 맵을 바인딩합니다.
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, passCB->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootDescriptorTable(5, m_pShadowMap->SrvGpuHandle()); // 루트 파라미터 5번 슬롯에 그림자맵

	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		for (auto& obj : m_vGameObjects) {
			if (obj) obj->Animate(m_fElapsedTime);
			if (obj->isRender) obj->Render(pd3dCommandList, pCamera);
		}
	}



	for (auto branch : m_listBranchObjects) {
		if (branch->isRender) {
			branch->Animate(m_fElapsedTime);
			branch->Render(pd3dCommandList, pCamera);
		}
	}

	for (auto branch : m_listRockObjects) {
		if (branch->isRender) {
			branch->Animate(m_fElapsedTime);
			branch->Render(pd3dCommandList, pCamera);
		}
	}

	//if(m_pPreviewPine->isRender)	m_pPreviewPine->Render(pd3dCommandList, pCamera);

	if (m_pPreviewPine->isRender)	m_pPreviewPine->Render(pd3dCommandList, pCamera);


	if (m_pPlayer) {
		if (m_pPlayer->invincibility) {
			auto endtime = std::chrono::system_clock::now();
			auto exectime = endtime - m_pPlayer->starttime;
			auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
			if (exec_ms > 1000.f) {
				m_pPlayer->SetInvincibility();
			}
		}
		m_pPlayer->Render(pd3dCommandList, pCamera);
	}
	for (auto& p : PlayerList) {
		if (p.second->m_pSkinnedAnimationController) p.second->Animate(m_fElapsedTime);
		if (p.second->isRender) p.second->Render(pd3dCommandList, pCamera);
	}


	if (obbRender) {
		CShader* pOBBShader = pShaderManager->GetShader("OBB", pd3dCommandList);
		if (pOBBShader) {
			SetGraphicsState(pd3dCommandList, pOBBShader);
			for (auto& obj : m_vGameObjects) {
				if (obj->ShouldRenderOBB()) {
					obj->RenderOBB(pd3dCommandList, pCamera);
				}
			}


			for (auto& branch : m_listBranchObjects) {
				if (branch->ShouldRenderOBB()) {
					branch->RenderOBB(pd3dCommandList, pCamera);
				}
			}
			for (auto& rock : m_listRockObjects) {
				if (rock->ShouldRenderOBB()) {
					rock->RenderOBB(pd3dCommandList, pCamera);
				}
			}

			if (m_pPreviewPine && m_pPreviewPine->ShouldRenderOBB()) {
				m_pPreviewPine->RenderOBB(pd3dCommandList, pCamera);
			}



			if (m_pPlayer && m_pPlayer->ShouldRenderOBB()) {
				m_pPlayer->RenderOBB(pd3dCommandList, pCamera);
			}


			//for (auto& entry : PlayerList) {
			//    CPlayer* pOtherPlayer = entry.second;
			//    if (pOtherPlayer && pOtherPlayer->ShouldRenderOBB()) {
			//        pOtherPlayer->RenderOBB(pd3dCommandList, pCamera);
			//    }
			//}
		}
		else {
			assert(!"OBB Shader (named 'OBB') not found in ShaderManager!");
		}
	}

}

void CScene::SetGraphicsState(ID3D12GraphicsCommandList* pd3dCommandList, CShader* pShader)
{
	if (!pShader || !pd3dCommandList) return;

	
	if (pShader != m_pCurrentShader)
	{
		m_pCurrentShader = pShader;

		
		ID3D12RootSignature* pRootSig = pShader->GetRootSignature();
		if (pRootSig && pRootSig != m_pCurrentRootSignature) {
			pd3dCommandList->SetGraphicsRootSignature(pRootSig);
			m_pCurrentRootSignature = pRootSig;
			
		}

		
		ID3D12PipelineState* pPSO = pShader->GetPipelineState();
		if (pPSO && pPSO != m_pCurrentPSO) {
			pd3dCommandList->SetPipelineState(pPSO);
			m_pCurrentPSO = pPSO;
		}
	}
	
}

ShaderManager* CScene::GetShaderManager() const {
	return m_pGameFramework ? m_pGameFramework->GetShaderManager() : nullptr;
}


bool CScene::CollisionCheck(CGameObject* a, CGameObject* b)
{
	if (!a || !b) {
		return false;
	}

	
	std::vector<DirectX::BoundingOrientedBox> obbListA;
	CollectHierarchyObjects(a, obbListA);

	
	std::vector<DirectX::BoundingOrientedBox> obbListB;
	CollectHierarchyObjects(b, obbListB);

	
	for (const auto& obbA : obbListA) { 
		for (const auto& obbB : obbListB) {
			if (obbA.Intersects(obbB)) {
				return true; 
			}
		}
	}

	
	return false;

}

void CScene::CollectHierarchyObjects(CGameObject* obj, std::vector<BoundingOrientedBox>& obbList) {
	if (!obj) {
		return; 
	}

	if(obj->m_pMesh)
		obbList.push_back(obj->m_worldOBB);

	
	CGameObject* currentChild = obj->m_pChild;
	while (currentChild) {
		CollectHierarchyObjects(currentChild, obbList); 
		currentChild = currentChild->m_pSibling;        
	}
}

#include "NonAtkState.h"
#include "AtkState.h"
void CScene::CheckPlayerInteraction(CPlayer* pPlayer) {
	if (!pPlayer) return;

	// Player <-> Object
	for (auto& obj : m_vGameObjects) {
		if (CollisionCheck(m_pPlayer, obj)) {
			if (!obj->isRender)   continue;
			
			if (obj->m_objectType == GameObjectType::Tree) {
				//obj->isRender = false;
				//m_pGameFramework->AddItem("wood", 3);
			}
			
			if (obj->m_objectType == GameObjectType::Rock) {
				//printf("[Rock ?겸뫖猷??類ㅼ뵥])\n");
				//obj->isRender = false;

				//int randValue = rand() % 100; // 0 ~ 99
				//if (randValue < 50) {
				//	m_pGameFramework->AddItem("stone",3);
				//}
				//else if (randValue < 75) {
				//	m_pGameFramework->AddItem("coal",1);
				//}
				//else {
				//	m_pGameFramework->AddItem("iron_material",1);
				//}
			}
			if (obj->m_objectType == GameObjectType::Cow || obj->m_objectType == GameObjectType::Pig) {
				auto npc = dynamic_cast<CMonsterObject*>(obj);
				if (npc->Gethp() <= 0) continue;
				if (npc->FSM_manager->GetInvincible()) continue;

				npc->Decreasehp(pPlayer->PlayerAttack);

				if (npc->Gethp() <= 0) {
					m_pGameFramework->AddItem("pork", 2);
					m_pPlayer->Playerxp += 10;
					if (m_pPlayer->Playerxp >= m_pPlayer->Totalxp) {
						m_pPlayer->PlayerLevel++;
						m_pPlayer->Playerxp = m_pPlayer->Playerxp - m_pPlayer->Totalxp;
						m_pPlayer->Totalxp *= 2;
						m_pPlayer->StatPoint += 5;
					}
				}
				if (obj->FSM_manager) {
					if (npc->Gethp() > 0) obj->FSM_manager->ChangeState(std::make_shared<NonAtkNPCRunAwayState>());
					else obj->FSM_manager->ChangeState(std::make_shared<NonAtkNPCDieState>());
					obj->FSM_manager->SetInvincible();
				}
			}

			if (obj->m_objectType != GameObjectType::Unknown && obj->m_objectType != GameObjectType::Cow && obj->m_objectType != GameObjectType::Pig &&
				obj->m_objectType != GameObjectType::Rock && obj->m_objectType != GameObjectType::Tree && obj->m_objectType != GameObjectType::Player) {
				auto npc = dynamic_cast<CMonsterObject*>(obj);
				if (npc->Gethp() <= 0) continue;
				if (npc->FSM_manager->GetInvincible()) continue;
				npc->Decreasehp(pPlayer->PlayerAttack);

				if (npc->Gethp() <= 0) {
					m_pPlayer->Playerxp += 20;
					if (m_pPlayer->Playerxp >= m_pPlayer->Totalxp) {
						m_pPlayer->PlayerLevel++;
						m_pPlayer->Playerxp = m_pPlayer->Playerxp - m_pPlayer->Totalxp;
						m_pPlayer->Totalxp *= 2;
						m_pPlayer->StatPoint += 5;
					}
				}
			}
		}
	}
}



void CScene::SpawnBranch(const XMFLOAT3& position, const XMFLOAT3& initialVelocity) {
	if (!m_pGameFramework || !m_pTerrain) return; 

	CBranchObject* newBranch = new CBranchObject(
		m_pGameFramework->GetDevice(),
		m_pGameFramework->GetCommandList(),
		m_pGameFramework,
		m_pTerrain
	);
	newBranch->SetPosition(position);
	newBranch->SetInitialVelocity(initialVelocity);
	
	newBranch->Rotate(0, (float)(rand() % 360), 0); 

	m_listBranchObjects.emplace_back(newBranch);
	//auto t_obj = std::make_unique<newBranch>(tree_obj_count++, gameObj->m_worldOBB.Center);
	//octree.insert(std::move(t_obj));
}

void CScene::SpawnRock(const XMFLOAT3& position, const XMFLOAT3& initialVelocity) {
	if (!m_pGameFramework || !m_pTerrain) return; 

	CRockDropObject* newBranch = new CRockDropObject(
		m_pGameFramework->GetDevice(),
		m_pGameFramework->GetCommandList(),
		m_pGameFramework,
		m_pTerrain
	);
	newBranch->SetPosition(position);
	newBranch->SetInitialVelocity(initialVelocity);
	
	newBranch->Rotate(0, (float)(rand() % 360), 0); 

	m_listRockObjects.emplace_back(newBranch);
	//auto t_obj = std::make_unique<newBranch>(tree_obj_count++, gameObj->m_worldOBB.Center);
	//octree.insert(std::move(t_obj));
}




// 그림자
void CScene::UpdateShadowTransform(const XMFLOAT3& focusPoint)
{
	// 주 directional light의 위치와 방향을 가져옵니다 (프로젝트에 맞게 수정)
	// 예시: 빛이 -Y 방향으로 내려온다고 가정
	LIGHT* pMainLight = &m_pLights[mMainLightIndex];
	XMVECTOR lightDir = XMLoadFloat3(&pMainLight->m_xmf3Direction);
	XMVECTOR lightPos = -2.0f * m_fSceneRadius * lightDir; // 씬의 크기에 맞춰 조절
	XMVECTOR targetPos = XMLoadFloat3(&focusPoint); // 씬의 중심 또는 플레이어 위치
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	// 빛의 시점에서 보이는 영역을 투영 행렬로 정의합니다.
	XMFLOAT3 sceneCenter = focusPoint;
	float x = sceneCenter.x, y = sceneCenter.y, z = sceneCenter.z;
	float sceneRadius = m_fSceneRadius; // 씬 전체를 감싸는 경계구의 반지름
	XMMATRIX proj = XMMatrixOrthographicLH(2.0f * sceneRadius, 2.0f * sceneRadius, 0.0f, 5.0f * sceneRadius);

	// 텍스처 좌표계로 변환하기 위한 행렬
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = view * proj * T;

	XMStoreFloat4x4(&mLightView, view);
	XMStoreFloat4x4(&mLightProj, proj);
	XMStoreFloat4x4(&mShadowTransform, S);
}


UINT CScene::GetAllObjectCount()
{
	//UINT objectCount = m_vGameObjects.size();
	return m_vGameObjects.size() + m_listBranchObjects.size() = m_listRockObjects.size();
}


XMMATRIX CScene::GetShadowTransform() const
{
	return XMLoadFloat4x4(&mShadowTransform);
}

// 모든 오브젝트를 순회하며 ObjectCB를 업데이트하는 함수
void CScene::UpdateObjectConstantBuffers(UploadBuffer<ObjectConstants>* pObjectCB)
{
	int i = 0;
	// GameObjects 업데이트
	for (auto& pObject : m_vGameObjects)
	{
		if (pObject)
		{
			ObjectConstants objConstants;
			// 월드 행렬 및 재질 정보를 채웁니다.
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(XMLoadFloat4x4(&pObject->m_xmf4x4World)));
			// ... pObject의 재질 정보를 objConstants에 복사 ...

			pObject->SetObjectConstantIndex(i); // 각 오브젝트가 CB에서 자신의 인덱스를 기억하게 함
			pObjectCB->CopyData(i++, objConstants);
		}
	}
	// Player, PlayerList, 기타 다른 오브젝트 리스트들도 동일한 방식으로 업데이트합니다.
	// ...
}