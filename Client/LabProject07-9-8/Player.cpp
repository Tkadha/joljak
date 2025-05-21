//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "Scene.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer(CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
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
	UpdateOBB(m_xmf3Position, playerSize, playerRotation);
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
//void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, bool obbRender, CCamera* pCamera)
//{
//	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
//	if (nCameraMode == THIRD_PERSON_CAMERA || nCameraMode == FIRST_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, obbRender, pCamera);
//}

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

CGameObject* CPlayer::AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* framename, char* modelname, CGameFramework* pGameFramework)
{
	CGameObject* handFrame = FindFrame(framename);
	if (handFrame) {
		CGameObject* weapon = new CStaticObject(pd3dDevice, pd3dCommandList, modelname, pGameFramework);
		weapon->SetPosition(0, 0, 0); 
		weapon->SetScale(1, 1, 1);
		weapon->Rotate(0.0f, 0.0f, 0.0f);

		handFrame->SetChild(weapon);
		UpdateTransform(nullptr); // 변환 행렬 즉시 갱신

		return weapon;
	}

	return nullptr;
}
CGameObject* CPlayer::AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* framename, char* modelname, CGameFramework* pGameFramework, XMFLOAT3 offset, XMFLOAT3 rotate = {0,0,0}, XMFLOAT3 scale = { 1,1,1 })
{
	CGameObject* handFrame = FindFrame(framename);
	if (handFrame) {
		CGameObject* weapon = new CStaticObject(pd3dDevice, pd3dCommandList, modelname, pGameFramework);
		weapon->SetPosition(offset);
		weapon->SetScale(scale.x, scale.y, scale.z);
		weapon->Rotate(rotate.x, rotate.y, rotate.z);

		handFrame->SetChild(weapon);
		UpdateTransform(nullptr); // 변환 행렬 즉시 갱신

		return weapon;
	}
	
	return nullptr;
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


CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext, CGameFramework* pGameFramework) : CPlayer(pGameFramework)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CLoadedModelInfo *pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList,
	"Model/SK_Hu_M_FullBody.bin", pGameFramework);
	SetChild(pAngrybotModel->m_pModelRootObject, true);

	m_pSword = AddObject(pd3dDevice, pd3dCommandList, "thumb_02_r", "Model/Sword_01.bin", pGameFramework, XMFLOAT3(0.05, 0.00, -0.05));
	m_pAxe = AddObject(pd3dDevice, pd3dCommandList, "thumb_01_r", "Model/Axe.bin", pGameFramework, XMFLOAT3(0.05, 0.25, -0.05), XMFLOAT3(90, 0, 00));
	weaponType = WeaponType::Sword;
	m_pSword->isRender = true;	
	m_pAxe->isRender = false;

	AddObject(pd3dDevice, pd3dCommandList, "Helmet", "Model/Hair_01.bin", pGameFramework, XMFLOAT3(0, 0.1, 0));
	//AddWeapon(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Boots_Peasant_Armor", "Model/Boots_Peasant_Armor.bin");
	AddObject(pd3dDevice, pd3dCommandList, "spine_01", "Model/Torso_Peasant_03_Armor.bin", pGameFramework, XMFLOAT3(-0.25, 0.1, 0), XMFLOAT3(90, 0, 90));
	//AddObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "spine_03", "Model/Torso_Peasant_03_Armor.bin", XMFLOAT3(0, 0, 0), XMFLOAT3(90, 0, 90));


	int nAnimation{10};
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimation, pAngrybotModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	for (int i = 1; i < nAnimation; ++i) {
		m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
		m_pSkinnedAnimationController->SetTrackEnable(i, false);
	}

	PropagateAnimController(m_pSkinnedAnimationController);

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

	m_pStateMachine = new PlayerStateMachine(this, m_pSkinnedAnimationController);

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;
	SetPosition(XMFLOAT3(10.0f, pTerrain->GetHeight(10.0f, 10.0f), 10.0f));
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
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
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
		//XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		//xmf3PlayerVelocity.y = 0.0f;
		//SetVelocity(xmf3PlayerVelocity);
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

//void CTerrainPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
//{
//	if (dwDirection) {
//		m_pSkinnedAnimationController->SetTrackEnable(nAni, false);
//		bAction = false;
//	}
//	if (dwDirection == DIR_FORWARD)
//	{
//		nAni = 1;
//	}
//	else if (dwDirection == DIR_LEFT)
//	{
//		nAni = 2;
//	}
//	else if (dwDirection == DIR_RIGHT)
//	{
//		nAni = 3;
//	}
//	else if (dwDirection == DIR_BACKWARD)
//	{
//		nAni = 4;
//	}
//	m_pSkinnedAnimationController->SetTrackEnable(nAni, true);
//
//
//	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
//}

void CTerrainPlayer::Update(float fTimeElapsed)
{
	//CPlayer::Update(fTimeElapsed);

	// --- 1. 상태 머신 업데이트 ---
	// 상태 머신이 상태 전환, 애니메이션 제어, 상태별 로직(이동 속도 설정 등)을 처리합니다.
	if (m_pStateMachine) {
		m_pStateMachine->Update(fTimeElapsed);
	}

	// --- 2. 물리 업데이트 (중력, 속도 제한 등) ---
	// 상태 머신이 설정한 속도(m_xmf3Velocity)에 중력을 적용하고 속도를 제한합니다.
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity); // 중력 적용

	// XZ 평면 속도 제한
	float fLengthXZ = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLengthXZ > m_fMaxVelocityXZ) {
		float fRatio = fMaxVelocityXZ / fLengthXZ;
		m_xmf3Velocity.x *= fRatio;
		m_xmf3Velocity.z *= fRatio;
	}

	// Y축 속도 제한
	float fLengthY = fabsf(m_xmf3Velocity.y); // 절대값으로 비교
	float fMaxVelocityY = m_fMaxVelocityY;
	if (fLengthY > m_fMaxVelocityY) {
		m_xmf3Velocity.y *= (fMaxVelocityY / fLengthY);
	}

	// --- 3. 최종 이동 적용 ---
	// 계산된 최종 속도를 기반으로 플레이어 위치를 이동시킵니다.
	XMFLOAT3 xmf3VelocityDelta = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3VelocityDelta, false); // Move 함수는 위치(m_xmf3Position)를 직접 변경

	// --- 4. 지형 충돌/높이 보정 (콜백) ---
	// 플레이어 위치가 지형 아래로 내려가지 않도록 조정합니다.
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	// OnPlayerUpdateCallback 내부에서 SetPosition, SetVelocity 등을 호출하여 위치/속도를 보정할 수 있습니다.

	// --- 5. 카메라 업데이트 ---
	// 플레이어 위치를 기반으로 카메라 위치/방향을 업데이트합니다.
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) {
		// 상태 머신에서 카메라 로직을 제어할 수도 있지만, 일단 기존 방식 유지
		m_pCamera->Update(m_xmf3Position, fTimeElapsed);
		// 카메라가 땅 아래로 내려가지 않도록 보정 (콜백)
		if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);

		// 플레이어를 바라보도록 설정 (기존 코드 약간 수정)
		XMFLOAT3 lookAtPos = m_xmf3Position;
		lookAtPos.y += 15.0f; // 플레이어 머리 위
		m_pCamera->SetLookAt(lookAtPos);
	}
	else if (nCurrentCameraMode == FIRST_PERSON_CAMERA) {
		// 1인칭 카메라는 플레이어 위치와 동일하게 업데이트
		m_pCamera->SetPosition(m_xmf3Position); // 필요시 오프셋 적용
		if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed); // 땅 밑 체크 등
	}
	m_pCamera->RegenerateViewMatrix(); // 최종 뷰 행렬 계산

	// --- 6. 마찰 적용 ---
	// 속도를 점진적으로 감소시킵니다. (Y축 속도에는 마찰 적용 안 함 가정)
	float fFriction = m_fFriction;
	float fSpeedXZ = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	if (fSpeedXZ > 0.0f) {
		float fDeceleration = fFriction * fTimeElapsed;
		if (fDeceleration > fSpeedXZ) fDeceleration = fSpeedXZ; // 현재 속도보다 더 많이 감속할 수 없음

		XMFLOAT3 xmf3ReverseVelocityXZ = Vector3::Normalize(XMFLOAT3(m_xmf3Velocity.x, 0.0f, m_xmf3Velocity.z));
		xmf3ReverseVelocityXZ = Vector3::ScalarProduct(xmf3ReverseVelocityXZ, -fDeceleration, false);

		m_xmf3Velocity.x += xmf3ReverseVelocityXZ.x;
		m_xmf3Velocity.z += xmf3ReverseVelocityXZ.z;
	}
}

void CTerrainPlayer::keyInput(UCHAR* key) {
	// --- 애니메이션 로직 (기존 로직을 바탕으로 약간 수정) ---
	bool fKeyPressed = (key['F'] & 0xF0);
	bool spacePressed = (key[VK_SPACE] & 0xF0);
	bool actionTriggered = false; // 이번 프레임에 액션이 시작되었는지 여부

	if (fKeyPressed) {
		bAction = true;
		actionTriggered = true;
		PerformActionInteractionCheck();
	}
	else if (spacePressed) {
		bAction = true;
		actionTriggered = true;
	}
	else {
		// F키나 스페이스바가 눌리지 않았을 때
		if (bAction) { // 이전에 액션(F 또는 점프) 중이었다면
			bAction = false; // 액션 상태 해제
		}
	}
}

#include "GameFramework.h"

CGameObject* CTerrainPlayer::FindObjectHitByAttack() {
	if (!m_pGameFramework) return nullptr;
	CScene* pScene = m_pGameFramework->GetScene();
	if (!pScene) return nullptr;

	BoundingOrientedBox weapon;
	if (weaponType == WeaponType::Sword)
		weapon = m_pSword->m_worldOBB;
	else if (weaponType == WeaponType::Axe)
		weapon = m_pAxe->m_worldOBB;


	for (const auto& obj : pScene->m_vGameObjects) {

		std::vector<DirectX::BoundingOrientedBox> obbList;
		pScene->CollectHierarchyObjects(obj, obbList);
		for (const auto& obb : obbList) {
			if (weapon.Intersects(obb)) return obj;
		}		
	}

	return nullptr;
}


// 'F' 키 액션 시 호출될 충돌/상호작용 검사 함수
void CPlayer::PerformActionInteractionCheck() {
	if (!m_pScene) {
		return;
	}

	m_pScene->CheckPlayerInteraction(this);

}



const PlayerInputData& CPlayer::GetStateMachineInput() const {
	if (m_pStateMachine) {
		if (m_pStateMachine) {
			// 임시로 m_LastInput 직접 접근 (friend 클래스 또는 public getter 필요)
			return m_pStateMachine->GetLastInput();
		}
	}
	static PlayerInputData defaultInput;
	return defaultInput;
}







///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CAirplanePlayer::CAirplanePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameFramework* pGameFramework, void* pContext) : CPlayer(pGameFramework)
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
