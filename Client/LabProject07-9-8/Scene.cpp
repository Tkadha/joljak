//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

ID3D12DescriptorHeap *CScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;

CScene::CScene()
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
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 76); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature); 

	BuildDefaultLightsAndMaterials();

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(5.f, 0.2f, 5.f);
	XMFLOAT4 xmf4Color(0.0f, 0.0f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/terrain_16.raw"), 2049, 2049, xmf3Scale, xmf4Color);
	
	// 랜덤 엔진
	std::random_device rd;
	std::mt19937 gen(rd());

	int nPineObjects = 100;
	for (int i = 0; i < nPineObjects; ++i) {
		CGameObject* gameObj = new CPineObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 2, 6, 2, 10);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}
	int nRockClusterAObjects = 20;
	for (int i = 0; i < nRockClusterAObjects; ++i) {
		CGameObject* gameObj = new CRockClusterAObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 10, 20, 20, 30);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}
	int nRockClusterBObjects = 20;
	for (int i = 0; i < nRockClusterBObjects; ++i) {
		CGameObject* gameObj = new CRockClusterBObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 10, 20, 20, 30);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}
	int nRockClusterCObjects = 30;
	for (int i = 0; i < nRockClusterCObjects; ++i) {
		CGameObject* gameObj = new CRockClusterCObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 10, 20, 20, 30);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}

	int nCliffFObjectCObjects = 5;
	for (int i = 0; i < nRockClusterCObjects; ++i) {
		CGameObject* gameObj = new CCliffFObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen,5, 10, 5, 10);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}


	// ������Ʈ ����
	m_nHierarchicalGameObjects = 6;
	m_ppHierarchicalGameObjects = new CGameObject*[m_nHierarchicalGameObjects];

	CLoadedModelInfo* pCowModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SK_Cow.bin", NULL);

	m_ppHierarchicalGameObjects[0] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1);
	m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[0]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[0]->SetPosition(1000.f, m_pTerrain->GetHeight(1000.0f, 1500.0f), 1500.f);
	m_ppHierarchicalGameObjects[0]->SetScale(8.0f, 8.0f, 8.0f);

	XMFLOAT3 cowCenter = XMFLOAT3(1000.0f, m_pTerrain->GetHeight(1000.0f, 1500.0f)+50, 1500.0f);
	XMFLOAT3 cowSize = XMFLOAT3(5.0f, 5.0f, 5.0f); // 실제 크기의 반
	XMFLOAT4 cowRotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	//m_ppHierarchicalGameObjects[0]->SetOBB(cowCenter, cowSize, cowRotation);
	m_ppHierarchicalGameObjects[0]->SetOBB();
	CShader* shader = new COBBShader();
	//m_ppHierarchicalGameObjects[0]->m_OBBShader.CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_ppHierarchicalGameObjects[0]->m_OBBMaterial->SetShader(shader);
	m_ppHierarchicalGameObjects[0]->InitializeOBBResources(pd3dDevice, pd3dCommandList);


	/*
	AllocConsole(); // 콘솔 생성
	freopen("CONOUT$", "w", stdout); // 표준 출력 리다이렉트
	SetConsoleTitle(L"Debug Console"); // 콘솔 제목 (선택사항)
	printf("[OBB 확인] Center = (%.2f, %.2f, %.2f)\n",
		m_ppHierarchicalGameObjects[0]->m_OBB.Center.x, m_ppHierarchicalGameObjects[0]->m_OBB.Center.y, m_ppHierarchicalGameObjects[0]->m_OBB.Center.z);
		*/
	//////////////////////////////////////////////////////////////////////////////////





	m_ppHierarchicalGameObjects[1] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1);
	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[1]->SetScale(1, 1, 1);
	m_ppHierarchicalGameObjects[1]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[1]->SetPosition(800.0f / 2, m_pTerrain->GetHeight(800.0f, 1400.0f) / 2, 1400.0f / 2);

	m_ppHierarchicalGameObjects[2] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1);
	m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	//m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController = nullptr;
	//m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->m_pAnimationTracks = nullptr;
	m_ppHierarchicalGameObjects[2]->SetScale(10.0f, 10.0f, 10.0f);
	m_ppHierarchicalGameObjects[2]->Rotate(0.f, 0.f, 0.f);
	m_ppHierarchicalGameObjects[2]->SetPosition(830.0f/2, m_pTerrain->GetHeight(830.0f, 1400.0f)/2, 1400.0f/2);

	m_ppHierarchicalGameObjects[3] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1);
	m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	m_ppHierarchicalGameObjects[3]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[3]->SetPosition(860.0f / 2, m_pTerrain->GetHeight(860.0f, 1400.0f) / 2, 1400.0f / 2);
	m_ppHierarchicalGameObjects[3]->SetScale(10.0f, 10.0f, 10.0f);

	m_ppHierarchicalGameObjects[4] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1);
	m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
	m_ppHierarchicalGameObjects[4]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[4]->SetPosition(890.0f /2 , m_pTerrain->GetHeight(890.0f, 1400.0f)/2, 1400.0f / 2);
	m_ppHierarchicalGameObjects[4]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[5] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pCowModel, 1);
	m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 4);
	m_ppHierarchicalGameObjects[5]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[5]->SetPosition(890.0f / 2, m_pTerrain->GetHeight(890.0f, 1400.0f) - 650, 1400.0f / 2);
	m_ppHierarchicalGameObjects[5]->SetScale(8.0f, 8.0f, 8.0f);
	if (pCowModel) delete pCowModel;

	for (int i = 1; i < m_nHierarchicalGameObjects; ++i) {
		//m_ppHierarchicalGameObjects[0]->SetOBB(cowCenter, cowSize, cowRotation);
		m_ppHierarchicalGameObjects[i]->SetOBB();
		CShader* shader = new COBBShader();
		//m_ppHierarchicalGameObjects[0]->m_OBBShader.CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		m_ppHierarchicalGameObjects[i]->m_OBBMaterial->SetShader(shader);
		m_ppHierarchicalGameObjects[i]->InitializeOBBResources(pd3dDevice, pd3dCommandList);
	}

	/*CLoadedModelInfo* pSpiderModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SK_Spider.bin", NULL);
	m_ppHierarchicalGameObjects[6] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSpiderModel, 1);
	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[6]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[6]->SetPosition(430.0f, m_pTerrain->GetHeight(430.0f, 700.0f) - 10, 680.0f);
	m_ppHierarchicalGameObjects[6]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[7] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSpiderModel, 1);
	m_ppHierarchicalGameObjects[7]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	m_ppHierarchicalGameObjects[7]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[7]->SetPosition(400.0f, m_pTerrain->GetHeight(400.0f, 720.0f) - 10, 680.0f);
	m_ppHierarchicalGameObjects[7]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[8] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSpiderModel, 1);
	m_ppHierarchicalGameObjects[8]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	m_ppHierarchicalGameObjects[8]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[8]->SetPosition(370.0f, m_pTerrain->GetHeight(380.0f, 750.0f) - 10, 680.0f);
	m_ppHierarchicalGameObjects[8]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[9] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSpiderModel, 1);
	m_ppHierarchicalGameObjects[9]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
	m_ppHierarchicalGameObjects[9]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[9]->SetPosition(340.0f, m_pTerrain->GetHeight(380.0f, 750.0f) - 10, 680.0f);
	m_ppHierarchicalGameObjects[9]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[10] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSpiderModel, 1);
	m_ppHierarchicalGameObjects[10]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 4);
	m_ppHierarchicalGameObjects[10]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[10]->SetPosition(310.0f, m_pTerrain->GetHeight(380.0f, 750.0f) - 10, 680.0f);
	m_ppHierarchicalGameObjects[10]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[11] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSpiderModel, 1);
	m_ppHierarchicalGameObjects[11]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 5);
	m_ppHierarchicalGameObjects[11]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[11]->SetPosition(280.0f, m_pTerrain->GetHeight(380.0f, 750.0f) - 10, 680.0f);
	m_ppHierarchicalGameObjects[11]->SetScale(8.0f, 8.0f, 8.0f);
	if (pSpiderModel) delete pSpiderModel;

	CLoadedModelInfo *pSnakeModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SK_Snake.bin", NULL);
	m_ppHierarchicalGameObjects[12] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSnakeModel, 1);
	m_ppHierarchicalGameObjects[12]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[12]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[12]->SetPosition(430.0f, m_pTerrain->GetHeight(430.0f, 700.0f) + 10, 720.0f);
	m_ppHierarchicalGameObjects[12]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[13] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSnakeModel, 1);
	m_ppHierarchicalGameObjects[13]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	m_ppHierarchicalGameObjects[13]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[13]->SetPosition(400.0f, m_pTerrain->GetHeight(400.0f, 720.0f) + 10, 720.0f);
	m_ppHierarchicalGameObjects[13]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[14] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSnakeModel, 1);
	m_ppHierarchicalGameObjects[14]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	m_ppHierarchicalGameObjects[14]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[14]->SetPosition(370.0f, m_pTerrain->GetHeight(380.0f, 750.0f) + 10, 720.0f);
	m_ppHierarchicalGameObjects[14]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[15] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSnakeModel, 1);
	m_ppHierarchicalGameObjects[15]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
	m_ppHierarchicalGameObjects[15]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[15]->SetPosition(340.0f, m_pTerrain->GetHeight(380.0f, 750.0f) + 10, 720.0f);
	m_ppHierarchicalGameObjects[15]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[16] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSnakeModel, 1);
	m_ppHierarchicalGameObjects[16]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 4);
	m_ppHierarchicalGameObjects[16]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[16]->SetPosition(310.0f, m_pTerrain->GetHeight(380.0f, 750.0f) + 10, 720.0f);
	m_ppHierarchicalGameObjects[16]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[17] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pSnakeModel, 1);
	m_ppHierarchicalGameObjects[17]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 5);
	m_ppHierarchicalGameObjects[17]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[17]->SetPosition(280.0f, m_pTerrain->GetHeight(380.0f, 750.0f) + 10, 720.0f);
	m_ppHierarchicalGameObjects[17]->SetScale(8.0f, 8.0f, 8.0f);
	if (pSnakeModel) delete pSnakeModel;
 
	CLoadedModelInfo* pToadModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/SK_Toad.bin", NULL);
	m_ppHierarchicalGameObjects[18] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pToadModel, 1);
	m_ppHierarchicalGameObjects[18]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[18]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[18]->SetPosition(430.0f, m_pTerrain->GetHeight(430.0f, 700.0f) - 20, 660.0f);
	m_ppHierarchicalGameObjects[18]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[19] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pToadModel, 1);
	m_ppHierarchicalGameObjects[19]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	m_ppHierarchicalGameObjects[19]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[19]->SetPosition(400.0f, m_pTerrain->GetHeight(400.0f, 720.0f) - 20, 660.0f);
	m_ppHierarchicalGameObjects[19]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[20] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pToadModel, 1);
	m_ppHierarchicalGameObjects[20]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	m_ppHierarchicalGameObjects[20]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[20]->SetPosition(370.0f, m_pTerrain->GetHeight(380.0f, 750.0f) - 20, 660.0f);
	m_ppHierarchicalGameObjects[20]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[21] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pToadModel, 1);
	m_ppHierarchicalGameObjects[21]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
	m_ppHierarchicalGameObjects[21]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[21]->SetPosition(340.0f, m_pTerrain->GetHeight(380.0f, 750.0f) - 20, 660.0f);
	m_ppHierarchicalGameObjects[21]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[22] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pToadModel, 1);
	m_ppHierarchicalGameObjects[22]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 4);
	m_ppHierarchicalGameObjects[22]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[22]->SetPosition(310.0f, m_pTerrain->GetHeight(380.0f, 750.0f) - 20, 660.0f);
	m_ppHierarchicalGameObjects[22]->SetScale(8.0f, 8.0f, 8.0f);

	m_ppHierarchicalGameObjects[23] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pToadModel, 1);
	m_ppHierarchicalGameObjects[23]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 5);
	m_ppHierarchicalGameObjects[23]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[23]->SetPosition(280.0f, m_pTerrain->GetHeight(380.0f, 750.0f) - 20, 660.0f);
	m_ppHierarchicalGameObjects[23]->SetScale(8.0f, 8.0f, 8.0f);
	if (pToadModel) delete pToadModel;*/

//	CLoadedModelInfo *pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Angrybot.bin", NULL);
//	m_ppHierarchicalGameObjects[0] = new CAngrybotObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLoadedModel, 1);
//	//m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[0]->SetPosition(310.0f, m_pTerrain->GetHeight(310.0f, 590.0f), 590.0f);
//	if (pAngrybotModel) delete pAngrybotModel;
//
//	CLoadedModelInfo *pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Monster.bin", NULL);
//	m_ppHierarchicalGameObjects[1] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMonsterModel, 1);
//	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[1]->SetPosition(430.0f, m_pTerrain->GetHeight(430.0f, 700.0f), 700.0f);
//	m_ppHierarchicalGameObjects[1]->SetScale(3.0f, 3.0f, 3.0f);
//	m_ppHierarchicalGameObjects[2] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMonsterModel, 1);
//	m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
//	m_ppHierarchicalGameObjects[2]->SetPosition(400.0f, m_pTerrain->GetHeight(400.0f, 720.0f), 720.0f);
//	m_ppHierarchicalGameObjects[2]->SetScale(3.0f, 3.0f, 3.0f);
//	m_ppHierarchicalGameObjects[3] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMonsterModel, 1);
//	m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
//	m_ppHierarchicalGameObjects[3]->SetPosition(380.0f, m_pTerrain->GetHeight(380.0f, 750.0f), 750.0f);
//	m_ppHierarchicalGameObjects[3]->SetScale(3.0f, 3.0f, 3.0f);
//	if (pMonsterModel) delete pMonsterModel;
//
//	CLoadedModelInfo *pHumanoidModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Humanoid.bin", NULL);
//	m_ppHierarchicalGameObjects[4] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanoidModel, 1);
//	m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[4]->SetPosition(400.0f, m_pTerrain->GetHeight(400.0f, 670.0f), 670.0f);
//	m_ppHierarchicalGameObjects[4]->Rotate(0.0f, 180.0f, 0.0f);
//	m_ppHierarchicalGameObjects[4]->SetScale(5.0f, 5.0f, 5.0f);
//
//	m_ppHierarchicalGameObjects[5] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanoidModel, 1);
//	m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
//	m_ppHierarchicalGameObjects[5]->SetPosition(410.0f, m_pTerrain->GetHeight(410.0f, 660.0f), 660.0f);
//	m_ppHierarchicalGameObjects[5]->SetScale(5.0f, 5.0f, 5.0f);
//	if (pHumanoidModel) delete pHumanoidModel;
//
/////*
//	float* pfData = new float[2];
//	pfData[0] = 0.0f;
//	pfData[1] = 1.0f;
//
//	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetCallbackKeys(0, 2);
//	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetCallbackKey(0, 0, 0.0f, &pfData[0]);
//	CAnimationSet* pAnimationSet = m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[1];
//	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetCallbackKey(0, 1, pAnimationSet->m_fLength, &pfData[1]);
//
//	CRootMotionCallbackHandler* pRootMotionCallbackHandler = new CRootMotionCallbackHandler();
//	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pRootMotionCallbackHandler);
////*/
//	m_ppHierarchicalGameObjects[6]->SetRootMotion(true);
//	m_ppHierarchicalGameObjects[6]->SetPosition(350.0f, m_pTerrain->GetHeight(350.0f, 670.0f), 670.0f);
//	m_ppHierarchicalGameObjects[6]->Rotate(0.0f, -90.0f, 0.0f);
//	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.75f);
//
//	CLoadedModelInfo *pZebraModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Zebra.bin", NULL);
//	m_ppHierarchicalGameObjects[7] = new CZebraObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pZebraModel, 1);
//	m_ppHierarchicalGameObjects[7]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[7]->SetPosition(280.0f, m_pTerrain->GetHeight(280.0f, 640.0f), 620.0f);
//	m_ppHierarchicalGameObjects[7]->SetScale(0.1f, 0.1f, 0.1f);
//	if (pZebraModel) delete pZebraModel;
//
//	CLoadedModelInfo *pLionModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Lion.bin", NULL);
//	m_ppHierarchicalGameObjects[8] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
//	m_ppHierarchicalGameObjects[8]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[8]->SetPosition(300.0f, m_pTerrain->GetHeight(300.0f, 650.0f), 630.0f);
//	m_ppHierarchicalGameObjects[8]->SetScale(1.0f, 1.0f, 1.0f);
//	m_ppHierarchicalGameObjects[9] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
//	m_ppHierarchicalGameObjects[9]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
//	m_ppHierarchicalGameObjects[9]->SetPosition(310.0f, m_pTerrain->GetHeight(310.0f, 630.0f), 630.0f);
//	m_ppHierarchicalGameObjects[9]->SetScale(1.0f, 1.0f, 1.0f);
//	m_ppHierarchicalGameObjects[10] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
//	m_ppHierarchicalGameObjects[10]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
//	m_ppHierarchicalGameObjects[10]->SetPosition(250.0f, m_pTerrain->GetHeight(250.0f, 600.0f), 600.0f);
//	m_ppHierarchicalGameObjects[10]->SetScale(1.0f, 1.0f, 1.0f);
//	m_ppHierarchicalGameObjects[11] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
//	m_ppHierarchicalGameObjects[11]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
//	m_ppHierarchicalGameObjects[11]->SetPosition(270.0f, m_pTerrain->GetHeight(270.0f, 620.0f), 620.0f);
//	m_ppHierarchicalGameObjects[11]->SetScale(1.0f, 1.0f, 1.0f);
//	m_xmf3RotatePosition = m_ppHierarchicalGameObjects[11]->GetPosition();
//
//	if (pLionModel) delete pLionModel;
//
//	m_ppHierarchicalGameObjects[12] = new CEthanObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, 2);
//	m_ppHierarchicalGameObjects[12]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[12]->m_pSkinnedAnimationController->SetTrackWeight(0, 0.85f);
//	m_ppHierarchicalGameObjects[12]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.5f);
//	m_ppHierarchicalGameObjects[12]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
//	m_ppHierarchicalGameObjects[12]->m_pSkinnedAnimationController->SetTrackWeight(1, 0.15f);
//	m_ppHierarchicalGameObjects[12]->m_pSkinnedAnimationController->SetTrackSpeed(1, 0.025f);
//	m_ppHierarchicalGameObjects[12]->SetPosition(380.0f, m_pTerrain->GetHeight(380.0f, 680.0f), 680.0f);
//
//	CLoadedModelInfo *pEagleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Eagle.bin", NULL);
//	m_ppHierarchicalGameObjects[13] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
//	m_ppHierarchicalGameObjects[13]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[13]->m_pSkinnedAnimationController->SetTrackPosition(0, 0.2f);
//	m_ppHierarchicalGameObjects[13]->SetRootMotion(true);
//	m_ppHierarchicalGameObjects[13]->SetPosition(330.0f, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
//	m_ppHierarchicalGameObjects[14] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
//	m_ppHierarchicalGameObjects[14]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[14]->SetRootMotion(true);
//	m_ppHierarchicalGameObjects[14]->SetPosition(350.0f, m_pTerrain->GetHeight(350.0f, 590.0f) + 20.0f, 590.0f);
//	m_ppHierarchicalGameObjects[15] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
//	m_ppHierarchicalGameObjects[15]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[15]->SetRootMotion(true);
//	m_ppHierarchicalGameObjects[15]->SetPosition(330.0f, m_pTerrain->GetHeight(330.0f, 750.0f) + 25.0f, 750.0f);
//	m_ppHierarchicalGameObjects[15]->Rotate(0.0f, 180.0f, 0.0f);
//	m_ppHierarchicalGameObjects[16] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
//	m_ppHierarchicalGameObjects[16]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[16]->SetRootMotion(true);
//	m_ppHierarchicalGameObjects[16]->SetPosition(350.0f, m_pTerrain->GetHeight(350.0f, 750.0f) + 25.0f, 750.0f);
//	m_ppHierarchicalGameObjects[16]->Rotate(0.0f, 180.0f, 0.0f);
//	m_ppHierarchicalGameObjects[17] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
//	m_ppHierarchicalGameObjects[17]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[17]->SetRootMotion(true);
//	m_ppHierarchicalGameObjects[17]->SetPosition(300.0f, m_pTerrain->GetHeight(300.0f, 700.0f) + 25.0f, 700.0f);
//	m_ppHierarchicalGameObjects[17]->Rotate(0.0f, 180.0f, 0.0f);
//	m_ppHierarchicalGameObjects[18] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
//	m_ppHierarchicalGameObjects[18]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[18]->SetRootMotion(true);
//	m_ppHierarchicalGameObjects[18]->SetPosition(250.0f, m_pTerrain->GetHeight(250.0f, 700.0f) + 25.0f, 700.0f);
//	m_ppHierarchicalGameObjects[18]->Rotate(0.0f, 180.0f, 0.0f);
//	m_ppHierarchicalGameObjects[19] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
//	m_ppHierarchicalGameObjects[19]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[19]->SetRootMotion(true);
//	m_ppHierarchicalGameObjects[19]->SetPosition(270.0f, m_pTerrain->GetHeight(270.0f, 750.0f) + 25.0f, 750.0f);
//	m_ppHierarchicalGameObjects[19]->Rotate(0.0f, 180.0f, 0.0f);
//	m_ppHierarchicalGameObjects[20] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
//	m_ppHierarchicalGameObjects[20]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
//	m_ppHierarchicalGameObjects[20]->m_pSkinnedAnimationController->SetTrackPosition(0, 0.2f);
//	m_ppHierarchicalGameObjects[20]->SetRootMotion(true);
//	m_ppHierarchicalGameObjects[20]->SetPosition(270.0f, m_pTerrain->GetHeight(270.0f, 800.0f) + 25.0f, 800.0f);
//	m_ppHierarchicalGameObjects[20]->Rotate(0.0f, 180.0f, 0.0f);
//
//	if (pEagleModel) delete pEagleModel;

///*
	m_nShaders = 1;
	m_ppShaders = new CShader*[m_nShaders];

	CEthanObjectsShader *pEthanObjectsShader = new CEthanObjectsShader();
	//pEthanObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, m_pTerrain);

	m_ppShaders[0] = pEthanObjectsShader;
//*/
	//if (pEthanModel) delete pEthanModel;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
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
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));

	//for (int j = 0; j < 100; j++)
	//{
	//	
	//	XMStoreFloat4x4(&m_pcbMappedGameObjects[j].m_xmf4x4Transform, XMMatrixTranspose(XMLoadFloat4x4(&m_vGameObjects[j]->m_xmf4x4World)));
	//}
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

	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) m_ppHierarchicalGameObjects[i]->ReleaseUploadBuffers();
}

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
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
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);

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
	AllocConsole(); // 콘솔 생성
	freopen("CONOUT$", "w", stdout); // 표준 출력 리다이렉트
	SetConsoleTitle(L"Debug Console"); // 콘솔 제목 (선택사항)

#ifndef test
#define test	
	printf("Center{%f %f %f} ", m_pPlayer->m_worldOBB.Center.x, m_pPlayer->m_worldOBB.Center.y, m_pPlayer->m_worldOBB.Center.z);
	printf("Extents{%f %f %f} ", m_pPlayer->m_worldOBB.Extents.x, m_pPlayer->m_worldOBB.Extents.y, m_pPlayer->m_worldOBB.Extents.z);
	printf("Orientation{%f %f %f}\n", m_pPlayer->m_worldOBB.Orientation.x, m_pPlayer->m_worldOBB.Orientation.y, m_pPlayer->m_worldOBB.Orientation.z);
	printf("Center{%f %f %f} ", m_ppHierarchicalGameObjects[0]->m_worldOBB.Center.x, m_ppHierarchicalGameObjects[0]->m_worldOBB.Center.y, m_ppHierarchicalGameObjects[0]->m_worldOBB.Center.z);
	printf("Extents{%f %f %f} ", m_ppHierarchicalGameObjects[0]->m_worldOBB.Extents.x, m_ppHierarchicalGameObjects[0]->m_worldOBB.Extents.y, m_ppHierarchicalGameObjects[0]->m_worldOBB.Extents.z);
	printf("Orientation{%f %f %f}\n", m_ppHierarchicalGameObjects[0]->m_worldOBB.Orientation.x, m_ppHierarchicalGameObjects[0]->m_worldOBB.Orientation.y, m_ppHierarchicalGameObjects[0]->m_worldOBB.Orientation.z);
	//printf("%f %f %f\n", m_ppHierarchicalGameObjects[0]->m_localOBB.Center.x, m_ppHierarchicalGameObjects[0]->m_localOBB.Center.y, m_ppHierarchicalGameObjects[0]->m_localOBB.Center.z);
	//printf("%f %f %f\n", m_ppHierarchicalGameObjects[0]->m_xmf4x4World._11, m_ppHierarchicalGameObjects[0]->m_xmf4x4World._12, m_ppHierarchicalGameObjects[0]->m_xmf4x4World._13);
	//printf("%f %f %f\n", m_ppHierarchicalGameObjects[0]->m_xmf4x4World._41, m_ppHierarchicalGameObjects[0]->m_xmf4x4World._42, m_ppHierarchicalGameObjects[0]->m_xmf4x4World._43);
	//printf("%f %f %f\n", m_ppHierarchicalGameObjects[0]->m_pChild->m_pChild->m_pChild->m_xmf4x4World._11, m_ppHierarchicalGameObjects[0]->m_pChild->m_pChild->m_pChild->m_xmf4x4World._12, m_ppHierarchicalGameObjects[0]->m_pChild->m_pChild->m_pChild->m_xmf4x4World._13);
#endif // !1

	if (m_pPlayer->CheckCollisionOBB(m_ppHierarchicalGameObjects[0])) {
		printf("[충돌 확인])\n");
	}
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights
	/*
	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i])
		{
			if (m_ppHierarchicalGameObjects[i]->FSM_manager) m_ppHierarchicalGameObjects[i]->FSMUpdate();
		}
	}
	*/
	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
	for (auto obj : m_vGameObjects) obj->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	//m_ppGameObjects[0]->RenderOBB(pd3dCommandList);

	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i])
		{
			m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
			if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);
			m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera);
		}
	}
}