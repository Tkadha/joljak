//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "Scene.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer()
{
	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
	//SetOBB(m_xmf3Position, playerSize, playerRotation);
	SetOBB();
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);
		if (Playerstamina > 0) {
			Playerstamina -= 2;
		}
		Move(xmf3Shift, bUpdateVelocity);
		if (checkmove == true) {
			checkmove = false;
		}
		
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (checkmove == false) {
		if (bUpdateVelocity)
		{
			m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
		}
		else
		{
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
			m_pCamera->Move(xmf3Shift);
		}
	}
	//UpdateOBB(m_xmf3Position, playerSize, playerRotation);
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	//m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	//m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	//m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	m_xmf3Position.y += 15.f;
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	m_xmf3Position.y -= 15.f;
	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
	if (Playerstamina < Maxstamina) {
		Playerstamina += 1;
	}
}

CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			pNewCamera = new CFirstPersonCamera(m_pCamera);
			break;
		case THIRD_PERSON_CAMERA:
			pNewCamera = new CThirdPersonCamera(m_pCamera);
			break;
		case SPACESHIP_CAMERA:
			pNewCamera = new CSpaceShipCamera(m_pCamera);
			break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA || nCameraMode == FIRST_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);
}
void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, bool obbRender, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA || nCameraMode == FIRST_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, obbRender, pCamera);
}

bool CPlayer::CheckCollisionOBB(CGameObject* other)
{
	return m_worldOBB.Intersects(other->m_worldOBB);
}

//void CPlayer::SetOBB(const XMFLOAT3& center, const XMFLOAT3& size, const XMFLOAT4& orientation)
//{
//	m_xmf3Position = center;
//	m_xmf3Size = size;
//
//	XMStoreFloat3(&playerObb.Center, XMLoadFloat3(&m_xmf3Position));
//	XMStoreFloat3(&playerObb.Extents, XMLoadFloat3(&m_xmf3Size));
//	XMStoreFloat4(&playerObb.Orientation, XMLoadFloat4(&orientation));
//}



void CPlayer::UpdateOBB(const XMFLOAT3& center, const XMFLOAT3& size, const XMFLOAT4& orientation)
{
	XMStoreFloat3(&playerObb.Center, XMLoadFloat3(&m_xmf3Position));
	XMStoreFloat3(&playerObb.Extents, XMLoadFloat3(&m_xmf3Size));

	XMVECTOR qRotation = XMQuaternionRotationMatrix(
		XMMatrixSet(
			m_xmf3Right.x, m_xmf3Up.x, m_xmf3Forward.x, 0.0f,
			m_xmf3Right.y, m_xmf3Up.y, m_xmf3Forward.y, 0.0f,
			m_xmf3Right.z, m_xmf3Up.z, m_xmf3Forward.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		)
	);
	XMStoreFloat4(&m_localOBB.Orientation, qRotation);
}


// 장비

void CPlayer::AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* framename, char* modelname, ResourceManager* pResourceManager)
{
	CGameObject* handFrame = FindFrame(framename);
	if (handFrame) {
		CGameObject* weapon = new CStaticObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, modelname, pResourceManager);
		weapon->SetPosition(0, 0, 0); 
		weapon->SetScale(1, 1, 1);
		weapon->Rotate(0.0f, 0.0f, 0.0f);

		handFrame->SetChild(weapon);
		UpdateTransform(nullptr); // 변환 행렬 즉시 갱신
	}
}
void CPlayer::AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* framename, char* modelname,ResourceManager* pResourceManager, XMFLOAT3 offset, XMFLOAT3 rotate = {0,0,0}, XMFLOAT3 scale = { 1,1,1 })
{
	CGameObject* handFrame = FindFrame(framename);
	if (handFrame) {
		CGameObject* weapon = new CStaticObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, modelname, pResourceManager);
		weapon->SetPosition(offset);
		weapon->SetScale(scale.x, scale.y, scale.z);
		weapon->Rotate(rotate.x, rotate.y, rotate.z);

		handFrame->SetChild(weapon);
		UpdateTransform(nullptr); // 변환 행렬 즉시 갱신
	}
}


CGameObject* CPlayer::FindFrame(char* framename)
{
	if (strcmp(m_pstrFrameName, framename) == 0) return this;
	if (m_pChild) {
		CGameObject* found = m_pChild->FindFrame(framename);
		if (found) return found;
	}
	if (m_pSibling) {
		CGameObject* found = m_pSibling->FindFrame(framename);
		if (found) return found;
	}
	return nullptr;
}



CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext, ResourceManager* pResourceManager)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CLoadedModelInfo *pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList,
		pd3dGraphicsRootSignature, "Model/SK_Hu_M_FullBody.bin", NULL, pResourceManager);
	SetChild(pAngrybotModel->m_pModelRootObject, true);

	AddObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "thumb_02_r", "Model/Sword_01.bin", pResourceManager, XMFLOAT3(0.05, 0.00, -0.05));
	AddObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Helmet", "Model/Hair_01.bin", pResourceManager, XMFLOAT3(0, 0.1, 0));
	//AddWeapon(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Boots_Peasant_Armor", "Model/Boots_Peasant_Armor.bin");
	AddObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "spine_01", "Model/Torso_Peasant_03_Armor.bin", pResourceManager, XMFLOAT3(-0.25, 0.05, 0), XMFLOAT3(90, 0, 90), XMFLOAT3(1.0, 1.5, 1.0));
	//AddObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "spine_03", "Model/Torso_Peasant_03_Armor.bin", XMFLOAT3(0, 0, 0), XMFLOAT3(90, 0, 90));
	
	
	SetGravity(DirectX::XMFLOAT3(0.0f, -9.8f * 10.0f, 0.0f)); // 중력 설정 (스케일 조절 필요 시)
	SetMaxVelocityXZ(40.0f); // 최대 수평 속도
	SetMaxVelocityY(40.0f);  // 최대 수직 속도 (낙하 속도)
	SetFriction(5.0f);       // 이동 중 마찰
	SetStopFriction(100.0f);  // 정지 시 마찰 (이동 중 마찰보다 훨씬 크게)
	m_fStaminaRegenRate = 8.0f; // 스태미나 회복률
	m_fStaminaMoveCost = 15.0f; // 스태미나 이동 소모율
	

	int nAnimation{10};
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimation, pAngrybotModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	for (int i = 1; i < nAnimation; ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}

	m_pSkinnedAnimationController->SetCallbackKeys(2, 2);
#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else
	m_pSkinnedAnimationController->SetCallbackKey(2, 0, 0.2f, _T("Sound/Footstep01.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 1, 0.5f, _T("Sound/Footstep02.wav"));
//	m_pSkinnedAnimationController->SetCallbackKey(1, 2, 0.39f, _T("Sound/Footstep03.wav"));
#endif
	CAnimationCallbackHandler *pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;
	SetPosition(XMFLOAT3(1500.0f, pTerrain->GetHeight(1500.0f, 1500.0f), 1500.0f));
	SetScale(XMFLOAT3(10.0f, 10.0f, 10.0f));

	m_pCamera->Move(XMFLOAT3(1500.0f, pTerrain->GetHeight(1500.0f, 1500.0f) + 20.0f, 1500.0f));
	if (pAngrybotModel) delete pAngrybotModel;
}

CTerrainPlayer::~CTerrainPlayer()
{
}

CCamera *CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:  // ���� ���� ī�޶�
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));  // �÷��̾� �Ӹ� ����
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(125.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case THIRD_PERSON_CAMERA:
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -30.0f));		// 카占쌨띰옙 占쏙옙치 占쏙옙占쏙옙
			m_pCamera->GenerateProjectionMatrix(1.01f, 20000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case TOP_VIEW_CAMERA:  // ž�� ī�޶�
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));  // �߷� ����
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(TOP_VIEW_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 100.0f, 0.0f));  // �÷��̾� �� 100 ����
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		default:
			break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}
}

void CTerrainPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad) + 5.0f;
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}

void CTerrainPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection) {
		m_pSkinnedAnimationController->SetTrackEnable(nAni, false);
		bAction = false;
	}
	if (dwDirection == DIR_FORWARD)
	{
		nAni = 1;
	}
	else if (dwDirection == DIR_LEFT)
	{
		nAni = 2;
	}
	else if (dwDirection == DIR_RIGHT)
	{
		nAni = 3;
	}
	else if (dwDirection == DIR_BACKWARD)
	{
		nAni = 4;
	}
	m_pSkinnedAnimationController->SetTrackEnable(nAni, true);


	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

//void CTerrainPlayer::Update(float fTimeElapsed)
//{
//	CPlayer::Update(fTimeElapsed);
//
//	if (m_pSkinnedAnimationController)
//	{
//		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
//		if (::IsZero(fLength) && nAni && !bAction)
//		{
//			m_pSkinnedAnimationController->SetTrackEnable(0, true);
//			m_pSkinnedAnimationController->SetTrackEnable(nAni, false);
//			m_pSkinnedAnimationController->SetTrackPosition(nAni, 0.0f);
//			nAni = 0;
//		}
//	}
//}

void CTerrainPlayer::Update(float fTimeElapsed)
{
	if (fTimeElapsed <= 0.0001f) return;

	// 1. 중력 적용 (시간에 따른 속도 변화)
	DirectX::XMFLOAT3 gravityEffect = Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false);
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, gravityEffect);

	// 2. 마찰/감속 적용
	float fHorizontalSpeed = Vector3::LengthXY(m_xmf3Velocity);
	if (fHorizontalSpeed > 0.0f)
	{
		float fFrictionCoefficient = m_bIsMovingInputActive ? m_fFriction : m_fStopFriction;
		float fDeceleration = fFrictionCoefficient * fTimeElapsed;

		if (fDeceleration > fHorizontalSpeed)
		{
			fDeceleration = fHorizontalSpeed;
		}

		DirectX::XMFLOAT3 xmf3HorizontalVelocity = DirectX::XMFLOAT3(m_xmf3Velocity.x, 0.0f, m_xmf3Velocity.z);
		DirectX::XMFLOAT3 xmf3DecelDirection = Vector3::Normalize(xmf3HorizontalVelocity);
		DirectX::XMFLOAT3 xmf3DecelVector = Vector3::ScalarProduct(xmf3DecelDirection, -fDeceleration, false);

		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3DecelVector);
	}

	// 3. 속도 제한
	float fLengthXZ = Vector3::LengthXY(m_xmf3Velocity);
	if (fLengthXZ > m_fMaxVelocityXZ)
	{
		float fScale = m_fMaxVelocityXZ / fLengthXZ;
		m_xmf3Velocity.x *= fScale;
		m_xmf3Velocity.z *= fScale;
	}
	if (fabsf(m_xmf3Velocity.y) > m_fMaxVelocityY)
	{
		m_xmf3Velocity.y = copysignf(m_fMaxVelocityY, m_xmf3Velocity.y);
	}

	// 4. 위치 업데이트
	DirectX::XMFLOAT3 xmf3Displacement = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Displacement);

	// OBB 업데이트 (쿼터니언 회전값 사용 필요 시 m_qRotation 같은 변수 사용)
	// 현재는 playerRotation 멤버를 사용
	UpdateOBB(m_xmf3Position, playerSize, playerRotation); // playerRotation 업데이트 로직 필요 시 추가

	// 5. 카메라 이동
	if (m_pCamera) {
		m_pCamera->Move(xmf3Displacement);
	}

	// 6. 콜백 및 카메라 시점 업데이트
	OnPlayerUpdateCallback(fTimeElapsed); // 오버라이딩된 함수 호출

	if (m_pCamera) {
		DWORD nCurrentCameraMode = m_pCamera->GetMode();
		if (nCurrentCameraMode == THIRD_PERSON_CAMERA) { // THIRD_PERSON_CAMERA 값 정의 필요
			m_pCamera->Update(m_xmf3Position, fTimeElapsed);
		}
		OnCameraUpdateCallback(fTimeElapsed); // 오버라이딩된 함수 호출

		DirectX::XMFLOAT3 lookAtPos = m_xmf3Position;
		// float m_fCameraLookAtHeightOffset = 15.0f; // 오프셋 값 필요 시 CPlayer에 멤버로 추가
		lookAtPos.y += 15.0f; // 카메라 시점 높이 조절
		if (nCurrentCameraMode == THIRD_PERSON_CAMERA) {
			m_pCamera->SetLookAt(lookAtPos);
		}
		m_pCamera->RegenerateViewMatrix();
	}

	// 7. 스태미나 회복
	if (Playerstamina < Maxstamina) {
		Playerstamina += m_fStaminaRegenRate * fTimeElapsed;
		if (Playerstamina > Maxstamina) Playerstamina = Maxstamina;
	}

	// --- 추가된 애니메이션 전환 로직 ---
// 9. 속도 기반 애니메이션 전환 (Idle 상태로)
// m_pSkinnedAnimationController 가 CGameObject 또는 CPlayer의 멤버라고 가정
	if (m_pSkinnedAnimationController) // 애니메이션 컨트롤러가 유효한지 확인
	{
		// 현재 수평 속도 계산 (마찰/감속 적용 후)
		// float fCurrentHorizontalSpeed = Vector3::LengthXY(m_xmf3Velocity); // 이미 위에서 계산됨 (fHorizontalSpeed)

		// 속도가 거의 0이고, 현재 애니메이션이 Idle(0)이 아니며, 특별한 액션 중이 아닐 때
		if (IsZero(fHorizontalSpeed) && nAni != 0 && !bAction)
		{
			// Idle 애니메이션(트랙 0) 활성화
			m_pSkinnedAnimationController->SetTrackEnable(0, true);
			// 현재 재생 중이던 애니메이션(트랙 nAni) 비활성화
			m_pSkinnedAnimationController->SetTrackEnable(nAni, false);
			// 비활성화된 트랙의 재생 위치 초기화 (선택 사항)
			m_pSkinnedAnimationController->SetTrackPosition(nAni, 0.0f);
			// 현재 애니메이션 상태를 Idle(0)로 설정
			nAni = 0;
		}
		// (선택 사항) 만약 걷거나 뛰는 애니메이션 전환 로직도 여기에 넣는다면:
		// else if (!IsNearlyZero(fCurrentHorizontalSpeed) && nAni == 0 && !bAction) {
		//     // 걷는 애니메이션 (트랙 1 가정) 시작
		//     m_pSkinnedAnimationController->SetTrackEnable(0, false); // Idle 비활성화
		//     m_pSkinnedAnimationController->SetTrackEnable(1, true);  // Walk 활성화
		//     nAni = 1;
		// }
	}

// 9. 이동 입력 상태 초기화 (Update 마지막)
	m_bIsMovingInputActive = false;
}

void CTerrainPlayer::keyInput(UCHAR* keys)
{
	float fSpeed = PlayerSpeed; // PlayerSpeed 멤버 사용 (조정 필요 시 스케일 적용)

	bool bMoved = false; // 이번 입력 처리에서 이동이 있었는지 체크
	int previousAni = nAni;     // 입력 처리 전의 애니메이션 상태 저장
	bool isActionKeyPressed = false; // F 또는 Space 키가 눌렸는지 여부


	// ---- 1. 액션 키 입력 처리 ('F', Space) ----
// 다른 키보다 액션 키를 먼저 처리하여 상태를 결정할 수 있습니다.
	if (keys['F'] & 0x80) { // 'F' 키가 눌려있는지 확인 (0x80: 현재 키 눌림 상태)
		// F 액션 (예: 공격, 상호작용 등)
		nAni = 5; // F 액션에 해당하는 애니메이션 번호
		bAction = true; // 액션 중 상태로 설정
		isActionKeyPressed = true;
		PerformActionInteractionCheck();
	}
	else if (keys[VK_SPACE] & 0x80) { // 'Space' 키가 눌려있는지 확인 (F가 안 눌렸을 때만 체크)
		// Space 액션 (예: 점프 준비, 구르기 등)
		nAni = 6; // Space 액션에 해당하는 애니메이션 번호
		bAction = true; // 액션 중 상태로 설정
		isActionKeyPressed = true;
		// 만약 점프 로직이라면, 여기서 점프 시작 처리를 할 수 있습니다.
		// Jump();
	}
	else {
		// 'F'와 'Space' 키가 모두 눌려있지 않으면 액션 상태 해제
		// (주의: 다른 요인으로 bAction이 true가 될 수 있다면 이 부분 수정 필요)
		if (bAction && (previousAni == 5 || previousAni == 6)) { // F 또는 Space로 인한 액션이었을 경우만 해제
			bAction = false;
		}
		// bAction = false; // 단순하게 처리할 수도 있음
	}

	//// 키 입력에 따라 Move 함수 호출 및 상태 변경
	//if (!bAction) {
	//	if (keys[VK_UP] || keys['W']) {
	//		Move(DIR_FORWARD, fSpeed); // Move 함수 내부에서 SetMovingInputActive(true) 호출됨
	//		bMoved = true;
	//	}
	//	if (keys[VK_DOWN] || keys['S']) {
	//		Move(DIR_BACKWARD, fSpeed);
	//		bMoved = true;
	//	}
	//	if (keys[VK_LEFT] || keys['A']) {
	//		// 캐릭터 회전 로직 (필요 시)
	//		// Rotate(0.0f, -rotationSpeed * fTimeElapsed, 0.0f);
	//		Move(DIR_LEFT, fSpeed); // 왼쪽 '이동'만 처리 (스트레이핑)
	//		bMoved = true;
	//	}
	//	if (keys[VK_RIGHT] || keys['D']) {
	//		// 캐릭터 회전 로직 (필요 시)
	//		// Rotate(0.0f, rotationSpeed * fTimeElapsed, 0.0f);
	//		Move(DIR_RIGHT, fSpeed); // 오른쪽 '이동'만 처리 (스트레이핑)
	//		bMoved = true;
	//	}
	//	// 점프 등 다른 키 입력 처리...
	//	// if (keys[VK_SPACE]) { /* Jump(); */ }
	//}

	 // ---- 4. 애니메이션 트랙 업데이트 (상태 변경 시) ----
	if (previousAni != nAni && m_pSkinnedAnimationController) {
		// 이전 애니메이션 비활성화
		m_pSkinnedAnimationController->SetTrackEnable(previousAni, false);

		// 새 애니메이션 활성화
		m_pSkinnedAnimationController->SetTrackEnable(nAni, true);
	}

	// 다른 키 입력 처리 (액션 등)
	if (keys['F']) {
		// 상호작용 시도
	}

	// ... 기타 키 입력 처리 ...
}

// 'F' 키 액션 시 호출될 충돌/상호작용 검사 함수
void CPlayer::PerformActionInteractionCheck() {
	if (!m_pScene) {
		return;
	}

	m_pScene->CheckPlayerInteraction(this);

}









///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CAirplanePlayer::CAirplanePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	//m_pCamera = ChangeCamera(/*SPACESHIP_CAMERA*/THIRD_PERSON_CAMERA, 0.0f);

	////CLoadedModelInfo* pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Mi24.bin", NULL);
	////SetChild(pModel->m_pModelRootObject, true);

	//OnPrepareAnimate();

	//CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//if (pModel) delete pModel;
}

CAirplanePlayer::~CAirplanePlayer()
{
}

void CAirplanePlayer::OnPrepareAnimate()
{
	m_pMainRotorFrame = FindFrame("Top_Rotor");
	m_pTailRotorFrame = FindFrame("Tail_Rotor");
}

void CAirplanePlayer::Animate(float fTimeElapsed)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4ToParent);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4ToParent);
	}

	CPlayer::Animate(fTimeElapsed);
}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

CCamera* CAirplanePlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(2.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(2.5f);
		SetMaxVelocityY(40.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(100.5f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(40.0f);
		SetMaxVelocityY(40.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(20.5f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(25.5f);
		SetMaxVelocityY(20.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 15.0f, -30.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
#define _WITH_DEBUG_CALLBACK_DATA

void CSoundCallbackHandler::HandleCallback(void* pCallbackData, float fTrackPosition)
{
	_TCHAR* pWavName = (_TCHAR*)pCallbackData;
#ifdef _WITH_DEBUG_CALLBACK_DATA
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s(%f)\n"), pWavName, fTrackPosition);
	OutputDebugString(pstrDebug);
#endif
#ifdef _WITH_SOUND_RESOURCE
	PlaySound(pWavName, ::ghAppInstance, SND_RESOURCE | SND_ASYNC);
#else
	PlaySound(pWavName, NULL, SND_FILENAME | SND_ASYNC);
#endif
}
