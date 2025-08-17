//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "GameFramework.h"

#include "NonAtkState.h"
#include "AtkState.h"
#include "WaveObject.h"
#include "NetworkManager.h"
#include "SoundManager.h"
#define MIN_HEIGHT                  1055.f      

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
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); // 256�� ���
	m_pd3dcbLightCamera = ::CreateBufferResource(pFramework->GetDevice(), nullptr, nullptr, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);
	m_pd3dcbLightCamera->Map(0, nullptr, (void**)&m_pcbMappedLightCamera);

	// ��: ���� ������� ���� �ֺ���
	m_xmf4DaylightAmbient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_xmf4DaylightDiffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_xmf4DaylightSpecular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	// ��: ���� ��ο� Ǫ������ �ֺ��� (�޺�)
	m_xmf4MoonlightAmbient = XMFLOAT4(0.02f, 0.02f, 0.05f, 1.0f);
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 5;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	m_pLights[0].m_bEnable = false; // ������ ���� ���� ����
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 450.0f; // ȶ���� ���ߴ� ����
	//m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.05f, 0.0f, 1.0f);		// ���� ��Ȳ�� �ֺ���
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.1f, 0.05f, 1.0f);
	//m_pLights[0].m_xmf4Diffuse = XMFLOAT4(1.0f, 0.7f, 0.3f, 1.0f);		// ���� ��Ȳ/����� �Һ�
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(1.0f, 0.6f, 0.1f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.8f, 0.5f, 0.2f, 0.0f);		
	m_pLights[0].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.07f, 0.017f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	
	
	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.5f, -0.707f, 0.5f);
	m_pLights[2].m_xmf3Position = XMFLOAT3(0.0f, 3000.0f, 0.0f);


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
		//OutputDebugString(L"Error: ResourceManager is not available in CScene::BuildObjects.\n");
		return;
	}

	std::vector<std::wstring> skyboxTextures = {
	   L"Skybox/Morning.dds",
	   L"Skybox/Night.dds",
	   L"Skybox/eve.dds"
	};

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pGameFramework);
	m_pSkyBox->LoadTextures(pd3dCommandList, skyboxTextures);

	srand((unsigned int)time(NULL));

	XMFLOAT3 xmf3Scale(5.f, 0.1f, 5.f);
	XMFLOAT4 xmf4Color(0.0f, 0.0f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, _T("Terrain/terrain_16.raw"), 2049, 2049, xmf3Scale, xmf4Color, m_pGameFramework);
	m_pTerrain->m_xmf4x4World = Matrix4x4::Identity();
	m_pTerrain->m_xmf4x4ToParent = Matrix4x4::Identity();



	// 1. Waves ��ü�� �����մϴ�.
	m_pWavesObject = new CWavesObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
	// ������ ���� ��ġ�� �����մϴ�. (���� �߾� ��ó, ���� ����)
	m_pWavesObject->SetPosition(5000.0f, 1070.0f, 5000.0f);
	m_pWavesObject->SetScale(15.f, 1.f, 15.f);
	// 2. Waves�� ���� ����(Material)�� �����մϴ�.
	CMaterial* pWavesMaterial = new CMaterial(1, m_pGameFramework);

	// 3. ShaderManager���� "Waves" ���̴��� ������ ������ �����մϴ�.
	pWavesMaterial->SetShader(m_pGameFramework->GetShaderManager()->GetShader("Waves"));

	// (����) �� �ؽ�ó�� �ִٸ� ���⼭ �ε��Ͽ� ������ �Ҵ��� �� �ֽ��ϴ�.
	// pWavesMaterial->AssignTexture(...);

	// 4. ������ ������ Waves ��ü�� �����մϴ�.
	m_pWavesObject->SetMaterial(0, pWavesMaterial);




	// 1. �׸��� �� ��ü ����
	m_pShadowMap = std::make_unique<ShadowMap>(m_pGameFramework->GetDevice(), 4096 * 2, 4096 * 2);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle;
	m_pGameFramework->AllocateSrvDescriptors(1, cpuSrvHandle, gpuSrvHandle);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDsvHandle = m_pGameFramework->GetShadowDsvHeap()->GetCPUDescriptorHandleForHeapStart();

	m_pShadowMap->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuSrvHandle),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuSrvHandle),
		CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuDsvHandle)
	);


	// 1. ȶ�ҿ� �׸��� �� ��ü�� �����մϴ�.
	m_pTorchShadowMap = std::make_unique<ShadowMap>(m_pGameFramework->GetDevice(), 1024, 1024);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuSrvHandleForTorch; 
	D3D12_GPU_DESCRIPTOR_HANDLE gpuSrvHandleForTorch; 
	m_pGameFramework->AllocateSrvDescriptors(1, cpuSrvHandleForTorch, gpuSrvHandleForTorch); 

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDsvHandleForTorch = m_pGameFramework->GetShadowDsvHeap()->GetCPUDescriptorHandleForHeapStart(); 
	cpuDsvHandleForTorch.ptr += m_pGameFramework->GetDsvDescriptorIncrementSize();

	m_pTorchShadowMap->BuildDescriptors( 
		CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuSrvHandleForTorch), 
		CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuSrvHandleForTorch), 
		CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuDsvHandleForTorch) 
	); 

	std::random_device rd;
	std::mt19937 gen(rd());

	LoadPrefabs(pd3dDevice, pd3dCommandList);

	// 2. ���� �Լ��� ����Ͽ� ���� ������Ʈ���� ��ġ�մϴ�.
	const wchar_t* barkTexture = L"Model/Textures/Tree_Bark_Diffuse.dds";
	const wchar_t* rockTexture = L"Model/Textures/RockClusters_AlbedoRoughness.dds";

	// ���� : ��ü�̸�, ����, ������ġ min, max, ũ�� min, max, ~ , �ؽ����ε���, �ؽ��� 

	SpawnStaticObjects("BushA", 1000, 500, 9500, 7, 12, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("BushB", 1000, 500, 9500, 7, 12, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("Fern", 1000, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("Chervil", 1000, 500, 9500, 5, 8, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("RedPoppy", 1000, 500, 9500, 5, 8, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("BluePoppy", 1000, 500, 9500, 5, 8, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("Dandelion", 1000, 500, 9500, 5, 8, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("Daisy", 1000, 500, 9500, 5, 8, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("ElephantEarA", 1000, 500, 9500, 5, 10, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("ElephantEarB", 1000, 500, 9500, 5, 10, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("GrassTall", 1000, 500, 9500, 7, 12, gen, pd3dDevice, pd3dCommandList);
	//SpawnStaticObjects("Sphere", 1000, 500, 9500, 7, 12, gen, pd3dDevice, pd3dCommandList);

	/*
	pResourceManager->RegisterPrefab("BushA", std::make_shared<CBushAObject>(pd3dDevice, pd3dCommandList, m_pGameFramework));
	pResourceManager->RegisterPrefab("BushB", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/Bush_B_LOD0.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("Fern", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/Fern_LOD0.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("Chervil", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/ChervilCluster.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("RedPoppy", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/RedPoppyCluster.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("BluePoppy", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/BluePoppyCluster.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("Dandelion", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/DandelionCluster.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("Daisy", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/DaisyCluster.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("ElephantEarA", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/ElephantEar_A.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("ElephantEarB", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/ElephantEar_B.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("GrassTall", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/GrassTall.bin", m_pGameFramework));
	*/


	// Ż����ġ
	//SpawnStaticObjects("Anthena", 1, 8050, 8050, 10, 10, gen, pd3dDevice, pd3dCommandList);
	//SpawnStaticObjects("Helipad", 1, 8000, 8000, 10, 10, gen, pd3dDevice, pd3dCommandList);

	std::shared_ptr<CGameObject> prefab = pResourceManager->GetPrefab("Helipad");
	CGameObject* gameObj = prefab->Clone();
	float x = 8000, z = 8450;
	gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
	gameObj->SetScale(10.0f, 10.0f, 10.0f);
	gameObj->SetOBB(1.f, 1.f, 1.f, XMFLOAT3{ 0.f,0.f,0.f });
	gameObj->InitializeOBBResources(pd3dDevice, pd3dCommandList);
	gameObj->m_id = 0;
	heilport_anthena.emplace_back(gameObj);
	m_vGameObjects.emplace_back(gameObj);
	{
		NetworkManager& nw = NetworkManager::GetInstance();
		OBJ_OBB_PACKET p;
		auto& obb = gameObj->GetOBB();
		p.Center.x = obb.Center.x;
		p.Center.y = obb.Center.y;
		p.Center.z = obb.Center.z;
		p.Extents.x = obb.Extents.x;
		p.Extents.y = obb.Extents.y;
		p.Extents.z = obb.Extents.z;
		p.Orientation.x = obb.Orientation.x;
		p.Orientation.y = obb.Orientation.y;
		p.Orientation.z = obb.Orientation.z;
		p.Orientation.w = obb.Orientation.w;
		p.oid = 0;
		p.type = static_cast<char>(E_PACKET::E_O_SETOBB);
		p.size = sizeof(OBJ_OBB_PACKET);
		nw.PushSendQueue(p, p.size);
	}

	prefab = pResourceManager->GetPrefab("Anthena");
	z = 8500;
	gameObj = prefab->Clone();
	gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
	gameObj->SetScale(10.0f, 10.0f, 10.0f);
	gameObj->SetOBB(1.f, 1.f, 1.f, XMFLOAT3{ 0.f,0.f,0.f });
	gameObj->InitializeOBBResources(pd3dDevice, pd3dCommandList);
	gameObj->m_id = 1;
	heilport_anthena.emplace_back(gameObj);
	m_vGameObjects.emplace_back(gameObj);

	{
		NetworkManager& nw = NetworkManager::GetInstance();
		OBJ_OBB_PACKET p;
		auto& obb = gameObj->GetOBB();
		p.Center.x = obb.Center.x;
		p.Center.y = obb.Center.y;
		p.Center.z = obb.Center.z;
		p.Extents.x = obb.Extents.x;
		p.Extents.y = obb.Extents.y;
		p.Extents.z = obb.Extents.z;
		p.Orientation.x = obb.Orientation.x;
		p.Orientation.y = obb.Orientation.y;
		p.Orientation.z = obb.Orientation.z;
		p.Orientation.w = obb.Orientation.w;
		p.oid = 1;
		p.type = static_cast<char>(E_PACKET::E_O_SETOBB);
		p.size = sizeof(OBJ_OBB_PACKET);
		nw.PushSendQueue(p, p.size);
	}
	/////////////////////////////////////////����Ʈ ������Ʈ
	const int effectPoolSize = 100;
	for (int i = 0; i < effectPoolSize; ++i)
	{

		auto* pEffect = new CAttackEffectObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		int materialIndexToChange = 0;
		UINT albedoTextureSlot = 0;
		const wchar_t* textureFile = L"Model/Textures/RockClusters_AlbedoRoughness.dds";
		ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager();
		ChangeAlbedoTexture(pEffect, materialIndexToChange, albedoTextureSlot, textureFile, pResourceManager, pd3dCommandList, pd3dDevice);

		pEffect->m_id = -1;

		m_vAttackEffects.push_back(pEffect);


		m_vGameObjects.push_back(pEffect);
	}

	CLoadedModelInfo* pWoodShardModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/Branch_A.bin", m_pGameFramework);
	CLoadedModelInfo* pRockShardModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/RockCluster_B_LOD0.bin", m_pGameFramework);

	CMesh* pWoodMesh = pWoodShardModel->m_pModelRootObject->m_pMesh;
	CMaterial* pWoodMaterial = pWoodShardModel->m_pModelRootObject->GetMaterial(0);
	CMesh* pRockMesh = pRockShardModel->m_pModelRootObject->m_pMesh;
	CMaterial* pRockMaterial = pRockShardModel->m_pModelRootObject->GetMaterial(0);

	const int shardPoolSize = 50; // Ǯ ũ��

	// 2. ���� ���� Ǯ ����
	for (int i = 0; i < shardPoolSize; ++i) {
		auto* pShard = new CResourceShardEffect(pd3dDevice, pd3dCommandList, m_pGameFramework, pWoodMesh, pWoodMaterial);
		pShard->SetScale(1.0f, 1.0f, 1.0f);
		pShard->m_id = -1;
		m_vWoodShards.push_back(pShard);
		m_vGameObjects.push_back(pShard);
	}

	// 3. �� ���� Ǯ ����
	for (int i = 0; i < shardPoolSize; ++i) {
		auto* pShard = new CResourceShardEffect(pd3dDevice, pd3dCommandList, m_pGameFramework, pRockMesh, pRockMaterial);

		pShard->SetScale(0.2f, 0.2f, 0.2f);
		pShard->m_id = -1;
		m_vRockShards.push_back(pShard);
		m_vGameObjects.push_back(pShard);
	}

	// �ε尡 ���� �ӽ� �� ������ ����
	if (pWoodShardModel) delete pWoodShardModel;
	if (pRockShardModel) delete pRockShardModel;

	const int bloodPoolSize = 30;
	for (int i = 0; i < bloodPoolSize; ++i) {

		//ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager();
		//std::shared_ptr<CGameObject> prefab = pResourceManager->GetPrefab("Sphere");
		//CGameObject* pBlood = prefab->Clone();
		auto* pBlood = new CBloodEffectObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		m_vBloodEffects.push_back(pBlood);
		m_vGameObjects.push_back(pBlood); // �������� ���� ���� ��Ͽ��� �߰�
	}
	/////////////////////////////////////////
	const int vortexPoolSize = 50; // �ҿ뵹�̿� ����� ���� ��
	for (int i = 0; i < vortexPoolSize; ++i) {
		auto* pVortex = new CVortexEffectObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		//pVortex->SetScale(1.0f, 1.0f, 1.0f);
		m_vVortexEffects.push_back(pVortex);
		m_vGameObjects.push_back(pVortex);
	}

	// ������ ���๰ ��� (������ �̸��� �����ؾ� ��)
	std::vector<std::string> buildableItems = { "wood_wall","furnace" /*, "wood_floor", ... */ };

	for (const auto& itemName : buildableItems) {
		std::shared_ptr<CGameObject> prefab = pResourceManager->GetPrefab(itemName);
		if (prefab) {
			CGameObject* previewObject = prefab->Clone();
			previewObject->isRender = false; // ó������ ������ �ʵ��� ����
			previewObject->SetPosition(5000.0f, 2600.0f, 5000.0f);
			previewObject->SetScale(10.0f, 10.0f, 10.0f);
			previewObject->m_id = -1;
			m_mapBuildPrefabs[itemName] = previewObject; // �ʿ� �̸����� ����
			m_vGameObjects.emplace_back(previewObject);     // ���� ���� ��Ͽ��� �߰�
		}
	}

	

	for (auto obj : m_vGameObjects) {
		if (obj->m_objectType == GameObjectType::Tree) {
			obj->SetOBB(0.1f, 1.0f, 0.1f, XMFLOAT3(0.0f, 0.0f, 0.0f));
		}
		else if (obj->m_objectType == GameObjectType::Pig) {
			obj->SetOBB(1.0f, 0.8f, 1.0f, XMFLOAT3(0.0f, 1.0f, -1.0f));
		}
		else if (obj->m_id = -1) {
			obj->SetOBB(1.0f, 1.0f, 1.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
		}
		else {
			obj->SetOBB(1.0f, 1.0f, 1.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
		}
		
		obj->InitializeOBBResources(pd3dDevice, pd3dCommandList);
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
		
		//OutputDebugString(L"Error: ResourceManager is not available in CScene::BuildObjects.\n");
		return;
	}

	std::vector<std::wstring> skyboxTextures = {
	   L"Skybox/Morning.dds",
	   L"Skybox/nig.dds"
	};

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pGameFramework);
	m_pSkyBox->LoadTextures(pd3dCommandList, skyboxTextures);
	
	srand((unsigned int)time(NULL));

	XMFLOAT3 xmf3Scale(5.f, 0.2f, 5.f);
	XMFLOAT4 xmf4Color(0.0f, 0.0f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, _T("Terrain/terrain_16.raw"), 2049, 2049, xmf3Scale, xmf4Color, m_pGameFramework);
	m_pTerrain->m_xmf4x4World = Matrix4x4::Identity();
	m_pTerrain->m_xmf4x4ToParent = Matrix4x4::Identity();



	// 1. Waves ��ü�� �����մϴ�.
	m_pWavesObject = new CWavesObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
	// ������ ���� ��ġ�� �����մϴ�. (���� �߾� ��ó, ���� ����)
	m_pWavesObject->SetPosition(5000.0f, 2600.0f, 5000.0f);

	// 2. Waves�� ���� ����(Material)�� �����մϴ�.
	CMaterial* pWavesMaterial = new CMaterial(1, m_pGameFramework);

	// 3. ShaderManager���� "Waves" ���̴��� ������ ������ �����մϴ�.
	pWavesMaterial->SetShader(m_pGameFramework->GetShaderManager()->GetShader("Waves"));
	
	// (����) �� �ؽ�ó�� �ִٸ� ���⼭ �ε��Ͽ� ������ �Ҵ��� �� �ֽ��ϴ�.
	// pWavesMaterial->AssignTexture(...);

	// 4. ������ ������ Waves ��ü�� �����մϴ�.
	m_pWavesObject->SetMaterial(0, pWavesMaterial);
	

	/////////////////////////////////////////����Ʈ ������Ʈ
	const int effectPoolSize = 20; 
	for (int i = 0; i < effectPoolSize; ++i)
	{
		
		auto* pEffect = new CAttackEffectObject(pd3dDevice, pd3dCommandList, m_pGameFramework);

		
		m_vAttackEffects.push_back(pEffect);

		
		m_vGameObjects.push_back(pEffect);
	}

	const int bloodPoolSize = 30;
	for (int i = 0; i < bloodPoolSize; ++i) {
		auto* pBlood = new CBloodEffectObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		m_vBloodEffects.push_back(pBlood);
		m_vGameObjects.push_back(pBlood); // �������� ���� ���� ��Ͽ��� �߰�
	}


	CLoadedModelInfo* pWoodShardModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/Branch_A.bin", m_pGameFramework);
	CLoadedModelInfo* pRockShardModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/RockCluster_A_LOD0.bin", m_pGameFramework);

	CMesh* pWoodMesh = pWoodShardModel->m_pModelRootObject->m_pMesh;
	CMaterial* pWoodMaterial = pWoodShardModel->m_pModelRootObject->GetMaterial(0);
	CMesh* pRockMesh = pRockShardModel->m_pModelRootObject->m_pMesh;
	CMaterial* pRockMaterial = pRockShardModel->m_pModelRootObject->GetMaterial(0);

	const int shardPoolSize = 50; // Ǯ ũ��

	// 2. ���� ���� Ǯ ����
	for (int i = 0; i < shardPoolSize; ++i) {
		auto* pShard = new CResourceShardEffect(pd3dDevice, pd3dCommandList, m_pGameFramework, pWoodMesh, pWoodMaterial);

		pShard->SetScale(2.0f, 2.0f, 2.0f);
		m_vWoodShards.push_back(pShard);
		m_vGameObjects.push_back(pShard);
	}

	// 3. �� ���� Ǯ ����
	for (int i = 0; i < shardPoolSize; ++i) {
		auto* pShard = new CResourceShardEffect(pd3dDevice, pd3dCommandList, m_pGameFramework, pRockMesh, pRockMaterial);

		pShard->SetScale(0.01f, 0.01f, 0.01f);

		m_vRockShards.push_back(pShard);
		m_vGameObjects.push_back(pShard);
	}

	// �ε尡 ���� �ӽ� �� ������ ����
	if (pWoodShardModel) delete pWoodShardModel;
	if (pRockShardModel) delete pRockShardModel;

	/////////////////////////////////////////


	// 1. �׸��� �� ��ü ����
	m_pShadowMap = std::make_unique<ShadowMap>(m_pGameFramework->GetDevice(), 4096 * 2, 4096 * 2);

	// 2. SRV �ڵ� �Ҵ�: Framework�� AllocateSrvDescriptors �Լ��� ����մϴ�.
	D3D12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle;
	m_pGameFramework->AllocateSrvDescriptors(1, cpuSrvHandle, gpuSrvHandle);

	// 3. DSV �ڵ� ��������: ��� Framework�� ���� �׸��ڿ� DSV ���� ���� �ڵ��� �����ɴϴ�.
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDsvHandle = m_pGameFramework->GetShadowDsvHeap()->GetCPUDescriptorHandleForHeapStart();

	// 4. ShadowMap�� ��� �ڵ��� �����Ͽ� ���� ���ҽ� ����
	m_pShadowMap->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuSrvHandle),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuSrvHandle), 
		CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuDsvHandle) 
	);

	LoadPrefabs(pd3dDevice, pd3dCommandList);

	std::random_device rd;
	std::mt19937 gen(rd());

	// 2. ���� �Լ��� ����Ͽ� ���� ������Ʈ���� ��ġ�մϴ�.
	const wchar_t* barkTexture = L"Model/Textures/Tree_Bark_Diffuse.dds";
	const wchar_t* rockTexture = L"Model/Textures/RockClusters_AlbedoRoughness.dds";

	// ���� : ��ü�̸�, ����, ������ġ min, max, ũ�� min, max, ~ , �ؽ����ε���, �ؽ��� 
	SpawnStaticObjects("PineTree", 100, 500, 9500, 15, 20, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("BirchTree", 100, 500, 9500, 15, 20, gen, pd3dDevice, pd3dCommandList, 1, barkTexture);

	SpawnStaticObjects("RockClusterA", 100, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList, 0, rockTexture);
	SpawnStaticObjects("RockClusterB", 100, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList, 0, rockTexture);
	SpawnStaticObjects("RockClusterC", 100, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList, 0, rockTexture);

	SpawnStaticObjects("Cliff", 30, 1700, 9000, 30, 50, gen, pd3dDevice, pd3dCommandList, 0, rockTexture);

	SpawnStaticObjects("BushA", 100, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("Chervil", 20, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("RedPoppy", 20, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("ElephantEar", 20, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("GrassPatch", 20, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("Clovers", 20, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("Daisies", 20, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("Leaves", 20, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList);
	SpawnStaticObjects("GroundPoppies", 20, 500, 9500, 10, 15, gen, pd3dDevice, pd3dCommandList);

	

	// ������ ���๰ ��� (������ �̸��� �����ؾ� ��)
	std::vector<std::string> buildableItems = { "wood_wall" ,"furnace"/*, "wood_floor", ... */ };

	for (const auto& itemName : buildableItems) {
		std::shared_ptr<CGameObject> prefab = pResourceManager->GetPrefab(itemName);
		if (prefab) {
			CGameObject* previewObject = prefab->Clone();
			previewObject->isRender = false; // ó������ ������ �ʵ��� ����
			previewObject->SetPosition(5000.0f,2600.0f, 5000.0f);
			previewObject->SetScale(10.0f, 10.0f, 10.0f);

			m_mapBuildPrefabs[itemName] = previewObject; // �ʿ� �̸����� ����
			m_vGameObjects.emplace_back(previewObject);     // ���� ���� ��Ͽ��� �߰�
		}
	}

	int animate_count = 13;
	// Cow ��ġ
	//std::shared_ptr<CGameObject> cowPrefab = pResourceManager->GetPrefab("Cow");
	//if (cowPrefab) {
	//	CGameObject* prefabInstance = cowPrefab.get();
	//	prefabInstance->SetPosition(5000, 2700, 5000); // ���� �� ��� ���� ��ġ
	//	m_vGameObjects.emplace_back(prefabInstance);

	//	prefabInstance->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	//	for (int j = 1; j < animate_count; ++j) {
	//		prefabInstance->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
	//		prefabInstance->m_pSkinnedAnimationController->SetTrackEnable(j, false);
	//	}

	//	prefabInstance->SetOwningScene(this);
	//	prefabInstance->FSM_manager = std::make_shared<FSMManager<CGameObject>>(prefabInstance);
	//	prefabInstance->FSM_manager->SetCurrentState(std::make_shared<NonAtkNPCStandingState>());
	//	prefabInstance->FSM_manager->SetGlobalState(std::make_shared<NonAtkNPCGlobalState>());

	//	prefabInstance->SetScale(12.0f, 12.0f, 12.0f);

	//	auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, prefabInstance->m_worldOBB.Center);
	//	octree.insert(std::move(t_obj));

	//	for (int i = 0; i < 10; ++i) {
	//		CMonsterObject* newCow = dynamic_cast<CMonsterObject*>(cowPrefab->Clone());
	//		newCow->m_objectType = GameObjectType::Cow;

	//		newCow->PostCloneAnimationSetup();	// ���� �翬��

	//		newCow->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	//		for (int j = 1; j < animate_count; ++j) {
	//			newCow->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
	//			newCow->m_pSkinnedAnimationController->SetTrackEnable(j, false);
	//		}

	//		newCow->SetOwningScene(this);
	//		newCow->FSM_manager = std::make_shared<FSMManager<CGameObject>>(newCow);
	//		newCow->FSM_manager->SetCurrentState(std::make_shared<NonAtkNPCStandingState>());
	//		newCow->FSM_manager->SetGlobalState(std::make_shared<NonAtkNPCGlobalState>());
	//		
	//		auto [x, z] = genRandom::generateRandomXZ(gen, 5000, 5500, 5000, 5500);
	//		newCow->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
	//		newCow->SetScale(12.0f, 12.0f, 12.0f);
	//		m_vGameObjects.emplace_back(newCow);

	//		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, newCow->m_worldOBB.Center);
	//		octree.insert(std::move(t_obj));
	//	}
	//}

	int nCowObjects = 10;
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
		//gameobj->FSM_manager->SetCurrentState(std::make_shared<NonAtkNPCStandingState>());
		//gameobj->FSM_manager->SetGlobalState(std::make_shared<NonAtkNPCGlobalState>());

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
		//gameobj->FSM_manager->SetCurrentState(std::make_shared<NonAtkNPCStandingState>());
		//gameobj->FSM_manager->SetGlobalState(std::make_shared<NonAtkNPCGlobalState>());
		gameobj->Rotate(0.f, 180.f, 0.f);
		auto [x, z] = genRandom::generateRandomXZ(gen, 5000, 5500, 5000, 5500);
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
		//gameobj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		//gameobj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

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
		//gameobj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		//gameobj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

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
		//gameobj->FSM_manager->SetCurrentState(std::make_shared<AtkNPCStandingState>());
		//gameobj->FSM_manager->SetGlobalState(std::make_shared<AtkNPCGlobalState>());

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
	if (m_pWavesObject) delete m_pWavesObject;

	ReleaseShaderVariables();
	for (auto& pObject : m_vGameObjects)
	{
		// ������ �ν��Ͻ��� delete
		if (pObject && !pObject->m_bIsPrefab)
		{
			delete pObject;
		}
	}
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
			//OutputDebugStringA("!!!!!!!! ERROR: Invalid m_nLights value detected! Clamping to 0. !!!!!!!!\n");
			m_nLights = 0; 
		}
		::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
		::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
		::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
		::memcpy(&m_pcbMappedLights->gIsDaytime, &m_bIsDaytime, sizeof(bool));
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
	if (m_pWavesObject) m_pWavesObject->ReleaseUploadBuffers();
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
					m_pPlayer->checkmove = true; // 충돌 발생 ???�동 금�?
					break;
				}
			}
		}
	}
	*/

	bool bIsVortexActive = false;
	XMFLOAT3 vortexCenter = XMFLOAT3(0.f, 0.f, 0.f);

	// ���� Ȱ��ȭ�� �ҿ뵹�̰� �ִ���, �ִٸ� �߽����� ������� ã���ϴ�.
	for (const auto& pVortex : m_vVortexEffects)
	{
		if (pVortex && pVortex->isRender)
		{
			bIsVortexActive = true;
			vortexCenter = pVortex->GetPosition(); // Ȱ��ȭ�� ��ƼŬ �ƹ��ų� �ϳ��� �߽����� ����
			break;
		}
	}

	// �ҿ뵹�̰� Ȱ��ȭ �����̰� �÷��̾ �����Ѵٸ� ����� ����� �����մϴ�.
	if (bIsVortexActive && m_pPlayer)
	{
		// ����� ���� ���� (SpawnVortexEffect���� ������ ���� ū �� ����)
		const float fDamageRadius = 250.0f;
		const float fDamageHeight = 100.0f;
		const float fDamageInterval = 0.5f; // 0.5�ʸ��� �����
		const int nDamageAmount = 5;       // 1ȸ�� ����� ��

		XMFLOAT3 playerPos = m_pPlayer->GetPosition();

		// 1. ���� �Ÿ� ��� (Y�� ����)
		float fDistanceXZ = sqrt(pow(playerPos.x - vortexCenter.x, 2) + pow(playerPos.z - vortexCenter.z, 2));

		// 2. ���� �Ÿ� ���
		float fDistanceY = abs(playerPos.y - vortexCenter.y);

		// 3. �÷��̾ ����� ����(����) �ȿ� �ִ��� Ȯ��
		if (fDistanceXZ <= fDamageRadius && fDistanceY <= fDamageHeight)
		{
			m_fVortexDamageTimer += fTimeElapsed;
			if (m_fVortexDamageTimer >= fDamageInterval)
			{
				m_pPlayer->DecreaseHp(nDamageAmount);
				SoundManager::GetInstance().Play(L"Sound/Player/hit.wav");
				// ������ ü�� ����ȭ�� ���� ��Ŷ ����
				auto& nwManager = NetworkManager::GetInstance();
				SET_HP_HIT_OBJ_PACKET p;
				p.hit_obj_id = m_pPlayer->m_id; 
				p.hp = m_pPlayer->getHp();
				p.size = sizeof(SET_HP_HIT_OBJ_PACKET);
				p.type = static_cast<char>(E_PACKET::E_P_SETHP);
				nwManager.PushSendQueue(p, p.size);

				m_fVortexDamageTimer = 0.0f; // Ÿ�̸� �ʱ�ȭ
			}
		}
	}
	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}

	if (m_pPlayer && m_pLights)
	{
		XMFLOAT3 playerPosition = m_pPlayer->GetPosition();
		playerPosition.y += 50.0f;

		m_pLights[0].m_xmf3Position = playerPosition;
		m_pLights[0].m_xmf3Direction = m_pPlayer->GetLookVector();
	}

	if (m_pWavesObject) m_pWavesObject->Animate(fTimeElapsed);

	
}


void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{

	assert(m_pGameFramework != nullptr && "GameFramework pointer is needed in CScene!");
	ShaderManager* pShaderManager = m_pGameFramework->GetShaderManager();
	assert(pShaderManager != nullptr && "ShaderManager is not available!");


	//UpdateShadowTransform(m_pPlayer->GetPosition()); // ���� ��ġ/���⿡ ���� ShadowTransform ��� ���
	UpdateShadowTransform(m_pPlayer->GetPosition());
	pCamera->UpdateShadowTransform(mShadowTransform); // ī�޶� ShadowTransform �����Ͽ� ��� ���� ������Ʈ �غ�

	// =================================================================
	// Pass 1: �׸��� �� ����
	// =================================================================
	//if(IsDaytime())
	{
		// ���� Ÿ���� �׸��� ������ ����
		m_pShadowMap->SetRenderTarget(pd3dCommandList);

		ID3D12DescriptorHeap* ppHeaps[] = { m_pGameFramework->GetCbvSrvHeap() };
		pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);


		// --- �� ī�޶� ��� ���� ������Ʈ �� ���ε� ---
		XMMATRIX view = XMLoadFloat4x4(&mLightView);
		XMMATRIX proj = XMLoadFloat4x4(&mLightProj);

		XMStoreFloat4x4(&m_pcbMappedLightCamera->m_xmf4x4View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&m_pcbMappedLightCamera->m_xmf4x4Projection, XMMatrixTranspose(proj));


		//CShader* pShadowShader = m_pGameFramework->GetShaderManager()->GetShader("Skinned_Shaodw");
		//pd3dCommandList->SetPipelineState(pShadowShader->GetPipelineState());
		//pd3dCommandList->SetGraphicsRootSignature(pShadowShader->GetRootSignature());

		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			for (auto& obj : m_vGameObjects) {
				if (obj->m_objectType != GameObjectType::Toad && obj->m_objectType != GameObjectType::Wasp &&
					obj->m_objectType != GameObjectType::Wolf && obj->m_objectType != GameObjectType::Bat &&
					obj->m_objectType != GameObjectType::Snake && obj->m_objectType != GameObjectType::Turtle &&
					obj->m_objectType != GameObjectType::Raptor && obj->m_objectType != GameObjectType::Snail && 
					obj->m_objectType != GameObjectType::Spider)
				{
					if (obj->isRender) obj->RenderShadow(pd3dCommandList);
				}
			}
		}
		const float fRenderDistanceSq = 800.0f * 800.0f;
		XMVECTOR playerPos = XMLoadFloat3(&m_pPlayer->GetPosition());
		for (auto& obj : m_lEnvironmentObjects) {

			if (!obj) continue;
			XMVECTOR objPos = XMLoadFloat3(&obj->GetPosition());
			float fDistanceSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(playerPos, objPos)));

			if (fDistanceSq <= fRenderDistanceSq)
			{
				obj->RenderShadow(pd3dCommandList);
			}
		}

		for (auto& obj : m_vConstructionObjects) {
			if (obj) obj->RenderShadow(pd3dCommandList);
		}

		if (m_pPlayer) m_pPlayer->RenderShadow(pd3dCommandList);

		for (auto& p : PlayerList) {
			if (p.second->isRender) p.second->RenderShadow(pd3dCommandList);
		}

		if (m_pTerrain) m_pTerrain->RenderShadow(pd3dCommandList);

		// 1. �׸��� �� ���ҽ��� �ȼ� ���̴����� ���� �� �ִ� ���·� ����
		m_pShadowMap->TransitionToReadable(pd3dCommandList);
	}

	// =================================================================
   // Pass 1.5: ȶ�� �׸��� �� ����
   // =================================================================
	LIGHT* pTorchLight = &m_pLights[0]; 
	if (pTorchLight->m_bEnable)
	{
		// 1. ���� Ÿ���� "ȶ�ҿ�" ������ ������ �����մϴ�.
		m_pTorchShadowMap->SetRenderTarget(pd3dCommandList);

		// 2. "ȶ���� ����"���� �ٶ󺸴� View/Projection ����� ����մϴ�.
		UpdateTorchShadowTransform(pTorchLight);
		pCamera->UpdateTorchShadowTransform(mTorchShadowTransform);

		// 3. ���� ����� �� ī�޶� ��� ����(b0)�� ������Ʈ�մϴ�.
		XMMATRIX view = XMLoadFloat4x4(&mLightView);
		XMMATRIX proj = XMLoadFloat4x4(&mLightProj);
		XMStoreFloat4x4(&m_pcbMappedLightCamera->m_xmf4x4View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&m_pcbMappedLightCamera->m_xmf4x4Projection, XMMatrixTranspose(proj));

		//pd3dCommandList->SetGraphicsRootConstantBufferView(0, m_pd3dcbLightCamera->GetGPUVirtualAddress());

		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			for (auto& obj : m_vGameObjects) {
				if (obj->m_objectType != GameObjectType::Toad && obj->m_objectType != GameObjectType::Wasp &&
					obj->m_objectType != GameObjectType::Wolf && obj->m_objectType != GameObjectType::Bat &&
					obj->m_objectType != GameObjectType::Snake && obj->m_objectType != GameObjectType::Turtle &&
					obj->m_objectType != GameObjectType::Raptor && obj->m_objectType != GameObjectType::Snail &&
					obj->m_objectType != GameObjectType::Spider)
				{
					if (obj->isRender) obj->RenderShadow(pd3dCommandList);
				}
			}
		}
		const float fRenderDistanceSq = 800.0f * 800.0f;
		XMVECTOR playerPos = XMLoadFloat3(&m_pPlayer->GetPosition());
		for (auto& obj : m_lEnvironmentObjects) {

			if (!obj) continue;
			XMVECTOR objPos = XMLoadFloat3(&obj->GetPosition());
			float fDistanceSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(playerPos, objPos)));

			if (fDistanceSq <= fRenderDistanceSq)
			{
				obj->RenderShadow(pd3dCommandList);
			}
		}

		for (auto& obj : m_vConstructionObjects) {
			if (obj) obj->RenderShadow(pd3dCommandList);
		}

		if (m_pPlayer) m_pPlayer->RenderShadow(pd3dCommandList);

		for (auto& p : PlayerList) {
			if (p.second->isRender) p.second->RenderShadow(pd3dCommandList);
		}

		if (m_pTerrain) m_pTerrain->RenderShadow(pd3dCommandList);
		// 5. ȶ�� ������ ���� �б� ������ ���·� ��ȯ�մϴ�.
		m_pTorchShadowMap->TransitionToReadable(pd3dCommandList);
	}


	// 2. ���� Ÿ���� �ٽ� ȭ��(���� �����)�� ���� ���� ���۷� ����
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pGameFramework->GetCurrentRtvCPUDescriptorHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pGameFramework->GetDsvCPUDescriptorHandle();
	pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	// 3. ����Ʈ�� ���� ��Ʈ�� ���� ī�޶� �������� �ٽ� ����
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


	

	if (m_pSkyBox)
		m_pSkyBox->Render(pd3dCommandList, pCamera);

	


	if (m_pTerrain) {
		m_pTerrain->Render(pd3dCommandList, pCamera);
	}



	
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		for (auto& obj : m_vGameObjects) {
			if (obj->m_objectType != GameObjectType::Toad && obj->m_objectType != GameObjectType::Wasp &&
				obj->m_objectType != GameObjectType::Wolf && obj->m_objectType != GameObjectType::Bat &&
				obj->m_objectType != GameObjectType::Snake && obj->m_objectType != GameObjectType::Turtle &&
				obj->m_objectType != GameObjectType::Raptor && obj->m_objectType != GameObjectType::Snail &&
				obj->m_objectType != GameObjectType::Spider) {
				if (obj) obj->Animate(m_fElapsedTime);
				if (obj->isRender) obj->Render(pd3dCommandList, pCamera);
			}
			else {
				if (!IsDaytime()) {
					if (obj) obj->Animate(m_fElapsedTime);
					if (obj->isRender) obj->Render(pd3dCommandList, pCamera);
				}
			}
		}
		const float fRenderDistanceSq = 1000.0f * 1000.0f;
		XMVECTOR playerPos = XMLoadFloat3(&m_pPlayer->GetPosition());
		for (auto& obj : m_lEnvironmentObjects) {

			if (!obj) continue;
			XMVECTOR objPos = XMLoadFloat3(&obj->GetPosition());
			float fDistanceSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(playerPos, objPos)));

			if (fDistanceSq <= fRenderDistanceSq)
			{
				obj->Render(pd3dCommandList, pCamera);
			}
		}
	}
	
	for (auto& constructionObj : m_vConstructionObjects) {
		if (constructionObj) constructionObj->Animate(m_fElapsedTime);
		if (constructionObj && constructionObj->isRender) constructionObj->Render(pd3dCommandList, pCamera);
	}

	if (m_pWavesObject) m_pWavesObject->Render(pd3dCommandList, pCamera);

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

	//if (m_pPreviewPine && m_pPreviewPine->isRender) {
	//	m_pPreviewPine->Animate(m_fElapsedTime);
	//	m_pPreviewPine->Render(pd3dCommandList, pCamera);
	//}



	if (m_pPlayer) {
		if (m_pPlayer->invincibility) {
			auto endtime = std::chrono::system_clock::now();
			auto exectime = endtime - m_pPlayer->starttime;
			auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
			if (exec_ms > 1000.f) {
				m_pPlayer->SetInvincibility();
			}
		}
		m_pPlayer->pos_mu.lock();
		m_pPlayer->Render(pd3dCommandList, pCamera);
		m_pPlayer->pos_mu.unlock();
	}
	for (auto& p : PlayerList) {
		if (p.second->m_pSkinnedAnimationController) p.second->Animate(m_fElapsedTime);
		if (p.second->isRender) p.second->Render(pd3dCommandList, pCamera);
	}


	if (obbRender) {
		CShader* pOBBShader = pShaderManager->GetShader("OBB");
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

			
			for (auto& constructionObj : m_vConstructionObjects) {
				if (constructionObj->ShouldRenderOBB()) 
					constructionObj->RenderOBB(pd3dCommandList, pCamera);
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


	{
		// --- �׸��� �� ����� ��� ---
		CShader* pDebugShader = pShaderManager->GetShader("Debug");
		pd3dCommandList->SetPipelineState(pDebugShader->GetPipelineState());
		pd3dCommandList->SetGraphicsRootSignature(pDebugShader->GetRootSignature());

		// ����� ���̴��� 0�� ���Կ� �׸��� ���� SRV �ڵ��� ���ε�
		pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTorchShadowMap->Srv());


		// ����׿� �簢���� ����/�ε��� ���۸� �����ϰ� �׸��ϴ�.
		pd3dCommandList->IASetVertexBuffers(0, 1, &GetGameFramework()->m_d3dDebugQuadVBView);
		pd3dCommandList->IASetIndexBuffer(&GetGameFramework()->m_d3dDebugQuadIBView);
		pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pd3dCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
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
				//printf("[Rock ?�⑸�??뺤씤])\n");
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

void CScene::SpawnGolemPunchEffect(const XMFLOAT3& origin, const XMFLOAT3& direction)
{
	std::vector<CResourceShardEffect*>& shardPool = m_vRockShards;

	// �� ���� ������ ���� ����
	int numShardsToSpawn = 5 + (rand() % 4); // 5~8��

	for (int i = 0; i < numShardsToSpawn; ++i)
	{
		for (auto& pShard : shardPool)
		{
			if (!pShard->isRender)
			{
				// [����] 1. �� ���� ���͸� �����ɴϴ�.
				XMVECTOR vDirection = XMLoadFloat3(&direction);

				// [����] 2. ���� ���� ���� ���͸� ����� ��¦ ������ Ƶ�ϴ�.
				//    �� ������ �������� ������ �� ������ ���ߵ˴ϴ�.
				XMVECTOR vRandomOffset = XMVectorSet(
					((float)(rand() % 100) - 50.0f) * 0.005f, // X: -0.25 ~ +0.25
					((float)(rand() % 100) - 50.0f) * 0.005f, // Y: -0.25 ~ +0.25
					((float)(rand() % 100) - 50.0f) * 0.005f, // Z: -0.25 ~ +0.25
					0.0f
				);

				// 3. ���� ���⿡ ���� ���͸� ���ϰ� ����ȭ�Ͽ� ���� ������ �����մϴ�.
				vDirection = XMVector3Normalize(vDirection + vRandomOffset);

				// 4. ���� �߻� �ӵ��� ����մϴ�. (�ӵ��� �״�� ������ ����)
				float launchSpeed = 4000.0f + (rand() % 2000);
				XMFLOAT3 velocity;
				XMStoreFloat3(&velocity, vDirection * launchSpeed);

				pShard->Activate(origin, velocity);
				break;
			}
		}
	}
}

void CScene::SpawnBloodEffect(const XMFLOAT3& position)
{
	int numEffects = 8 + (rand() % 5); // 5~9��

	for (int i = 0; i < numEffects; ++i)
	{
		for (auto& pBlood : m_vBloodEffects) {
			if (!pBlood->isRender) {
				// [���� 1] �÷��̾��� ��ü ���̸� �������� �� ��ġ���� �����մϴ�.
				// �¿�(X,Z)�� �ʹ� ũ�� ����� �ʵ��� �������� ���Դϴ�.
				XMFLOAT3 spawnPos = position;
				spawnPos.y += 15.0f; // ���� ���� ����
				spawnPos.x += ((float)(rand() % 20) - 10.0f); // -10 ~ +10
				spawnPos.z += ((float)(rand() % 20) - 10.0f); // -10 ~ +10

				// [���� 2] ���θ� �ڱ�ġ�°� �ƴ϶� ������� ������������ �ʱ� �ӵ��� �����մϴ�.
				XMFLOAT3 velocity = XMFLOAT3(
					((float)(rand() % 200) - 100.0f), // X: -100 ~ +100
					((float)(rand() % 200) - 50.0f),  // Y: -50 ~ +150 (���� ���� Ƣ�� ����)
					((float)(rand() % 200) - 100.0f)  // Z: -100 ~ +100
				);

				pBlood->Activate(spawnPos, velocity);
				break;
			}
		}
	}
}

void CScene::SpawnVortexEffect(const XMFLOAT3& centerPosition)
{
	// --- ���� ���� ���� ---
	const int numLayers = 3; // �� ���� ����
	const int particlesPerLayer = 20; // ���� ��ƼŬ ����

	// �� ���� �Ӽ� (����, �ݰ�, �ӵ�)
	float layerHeights[] = { 20.0f, 60.0f, 100.0f };
	float layerRadii[] = { 100.0f, 100.0f, 100.0f };
	float layerSpeeds[] = { 90.0f, -110.0f, 130.0f }; // �ӵ��� ������ �ָ� �ݴ�� ȸ��

	int particlePoolIndex = 0; // ��ü ��ƼŬ Ǯ�� ��ȸ�ϱ� ���� �ε���

	// --- ������ ��ƼŬ ���� ---
	for (int i = 0; i < numLayers; ++i) // 3���� ���� ��ȸ
	{
		for (int j = 0; j < particlesPerLayer; ++j) // �� ������ 20���� ��ƼŬ ����
		{
			// ��� ������ ��ƼŬ�� Ǯ���� ã��
			if (particlePoolIndex >= m_vVortexEffects.size()) break; // Ǯ�� �����ϸ� �ߴ�

			CVortexEffectObject* pVortex = m_vVortexEffects[particlePoolIndex++];
			if (pVortex->isRender) continue; // �̹� ��� ���̸� �ǳʶٱ�

			// ��ƼŬ���� ���� ������ �ٸ��� �־� �������� ��ġ
			float startAngle = (360.0f / particlesPerLayer) * j;

			// ���� ��(i)�� ���������� Activate �Լ� ȣ��
			pVortex->Activate(
				centerPosition,
				layerRadii[i],   // i��° ���� �ݰ�
				layerHeights[i], // i��° ���� ����
				startAngle,
				layerSpeeds[i]   // i��° ���� �ӵ�
			);
		}
	}
}

void CScene::NewGameBuildObj()
{
	for (auto& obj : heilport_anthena)
	{
		m_vGameObjects.push_back(obj);
	}
	for (auto& obj : m_vAttackEffects)
	{
		m_vGameObjects.push_back(obj);
	}
	for (auto& obj : m_vWoodShards)
	{
		m_vGameObjects.push_back(obj);
	}
	for (auto& obj : m_vRockShards)
	{
		m_vGameObjects.push_back(obj);
	}
	for (auto& obj : m_mapBuildPrefabs)
	{
		m_vGameObjects.emplace_back(obj.second);
	}
}

void CScene::ClearObj()
{
	m_vGameObjects.clear();
	m_vConstructionObjects.clear();
	m_listGameObjects.clear();
}






void CScene::UpdateShadowTransform(const XMFLOAT3& focusPoint)
{
	LIGHT* pMainLight = nullptr;
	for (int i = 0; i < m_nLights; ++i) {
		if (m_pLights[i].m_nType == DIRECTIONAL_LIGHT) {
			pMainLight = &m_pLights[i];
			break;
		}
	}
	if (!pMainLight) return;

	XMVECTOR lightDir = XMLoadFloat3(&pMainLight->m_xmf3Direction);
	XMVECTOR lightPos = XMLoadFloat3(&focusPoint) - 2000.0f * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&focusPoint);
	XMVECTOR lightUp;

	float dot = fabsf(XMVectorGetY(lightDir));

	if (dot > 0.99f) {
		lightUp = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	}
	else {
		lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	}

	XMMATRIX view = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	float sceneRadius = 2500.0f;
	XMMATRIX proj = XMMatrixOrthographicLH(sceneRadius * 2.0f, sceneRadius * 2.0f, 0.0f, 5000.0f);

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

// ���� ����
//void CScene::UpdateShadowTransform()
//{
//
//	LIGHT* pMainLight = nullptr;
//	for (int i = 0; i < m_nLights; ++i) {
//		if (m_pLights[i].m_nType == DIRECTIONAL_LIGHT) {
//			pMainLight = &m_pLights[i];
//			break;
//		}
//	}
//	if (!pMainLight) return;
//
//	XMVECTOR lightDir = XMVectorSet(0.5f, -0.707f, 0.5f, 0.0f);
//	lightDir = XMVector3Normalize(lightDir);
//
//	XMVECTOR targetPos = XMVectorSet(5000.0f, 0.0f, 5000.0f, 0.0f);
//	XMVECTOR lightPos = targetPos - (lightDir * 10000.0f);
//	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//
//	XMMATRIX view = XMMatrixLookAtLH(lightPos, targetPos, lightUp);
//
//	float sceneSpan = 5000.0f; 
//	float sceneNear = 1.0f;
//	float sceneFar = 9000.0f;
//	XMMATRIX proj = XMMatrixOrthographicLH(sceneSpan, sceneSpan, sceneNear, sceneFar);
//
//	XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);
//	XMMATRIX S = view * proj * T;
//
//	XMStoreFloat4x4(&mLightView, view);
//	XMStoreFloat4x4(&mLightProj, proj);
//	XMStoreFloat4x4(&mShadowTransform, S);
//}


// ī�޶� �������� ���� ����
void CScene::UpdateShadowTransform()
{
	CCamera* m_pCamera = m_pPlayer->GetCamera();
	if (m_pCamera) return;

	LIGHT* pMainLight = nullptr;
	for (int i = 0; i < m_nLights; ++i) {
		if (m_pLights[i].m_nType == DIRECTIONAL_LIGHT) {
			pMainLight = &m_pLights[i];
			break;
		}
	}
	if (!pMainLight) return;

	XMVECTOR lightDir = XMVectorSet(0.5f, -0.707f, 0.5f, 0.0f);
	lightDir = XMVector3Normalize(lightDir);

	XMFLOAT3 frustumCorners[8];
	m_pCamera->GetFrustumCorners(frustumCorners);

	XMVECTOR frustumCenter = XMVectorZero();
	for (int i = 0; i < 8; ++i)
	{
		frustumCenter += XMLoadFloat3(&frustumCorners[i]);
	}
	frustumCenter /= 8.0f;

	float maxDistSq = 0.0f;
	for (int i = 0; i < 8; ++i)
	{
		XMVECTOR dist = XMLoadFloat3(&frustumCorners[i]) - frustumCenter;
		maxDistSq = XMVectorGetX(XMVectorMax(XMVector3LengthSq(dist), XMVectorSet(maxDistSq, maxDistSq, maxDistSq, maxDistSq)));
	}
	float sphereRadius = sqrtf(maxDistSq);

	XMVECTOR lightPos = frustumCenter - lightDir * sphereRadius;
	XMVECTOR targetPos = frustumCenter;
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	float viewWidth = sphereRadius * 2.0f;
	float viewHeight = sphereRadius * 2.0f;
	float viewNear = 0.0f;
	float viewFar = sphereRadius * 2.0f;
	XMMATRIX proj = XMMatrixOrthographicLH(viewWidth, viewHeight, viewNear, viewFar);

	XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);
	XMMATRIX S = view * proj * T;

	XMStoreFloat4x4(&mLightView, view);
	XMStoreFloat4x4(&mLightProj, proj);
	XMStoreFloat4x4(&mShadowTransform, S);
}

void CScene::UpdateTorchShadowTransform(LIGHT* pTorchLight)
{
	if (!pTorchLight || !m_pPlayer) return;

	XMFLOAT3 playerPos = m_pPlayer->GetPosition();

	XMVECTOR lightPos = XMVectorSet(playerPos.x, playerPos.y + 50.0f, playerPos.z, 1.0f);
	XMVECTOR targetPos = XMLoadFloat3(&playerPos);

	XMVECTOR lightUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // ������ Z��

	XMMATRIX view = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	float fovAngleY = 90.0f;
	float aspectRatio = 1.0f;
	float nearZ = 1.0f;
	float farZ = pTorchLight->m_fRange; // ���� �ִ� ����
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), aspectRatio, nearZ, farZ);

	// 3. ���� �׸��� ��ȯ ����� ����Ͽ� �����մϴ�.
	XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);
	XMMATRIX S = view * proj * T;

	// ���� ��ĵ��� �����մϴ�.
	XMStoreFloat4x4(&mLightView, view);
	XMStoreFloat4x4(&mLightProj, proj);
	XMStoreFloat4x4(&mTorchShadowTransform, S);
}


void CScene::UpdateLights(float fTimeElapsed)
{
	// 1. ���� ȸ�� ������ ������Ʈ�մϴ�.
	float rotationSpeed = 0.75f; // �ӵ��� �ణ ����
	m_fLightRotationAngle += fTimeElapsed * rotationSpeed;
	if (m_fLightRotationAngle > 360.0f) m_fLightRotationAngle -= 360.0f;

	// �� ���Ɽ�� ã���ϴ�.
	LIGHT* pMainLight = nullptr;
	for (int i = 0; i < m_nLights; ++i) {
		if (m_pLights[i].m_nType == DIRECTIONAL_LIGHT) {
			pMainLight = &m_pLights[i];
			break;
		}
	}
	if (!pMainLight) return;

	//���� ���� ������ ����մϴ�. (���ʿ��� ���� �������� ���� Z�� ȸ��)
	XMVECTOR xmvBaseLightDirection = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	XMMATRIX xmmtxLightRotate = XMMatrixRotationZ(XMConvertToRadians(m_fLightRotationAngle));
	XMVECTOR xmvCurrentLightDirection = XMVector3TransformNormal(xmvBaseLightDirection, xmmtxLightRotate);

	// ���� ���ο� ������ ���� ���� �����Ϳ� ������Ʈ�մϴ�.
	XMStoreFloat3(&pMainLight->m_xmf3Direction, xmvCurrentLightDirection);

	// ���� Y ������ �������� ���� ���� �Ǵ�
	if (XMVectorGetY(xmvCurrentLightDirection) < 0.0f) // ���� �Ʒ��� ���ϸ� ��
	{
		// ���籤�� �Ѱ�, �ֺ����� ���
		m_bIsDaytime = true;
		m_xmf4GlobalAmbient = m_xmf4DaylightAmbient;
		pMainLight->m_xmf4Diffuse = m_xmf4DaylightDiffuse;
		pMainLight->m_xmf4Specular = m_xmf4DaylightSpecular;
		if(GetSkyBox()->GetCurrentTextureIndex() != 0)
			GetSkyBox()->SetSkyboxIndex(0);
	}
	else // ���� ���� ���ϸ� ��
	{
		//���籤�� ����, �ֺ����� ��ο� �޺����� ��ü
		m_bIsDaytime = false;
		m_xmf4GlobalAmbient = m_xmf4MoonlightAmbient;
		pMainLight->m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); // �� ������ ������
		pMainLight->m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		if (GetSkyBox()->GetCurrentTextureIndex() != 1)
		GetSkyBox()->SetSkyboxIndex(1);
	}
}

void CScene::SetGraphicsState(ID3D12GraphicsCommandList* pd3dCommandList, CShader* pShader)
{
	if (!pShader || !pd3dCommandList) return;


	//if (pShader != m_pCurrentShader)
	//{
	m_pCurrentShader = pShader;


	ID3D12RootSignature* pRootSig = pShader->GetRootSignature();
	pd3dCommandList->SetGraphicsRootSignature(pRootSig);
	if (pRootSig && pRootSig != m_pCurrentRootSignature) {
		m_pCurrentRootSignature = pRootSig;

	}


	ID3D12PipelineState* pPSO = pShader->GetPipelineState();
	pd3dCommandList->SetPipelineState(pPSO);
	if (pPSO && pPSO != m_pCurrentPSO) {
		m_pCurrentPSO = pPSO;
	}
	//}

}


void CScene::LoadPrefabs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager();


	/*
	auto pballPrefab = std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Tool/Sphere.bin", m_pGameFramework);
	pballPrefab->m_bIsPrefab = true;

	int materialIndexToChange = 0;
	UINT albedoTextureSlot = 0;
	const wchar_t* textureFile2 = L"Model/Textures/red.dds";
	ChangeAlbedoTexture(pballPrefab.get(), materialIndexToChange, albedoTextureSlot, textureFile2, pResourceManager, pd3dCommandList, pd3dDevice);

	pResourceManager->RegisterPrefab("Sphere", pballPrefab);
	*/
	// ����
	pResourceManager->RegisterPrefab("PineTree", std::make_shared<CPineObject>(pd3dDevice, pd3dCommandList, m_pGameFramework));

	auto pBirchPrefab = std::make_shared<CBirchObject>(pd3dDevice, pd3dCommandList, m_pGameFramework);
	pBirchPrefab->m_bIsPrefab = true;

	int materialIndexToChange = 1;
	UINT albedoTextureSlot = 0;
	const wchar_t*  textureFile = L"Model/Textures/Tree_Bark_Diffuse.dds";
	ChangeAlbedoTexture(pBirchPrefab.get(), materialIndexToChange, albedoTextureSlot, textureFile, pResourceManager, pd3dCommandList, pd3dDevice);

	pResourceManager->RegisterPrefab("BirchTree", pBirchPrefab);

	// ����
	pResourceManager->RegisterPrefab("RockClusterA", std::make_shared<CRockClusterAObject>(pd3dDevice, pd3dCommandList, m_pGameFramework));
	pResourceManager->RegisterPrefab("RockClusterB", std::make_shared<CRockClusterBObject>(pd3dDevice, pd3dCommandList, m_pGameFramework));
	pResourceManager->RegisterPrefab("RockClusterC", std::make_shared<CRockClusterCObject>(pd3dDevice, pd3dCommandList, m_pGameFramework)); // "RockClusterB" -> "RockClusterC"

	// ����
	pResourceManager->RegisterPrefab("Cliff", std::make_shared<CCliffFObject>(pd3dDevice, pd3dCommandList, m_pGameFramework));

	// ��Ǯ �� �Ĺ�
	pResourceManager->RegisterPrefab("BushA", std::make_shared<CBushAObject>(pd3dDevice, pd3dCommandList, m_pGameFramework));
	pResourceManager->RegisterPrefab("BushB", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/Bush_B_LOD0.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("Fern", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/Fern_LOD0.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("Chervil", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/ChervilCluster.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("RedPoppy", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/RedPoppyCluster.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("BluePoppy", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/BluePoppyCluster.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("Dandelion", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/DandelionCluster.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("Daisy", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/DaisyCluster.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("ElephantEarA", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/ElephantEar_A.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("ElephantEarB", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/ElephantEar_B.bin", m_pGameFramework));
	pResourceManager->RegisterPrefab("GrassTall", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Vegetation/GrassTall.bin", m_pGameFramework));

	// Ż����ġ?
	pResourceManager->RegisterPrefab("Helipad", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Anthena/Props_Roof_Helipad.bin", m_pGameFramework));
	auto pAntennaPrefab = std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/Anthena/Props_Roof_Antenna.bin", m_pGameFramework);
	pAntennaPrefab->m_objectType = GameObjectType::Antenna; // Ÿ�� ����
	pResourceManager->RegisterPrefab("Anthena", pAntennaPrefab);
	// ����
	CLoadedModelInfo* pLoadedModel = nullptr;

	pLoadedModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Cow.bin", m_pGameFramework);
	auto pCowPrefab = std::make_shared<CMonsterObject>(pd3dDevice, pd3dCommandList, pLoadedModel, 13, m_pGameFramework);

	pCowPrefab->m_bIsPrefab = true;

	pResourceManager->RegisterPrefab("Cow", pCowPrefab);
	if (pLoadedModel) delete pLoadedModel;

	pLoadedModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Pig.bin", m_pGameFramework);
	pResourceManager->RegisterPrefab("Pig", std::make_shared<CMonsterObject>(pd3dDevice, pd3dCommandList, pLoadedModel, 13, m_pGameFramework));
	if (pLoadedModel) delete pLoadedModel;

	pLoadedModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Spider.bin", m_pGameFramework);
	pResourceManager->RegisterPrefab("Spider", std::make_shared<CMonsterObject>(pd3dDevice, pd3dCommandList, pLoadedModel, 13, m_pGameFramework));
	if (pLoadedModel) delete pLoadedModel;

	pLoadedModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Toad.bin", m_pGameFramework);
	pResourceManager->RegisterPrefab("Toad", std::make_shared<CMonsterObject>(pd3dDevice, pd3dCommandList, pLoadedModel, 13, m_pGameFramework));
	if (pLoadedModel) delete pLoadedModel;

	pLoadedModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Wolf.bin", m_pGameFramework);
	pResourceManager->RegisterPrefab("Wolf", std::make_shared<CMonsterObject>(pd3dDevice, pd3dCommandList, pLoadedModel, 13, m_pGameFramework));
	if (pLoadedModel) delete pLoadedModel;

	//����
	pResourceManager->RegisterPrefab("wood_wall", std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/buildobject/Fence_WoodC_A.bin", m_pGameFramework));
	auto pFurnacePrefab = std::make_shared<CStaticObject>(pd3dDevice, pd3dCommandList, "Model/buildobject/furnace.bin", m_pGameFramework);
	pFurnacePrefab->m_objectType = GameObjectType::Furnace; // Ÿ�� ����
	pResourceManager->RegisterPrefab("furnace", pFurnacePrefab);


}


void CScene::SpawnStaticObjects(const std::string& prefabName, int count, float spawnMin, float spawnMax, float scaleMin, float scaleMax, std::mt19937& gen, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int matIdx, const wchar_t* texFile)
{
	ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager();
	std::shared_ptr<CGameObject> prefab = pResourceManager->GetPrefab(prefabName);
	if (!prefab) return;

	for (int i = 0; i < count; ++i)
	{
		CGameObject* gameObj = prefab->Clone();
		std::pair<float, float> randompos = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		while (m_pTerrain->GetHeight(randompos.first, randompos.second) < MIN_HEIGHT) {
			randompos = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		}

		gameObj->SetPosition(randompos.first, m_pTerrain->GetHeight(randompos.first, randompos.second), randompos.second);

		auto [w, h] = genRandom::generateRandomXZ(gen, scaleMin, scaleMax, scaleMin, scaleMax);
		gameObj->SetScale(w, h, w);

		if (matIdx != -1 && texFile != nullptr) {
			ChangeAlbedoTexture(gameObj, matIdx, 0, texFile, pResourceManager, pd3dCommandList, pd3dDevice);
		}
		gameObj->m_id = -1;
		gameObj->m_treecount = tree_obj_count;
		m_lEnvironmentObjects.emplace_back(gameObj);
	}
}

void CScene::SpawnAttackEffect(const XMFLOAT3& centerPosition, int numEffects, float radius)
{
	
	// [�߰�] ��Ȱ���� ���� �ε��� ����
	static int nEffectPoolIndex = 0;

	float angleStep = 360.0f / numEffects;

	for (int i = 0; i < numEffects; ++i)
	{
		float angle = XMConvertToRadians(i * angleStep);
		XMFLOAT3 offset = XMFLOAT3(cos(angle) * radius, 0, sin(angle) * radius);
		XMFLOAT3 spawnPos = Vector3::Add(centerPosition, offset);

		if (m_pTerrain) {
			spawnPos.y = m_pTerrain->GetHeight(spawnPos.x, spawnPos.z) + 5.0f;
		}

		CAttackEffectObject* pEffectToUse = nullptr;

		
		for (auto& pEffect : m_vAttackEffects)
		{
			if (!pEffect->isRender)
			{
				pEffectToUse = pEffect;
				break;
			}
		}

		
		if (!pEffectToUse)
		{
			
			pEffectToUse = m_vAttackEffects[nEffectPoolIndex];
			nEffectPoolIndex = (nEffectPoolIndex + 1) % m_vAttackEffects.size();
		}

		
		if (pEffectToUse)
		{
			pEffectToUse->Activate(spawnPos);
		}
	}
}

void CScene::SpawnResourceShards(const XMFLOAT3& origin, ShardType type)
{
	
	std::vector<CResourceShardEffect*>& shardPool = (type == ShardType::Wood) ? m_vWoodShards : m_vRockShards;

	int numShardsToSpawn = 8 + (rand() % 5); //8~12

	for (int i = 0; i < numShardsToSpawn; ++i)
	{
		
		for (auto& pShard : shardPool)
		{
			if (!pShard->isRender)
			{
				
				XMFLOAT3 velocity = XMFLOAT3(
					((float)(rand() % 10) - 5.0f), // X: -2000 ~ +2000
					((float)(rand() % 4) + 2.0f), // Y: +1500 ~ +3500 (���� �ſ� ���ϰ�)
					((float)(rand() % 10) - 5.0f)  // Z: -2000 ~ +2000
				);
				pShard->Activate(origin, velocity);
				break;
			}
		}
	}
}