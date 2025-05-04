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

	ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager();
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
		CGameObject* gameObj = new CPineObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 2, 6, 2, 10);
		gameObj->SetScale(w, h, w);
		
		m_vGameObjects.emplace_back(gameObj);
	}
	{
		CGameObject* gameObj = new CSwordObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z)+10, z);
		gameObj->SetScale(100,100,100);
		m_vGameObjects.emplace_back(gameObj);
	}
	int nRockClusterAObjects = 10;
	for (int i = 0; i < nRockClusterAObjects; ++i) {
		CGameObject* gameObj = new CRockClusterAObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 10, 20, 20, 30);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}
	
	int nRockClusterBObjects = 10;
	for (int i = 0; i < nRockClusterBObjects; ++i) {
		CGameObject* gameObj = new CRockClusterBObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 10, 20, 20, 30);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}
	int nRockClusterCObjects = 10;
	for (int i = 0; i < nRockClusterCObjects; ++i) {
		CGameObject* gameObj = new CRockClusterCObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, 10, 20, 20, 30);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}

	int nCliffFObjectCObjects = 5;
	for (int i = 0; i < nRockClusterCObjects; ++i) {
		CGameObject* gameObj = new CCliffFObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen,5, 10, 5, 10);
		gameObj->SetScale(w, h, w);
		m_vGameObjects.emplace_back(gameObj);
	}


	// ������Ʈ ����
	m_nHierarchicalGameObjects = 6;
	m_ppHierarchicalGameObjects = new CGameObject*[m_nHierarchicalGameObjects];

	CLoadedModelInfo* pCowModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Cow.bin", m_pGameFramework);

	m_ppHierarchicalGameObjects[0] = new CMonsterObject(pd3dDevice, pd3dCommandList, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
	m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_ppHierarchicalGameObjects[0]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[0]->SetPosition(1000.f/2, m_pTerrain->GetHeight(1000.0f, 1500.0f)/2, 1500.f/2);
	m_ppHierarchicalGameObjects[0]->SetScale(8.0f, 8.0f, 8.0f);
	
	tree_objects.emplace_back(0, m_ppHierarchicalGameObjects[0]->m_worldOBB.Center);
	octree.insert(&tree_objects[0]);
	
	m_ppHierarchicalGameObjects[1] = new CMonsterObject(pd3dDevice, pd3dCommandList, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackEnable(2, false);

	m_ppHierarchicalGameObjects[1]->SetScale(8.0f, 8.0f, 8.0f);
	m_ppHierarchicalGameObjects[1]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[1]->SetPosition(800.0f / 2, m_pTerrain->GetHeight(800.0f, 1400.0f) / 2, 1400.0f / 2);
	m_ppHierarchicalGameObjects[1]->SetTerraindata(m_pTerrain);

	m_ppHierarchicalGameObjects[2] = new CMonsterObject(pd3dDevice, pd3dCommandList, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	m_ppHierarchicalGameObjects[2]->SetScale(10.0f, 10.0f, 10.0f);
	m_ppHierarchicalGameObjects[2]->Rotate(0.f, 0.f, 0.f);
	m_ppHierarchicalGameObjects[2]->SetPosition(500.0f/2, m_pTerrain->GetHeight(500.0f, 800.0f)/2, 800.0f/2);
	m_ppHierarchicalGameObjects[2]->SetTerraindata(m_pTerrain);

	m_ppHierarchicalGameObjects[3] = new CMonsterObject(pd3dDevice, pd3dCommandList, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	m_ppHierarchicalGameObjects[3]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[3]->SetPosition(100.0f / 2, m_pTerrain->GetHeight(100.0f, 1400.0f) / 2, 1400.0f / 2);
	m_ppHierarchicalGameObjects[3]->SetScale(10.0f, 10.0f, 10.0f);
	m_ppHierarchicalGameObjects[3]->SetTerraindata(m_pTerrain);

	m_ppHierarchicalGameObjects[4] = new CMonsterObject(pd3dDevice, pd3dCommandList, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
	m_ppHierarchicalGameObjects[4]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[4]->SetPosition(200.0f /2 , m_pTerrain->GetHeight(200.0f, 500.0f)/2, 500.0f / 2);
	m_ppHierarchicalGameObjects[4]->SetScale(8.0f, 8.0f, 8.0f);
	m_ppHierarchicalGameObjects[4]->SetTerraindata(m_pTerrain);

	m_ppHierarchicalGameObjects[5] = new CMonsterObject(pd3dDevice, pd3dCommandList, pCowModel, 1, m_pGameFramework);
	m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[5]->Rotate(0.f, 180.f, 0.f);
	m_ppHierarchicalGameObjects[5]->SetPosition(1000.f / 2, m_pTerrain->GetHeight(1000.0f, 1500.0f) / 2, 1500.f / 2);
	m_ppHierarchicalGameObjects[5]->SetScale(8.0f, 8.0f, 8.0f);
	m_ppHierarchicalGameObjects[5]->SetTerraindata(m_pTerrain);
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


	for (int i = 0; i < m_nHierarchicalGameObjects; ++i)
		m_ppHierarchicalGameObjects[i]->PropagateAnimController(m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController);


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


	//----------------------충돌체크------------------------------------

	// Player <-> Object
	//for (auto& obj : m_vGameObjects) {
	//	if (CollisionCheck(m_pPlayer, obj)) {
	//		if (!obj->isRender)	continue;
	//		// 나무 충돌처리
	//		if (obj->m_objectType == GameObjectType::Tree) {
	//			//auto [x, z] = genRandom::generateRandomXZ(gen, 1000, 2000, 1000, 2000);
	//			//obj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
	//			//auto [w, h] = genRandom::generateRandomXZ(gen, 2, 6, 2, 10);
	//			//obj->SetScale(w, h, w);
	//			obj->isRender = false;
	//		}

	//		// 돌 충돌처리
	//		if (obj->m_objectType == GameObjectType::Rock) {
	//			printf("[Rock 충돌 확인])\n");
	//			obj->isRender = false;
	//		}

	//	}
	//}

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

	// 디스크립터 힙 설정 
	ID3D12DescriptorHeap* ppHeaps[] = { m_pGameFramework->GetCbvSrvHeap() }; // CBV/SRV/UAV 힙 가져오기
	if (ppHeaps[0]) { // 힙 포인터 유효성 검사
		pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	}
	else {
		assert(!"CBV/SRV Descriptor Heap is NULL in CScene::Render!");
		return; // 힙 없으면 렌더링 불가
	}

	// --- 4. 렌더링 상태 추적 변수 ---
	m_pCurrentRootSignature = nullptr;
	m_pCurrentPSO = nullptr;
	m_pCurrentShader = nullptr;


	// --- 5. 렌더링 단계별 처리 ---

	// 5.1. 스카이박스 렌더링
	if (m_pSkyBox) {
		m_pSkyBox->Render(pd3dCommandList, pCamera); // SkyBox::Render 내부에서 상태 설정 및 렌더링
	}

	// 5.2. 지형 렌더링
	if (m_pTerrain) {
		m_pTerrain->Render(pd3dCommandList, pCamera); // Terrain::Render 내부에서 상태 설정 및 렌더링
	}

	// 5.3. 일반 게임 오브젝트 렌더링
	for (auto& obj : m_vGameObjects) {
		if (obj /*&& obj->IsVisible()*/) {
			obj->Render(pd3dCommandList, pCamera); // GameObject::Render 내부에서 상태 설정 및 렌더링 (재귀 포함)
		}
	}

	// 5.4. 계층적 게임 오브젝트 렌더링
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) {
		if (m_ppHierarchicalGameObjects[i] && m_ppHierarchicalGameObjects[i]->isRender == true) {
			// 애니메이션 처리 
			m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
			if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);

			m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera); // GameObject::Render 내부에서 상태 설정 및 렌더링 (재귀 포함)
		}
	}

	// 5.5. 플레이어 렌더링
	if (m_pPlayer)
	{
		m_pPlayer->Render(pd3dCommandList, pCamera); // Player::Render (GameObject::Render 상속) 내부에서 상태 설정 및 렌더링
	}

	// 5.5. OBB 렌더링 (선택적)
	bool bRenderOBBs = true; // OBB 렌더링 여부 플래그 (예시)
	if (bRenderOBBs) {
		CShader* pOBBShader = pShaderManager->GetShader("OBB", pd3dCommandList);
		if (pOBBShader) {
			// OBB 렌더링 시작 전에 상태 설정
			SetGraphicsState(pd3dCommandList, pOBBShader); // CScene의 멤버 함수 호출

			for (auto& obj : m_vGameObjects) {
				if (obj /*&& obj->ShouldRenderOBB()*/) {
					// RenderOBB 내부에서는 OBB용 CBV만 바인딩
					obj->RenderOBB(pd3dCommandList, pCamera);
			pOBBShader->Release();
			}
			for (int i = 0; i < m_nHierarchicalGameObjects; ++i) {
	*/
					m_ppHierarchicalGameObjects[i]->RenderOBB(pd3dCommandList, pCamera);
				}
			}

			// 플레이어 OBB 렌더링 등
			if (m_pPlayer) {
				m_pPlayer->RenderOBB(pd3dCommandList, pCamera);
			}

			pOBBShader->Release();
		}
	}
	*/


}

void CScene::SetGraphicsState(ID3D12GraphicsCommandList* pd3dCommandList, CShader* pShader)
{
	if (!pShader || !pd3dCommandList) return;

	// 셰이더 객체 자체가 바뀌었는지 확인
	if (pShader != m_pCurrentShader)
	{
		m_pCurrentShader = pShader; // 현재 셰이더 업데이트

		// 루트 서명 설정 (셰이더에 저장된 루트 서명 사용)
		ID3D12RootSignature* pRootSig = pShader->GetRootSignature();
		if (pRootSig && pRootSig != m_pCurrentRootSignature) {
			pd3dCommandList->SetGraphicsRootSignature(pRootSig);
			m_pCurrentRootSignature = pRootSig;
			// !!! 여기서 공통 CBV 바인딩 로직은 제거됨 !!!
		}

		// PSO 설정 (셰이더에 저장된 PSO 사용)
		ID3D12PipelineState* pPSO = pShader->GetPipelineState();
		if (pPSO && pPSO != m_pCurrentPSO) {
			pd3dCommandList->SetPipelineState(pPSO);
			m_pCurrentPSO = pPSO;
		}
	}
	// 이미 같은 셰이더(같은 RS, 같은 PSO)라면 아무것도 변경 안 함
}

ShaderManager* CScene::GetShaderManager() const {
	return m_pGameFramework ? m_pGameFramework->GetShaderManager() : nullptr;
}

	for (auto obj : m_vGameObjects)
	{
		if (obj->isRender) obj->Render(pd3dCommandList, obbRender,pCamera);
	}
	for (int i = 0; i < 3; i++)
	{
		if (m_ppHierarchicalGameObjects[i] && m_ppHierarchicalGameObjects[i]->isRender == true)
		{
			m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
			if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);
			m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, obbRender, pCamera);
		}
	}
}


bool CScene::CollisionCheck(CGameObject* a, CGameObject* b)
{
	if (!a || !b) {
		return false;
	}

	// a 모든 OBB 수집
	std::vector<DirectX::BoundingOrientedBox> obbListA;
	CollectHierarchyObjects(a, obbListA);

	//b 모든 OBB 수집
	std::vector<DirectX::BoundingOrientedBox> obbListB;
	CollectHierarchyObjects(b, obbListB);

	// 충돌 검사
	for (const auto& obbA : obbListA) { 
		for (const auto& obbB : obbListB) {
			if (obbA.Intersects(obbB)) {
				return true; // 충돌 시 즉시 true 반환
			}
		}
	}

	// 충돌 없으면 false 반환
	return false;

}

void CScene::CollectHierarchyObjects(CGameObject* obj, std::vector<BoundingOrientedBox>& obbList) {
	if (!obj) {
		return; // 재귀 탈출 조건
	}

	if(obj->m_pMesh)
		obbList.push_back(obj->m_worldOBB);

	// 재귀 호출
	CGameObject* currentChild = obj->m_pChild;
	while (currentChild) {
		CollectHierarchyObjects(currentChild, obbList); // 자식 노드에 대해 
		currentChild = currentChild->m_pSibling;        // 다음 형제 자식
	}
}


void CScene::CheckPlayerInteraction(CPlayer* pPlayer) {
	if (!pPlayer) return;

	// Player <-> Object
	for (auto& obj : m_vGameObjects) {
		if (CollisionCheck(m_pPlayer, obj)) {
			if (!obj->isRender)	continue;
			// 나무 충돌처리
			if (obj->m_objectType == GameObjectType::Tree) {
				obj->isRender = false;
				m_pGameFramework->AddItem("wood");
			}

			// 돌 충돌처리
			if (obj->m_objectType == GameObjectType::Rock) {
				printf("[Rock 충돌 확인])\n");
				obj->isRender = false;
				m_pGameFramework->AddItem("stone");
			}

		}
	}
}
