#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#include "Object.h"
#include "Camera.h"
#include <chrono>
#include "PlayerStateMachine.h" 

enum class WeaponType : int {
	Sword,
	Axe,
	Pick
};

class CPlayer : public CGameObject
{
protected:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	LPVOID						m_pPlayerUpdatedContext = NULL;
	LPVOID						m_pCameraUpdatedContext = NULL;

	BoundingOrientedBox playerObb;
	XMFLOAT3 playerSize = XMFLOAT3(4.0f, 4.0f, 4.0f); // 실제 크기의 반
	XMFLOAT4 playerRotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	CCamera						*m_pCamera = NULL;


public:
	PlayerStateMachine* m_pStateMachine = nullptr;


	CPlayer(CGameFramework* pGameFramework);
	virtual ~CPlayer();
	bool					    checkmove = false;
	float PlayerHunger = 1.0f;
	float PlayerThirst = 1.0f;
	int PlayerLevel = 1;
	int Playerhp = 300;
	int Maxhp = 300;
	int Playerstamina = 150;
	int Maxstamina = 150;
	int StatPoint = 5;
	int PlayerAttack = 10;
	float PlayerSpeed = 10.0f;
	int Playerxp = 0;
	int Totalxp = 20;
	bool invincibility = false;
	std::chrono::time_point<std::chrono::system_clock> starttime; // 무적 시작시간

	WeaponType weaponType;

	CGameObject* m_pSword;
	CGameObject* m_pAxe;
	CGameObject* m_pPick;
	
	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }

	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	virtual void Move(DWORD nDirection, float fDistance, bool bVelocity = false);
	virtual void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

	virtual void Update(float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	//virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, bool obbRender, CCamera* pCamera = NULL);

	virtual void keyInput(UCHAR* key) {};

	bool CheckCollisionOBB(CGameObject* other);
	//void SetOBB(const XMFLOAT3& center, const XMFLOAT3& size, const XMFLOAT4& orientation);
	void UpdateOBB(const XMFLOAT3& center, const XMFLOAT3& size, const XMFLOAT4& orientation);

	// 장비
	CGameObject* AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* framename, char* modelname, CGameFramework* pGameFramework);
	CGameObject* AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* framename, char* modelname, CGameFramework* pGameFramework, XMFLOAT3 offset, XMFLOAT3 rotate, XMFLOAT3 scale);
	CGameObject* FindFrame(char* framename);


	void PerformActionInteractionCheck();

	void SetInvincibility() {
		if (invincibility) invincibility = false;
		else {
			invincibility = true;
			starttime = std::chrono::system_clock::now();
		}
	}
	void DecreaseHp(int value) { Playerhp -= value; }

	const PlayerInputData& GetStateMachineInput() const;
};

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CGameFramework* pGameFramework, void *pContext=NULL);
	virtual ~CAirplanePlayer();

	CGameObject					*m_pMainRotorFrame = NULL;
	CGameObject					*m_pTailRotorFrame = NULL;

private:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed);

public:
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
};

class CSoundCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSoundCallbackHandler() { }
	~CSoundCallbackHandler() { }

public:
	virtual void HandleCallback(void *pCallbackData, float fTrackPosition); 
};

class CTerrainPlayer : public CPlayer
{
public:
	CTerrainPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext, CGameFramework* pGameFramework);
	virtual ~CTerrainPlayer();

public:
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);

	//virtual void Move(DWORD nDirection, float fDistance, bool bVelocity = false);
	
	virtual void Update(float fTimeElapsed);

	int nAni{};
	BOOL bAction = false;
	void keyInput(UCHAR* key);


	CGameObject* FindObjectHitByAttack();
	
};

