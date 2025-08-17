//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "d3dx12.h"
#include "Shader.h"
#include "Player.h"
#include "Octree.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "ShadowMap.h"
#include <unordered_map>
#include <mutex>

#define MAX_LIGHTS						16 

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

class CGameFramework;

class CWavesObject;

struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	int									m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};										
										
struct LIGHTS							
{										
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
	bool     gIsDaytime;
};

struct VS_VB_INSTANCE
{
	XMFLOAT4X4 m_xmf4x4Transform;
};


bool ChangeAlbedoTexture(
	CGameObject* pParentGameObject,
	int materialIndex,
	UINT textureSlot,
	const wchar_t* textureFilePath,
	ResourceManager* pResourceManager,
	ID3D12GraphicsCommandList* pd3dCommandList,
	ID3D12Device* pd3dDevice);

class CScene
{
public:
	CScene(CGameFramework*);
	~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();
	void ServerBuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	//void Render(ID3D12GraphicsCommandList *pd3dCommandList,bool obbRender, CCamera *pCamera=NULL);

	void ReleaseUploadBuffers();

	void SetGraphicsState(ID3D12GraphicsCommandList* pd3dCommandList, CShader* pShader);

	// 조명 버퍼 접근자 
	ID3D12Resource* GetLightsConstantBuffer() const { return m_pd3dcbLights; }

	// ShaderManager 접근자 
	ShaderManager* GetShaderManager() const; // 구현 필요 (m_pGameFramework 통해)

	// CGameFramework 접근자 
	CGameFramework* GetGameFramework() const { return m_pGameFramework; }
	bool CollisionCheck(CGameObject* a, CGameObject* b);
	void CollectHierarchyObjects(CGameObject* node, std::vector<BoundingOrientedBox>& obbList);

	// 플레이어의 'F' 키 상호작용 요청을 처리하는 함수
	void CheckPlayerInteraction(CPlayer* pPlayer);

	void SpawnAttackEffect(const XMFLOAT3& centerPosition, int numEffects, float radius);

	CSkyBox* GetSkyBox() const { return m_pSkyBox; }

	CPlayer* GetPlayerInfo() { return m_pPlayer; };
	CPlayer* m_pPlayer = NULL;

public:
	float								m_fElapsedTime = 0.0f;

	std::mutex							m_Mutex; // 멀티스레드 안전성을 위한 뮤텍스
	std::list<CGameObject*>				m_vGameObjects{};
	vector<CGameObject*>				m_vConstructionObjects{};
	std::list<CGameObject*> 			m_listGameObjects;
	std::list<CGameObject*>				m_lEnvironmentObjects;
	std::unordered_map<std::string, CGameObject*> m_mapBuildPrefabs;
	std::list<CGameObject*>				heilport_anthena;

	// 파도 오브젝트
	CWavesObject* m_pWavesObject = nullptr;
	//이펙트 오브젝트
	std::vector<CAttackEffectObject*> m_vAttackEffects;
	std::vector<CBloodEffectObject*> m_vBloodEffects;


	enum class ShardType { Wood, Rock };
	void SpawnResourceShards(const XMFLOAT3& origin, ShardType type);

	//int									m_nHierarchicalGameObjects = 0;
	//CGameObject							**m_ppHierarchicalGameObjects = NULL;

	XMFLOAT3							m_xmf3RotatePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	CSkyBox* m_pSkyBox = NULL;

	ID3D12Resource* m_pd3dcbLightCamera = nullptr;
	VS_CB_CAMERA_INFO* m_pcbMappedLightCamera = nullptr;

	CHeightMapTerrain* m_pTerrain = NULL;

	LIGHT* m_pLights = NULL;
	int									m_nLights = 0;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;

	// 인스턴싱
	ID3D12Resource* m_pd3dcbGameObjects = NULL;
	VS_VB_INSTANCE* m_pcbMappedGameObjects = NULL;

	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;

	int tree_obj_count{ 0 };
	std::vector<tree_obj>				tree_objects;
	Octree								octree{ XMFLOAT3 {0,0,0}, XMFLOAT3{10200,6000,10200} };
	bool								checkTree = false;
	bool								checkRock = false;

	bool								obbRender = false;

	std::unordered_map<ULONGLONG, std::unique_ptr<UserObject>> PlayerList;	// 오브젝트 수정해야함

	CGameFramework* m_pGameFramework;

	ID3D12RootSignature* m_pCurrentRootSignature = nullptr;
	ID3D12PipelineState* m_pCurrentPSO = nullptr;
	CShader* m_pCurrentShader = nullptr;

	vector<CGameObject*> m_listBranchObjects; // 생성된 나뭇가지 저장 리스트
	vector<CGameObject*> m_listRockObjects; // 생성된 나뭇가지 저장 리스트
	

	void SpawnBranch(const XMFLOAT3& position, const XMFLOAT3& initialVelocity);
	void SpawnRock(const XMFLOAT3& position, const XMFLOAT3& initialVelocity);
	void SpawnGolemPunchEffect(const XMFLOAT3& origin, const XMFLOAT3& direction);
	void SpawnBloodEffect(const XMFLOAT3& position);
	void SpawnVortexEffect(const XMFLOAT3& centerPosition);

	void NewGameBuildObj();
	void ClearObj();
private:
	bool m_bIsDaytime = true;
	std::vector<CResourceShardEffect*> m_vWoodShards;
	std::vector<CResourceShardEffect*> m_vRockShards;
	std::vector<CVortexEffectObject*> m_vVortexEffects;

	float m_fVortexDamageTimer = 0.0f;
public:	// 그림자
	bool IsDaytime() const { return m_bIsDaytime; }

	std::unique_ptr<ShadowMap> m_pShadowMap;
	std::unique_ptr<ShadowMap> m_pTorchShadowMap;

	DirectX::BoundingSphere mSceneBounds;

	float mLightNearZ = 0.0f;
	float mLightFarZ = 0.0f;
	XMFLOAT3 mLightPosW;
	DirectX::XMFLOAT4X4 mLightView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mLightProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mTorchShadowTransform = MathHelper::Identity4x4();

	float mLightRotationAngle = 0.0f;
	XMFLOAT3 mBaseLightDirections[3] = {
		XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(0.0f, -0.707f, -0.707f)
	};
	XMFLOAT3 mRotatedLightDirections[3];

	POINT mLastMousePos;

	void UpdateShadowTransform(const XMFLOAT3& focusPoint);
	void UpdateShadowTransform();
	void UpdateTorchShadowTransform(LIGHT* pTorchLight);


	D3D12_GPU_DESCRIPTOR_HANDLE GetShadowMapSrv() { return m_pShadowMap->Srv(); }

	float m_fLightRotationAngle = 0.0f;

	XMFLOAT4 m_xmf4DaylightAmbient;
	XMFLOAT4 m_xmf4MoonlightAmbient;

	XMFLOAT4 m_xmf4DaylightDiffuse;
	XMFLOAT4 m_xmf4DaylightSpecular;

	void UpdateLights(float fTimeElapsed);


public:	// prefab
	void LoadPrefabs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void SpawnStaticObjects(const std::string& prefabName, int count, float spawnMin, float spawnMax, float scaleMin, float scaleMax,
		std::mt19937& gen, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		int matIdx = -1, const wchar_t* texFile = nullptr);
}	;
