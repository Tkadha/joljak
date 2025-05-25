//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "GameFramework.h"

#include "NonAtkState.h"
#include "AtkState.h"

#include "ComputeShader.h"


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

	// 2. 주요 방향광 (태양) 설정
	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f); // 방향광 자체의 약한 주변광
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.75f, 0.7f, 1.0f); // 약간 따뜻한 느낌의 태양광
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f); // 반사광
	m_pLights[2].m_xmf3Direction = XMFLOAT3(0.5f, -0.707f, 0.5f); // 남동쪽 위에서 비추는 느낌 (벡터 정규화 필요할 수 있음)


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

struct WaveSimConstants
{
	float gWaveConstant0;
	float gWaveConstant1;
	float gWaveConstant2;
	float gDisturbMag;
	DirectX::XMINT2 gDisturbIndex; // HLSL의 int2와 유사
	float gTimeStep;
	float gSpatialStep;
	// 필요시 패딩 추가하여 256 바이트 배수로 맞춤
	float padding[1]; // 예시 (실제 크기 계산 필요)
};

void CScene::BuildWaves(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	// CGameFramework 포인터 유효성 검사 (생성자나 SetGameFramework에서 설정되었다고 가정)
	assert(m_pGameFramework != nullptr && "CGameFramework pointer is null in CScene::BuildWaves. Call SetGameFramework first.");

	ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager();
	ShaderManager* pShaderManager = m_pGameFramework->GetShaderManager(); // ShaderManager는 CGameFramework 멤버라고 가정

	assert(pResourceManager != nullptr && "ResourceManager is null in CScene::BuildWaves.");
	assert(pShaderManager != nullptr && "ShaderManager is null in CScene::BuildWaves.");

	// 1. Waves 로직 객체 생성
	int numRows = 200;
	int numCols = 200;
	float spatialStep = 0.25f; // 파티클 간 간격 (월드 단위)
	float timeStep = 0.03f;    // 시뮬레이션 시간 간격
	float speed = 3.25f;       // 파도 전파 속도
	float damping = 0.05f;     // 감쇠 (파도가 점차 약해지는 정도)
	m_pWaves = std::make_unique<Waves>(numRows, numCols, spatialStep, timeStep, speed, damping);

	// 2. Waves 시뮬레이션용 UAV Texture2D 리소스 생성
	UINT waveWidth = m_pWaves->ColumnCount();
	UINT waveHeight = m_pWaves->RowCount();
	DXGI_FORMAT waveFormat = DXGI_FORMAT_R32G32B32_FLOAT; // WaveSimCS.hlsl의 RWTexture2D<float3> 와 일치

	//m_pWaves->m_pPrevSolTexture = pResourceManager->CreateUAVTexture2D(pd3dDevice, L"water1", waveWidth, waveHeight, waveFormat);
	//m_pWaves->m_pPrevSolTexture = pResourceManager->CreateUAVTexture2D(pd3dDevice, L"water1", waveWidth, waveHeight, waveFormat);
	//m_pWaves->m_pCurrSolTexture = pResourceManager->CreateUAVTexture2D(pd3dDevice, L"water1", waveWidth, waveHeight, waveFormat);
	//m_pWaves->m_pNextSolTexture = pResourceManager->CreateUAVTexture2D(pd3dDevice, L"water1", waveWidth, waveHeight, waveFormat);

	//if (!m_pWaves->m_pPrevSolTexture || !m_pWaves->m_pCurrSolTexture || !m_pWaves->m_pNextSolTexture) {
	//	OutputDebugStringA("Error: Failed to create UAV textures for Waves simulation in CScene::BuildWaves.\n");
	//	return; // 또는 예외 처리
	//}

	// 3. 해당 리소스에 대한 UAV 및 SRV 디스크립터 생성
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = waveFormat;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = waveFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	bool bAllocationSuccess = true;
	// PrevSol UAV
	bAllocationSuccess &= m_pGameFramework->AllocateSrvDescriptors(1, m_pWaves->m_hPrevSolUavCPU, m_pWaves->m_hPrevSolUavGPU);
	if (bAllocationSuccess) pd3dDevice->CreateUnorderedAccessView(m_pWaves->m_pPrevSolTexture.Get(), nullptr, &uavDesc, m_pWaves->m_hPrevSolUavCPU);
	// CurrSol UAV
	bAllocationSuccess &= m_pGameFramework->AllocateSrvDescriptors(1, m_pWaves->m_hCurrSolUavCPU, m_pWaves->m_hCurrSolUavGPU);
	if (bAllocationSuccess) pd3dDevice->CreateUnorderedAccessView(m_pWaves->m_pCurrSolTexture.Get(), nullptr, &uavDesc, m_pWaves->m_hCurrSolUavCPU);
	// NextSol UAV
	bAllocationSuccess &= m_pGameFramework->AllocateSrvDescriptors(1, m_pWaves->m_hNextSolUavCPU, m_pWaves->m_hNextSolUavGPU);
	if (bAllocationSuccess) pd3dDevice->CreateUnorderedAccessView(m_pWaves->m_pNextSolTexture.Get(), nullptr, &uavDesc, m_pWaves->m_hNextSolUavCPU);
	// CurrSol SRV (렌더링 시 사용)
	bAllocationSuccess &= m_pGameFramework->AllocateSrvDescriptors(1, m_pWaves->m_hCurrSolSrvCPU, m_pWaves->m_hCurrSolSrvGPU);
	if (bAllocationSuccess) pd3dDevice->CreateShaderResourceView(m_pWaves->m_pCurrSolTexture.Get(), &srvDesc, m_pWaves->m_hCurrSolSrvCPU);

	if (!bAllocationSuccess) {
		OutputDebugStringA("Error: Failed to allocate descriptors for Waves simulation in CScene::BuildWaves.\n");
		return; // 또는 예외 처리
	}

	// 4. 파도 렌더링 객체(CWaveObject) 생성
	m_pWaveObject = std::make_unique<CWaveObject>(pd3dDevice, m_pWaves.get(), pResourceManager, pShaderManager);


	// 5. 컴퓨트 셰이더용 상수 버퍼 생성
	m_alignedWaveSimCBSize = (sizeof(WaveSimConstants) + 255) & ~255; // 256바이트 정렬

	auto heapPropsUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resourceDescBuffer = CD3DX12_RESOURCE_DESC::Buffer(m_alignedWaveSimCBSize);
	HRESULT hr = pd3dDevice->CreateCommittedResource(
		&heapPropsUpload,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescBuffer,
		D3D12_RESOURCE_STATE_GENERIC_READ, // 업로드 힙은 항상 GENERIC_READ 상태
		nullptr,
		IID_PPV_ARGS(&m_pWaveSimConstantBuffer));
	if (FAILED(hr)) { OutputDebugStringA("Failed to create wave sim constant buffer (upload heap).\n"); return; }

	// CPU에서 쓰기 위해 매핑 (한 번만 해두고 계속 사용)
	CD3DX12_RANGE readRange(0, 0); // 읽지 않을 것이므로 범위 0
	hr = m_pWaveSimConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pMappedWaveSimConstantBuffer));
	if (FAILED(hr)) { OutputDebugStringA("Failed to map wave sim constant buffer.\n"); return; }

	// (옵션 2: Default 힙에 생성하고 매 프레임 업로드 힙을 통해 데이터 복사 - 더 일반적)
	// 이 경우, m_pWaveSimConstantBuffer는 Default 힙, 별도의 업로드 힙 리소스 필요.
	// m_pGameFramework 또는 ResourceManager에 상수 버퍼 생성/업데이트 헬퍼 함수가 있다면 그것을 사용.

	OutputDebugStringA("Waves simulation objects and resources built successfully.\n");
}

void CScene::UpdateWaves(float fTimeDelta, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (!m_pWaves || !m_pGameFramework) // Waves 로직 객체와 프레임워크 유효성 확인
		return;

	ShaderManager* pShaderManager = m_pGameFramework->GetShaderManager();
	if (!pShaderManager) return;

	// 1. CPU 기반 로직 업데이트 (예: 사용자 입력에 따른 파동 생성 설정)
	m_pWaves->Update(fTimeDelta); // 현재는 비어있거나 간단한 로직

	// --- 컴퓨트 셰이더 실행 ---
	CShader* pComputeShaderBase = pShaderManager->GetShader("WaveSimCompute", pd3dCommandList);
	if (!pComputeShaderBase) return;

	CComputeShaderWrapper* pComputeShader = static_cast<CComputeShaderWrapper*>(pComputeShaderBase);
	if (!pComputeShader || !pComputeShader->GetComputePSO()) {
		pComputeShaderBase->Release();
		return;
	}

	// 2. 파이프라인 상태 및 루트 시그니처 설정
	pd3dCommandList->SetPipelineState(pComputeShader->GetComputePSO());
	pd3dCommandList->SetComputeRootSignature(pComputeShader->GetRootSignature());

	// 3. 상수 버퍼 업데이트 및 바인딩
	if (m_pWaveSimConstantBuffer && m_pMappedWaveSimConstantBuffer) {
		WaveSimConstants simConstants;
		simConstants.gWaveConstant0 = m_pWaves->mK1; // Waves 클래스에 mK1, mK2, mK3 멤버가 public이거나 getter 필요
		simConstants.gWaveConstant1 = m_pWaves->mK2;
		simConstants.gWaveConstant2 = m_pWaves->mK3;
		simConstants.gTimeStep = m_pWaves->mTimeStep;     // Waves 클래스에 mTimeStep 멤버 필요
		simConstants.gSpatialStep = m_pWaves->mSpatialStep; // Waves 클래스에 mSpatialStep 멤버 필요

		memcpy(m_pMappedWaveSimConstantBuffer, &simConstants, sizeof(WaveSimConstants)); // 업로드 힙에 직접 복사

		// 루트 파라미터 0에 CBV 바인딩 (b0)
		pd3dCommandList->SetComputeRootConstantBufferView(0, m_pWaveSimConstantBuffer->GetGPUVirtualAddress());
	}
	else {
		OutputDebugStringA("Error: Wave simulation constant buffer not initialized properly.\n");
		pComputeShaderBase->Release();
		return;
	}

	// 4. UAV 디스크립터 테이블 바인딩
	// 루트 파라미터 1에 UAV 3개(Prev, Curr, Next 순서)를 위한 디스크립터 테이블 바인딩.
	// GetCurrentPrevSolUAV_GPU()가 이 연속된 테이블의 시작 핸들을 반환한다고 가정합니다.
	// (CScene::BuildWaves에서 연속 할당 및 Waves::SwapSimBuffersAndDescriptors에서 핸들 값 스왑이 중요)
	D3D12_GPU_DESCRIPTOR_HANDLE uavTableStartGpuHandle = m_pWaves->GetCurrentPrevSolUAV_GPU();
	if (uavTableStartGpuHandle.ptr != 0) { // 간단히 Prev 핸들을 테이블 시작으로 가정
		pd3dCommandList->SetComputeRootDescriptorTable(1, uavTableStartGpuHandle);
	}
	else {
		OutputDebugStringA("Error: Invalid GPU descriptor handle for UAV table start.\n");
	}


	// 5. 컴퓨트 셰이더 디스패치
	UINT numGroupsX = (UINT)ceilf((float)m_pWaves->ColumnCount() / 16.0f); // 16은 WaveSimCS.hlsl의 numthreads X
	UINT numGroupsY = (UINT)ceilf((float)m_pWaves->RowCount() / 16.0f);   // 16은 WaveSimCS.hlsl의 numthreads Y
	pd3dCommandList->Dispatch(numGroupsX, numGroupsY, 1);

	// 6. UAV 배리어 (출력 버퍼인 NextSol 텍스처에 대해)
	ID3D12Resource* pOutputResource = m_pWaves->GetCurrentNextSolTexture();
	if (pOutputResource) {
		auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(pOutputResource);
		pd3dCommandList->ResourceBarrier(1, &barrier);
	}

	// 7. 다음 프레임을 위한 버퍼 및 디스크립터 스왑
	m_pWaves->SwapSimBuffersAndDescriptors();

	pComputeShaderBase->Release(); // ShaderManager::GetShader에서 AddRef 했으므로
}

// 이 함수는 주로 파도 렌더링에 필요한 리소스 상태를 전이합니다.
// 실제 렌더링은 CScene::Render() 함수 내에서 게임 오브젝트 목록을 순회하며 CWaveObject::Render()가 호출될 때 이루어집니다.
void CScene::RenderWaves(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (!m_pWaves || !m_pWaveObject) // 파도 관련 객체 유효성 확인
		return;

	// --- 파도 렌더링 전 리소스 상태 전이 ---
	// 현재 시뮬레이션 결과가 담긴 텍스처 (CurrSol - 이전 프레임의 NextSol이었고, UpdateWaves에서 Curr로 스왑됨)를
	// UAV 상태에서 SRV로 읽을 수 있는 상태로 변경합니다.
	ID3D12Resource* pWaveSolutionResourceToRender = m_pWaves->GetCurrentCurrSolTextureForRendering();
	if (pWaveSolutionResourceToRender) {
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			pWaveSolutionResourceToRender,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,      // 컴퓨트 셰이더에서 쓰기 완료된 상태
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE // VS/PS에서 읽기 위한 상태
		);
		pd3dCommandList->ResourceBarrier(1, &barrier);
	}
}


void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	// ShaderManager 가져오기
	assert(m_pGameFramework != nullptr && "GameFramework pointer is needed!");
	ShaderManager* pShaderManager = m_pGameFramework->GetShaderManager();
	assert(pShaderManager != nullptr && "ShaderManager is not available!");
	ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager(); // 기존 코드 유지

	BuildDefaultLightsAndMaterials();

	BuildWaves(pd3dDevice, pd3dCommandList);

	if (!pResourceManager) {
		// 리소스 매니저가 없다면 로딩 불가! 오류 처리
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

	// 랜덤 엔진
	std::random_device rd;
	std::mt19937 gen(rd());

	int tree_obj_count{ 0 };

	float spawnMin = 500, spawnMax = 9500;
	float objectMinSize = 15, objectMaxSize = 20;

	int nPineObjects = 100;
	for (int i = 0; i < nPineObjects; ++i) {
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
	for (int i = 0; i < nPineObjects; ++i) {
		CGameObject* gameObj = new CBirchObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;
		m_vGameObjects.emplace_back(gameObj);
		auto t_obj = std::make_unique<tree_obj>(tree_obj_count++, gameObj->m_worldOBB.Center);
		octree.insert(std::move(t_obj));
	}
	for (int i = 0; i < nPineObjects; ++i) {
		CGameObject* gameObj = new CWillowObject(pd3dDevice, pd3dCommandList, m_pGameFramework);
		auto [x, z] = genRandom::generateRandomXZ(gen, spawnMin, spawnMax, spawnMin, spawnMax);
		gameObj->SetPosition(x, m_pTerrain->GetHeight(x, z), z);
		auto [w, h] = genRandom::generateRandomXZ(gen, objectMinSize, objectMaxSize, objectMinSize, objectMaxSize);
		gameObj->SetScale(w, h, w);
		gameObj->m_treecount = tree_obj_count;
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



	for (auto obj : m_vGameObjects) {
		obj->SetOBB();
		obj->InitializeOBBResources(pd3dDevice, pd3dCommandList);
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
			break;
		default:	// 잘못된 타입이다.
			break;
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;

	ReleaseShaderVariables();

	m_listBranchObjects.clear();

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


	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
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



	// 5.1. 스카이박스 렌더링
	if (m_pSkyBox) {
		m_pSkyBox->Render(pd3dCommandList, pCamera); // SkyBox::Render 내부에서 상태 설정 및 렌더링
	}

	// 5.2. 지형 렌더링
	if (m_pTerrain) {
		m_pTerrain->Render(pd3dCommandList, pCamera); // Terrain::Render 내부에서 상태 설정 및 렌더링
	}

	if (m_pWaves && m_pWaveObject) { // 파도 관련 객체가 유효할 때만
		// UAV -> SRV 상태 전이
		/*ID3D12Resource* pWaveSolutionResourceToRender = m_pWaves->GetCurrentCurrSolTextureForRendering();
		if (pWaveSolutionResourceToRender) {
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				pWaveSolutionResourceToRender,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
			);
			pd3dCommandList->ResourceBarrier(1, &barrier);
		}*/
		
		//RenderWaves(pd3dCommandList, pCamera);
	}


	// octree 렌더링
	std::vector<tree_obj*> results;
	tree_obj player_obj{ -1, m_pPlayer->GetPosition() };

	octree.query(player_obj, XMFLOAT3{ 2500,1000,2500 }, results);

	for (auto& obj : results) {
		if (m_vGameObjects[obj->u_id]) {
			if (m_vGameObjects[obj->u_id]->FSM_manager) m_vGameObjects[obj->u_id]->FSMUpdate();
			//if (m_vGameObjects[obj->u_id]->m_pSkinnedAnimationController) m_vGameObjects[obj->u_id]->Animate(m_fElapsedTime);
			if (m_vGameObjects[obj->u_id]) m_vGameObjects[obj->u_id]->Animate(m_fElapsedTime);
			if (m_vGameObjects[obj->u_id]->isRender) m_vGameObjects[obj->u_id]->Render(pd3dCommandList, pCamera);
		}
	}

	//for (auto it = m_listBranchObjects.begin(); it != m_listBranchObjects.end(); ) {
	//	(*it)->Animate(m_fElapsedTime);
	//	if (!(*it)->isRender) { // isRender가 false이면 (수명이 다하면) 리스트에서 제거
	//		it = m_listBranchObjects.erase(it);
	//	}
	//	else {
	//		++it;
	//	}
	//}

	for (auto branch : m_listBranchObjects) {
		if (branch->isRender) { // 렌더링 플래그 확인
			branch->Animate(m_fElapsedTime);
			branch->Render(pd3dCommandList, pCamera);
		}
	}

	for (auto branch : m_listRockObjects) {
		if (branch->isRender) { // 렌더링 플래그 확인
			branch->Animate(m_fElapsedTime);
			branch->Render(pd3dCommandList, pCamera);
		}
	}

	if (m_pWaveObject) { 
		// m_pWaveObject->Animate(m_fElapsedTime);
		m_pWaveObject->Render(pd3dCommandList, pCamera);
	}


	if(m_pPreviewPine->isRender)	m_pPreviewPine->Render(pd3dCommandList, pCamera);

	// 5.5. 플레이어 렌더링
	if (m_pPlayer) {
		if (m_pPlayer->invincibility) {
			auto endtime = std::chrono::system_clock::now();
			auto exectime = endtime - m_pPlayer->starttime;
			auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(exectime).count();
			if (exec_ms > 1000.f) { // 무적시간이 1초가 경과되면
				m_pPlayer->SetInvincibility();	// 변경
			}
		}
		m_pPlayer->Render(pd3dCommandList, pCamera);
	}
	for (auto& p : PlayerList) {
		if (p.second->m_pSkinnedAnimationController) p.second->Animate(m_fElapsedTime);
		if (p.second->isRender) p.second->Render(pd3dCommandList, pCamera);
	}


	//if (m_pWaves && m_pWaveObject) { // 파도 관련 객체가 유효할 때만
	//	ID3D12Resource* pWaveSolutionResourceRendered = m_pWaves->GetCurrentCurrSolTextureForRendering();
	//	if (pWaveSolutionResourceRendered) {
	//		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
	//			pWaveSolutionResourceRendered,
	//			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, // 현재 상태
	//			D3D12_RESOURCE_STATE_UNORDERED_ACCESS                                                     // 다음 프레임 CS를 위한 상태
	//		);
	//		pd3dCommandList->ResourceBarrier(1, &barrier);
	//	}
	//}

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

#include "NonAtkState.h"
#include "AtkState.h"
void CScene::CheckPlayerInteraction(CPlayer* pPlayer) {
	if (!pPlayer) return;

	// Player <-> Object
	for (auto& obj : m_vGameObjects) {
		if (CollisionCheck(m_pPlayer, obj)) {
			if (!obj->isRender)   continue;
			// 나무 충돌처리
			if (obj->m_objectType == GameObjectType::Tree) {
				//obj->isRender = false;
				//m_pGameFramework->AddItem("wood", 3);
			}
			// 돌 충돌처리
			if (obj->m_objectType == GameObjectType::Rock) {
				//printf("[Rock 충돌 확인])\n");
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
			/*if (obj->m_objectType == GameObjectType::Cow || obj->m_objectType == GameObjectType::Pig) {
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
			}*/
		}
	}
}



void CScene::SpawnBranch(const XMFLOAT3& position, const XMFLOAT3& initialVelocity) {
	if (!m_pGameFramework || !m_pTerrain) return; // 프레임워크와 지형 포인터 유효성 검사

	CBranchObject* newBranch = new CBranchObject(
		m_pGameFramework->GetDevice(),
		m_pGameFramework->GetCommandList(),
		m_pGameFramework,
		m_pTerrain
	);
	newBranch->SetPosition(position);
	newBranch->SetInitialVelocity(initialVelocity);
	// 필요시 초기 회전 등 설정
	newBranch->Rotate(0, (float)(rand() % 360), 0); // Y축으로 랜덤 회전

	m_listBranchObjects.emplace_back(newBranch);
	//auto t_obj = std::make_unique<newBranch>(tree_obj_count++, gameObj->m_worldOBB.Center);
	//octree.insert(std::move(t_obj));
}

void CScene::SpawnRock(const XMFLOAT3& position, const XMFLOAT3& initialVelocity) {
	if (!m_pGameFramework || !m_pTerrain) return; // 프레임워크와 지형 포인터 유효성 검사

	CRockDropObject* newBranch = new CRockDropObject(
		m_pGameFramework->GetDevice(),
		m_pGameFramework->GetCommandList(),
		m_pGameFramework,
		m_pTerrain
	);
	newBranch->SetPosition(position);
	newBranch->SetInitialVelocity(initialVelocity);
	// 필요시 초기 회전 등 설정
	newBranch->Rotate(0, (float)(rand() % 360), 0); // Y축으로 랜덤 회전

	m_listRockObjects.emplace_back(newBranch);
	//auto t_obj = std::make_unique<newBranch>(tree_obj_count++, gameObj->m_worldOBB.Center);
	//octree.insert(std::move(t_obj));
}