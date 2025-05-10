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
#include <unordered_map>
#include <mutex>

#define MAX_LIGHTS						16 

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

class CGameFramework;


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
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};										
										
struct LIGHTS							
{										
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

struct VS_VB_INSTANCE
{
	XMFLOAT4X4 m_xmf4x4Transform;
};

class CScene
{
public:
    CScene(CGameFramework*);
    ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	bool ProcessInput(UCHAR *pKeysBuffer);
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



	CPlayer* GetPlayerInfo() { return m_pPlayer; };
	CPlayer								*m_pPlayer = NULL;
	
public:
	float								m_fElapsedTime = 0.0f;

	vector<CGameObject*>				m_vGameObjects{};
	std::unordered_map<std::string, CGameObject*> m_mapBuildPrefabs;
	CPineObject* m_pPreviewPine = nullptr;


	//int									m_nHierarchicalGameObjects = 0;
	//CGameObject							**m_ppHierarchicalGameObjects = NULL;

	XMFLOAT3							m_xmf3RotatePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	CSkyBox								*m_pSkyBox = NULL;
	CHeightMapTerrain					*m_pTerrain = NULL;

	LIGHT								*m_pLights = NULL;
	int									m_nLights = 0;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;

	// 인스턴싱
	ID3D12Resource* m_pd3dcbGameObjects = NULL;
	VS_VB_INSTANCE* m_pcbMappedGameObjects = NULL;

	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;

	std::vector<tree_obj>				tree_objects;
	Octree								octree{ XMFLOAT3 {0,0,0}, XMFLOAT3{10200,6000,10200} };
	bool								checkTree = false;
	bool								checkRock = false;


	std::unordered_map<ULONGLONG, std::unique_ptr<UserObject>> PlayerList;	// 오브젝트 수정해야함

	CGameFramework* m_pGameFramework;



	ID3D12RootSignature* m_pCurrentRootSignature = nullptr;
	ID3D12PipelineState* m_pCurrentPSO = nullptr;
	CShader* m_pCurrentShader = nullptr;
};
