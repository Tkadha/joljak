//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "GameFramework.h"

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

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 300.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.3f, 0.8f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(230.0f, 330.0f, 480.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	m_pLights[1].m_bEnable = true;
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
	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
	m_pLights[3].m_bEnable = true;
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
	m_pLights[4].m_bEnable = true;
	m_pLights[4].m_nType = POINT_LIGHT;
	m_pLights[4].m_fRange = 200.0f;
	m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.3f, 0.3f, 1.0f);
	m_pLights[4].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[4].m_xmf3Position = XMFLOAT3(600.0f, 250.0f, 700.0f);
	m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	// ShaderManager 가져오기
	assert(m_pGameFramework != nullptr && "GameFramework pointer is needed!");
	ShaderManager* pShaderManager = m_pGameFramework->GetShaderManager();
	assert(pShaderManager != nullptr && "ShaderManager is not available!");
	ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager(); // 기존 코드 유지

	BuildDefaultLightsAndMaterials();

	if (!pResourceManager) {
		// 리소스 매니저가 없다면 로딩 불가! 오류 처리
		OutputDebugString(L"Error: ResourceManager is not available in CScene::BuildObjects.\n");
		return;
	}

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pGameFramework);

	XMFLOAT3 xmf3Scale(5.f, 0.2f, 5.f);
	XMFLOAT4 xmf4Color(0.0f, 0.0f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, _T("Terrain/terrain_16.raw"), 2049, 2049, xmf3Scale, xmf4Color, m_pGameFramework);
	
	// 랜덤 엔진
	std::random_device rd;
	std::mt19937 gen(rd());




	int nPineObjects = 10;
	for (int i = 0; i < nPineObjects; ++i) {
		CGameObject* gameObj = new CPineObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 2, 6, 2, 10);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}
	{
		CGameObject* gameObj = new CSwordObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z)+10, z);
		gameObj->SetScale(100,100,100);
		m_vGameObjects.emplace_back(gameObj);
	}
	int nRockClusterAObjects = 10;
	for (int i = 0; i < nRockClusterAObjects; ++i) {
		CGameObject* gameObj = new CRockClusterAObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 10, 20, 20, 30);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}
	
	int nRockClusterBObjects = 10;
	for (int i = 0; i < nRockClusterBObjects; ++i) {
		CGameObject* gameObj = new CRockClusterBObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 10, 20, 20, 30);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}
	int nRockClusterCObjects = 10;
	for (int i = 0; i < nRockClusterCObjects; ++i) {
		CGameObject* gameObj = new CRockClusterCObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 10, 20, 20, 30);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}

	int nCliffFObjectCObjects = 5;
	for (int i = 0; i < nRockClusterCObjects; ++i) {
		CGameObject* gameObj = new CCliffFObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen,5, 10, 5, 10);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}


	// ������Ʈ ����
	m_nHierarchicalGameObjects = 6;
	m_ppHierarchicalGameObjects = new CGameObject*[m_nHierarchicalGameObjects];

	CLoadedModelInfo* pCowModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SK_Cow.bin", m_pGameFramework);

	m_ppHierarchicalGameObjects[0] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[0]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[0]->SetPosition(1000.f, m_pTerrain->GetHeight(1000.0f, 1500.0f), 1500.f);
	m_ppHierarchicalGameObjects[0]->SetScale(8.0f, 8.0f, 8.0f);

	XMFLOAT3 cowCenter = XMFLOAT3(1000.0f, m_pTerrain->GetHeight(1000.0f, 1500.0f)+50, 1500.0f);
	XMFLOAT3 cowSize = XMFLOAT3(5.0f, 5.0f, 5.0f); // 실제 크기의 반
	XMFLOAT4 cowRotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	
	tree_objects.emplace_back(0, m_ppHierarchicalGameObjects[0]->m_worldOBB.Center);
	octree.insert(&tree_objects[0]);
	

	m_ppHierarchicalGameObjects[1] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[1]->SetScale(8.0f, 8.0f, 8.0f);
	m_ppHierarchicalGameObjects[1]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[1]->SetPosition(800.0f / 2, m_pTerrain->GetHeight(800.0f, 1400.0f) / 2, 1400.0f / 2);

	XMFLOAT3 cowCenter2 = XMFLOAT3(800.0f, m_pTerrain->GetHeight(800.0f, 1400.0f), 1400.0f);
	XMFLOAT3 cowSize2 = XMFLOAT3(5.0f, 5.0f, 5.0f); // 실제 크기의 반
	XMFLOAT4 cowRotation2 = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	m_ppHierarchicalGameObjects[1]->SetOBB(cowCenter2, cowSize2, cowRotation2);



	m_ppHierarchicalGameObjects[2] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	//m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController = nullptr;
	//m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->m_pAnimationTracks = nullptr;
	m_ppHierarchicalGameObjects[2]->SetScale(10.0f, 10.0f, 10.0f);
	m_ppHierarchicalGameObjects[2]->Rotate(0.f, 0.f, 0.f);
	m_ppHierarchicalGameObjects[2]->SetPosition(500.0f/2, m_pTerrain->GetHeight(500.0f, 800.0f)/2, 800.0f/2);

	m_ppHierarchicalGameObjects[3] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	m_ppHierarchicalGameObjects[3]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[3]->SetPosition(100.0f / 2, m_pTerrain->GetHeight(100.0f, 1400.0f) / 2, 1400.0f / 2);
	m_ppHierarchicalGameObjects[3]->SetScale(10.0f, 10.0f, 10.0f);

	m_ppHierarchicalGameObjects[4] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
	m_ppHierarchicalGameObjects[4]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[4]->SetPosition(200.0f /2 , m_pTerrain->GetHeight(200.0f, 500.0f)/2, 500.0f / 2);
	m_ppHierarchicalGameObjects[4]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[5] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[5]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[5]->SetPosition(1000.f / 2, m_pTerrain->GetHeight(1000.0f, 1500.0f) / 2, 1500.f / 2);
	m_ppHierarchicalGameObjects[5]->SetScale(8.0f, 8.0f, 8.0f);
	if (pCowModel) delete pCowModel;
	


	for (int i = 0; i < m_nHierarchicalGameObjects; ++i) {
		//m_ppHierarchicalGameObjects[0]->SetOBB(cowCenter, cowSize, cowRotation);
		m_ppHierarchicalGameObjects[i]->SetOBB();
		m_ppHierarchicalGameObjects[i]->InitializeOBBResources(pd3dDevice, pd3dCommandList);
	}

	for (auto obj : m_vGameObjects) {
		obj->SetOBB();
		obj->InitializeOBBResources(pd3dDevice, pd3dCommandList);
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;

	if (m_ppHierarchicalGameObjects)
	{
		for (int i = 0; i < m_nHierarchicalGameObjects; i++) if (m_ppHierarchicalGameObjects[i]) m_ppHierarchicalGameObjects[i]->Release();
		delete[] m_ppHierarchicalGameObjects;
	}

	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[10];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtEmissionTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtEmissionTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[15];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[14].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[14].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256�� ���
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);


	// 인스턴싱
	UINT m_nObjects = 100;
	//인스턴스 정보를 저장할 정점 버퍼를 업로드 힙 유형으로 생성한다. 
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL,
		sizeof(VS_VB_INSTANCE) * m_nObjects, D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	//정점 버퍼(업로드 힙)에 대한 포인터를 저장한다. 
	m_pd3dcbGameObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);
	//정점 버퍼에 대한 뷰를 생성한다. 
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
			m_nLights = 0; //임시
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

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) m_ppHierarchicalGameObjects[i]->ReleaseUploadBuffers();
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

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Animate(fTimeElapsed);
	
	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}

	//**/
	static float fAngle = 0.0f;
	fAngle += 1.50f;
	//	XMFLOAT3 xmf3Position = XMFLOAT3(50.0f, 0.0f, 0.0f);
	XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Rotate(0.0f, -fAngle, 0.0f);
	XMFLOAT3 xmf3Position = Vector3::TransformCoord(XMFLOAT3(65.0f, 0.0f, 0.0f), xmf4x4Rotate);
	//	m_ppHierarchicalGameObjects[11]->m_xmf4x4ToParent._41 = m_xmf3RotatePosition.x + xmf3Position.x;
	//	m_ppHierarchicalGameObjects[11]->m_xmf4x4ToParent._42 = m_xmf3RotatePosition.y + xmf3Position.y;
	//	m_ppHierarchicalGameObjects[11]->m_xmf4x4ToParent._43 = m_xmf3RotatePosition.z + xmf3Position.z;

		//m_ppHierarchicalGameObjects[11]->m_xmf4x4ToParent = Matrix4x4::AffineTransformation(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, -fAngle, 0.0f), Vector3::Add(m_xmf3RotatePosition, xmf3Position));
		//m_ppHierarchicalGameObjects[11]->Rotate(0.0f, -1.5f, 0.0f);

	//**/
	//AllocConsole(); // 콘솔 생성
	//freopen("CONOUT$", "w", stdout); // 표준 출력 리다이렉트
	//SetConsoleTitle(L"Debug Console"); // 콘솔 제목 (선택사항)

#ifndef test
#define test
	//m_pPlayer->PrintFrameInfo(m_pPlayer, NULL);

	//printf("Center{%f %f %f} ", m_pPlayer->m_worldOBB.Center.x, m_pPlayer->m_worldOBB.Center.y, m_pPlayer->m_worldOBB.Center.z);
	//printf("Extents{%f %f %f} ", m_pPlayer->m_worldOBB.Extents.x, m_pPlayer->m_worldOBB.Extents.y, m_pPlayer->m_worldOBB.Extents.z);
	//printf("Orientation{%f %f %f}\n", m_pPlayer->m_worldOBB.Orientation.x, m_pPlayer->m_worldOBB.Orientation.y, m_pPlayer->m_worldOBB.Orientation.z);
	//printf("Center{%f %f %f} ", m_ppHierarchicalGameObjects[0]->m_worldOBB.Center.x, m_ppHierarchicalGameObjects[0]->m_worldOBB.Center.y, m_ppHierarchicalGameObjects[0]->m_worldOBB.Center.z);
	//printf("Extents{%f %f %f} ", m_ppHierarchicalGameObjects[0]->m_worldOBB.Extents.x, m_ppHierarchicalGameObjects[0]->m_worldOBB.Extents.y, m_ppHierarchicalGameObjects[0]->m_worldOBB.Extents.z);
	//printf("Orientation{%f %f %f}\n", m_ppHierarchicalGameObjects[0]->m_worldOBB.Orientation.x, m_ppHierarchicalGameObjects[0]->m_worldOBB.Orientation.y, m_ppHierarchicalGameObjects[0]->m_worldOBB.Orientation.z);
	//printf("%f %f %f\n", m_ppHierarchicalGameObjects[0]->m_localOBB.Center.x, m_ppHierarchicalGameObjects[0]->m_localOBB.Center.y, m_ppHierarchicalGameObjects[0]->m_localOBB.Center.z);
	//printf("%f %f %f\n", m_ppHierarchicalGameObjects[0]->m_xmf4x4World._11, m_ppHierarchicalGameObjects[0]->m_xmf4x4World._12, m_ppHierarchicalGameObjects[0]->m_xmf4x4World._13);
	//printf("%f %f %f\n", m_ppHierarchicalGameObjects[0]->m_xmf4x4World._41, m_ppHierarchicalGameObjects[0]->m_xmf4x4World._42, m_ppHierarchicalGameObjects[0]->m_xmf4x4World._43);
	//printf("%f %f %f\n", m_ppHierarchicalGameObjects[0]->m_pChild->m_pChild->m_pChild->m_xmf4x4World._11, m_ppHierarchicalGameObjects[0]->m_pChild->m_pChild->m_pChild->m_xmf4x4World._12, m_ppHierarchicalGameObjects[0]->m_pChild->m_pChild->m_pChild->m_xmf4x4World._13);
#endif // !1

	if (m_pPlayer->CheckCollisionOBB(m_ppHierarchicalGameObjects[0])) {
		printf("[충돌 확인])\n");
		m_pPlayer->checkmove = true;

	}
	if (m_pPlayer->CheckCollisionOBB(m_ppHierarchicalGameObjects[1])) {

		m_ppHierarchicalGameObjects[1]->isRender = false;
	}

	std::vector<tree_obj*> results;

	tree_obj p(-1, m_pPlayer->GetPosition());
	octree.query(p, XMFLOAT3(200, 200, 200), results);
	for (auto& obj : results) {
		if (m_pPlayer->CheckCollisionOBB(m_ppHierarchicalGameObjects[obj->u_id])) {
			printf("contect!\n");
		}
	}
}


void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	// ShaderManager 가져오기 (매번 호출하는 대신 멤버 변수로 캐싱해도 좋음)
	assert(m_pGameFramework != nullptr && "GameFramework pointer is needed in CScene!");
	ShaderManager* pShaderManager = m_pGameFramework->GetShaderManager();
	assert(pShaderManager != nullptr && "ShaderManager is not available!");

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	// 카메라 상수 버퍼(b1) 업데이트
	pCamera->UpdateShaderVariables(pd3dCommandList);

	// 3. 전역 조명 상수 버퍼 업데이트 
	UpdateShaderVariables(pd3dCommandList); 

	// --- 중요: 디스크립터 힙 설정 ---
	ID3D12DescriptorHeap* ppHeaps[] = { m_pGameFramework->GetCbvSrvHeap() }; // CBV/SRV/UAV 힙 가져오기
	if (ppHeaps[0]) { // 힙 포인터 유효성 검사
		pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	}
	else {
		assert(!"CBV/SRV Descriptor Heap is NULL in CScene::Render!");
		return; // 힙 없으면 렌더링 불가
	}
	// 만약 샘플러 힙도 동적으로 사용한다면 함께 설정:
	// ID3D12DescriptorHeap* ppHeaps[] = { cbvSrvHeap, samplerHeap };
	// pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	// --- 힙 설정 끝 ---

	// --- 4. 렌더링 상태 추적 변수 ---
	ID3D12RootSignature* currentRootSig = nullptr;
	ID3D12PipelineState* currentPSO = nullptr;
	CShader* pCurrentShader = nullptr; // 현재 활성화된 셰이더 추적

	// --- 헬퍼 람다: 그래픽 상태 설정 (루트 서명 및 PSO) ---
	auto SetGraphicsState = [&](CShader* pShader) {
		if (!pShader) return; // 셰이더가 없으면 아무것도 안 함

		// 셰이더 객체 자체가 바뀌었는지 확인 (가장 포괄적인 검사)
		if (pShader != pCurrentShader)
		{
			pCurrentShader = pShader; // 현재 셰이더 업데이트

			// 루트 서명 설정 (셰이더에 저장된 루트 서명 사용)
			ID3D12RootSignature* pRootSig = pShader->GetRootSignature();
			if (pRootSig && pRootSig != currentRootSig) {
				pd3dCommandList->SetGraphicsRootSignature(pRootSig);
				currentRootSig = pRootSig;

				// --- 카메라/조명 등 공통 CBV 바인딩 위치 이동 ---
				// 현재 루트 서명이 카메라(b1 @ Param 0)를 사용하는지 확인 후 바인딩
				// (주의: OBB 루트 서명은 Param 0을 다른 용도로 사용!)
				// 더 정확한 방법은 각 루트 서명 생성 시 필요한 CBV 슬롯 정보를 저장해두는 것이지만,
				// 여기서는 셰이더 타입 등으로 임시 구분
				if (pShader != pShaderManager->GetShader("OBB", pd3dCommandList)) // OBB 셰이더가 아닐 때만 카메라 바인딩 시도
				{
					if (pCamera && pCamera->GetCameraConstantBuffer()) {
						pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress()); // 카메라 CBV (b1)를 파라미터 0번에 바인딩
					}
					if (m_pd3dcbLights) {
						// 조명 CBV (b4)를 파라미터 2번에 바인딩 (루트 서명 정의 기준)
						pd3dCommandList->SetGraphicsRootConstantBufferView(2, m_pd3dcbLights->GetGPUVirtualAddress());
					}
				}
				// --- 바인딩 로직 끝 ---
			}

			// PSO 설정 (셰이더에 저장된 PSO 사용)
			ID3D12PipelineState* pPSO = pShader->GetPipelineState();
			if (pPSO && pPSO != currentPSO) {
				pd3dCommandList->SetPipelineState(pPSO);
				currentPSO = pPSO;
			}
		}
		// 이미 같은 셰이더(같은 RS, 같은 PSO)라면 아무것도 변경 안 함
	};


	// --- 5. 렌더링 단계별 처리 ---

	// 5.1. 스카이박스 렌더링
	//if (m_pSkyBox) {
	//	// CSkyBox 객체가 자신의 CMaterial과 CShader를 가지고 있다고 가정
	//	CMaterial* pSkyboxMat = m_pSkyBox->GetMaterial(0); // 첫 번째 재질 가정
	//	if (pSkyboxMat && pSkyboxMat->m_pShader) {
	//		SetGraphicsState(pSkyboxMat->m_pShader); // Skybox 셰이더/RS/PSO 설정
	//		// CSkyBox::Render 내부에서는 필요한 CBV(카메라 b1), 텍스처(t13 테이블) 바인딩 및 Draw 호출
	//		m_pSkyBox->Render(pd3dCommandList, pCamera);
	//	}
	//}

	// 5.2. 지형 렌더링
	if (m_pTerrain) {
		CMaterial* pTerrainMat = m_pTerrain->GetMaterial(0);
		if (pTerrainMat && pTerrainMat->m_pShader) {
			SetGraphicsState(pTerrainMat->m_pShader); // Terrain 셰이더/RS/PSO 설정
			// CHeightMapTerrain::Render 내부에서는 필요한 CBV(카메라 b1, 지형객체 b2), 텍스처(t1, t2 테이블) 바인딩 및 Draw 호출
			m_pTerrain->Render(pd3dCommandList, pCamera);
		}
	}

	// 5.3. 일반 게임 오브젝트 렌더링 (m_ppGameObjects, m_vGameObjects)
	// 중요: 성능을 위해서는 이 객체들을 렌더링 전에 CShader 포인터 기준으로 정렬하는 것이 좋습니다!
	//      여기서는 간단하게 순차적으로 처리하며 상태를 변경합니다.
	for (int i = 0; i < m_nGameObjects; i++) { // m_ppGameObjects 처리 (예시)
		if (m_ppGameObjects[i] /*&& m_ppGameObjects[i]->IsVisible()*/) { // IsVisible() 같은 가시성 체크 추가 권장
			CMaterial* pMaterial = m_ppGameObjects[i]->GetMaterial(0); // 또는 주 재질 인덱스
			if (pMaterial && pMaterial->m_pShader) {
				SetGraphicsState(pMaterial->m_pShader); // 객체의 셰이더에 맞는 상태 설정
				// m_ppGameObjects[i]->Render 내부에서는 필요한 상수(b2), 텍스처(t6-t12 테이블) 바인딩 및 Draw 호출
				m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
			}
		}
	}
	for (auto& obj : m_vGameObjects) { // m_vGameObjects 처리
		if (obj /*&& obj->IsVisible()*/) {
			CMaterial* pMaterial = obj->GetMaterial(0);
			if (pMaterial && pMaterial->m_pShader) {
				SetGraphicsState(pMaterial->m_pShader);
				obj->Render(pd3dCommandList, pCamera);
			}
		}
	}

	// 5.4. 계층적 게임 오브젝트 렌더링 (Skinned)
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) {
		if (m_ppHierarchicalGameObjects[i] && m_ppHierarchicalGameObjects[i]->isRender == true) {
			// 애니메이션 처리
			m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
			if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);

			CMaterial* pMaterial = m_ppHierarchicalGameObjects[i]->GetMaterial(0);
			if (pMaterial && pMaterial->m_pShader) {
				SetGraphicsState(pMaterial->m_pShader); // Skinned 셰이더/RS/PSO 설정
				// m_ppHierarchicalGameObjects[i]->Render 내부에서는 필요한 상수(b2), 본(b7, b8), 텍스처(t6-t12 테이블) 바인딩 및 Draw 호출
				m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera);
			}
		}
	}

	if (m_pPlayer)
	{
		CMaterial* pPlayerMaterial = m_pPlayer->GetMaterial(0);
		if (pPlayerMaterial && pPlayerMaterial->m_pShader)
		{
			// !!! 플레이어 렌더링 직전에 상태 설정 !!!
			SetGraphicsState(pPlayerMaterial->m_pShader);
			// 이제 플레이어에 맞는 RS/PSO 및 공통 CBV가 설정됨
	
			// 플레이어 렌더링 호출
			m_pPlayer->Render(pd3dCommandList, pCamera);
		}
		else {
			OutputDebugString(L"Warning: Player has no material or shader set for rendering.\n");
		}
	}

	// 5.5. OBB 렌더링 (선택적)
	bool bRenderOBBs = true; // OBB 렌더링 여부 플래그 (예시)
	if (bRenderOBBs) {
		CShader* pOBBShader = pShaderManager->GetShader("OBB", pd3dCommandList);
		if (pOBBShader) {
			SetGraphicsState(pOBBShader); // OBB 셰이더/RS/PSO 설정

			// OBB 렌더링이 필요한 객체들 순회
			for (auto& obj : m_vGameObjects) {
				if (obj /*&& obj->ShouldRenderOBB()*/) { // OBB 렌더링 조건 확인
					// obj->RenderOBB 내부에서는 OBB의 월드 변환(b0) 업데이트 및 Draw 호출
					obj->RenderOBB(pd3dCommandList, pCamera); // RenderOBB 함수 구현 필요
				}
			}
			for (int i = 0; i < m_nHierarchicalGameObjects; ++i) {
				if (m_ppHierarchicalGameObjects[i] /*&& m_ppHierarchicalGameObjects[i]->ShouldRenderOBB()*/) {
					m_ppHierarchicalGameObjects[i]->RenderOBB(pd3dCommandList, pCamera);
				}
			}
			pOBBShader->Release(); // GetShader로 얻은 참조 해제
		}
	}


}
