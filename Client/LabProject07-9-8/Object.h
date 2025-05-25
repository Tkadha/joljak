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
	Spider

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
	int								m_id{}; // server?êÏÑú Í¥ÄÎ¶¨Ìïò??Í∞ùÏ≤¥ Í≥†Ïú† id
	int								m_anitype;

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
	// OBB ËπÇ¬Ä????∞Ï†π???Í≥∏Îãî Ë∏∞Íæ™??
	ID3D12Resource* m_pd3dcbOBBTransform = nullptr;
	XMFLOAT4X4* m_pcbMappedOBBTransform = nullptr; // ÔßçÎìØÎ∏???????

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

	// Ë´õÎ∂æ???¥—ä‚Ä?Î®?Ωå ?®Íæ©Îß??¥—ä‚Ä?Ôß£ÏÑé?ÅÁëú??Íæ™Îπê ?Íæ©ÏäÇ
	CAnimationController* m_pSharedAnimController = nullptr;
	void PropagateAnimController(CAnimationController* controller); 


	CScene* m_pScene = nullptr; // ?Î®?ñä????????Scene ?????
	GameObjectType m_objectType = GameObjectType::Unknown;

	virtual void FSMUpdate() {}
	void ChangeAnimation(ANIMATION_TYPE type);

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
	void RenderOBB(ID3D12GraphicsCommandList* pd3dCommandList);
	void InitializeOBBResources(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void SetColor(const XMFLOAT4& color);

	virtual bool ShouldRenderOBB() const { return isRender; } // Í∏∞Î≥∏?ÅÏúºÎ°??åÎçîÎß??Ä?ÅÏù¥Î©?OBB??Í∑∏Î¶º (?ÑÏöî???∞Îùº ?òÏ†ï)


	// --- ?¨Ïßà ?ëÍ∑º??Ï∂îÍ? ---
	CMaterial* GetMaterial(int nIndex = 0) const {
		// ?Î™ÉÎú≥??Ë∏∞Î∂ø??Ë´???????Ï¢èÏäö??ÂØÉ¬Ä??
		if (nIndex >= 0 && nIndex < m_nMaterials && m_ppMaterials) {
			return m_ppMaterials[nIndex];
		}
		return nullptr; // ?Ï¢èÏäö??? ??ÜÏëùÔß?nullptr Ë´õÏÑë??
	}
	int GetMaterialCount() const { return m_nMaterials; }

	// --- OBB ???úëÔß???•Îãî ?Ï¢éÎºµ ---
	virtual void RenderOBB(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void SetTerraindata(LPVOID pContext) {terraindata = pContext;}
	void SetOwningScene(CScene* pScene) { m_pScene = pScene; };

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

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL) override;
};


class CMonsterObject : public CGameObject
{
	int _level = 0;
	int _hp = 20;
	int _atk = 3;
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
	void Sethp(int hp) { _hp = hp; }
	void Decreasehp(int num) { _hp -= num; }
	int Gethp() { return _hp; }
	int GetAtk() { return _atk; }
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
	XMFLOAT3 m_xmf3Gravity = XMFLOAT3(0.0f, -980.0f, 0.0f); // ‰ª•Î¨ê??Â™õ¬Ä??æÎ£Ñ (Ë≠∞Í≥ó???Íæ©ÏäÇ)
	bool m_bOnGround = false;
	float m_fLifeTime = 15.0f; // Ë´õÎ∂æ?????•Îº±Ôß??????™Ôßû?Êπ≤Í≥å?¥Ôßû? ??ìÏªô (??
	float m_fElapsedAfterLanding = 0.0f;

	CHeightMapTerrain* m_pTerrainRef = nullptr; // Ôßû¬Ä??Ôß°Î™Ñ??(?∞‚ë∏Î£?Â™õÎ®Ø???

	CItemObject() { m_objectType = GameObjectType::Item; };
	virtual ~CItemObject() {};

	virtual void Animate(float fTimeElapsed) override; // ?æÏá∞??Ë´???éÏ±∏ Ôß£ÏÑé??
	void SetInitialVelocity(const XMFLOAT3& velocity) { m_xmf3Velocity = velocity; }
};


// ------------------ ??é–?------------------
class CTreeObject : virtual public CGameObject
{
	int hp{ 30 };
public:



	CTreeObject() { m_objectType = GameObjectType::Tree; };
	CTreeObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) {};
	virtual ~CTreeObject() {};

	int getHp() { return hp; }
	void setHp(int n) { hp = n; }

// ------- ?Í≥ïÏú≠Ôßû¬Ä???Ï¢äÎï≤ÔßéÎ∂ø???-------

	// ?Í≥ïÏú≠Ôßû¬Ä???Ï¢äÎï≤ÔßéÎ∂ø?????ñÏòâ ??•Îãî
	void StartFalling(const XMFLOAT3& hitDirection); // ???†Ö??ÅÎº±???®Îì¶Í∫?Ë´õ‚ë∫Îº??ÍπÜÏì£ Ë´õÏèÜ??????âÏì¨

	// Ôß??Íæ®Ï†Ö???Ï¢äÎï≤ÔßéÎ∂ø?????ÖÎú≤??ÑÎìÉ
	virtual void Animate(float fTimeElapsed) override;

	bool IsFalling() const { return	m_bIsFalling; }
	bool HasFallen() const { return m_bHasFallen; }

	bool m_bIsFalling = false;       // ?Íæ©Ïò± ?Í≥ïÏú≠Ôßû¬Ä???Ï¢äÎï≤ÔßéÎ∂ø???‰ª•Î¨í?§Â™õ??
	bool m_bHasFallen = false;       // ??Ä? ?Í≥ïÏú≠Ôß??Í≥πÍπ≠?Î©??
	float m_fFallingDuration = 2.5f;  // ?Í≥ïÏú≠Ôßû¬Ä????Â´ÑÎ™É?????ìÏªô (??
	float m_fFallingTimer = 0.0f;     // ?Í≥ïÏú≠Ôßû¬ÄÊπ???ñÏòâ????Ôßû¬Ä????ìÏªô
	XMFLOAT3 m_xmf3FallingAxis;       // ???üæ ??(?Í≥ïÏú≠Ôßû¬Ä??Ë´õ‚ë∫Îº?ÂØÉÍ≥ó??
	float m_fCurrentFallAngle = 0.0f; // ?Íæ©Ïò±Ê∫êÎöØ? ???üæ??Â™õÍ≥∑Î£?
	float m_fTargetFallAngle = XM_PIDIV2; // Ôßè‚ë∫Î™????üæ Â™õÍ≥∑Î£?(90??

	// ?Í≥ïÏú≠Ôßû¬ÄÊπ???ñÏòâ?????ìΩ ?•ÎçáÎ¶?m_xmf4x4ToParent Â™õÎ???????(?Í≥? ËπÇ¬Ä??Êπ≤Í≥ó?)
	XMFLOAT4X4 m_xmf4x4InitialToParent;

	// ???üæ??‰ª•Î¨í???(??é–?Ë´õÎ¨êÎ£??∫¬Ä?? Êø°ÏíñÎ∫??´Îö∞Î™¥ÊÄ?Êπ≤Í≥ó?)
	// Ôßè‚ë§????Î®?†è????Ä? Ë´õÎ¨êÎ£??Ä?™Ôßé?(0,0,0) ????Â™õ¬Ä??
	XMFLOAT3 m_xmf3RotationPivot = XMFLOAT3(0.0f, 0.0f, 0.0f);
};


class CBirchObject : public CTreeObject
{
public:
	CBirchObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CBirchObject() {}
};

class CWillowObject : public CTreeObject
{
public:
	CWillowObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CWillowObject() {}
};

class CPineObject : public CTreeObject
{
public:
	CPineObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CPineObject() {}
};

// ??éÏ∂™Â™õ¬ÄÔßû¬Ä(??ï‚àº ?Íæ©Ïî†??
class CBranchObject : public CItemObject {
public:
	CBranchObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework, CHeightMapTerrain* pTerrain);
	virtual ~CBranchObject() {};
};


// ------------------ ??------------------
class CRockObject : virtual public CGameObject
{
	int hp{ 30 };
public:
	CRockObject() { m_objectType = GameObjectType::Rock; };
	CRockObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) {};
	virtual ~CRockObject() {};

	int getHp() { return hp; }
	void setHp(int n) { hp = n; }

	void EraseRock();
};

class CRockClusterAObject : public CRockObject
{
public:
	CRockClusterAObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CRockClusterAObject() {}
};

class CRockClusterBObject : public CRockObject
{
public:
	CRockClusterBObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CRockClusterBObject() {}
};

class CRockClusterCObject : public CRockObject
{
public:
	CRockClusterCObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CRockClusterCObject() {}
};

class CCliffFObject : public CGameObject
{
public:
	CCliffFObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CCliffFObject() {}
};

// ????∞Î†™(??ï‚àº ?Íæ©Ïî†??
class CRockDropObject : public CItemObject {
public:
	CRockDropObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework, CHeightMapTerrain* pTerrain);
	virtual ~CRockDropObject() {};

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
	CStaticObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* modelname, CGameFramework* pGameFramework);
	virtual ~CStaticObject() {}
};

class CConstructionObject : public CGameObject
{
public:
	CConstructionObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
	virtual ~CConstructionObject() {}
};