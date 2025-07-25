//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Material.h"
#include "Animation.h"
#include "Camera.h"
#include "FSMManager.h"
#include "ResourceManager.h"
#include "ShaderManager.h"


#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

class CShader;
class CStandardShader;
class COBBShader;
class CScene;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
enum class GameObjectType : int {
	Unknown,
	Rock,
	Tree,
	Cliff,
	Vegetation,
	Item,
	Cow,
	Pig,
	Player,
	Terrain,
	Toad,
	Wasp,
	Wolf,
	Bat,
	Snake,
	Turtle,
	Snail,
	Spider,
	Raptor,
	Golem

};

enum class ANIMATION_TYPE;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGameObject
{
private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	CGameObject();
	CGameObject(CGameFramework* pGameFramework);
	CGameObject(int nMaterials, CGameFramework* pGameFramework);
	virtual ~CGameObject();
	CGameObject(const CGameObject&) = delete;
	CGameObject& operator=(const CGameObject&) = delete;
public:
	char							m_pstrFrameName[64];

	CMesh							*m_pMesh = NULL;
	int m_nMeshes = 0;

	bool							isRender = true;
	int								m_nMaterials = 0;
	CMaterial						**m_ppMaterials = NULL;
	int 							m_treecount{};
	int								m_id{};
	int								m_anitype;
	bool							_invincible = false;

	bool							m_bIsPrefab = false; // 프리팹 원본인지 확인

	XMFLOAT4 m_xmf4DebugColor = XMFLOAT4(1, 1, 1, 1);
	// OBB
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Size;
	XMFLOAT3 m_xmf3Right, m_xmf3Up, m_xmf3Forward;
	BoundingOrientedBox m_localOBB, m_worldOBB;

	ID3D12Resource* m_pOBBVertexBuffer;
	ID3D12Resource* m_pOBBIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_OBBVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_OBBIndexBufferView;
	ID3D12Resource* m_pd3dcbOBBTransform = nullptr;
	XMFLOAT4X4* m_pcbMappedOBBTransform = nullptr;

	CMaterial* m_OBBMaterial = NULL;
	//COBBShader m_OBBShader;

	XMFLOAT4X4						m_xmf4x4ToParent;
	XMFLOAT4X4						m_xmf4x4World;

	CGameObject 					*m_pParent = NULL;
	CGameObject 					*m_pChild = NULL;
	CGameObject 					*m_pSibling = NULL;

	CAnimationController*			m_pSkinnedAnimationController = NULL;

	std::shared_ptr<FSMManager<CGameObject>> FSM_manager = NULL;
	LPVOID									terraindata = NULL;

	CGameFramework* m_pGameFramework;
	CAnimationController* m_pSharedAnimController = nullptr;
	void PropagateAnimController(CAnimationController* controller); 


	CScene* m_pScene = nullptr; 
	GameObjectType m_objectType = GameObjectType::Unknown;

	virtual void FSMUpdate() {}
	void ChangeAnimation(ANIMATION_TYPE type);


	// 그림자 렌더용 함수
	virtual void RenderShadow(ID3D12GraphicsCommandList* pd3dCommandList);


	void SetMesh(CMesh *pMesh);
	//void SetShader(CShader *pShader);
	void SetShader(int nMaterial, CShader *pShader);
	void SetMaterial(int nMaterial, CMaterial *pMaterial);

	void SetChild(CGameObject *pChild, bool bReferenceUpdate=false);

	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }

	virtual void OnPrepareAnimate() { }
	virtual void Animate(float fTimeElapsed);

	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	//virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, bool obbRender, CCamera* pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView);

	virtual void OnLateUpdate() { }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	void SetLook(XMFLOAT3 xmf3Look);
	void SetUp(XMFLOAT3 xmf3Up);
	void SetRight(XMFLOAT3 xmf3Right);


	XMFLOAT3 GetToParentPosition();
	void Move(XMFLOAT3 xmf3Offset);

	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);

	CGameObject *GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent=NULL);
	CGameObject *FindFrame(char *pstrFrameName);

	std::shared_ptr<CTexture> FindReplicatedTexture(_TCHAR* pstrTextureName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

	bool CheckCollisionOBB(CGameObject* other);
	void SetOBB(const XMFLOAT3& center, const XMFLOAT3& size, const XMFLOAT4& orientation);
	void SetOBB(float scalex, float scaley, float scalez, const XMFLOAT3& centerOffset);
	void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CShader* shader);
	BoundingOrientedBox GetOBB();
	BoundingOrientedBox GetBossOBB();
	void RenderOBB(ID3D12GraphicsCommandList* pd3dCommandList);
	void InitializeOBBResources(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void SetColor(const XMFLOAT4& color);

	virtual bool ShouldRenderOBB() const { return isRender; } 
	CMaterial* GetMaterial(int nIndex = 0) const {
		if (nIndex >= 0 && nIndex < m_nMaterials && m_ppMaterials) {
			return m_ppMaterials[nIndex];
		}
		return nullptr; 
	}
	int GetMaterialCount() const { return m_nMaterials; }

	virtual void RenderOBB(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void SetTerraindata(LPVOID pContext) {terraindata = pContext;}
	void SetOwningScene(CScene* pScene) { m_pScene = pScene; };
	void SetInvincible(bool invincible) { _invincible = invincible; }

public:
	void FindAndSetSkinnedMesh(CSkinnedMesh **ppSkinnedMeshes, int *pnSkinnedMesh);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	void SetRootMotion(bool bRootMotion) { if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetRootMotion(bRootMotion); }

	void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CGameFramework* pGameFramework);

	static void LoadAnimationFromFile(FILE *pInFile, CLoadedModelInfo *pLoadedModel);
	static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, int* pnSkinnedMeshes, CGameFramework* pGameFramework);
	static CLoadedModelInfo *LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, CGameFramework* pGameFramework);

	
	static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CGameFramework* pGameFramework);
	static CGameObject* LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName, CGameFramework* pGameFramework);

	static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);


public:
	int hp{ 20 };
	int level = 0;
	int atk = 5;

	int getHp() { return hp; }
	void setHp(int n) { hp = n; }

	void Sethp(int ghp) { hp = ghp; }
	void Decreasehp(int num) { hp -= num; }
	int Gethp() { return hp; }
	int GetAtk() { return atk; }

	void Check_attack();


	// Prefab
	void CopyDataFrom(CGameObject* pSource);
	virtual CGameObject* Clone();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, LPCTSTR pFileName,
		int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, CGameFramework* pGameFramework);
	virtual ~CHeightMapTerrain();
private:
	CHeightMapImage				*m_pHeightMapImage;

	int							m_nWidth;
	int							m_nLength;

	XMFLOAT3					m_xmf3Scale;

public:
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMapImage->GetHeight(x, z, bReverseQuad) * m_xmf3Scale.y); } //World
	XMFLOAT3 GetNormal(float x, float z) { return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
	virtual void RenderShadow(ID3D12GraphicsCommandList* pd3dCommandList);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);

	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL) override;
	void SetSkyboxIndex(int index);
	void LoadTextures(ID3D12GraphicsCommandList* cmdList, const std::vector<std::wstring>& texturePaths);
	int  GetTextureCount() const { return static_cast<int>(m_vSkyboxTextures.size()); }
	int GetCurrentTextureIndex() const { return m_nCurrentTextureIndex; }
private:
	std::vector<std::shared_ptr<CTexture>> m_vSkyboxTextures;
	int m_nCurrentTextureIndex = 0;
};


class CMonsterObject : public CGameObject
{

public:
	CMonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* pModel, int nAnimationTracks, CGameFramework* pGameFramework);
	virtual ~CMonsterObject();

	virtual void FSMUpdate()
	{
		if (FSM_manager) FSM_manager->Update();
	}
	virtual void ChangeState(std::shared_ptr<FSMState<CGameObject>> newstate)
	{
		FSM_manager->ChangeState(newstate);
	}

	CMonsterObject(CGameFramework* pGameFramework);

	virtual CGameObject* Clone() override;

	void PostCloneAnimationSetup();
};

class PlayerInput;
class UserObject : public CGameObject
{
public:
	int on_track = 0;

	UserObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* pModel, int nAnimationTracks, CGameFramework* pGameFramework);
	virtual ~UserObject();

	void ChangeAnimation(PlayerInput inputData);
	void AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* framename, char* modelname, CGameFramework* pGameFramework);
	void AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* framename, char* modelname, CGameFramework* pGameFramework, XMFLOAT3 offset, XMFLOAT3 rotate, XMFLOAT3 scale);
	CGameObject* FindFrame(char* framename);

	//UserObject(CGameFramework* pGameFramework);
	//virtual CGameObject* Clone();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CRootMotionCallbackHandler : public CAnimationCallbackHandler
{
public:
	CRootMotionCallbackHandler() { }
	~CRootMotionCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition);
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//class CEagleAnimationController : public CAnimationController
//{
//public:
//	CEagleAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel);
//	~CEagleAnimationController();
//
//	virtual void OnRootMotion(CGameObject* pRootGameObject);
//};
//
//class CEagleObject : public CGameObject
//{
//public:
//	CEagleObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks);
//	virtual ~CEagleObject();
//
//	virtual void SetPosition(float x, float y, float z);
//
//	XMFLOAT3				m_xmf3StartPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
//};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CHairObject : public CGameObject
{
public:
	CHairObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CHairObject() {};
};

class CItemObject : virtual public CGameObject
{
public:
	XMFLOAT3 m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3Gravity = XMFLOAT3(0.0f, -980.0f, 0.0f); 
	bool m_bOnGround = false;
	float m_fLifeTime = 15.0f; 
	float m_fElapsedAfterLanding = 0.0f;

	CHeightMapTerrain* m_pTerrainRef = nullptr; 

	CItemObject() { m_objectType = GameObjectType::Item; };
	CItemObject(CGameFramework* pGameFramework) : CGameObject(1, pGameFramework) { m_objectType = GameObjectType::Item; };
	virtual ~CItemObject() {};

	virtual void Animate(float fTimeElapsed) override; 
	void SetInitialVelocity(const XMFLOAT3& velocity) { m_xmf3Velocity = velocity; }
};


class CTreeObject : virtual public CGameObject
{
public:
	CTreeObject() { m_objectType = GameObjectType::Tree; };
	CTreeObject(CGameFramework* pGameFramework) : CGameObject(1, pGameFramework) { m_objectType = GameObjectType::Tree; };
	CTreeObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) {};
	virtual ~CTreeObject() {};


	void StartFalling(const XMFLOAT3& hitDirection); 
	virtual void Animate(float fTimeElapsed) override;

	bool IsFalling() const { return	m_bIsFalling; }
	bool HasFallen() const { return m_bHasFallen; }

	bool m_bIsFalling = false;       
	bool m_bHasFallen = false;       
	float m_fFallingDuration = 2.5f;  
	float m_fFallingTimer = 0.0f;     
	XMFLOAT3 m_xmf3FallingAxis;       
	float m_fCurrentFallAngle = 0.0f; 
	float m_fTargetFallAngle = XM_PIDIV2; 

	XMFLOAT4X4 m_xmf4x4InitialToParent;

	XMFLOAT3 m_xmf3RotationPivot = XMFLOAT3(0.0f, 0.0f, 0.0f);


};


class CBirchObject : public CTreeObject
{
public:
	CBirchObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CBirchObject() {}

	CBirchObject(CGameFramework* pGameFramework);
	virtual CGameObject* Clone();
};

class CWillowObject : public CTreeObject
{
public:
	CWillowObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CWillowObject() {}

	//CWillowObject(CGameFramework* pGameFramework);
	//virtual CGameObject* Clone();
};

class CPineObject : public CTreeObject
{
public:
	CPineObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CPineObject() {}

	CPineObject(CGameFramework* pGameFramework);
	virtual CGameObject* Clone();
};

class CBranchObject : public CItemObject {
public:
	CBranchObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework, CHeightMapTerrain* pTerrain);
	virtual ~CBranchObject() {};

	//CBranchObject(CGameFramework* pGameFramework);
	//virtual CGameObject* Clone();
};


// ------------------ ??------------------
class CRockObject : virtual public CGameObject
{
public:
	CRockObject() { m_objectType = GameObjectType::Rock; };
	CRockObject(CGameFramework* pGameFramework) : CGameObject(1, pGameFramework) { m_objectType = GameObjectType::Rock; };
	CRockObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) {};
	virtual ~CRockObject() {};

	void EraseRock();
};

class CRockClusterAObject : public CRockObject
{
public:
	CRockClusterAObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CRockClusterAObject() {}

	CRockClusterAObject(CGameFramework* pGameFramework);
	virtual CGameObject* Clone();
};

class CRockClusterBObject : public CRockObject
{
public:
	CRockClusterBObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CRockClusterBObject() {}

	CRockClusterBObject(CGameFramework* pGameFramework);
	virtual CGameObject* Clone();
};

class CRockClusterCObject : public CRockObject
{
public:
	CRockClusterCObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CRockClusterCObject() {}

	CRockClusterCObject(CGameFramework* pGameFramework);
	virtual CGameObject* Clone();
};

class CCliffFObject : public CGameObject
{
public:
	CCliffFObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CCliffFObject() {}

	CCliffFObject(CGameFramework* pGameFramework);
	virtual CGameObject* Clone();
};

class CRockDropObject : public CItemObject {
public:
	CRockDropObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework, CHeightMapTerrain* pTerrain);
	virtual ~CRockDropObject() {};

	CRockDropObject(CGameFramework* pGameFramework);
	virtual CGameObject* Clone();

};


// ------------------ ?? ?? ------------------
class VegetationObject : virtual public CGameObject
{
public:
	VegetationObject() {};
	VegetationObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) {};
	virtual ~VegetationObject() {};
};

class CBushAObject : public VegetationObject
{
public:
	CBushAObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CBushAObject() {}

	CBushAObject(CGameFramework* pGameFramework);
	virtual CGameObject* Clone();
};


class CSwordObject : public CGameObject
{
public:
	CSwordObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CSwordObject() {}
};

class CStaticObject : public CGameObject
{
public:
	CStaticObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* modelname, CGameFramework* pGameFramework);
	virtual ~CStaticObject() {}

	CStaticObject(CGameFramework* pGameFramework);
	virtual CGameObject* Clone();
};



class CRockShardEffect : public CGameObject
{
public:
	CRockShardEffect(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* framework);

	void Activate(const XMFLOAT3& position, const XMFLOAT3& velocity);
	void Update(float deltaTime);
	bool IsActive() const { return m_bActive; }

	CRockShardEffect(CGameFramework* pGameFramework);
	virtual CGameObject* Clone();
private:
	bool m_bActive = false;
	float m_fElapsedTime = 0.0f;
	float m_fLifeTime = 2.0f;
	XMFLOAT3 m_vVelocity = { 0, 0, 0 };
};

class CAttackEffectObject : public CGameObject
{
public:
	
	CAttackEffectObject(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* framework);
	virtual ~CAttackEffectObject() {}

	
	void Activate(const XMFLOAT3& position, float lifeTime = 0.5f);

	
	virtual void Animate(float fTimeElapsed) override;

private:
	bool  m_bIsActive = false;    
	float m_fLifeTime = 0.5f;     // 이펙트가 유지될 시간
	float m_fElapsedTime = 0.0f;  // 활성화된 후 지난 시간
};

class CResourceShardEffect : public CGameObject
{
public:
	CResourceShardEffect(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* framework, CMesh* pSharedMesh, CMaterial* pSharedMaterial);
	virtual ~CResourceShardEffect() {}

	void Activate(const XMFLOAT3& position, const XMFLOAT3& velocity);
	virtual void Animate(float fTimeElapsed) override;

private:
	bool     m_bIsActive = false;
	float    m_fLifeTime = 2.0f;     // 파편 유지 시간
	float    m_fElapsedTime = 0.0f;
	XMFLOAT3 m_xmf3Velocity = { 0,0,0 };
	XMFLOAT3 m_xmf3Gravity = { 0, -9800.0f, 0 }; // 중력
};