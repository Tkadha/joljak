#include "stdafx.h"
#include "Player.h"
#include "Camera.h"

CCamera::CCamera()
{
	m_xmf4x4View = Matrix4x4::Identity();
	m_xmf4x4Projection = Matrix4x4::Identity();
	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fTimeLag = 0.0f;
	m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_nMode = 0x00;
	m_pPlayer = NULL;
}

CCamera::CCamera(CCamera *pCamera)
{
	if (pCamera)
	{
		*this = *pCamera;
	}
	else
	{
		m_xmf4x4View = Matrix4x4::Identity();
		m_xmf4x4Projection = Matrix4x4::Identity();
		m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
		m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = 0.0f;
		m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_fTimeLag = 0.0f;
		m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_nMode = 0x00;
		m_pPlayer = NULL;
	}
}

CCamera::~CCamera()
{ 
}

void CCamera::SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ, float fMaxZ)
{
	m_d3dViewport.TopLeftX = float(xTopLeft);
	m_d3dViewport.TopLeftY = float(yTopLeft);
	m_d3dViewport.Width = float(nWidth);
	m_d3dViewport.Height = float(nHeight);
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;
}

void CCamera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
	m_d3dScissorRect.left = xLeft;
	m_d3dScissorRect.top = yTop;
	m_d3dScissorRect.right = xRight;
	m_d3dScissorRect.bottom = yBottom;
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle)
{
	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
//	XMMATRIX xmmtxProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
//	XMStoreFloat4x4(&m_xmf4x4Projection, xmmtxProjection);
}

void CCamera::GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up)
{
	m_xmf3Position = xmf3Position;
	m_xmf3LookAtWorld = xmf3LookAt;
	m_xmf3Up = xmf3Up;

	GenerateViewMatrix();
}

void CCamera::GenerateViewMatrix()
{
	m_xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, m_xmf3LookAtWorld, m_xmf3Up);
}

void CCamera::RegenerateViewMatrix()
{
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x;
	m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y;
	m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z;
	m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);
}

void CCamera::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); // 256??ë°°ìˆ˜

	// ë¦¬ì†Œ???ì„± ?¨ìˆ˜ ê²°ê³¼ ?•ì¸
	m_pd3dcbCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	// --- ë¦¬ì†Œ???ì„± ?•ì¸ ---
	if (!m_pd3dcbCamera) {
		//OutputDebugString(L"!!!!!!!! ERROR: Failed to create Camera Constant Buffer! !!!!!!!!\n");
		// ?¤íŒ¨ ??m_pcbMappedCamera???¹ì—°??nullptr ?íƒœ ? ì?
		return;
	}

	// ë§µí•‘ ?œë„ ë°?ê²°ê³¼ ?•ì¸
	HRESULT hResult = m_pd3dcbCamera->Map(0, NULL, (void**)&m_pcbMappedCamera);

	// --- ë§µí•‘ ?•ì¸ ---
	if (FAILED(hResult) || !m_pcbMappedCamera) {
		//OutputDebugString(L"!!!!!!!! ERROR: Failed to map Camera Constant Buffer! !!!!!!!!\n");
		m_pcbMappedCamera = nullptr; // ?ˆì „?˜ê²Œ nullptr ì²˜ë¦¬
		// ?„ìš”??m_pd3dcbCamera??Release ì²˜ë¦¬ ê³ ë ¤
	}
}



void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	// --- ì¤‘ìš”: ë§µí•‘???¬ì¸??? íš¨??ê²€??---
	if (!m_pcbMappedCamera || !m_pd3dcbCamera) {
		//OutputDebugString(L"!!!!!!!! ERROR: Camera Constant Buffer or Mapped Pointer is NULL in UpdateShaderVariables! !!!!!!!!\n");
		return; // ?…ë°?´íŠ¸ ë°?ë°”ì¸??ë¶ˆê?
	}

	// ?°ì´??ë³µì‚¬ (memcpy ?€??êµ¬ì¡°ì²?ë©¤ë²„ ì§ì ‘ ?€?…ì´ ???ˆì „?????ˆìŒ)
	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_xmf4x4View);
	XMMATRIX projMatrix = XMLoadFloat4x4(&m_xmf4x4Projection);

	// Transpose??HLSL?ì„œ ?˜í–‰?˜ê±°??C++?ì„œ ?˜í–‰ (?¼ê???? ì?)
	// HLSL?ì„œ Transpose ???œë‹¤ë©??¬ê¸°???˜í–‰
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4View, XMMatrixTranspose(viewMatrix));
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4Projection, XMMatrixTranspose(projMatrix));
	m_pcbMappedCamera->m_xmf3Position = m_xmf3Position;

	// ?ˆê°œ ?ìš©
	m_pcbMappedCamera->FogColor = m_xmf4FogColor;
	m_pcbMappedCamera->FogStart = m_fFogStart;
	m_pcbMappedCamera->FogRange = m_fFogRange;
}


void CCamera::ReleaseShaderVariables()
{
	if (m_pd3dcbCamera)
	{
		m_pd3dcbCamera->Unmap(0, NULL);
		m_pd3dcbCamera->Release();
	}
}

void CCamera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CSpaceShipCamera

CSpaceShipCamera::CSpaceShipCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = SPACESHIP_CAMERA;
}

void CSpaceShipCamera::Rotate(float x, float y, float z)
{
	if (m_pPlayer && (x != 0.0f))
	{
		XMFLOAT3 xmf3Right = m_pPlayer->GetRightVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Right), XMConvertToRadians(x));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
	}
	if (m_pPlayer && (y != 0.0f))
	{
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(y));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
	}
	if (m_pPlayer && (z != 0.0f))
	{
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look), XMConvertToRadians(z));
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CFirstPersonCamera

CFirstPersonCamera::CFirstPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = FIRST_PERSON_CAMERA;
	if (pCamera)
	{
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xmf3Right.y = 0.0f;
			m_xmf3Look.y = 0.0f;
			m_xmf3Right = Vector3::Normalize(m_xmf3Right);
			m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		}
	}
}

void CFirstPersonCamera::Rotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (m_pPlayer && (y != 0.0f))
	{
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (m_pPlayer && (z != 0.0f))
	{
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look), XMConvertToRadians(z));
		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CThirdPersonCamera

CThirdPersonCamera::CThirdPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = THIRD_PERSON_CAMERA;
	if (pCamera)
	{
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xmf3Right.y = 0.0f;
			m_xmf3Look.y = 0.0f;
			m_xmf3Right = Vector3::Normalize(m_xmf3Right);
			m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		}
	}
}

void CThirdPersonCamera::Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
	if (m_pPlayer)
	{
		XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
		XMFLOAT3 xmf3Right = m_pPlayer->GetRightVector();
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
		xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
		xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;

		XMFLOAT3 xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
		XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
		XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);
		float fLength = Vector3::Length(xmf3Direction);
		xmf3Direction = Vector3::Normalize(xmf3Direction);
		float fTimeLagScale = (m_fTimeLag) ? fTimeElapsed * (1.0f / m_fTimeLag) : 1.0f;
		float fDistance = fLength * fTimeLagScale;
		if (fDistance > fLength) fDistance = fLength;
		if (fLength < 0.01f) fDistance = fLength;
		if (fDistance > 0)
		{
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fDistance);
			SetLookAt(xmf3LookAt);
		}
	}
}

void CThirdPersonCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, m_pPlayer->GetUpVector());
	m_xmf3Right = XMFLOAT3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	m_xmf3Up = XMFLOAT3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	m_xmf3Look = XMFLOAT3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
}


//void CThirdPersonCamera::Rotate(float x, float y, float z)
//{
//	if (x != 0.0f)
//	{
//		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
//		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
//		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
//		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
//	}
//	if (m_pPlayer && (y != 0.0f))
//	{
//		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
//		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(y));
//		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
//		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
//		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
//	}
//	if (m_pPlayer && (z != 0.0f))
//	{
//		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
//		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look), XMConvertToRadians(z));
//		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
//		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
//		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());
//		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
//		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
//		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
//	}
//}

void CThirdPersonCamera::Rotate(float pitchDelta, float yawDelta, float rollDelta)
{
	// Pitch ?Œì „ (ì¹´ë©”?¼ì˜ ë¡œì»¬ Right ë²¡í„° ê¸°ì?)
	// ì¹´ë©”?¼ê? ?Œë ˆ?´ì–´ë¥?ë°”ë¼ë³´ë©´???„ì•„?˜ë¡œ ?€ì§ì´???¨ê³¼
	if (pitchDelta != 0.0f && m_pPlayer) // ?Œë ˆ?´ì–´ê°€ ?ˆì„ ?Œë§Œ
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(pitchDelta));

		// ì¹´ë©”?¼ì˜ Look, Up ë²¡í„°ë¥??Œì „
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);

		// Pitch ?Œì „ ??ì¹´ë©”???„ì¹˜???…ë°?´íŠ¸ (?Œë ˆ?´ì–´ ì¤‘ì‹¬)
		XMFLOAT3 playerPos = m_pPlayer->GetPosition();
		XMVECTOR camPosGlobal = XMLoadFloat3(&m_xmf3Position);
		XMVECTOR playerPosGlobal = XMLoadFloat3(&playerPos);

		// 1. ì¹´ë©”???„ì¹˜ë¥??Œë ˆ?´ì–´??ë¡œì»¬ ê³µê°„?¼ë¡œ ë³€??(ì¹´ë©”???„ì¹˜ - ?Œë ˆ?´ì–´ ?„ì¹˜)
		XMVECTOR relativeCamPos = camPosGlobal - playerPosGlobal;
		// 2. ë¡œì»¬ ê³µê°„?ì„œ ?Œì „ (ì¹´ë©”?¼ì˜ Right ë²¡í„° ê¸°ì?)
		relativeCamPos = XMVector3TransformCoord(relativeCamPos, xmmtxRotate);
		// 3. ?¤ì‹œ ?”ë“œ ê³µê°„?¼ë¡œ ë³€??(?Œì „???ë? ?„ì¹˜ + ?Œë ˆ?´ì–´ ?„ì¹˜)
		XMStoreFloat3(&m_xmf3Position, relativeCamPos + playerPosGlobal);
	}

	// Yaw ?Œì „ (?Œë ˆ?´ì–´???”ë“œ Up ë²¡í„° ?ëŠ” ?Œë ˆ?´ì–´??Up ë²¡í„° ê¸°ì?)
	// ì¹´ë©”?¼ê? ?Œë ˆ?´ì–´ ì£¼ìœ„ë¥??˜í‰?¼ë¡œ ?„ëŠ” ?¨ê³¼
	if (yawDelta != 0.0f && m_pPlayer) // ?Œë ˆ?´ì–´ê°€ ?ˆì„ ?Œë§Œ
	{
		XMFLOAT3 playerUp = m_pPlayer->GetUpVector(); // ?Œë ˆ?´ì–´??Up ë²¡í„° ?¬ìš©
		// ?ëŠ” ?”ë“œ Up ë²¡í„° ?¬ìš©: XMFLOAT3 playerUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&playerUp), XMConvertToRadians(yawDelta));

		// ì¹´ë©”?¼ì˜ Look, Up, Right ë²¡í„°ë¥??Œì „
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);

		XMFLOAT3 playerPos = m_pPlayer->GetPosition();
		XMVECTOR camPosGlobal = XMLoadFloat3(&m_xmf3Position);
		XMVECTOR playerPosGlobal = XMLoadFloat3(&playerPos);

		XMVECTOR relativeCamPos = camPosGlobal - playerPosGlobal;
		relativeCamPos = XMVector3TransformCoord(relativeCamPos, xmmtxRotate);
		XMStoreFloat3(&m_xmf3Position, relativeCamPos + playerPosGlobal);
	}

	if (rollDelta != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(rollDelta));
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);

	}

	XMVECTOR L = XMLoadFloat3(&m_xmf3Look);
	XMVECTOR U = XMLoadFloat3(&m_xmf3Up);
	XMVECTOR R = XMLoadFloat3(&m_xmf3Right);

	L = XMVector3Normalize(L); 

	XMFLOAT3 worldOrPlayerUpFloat = m_pPlayer ? m_pPlayer->GetUpVector() : XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR worldOrPlayerUp = XMLoadFloat3(&worldOrPlayerUpFloat);
	R = XMVector3Normalize(XMVector3Cross(worldOrPlayerUp, L)); // Right = Up x Look 


	U = XMVector3Normalize(XMVector3Cross(L, R)); // Up = Look x Right

	XMStoreFloat3(&m_xmf3Look, L);
	XMStoreFloat3(&m_xmf3Up, U);
	XMStoreFloat3(&m_xmf3Right, R);

	
	// if (m_pPlayer) {
	//     LookAt(m_pPlayer->GetPosition()); 
	// }
}




void CCamera::UpdateShadowTransform(const DirectX::XMFLOAT4X4& xmf4x4ShadowTransform)
{
	if (m_pcbMappedCamera)
	{
		XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4ShadowTransform, XMMatrixTranspose(XMLoadFloat4x4(&xmf4x4ShadowTransform)));
	}
}

void CCamera::UpdateTorchShadowTransform(const DirectX::XMFLOAT4X4& xmf4x4ShadowTransform)
{
	if (m_pcbMappedCamera)
	{
		XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4TorchShadowTransform, XMMatrixTranspose(XMLoadFloat4x4(&xmf4x4ShadowTransform)));
		//m_pcbMappedCamera->m_xmf4x4TorchShadowTransform = xmf4x4ShadowTransform;
	}
}

void CCamera::GetFrustumCorners(XMFLOAT3* pCorners) const
{
	// 1. FOV¸¦ ÀÌ¿ëÇØ Near/Far Æò¸éÀÇ ³ôÀÌ¿Í ³Êºñ¸¦ °è»êÇÕ´Ï´Ù.
	float halfFovY = XMConvertToRadians(m_fFovAngle * 0.5f);
	float nearHeight = 2.0f * m_fNearPlaneDistance * tanf(halfFovY);
	float nearWidth = nearHeight * m_fAspectRatio;
	float farHeight = 2.0f * m_fFarPlaneDistance * tanf(halfFovY);
	float farWidth = farHeight * m_fAspectRatio;

	// 2. Ä«¸Þ¶óÀÇ ·ÎÄÃ Ãà(Right, Up, Look)°ú À§Ä¡¸¦ XMVECTOR·Î ·ÎµåÇÕ´Ï´Ù.
	XMVECTOR xmvPosition = XMLoadFloat3(&m_xmf3Position);
	XMVECTOR xmvRight = XMLoadFloat3(&m_xmf3Right);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xmf3Up);
	XMVECTOR xmvLook = XMLoadFloat3(&m_xmf3Look);

	// 3. Near/Far Æò¸éÀÇ Áß½ÉÁ¡À» °è»êÇÕ´Ï´Ù.
	XMVECTOR nearPlaneCenter = xmvPosition + (xmvLook * m_fNearPlaneDistance);
	XMVECTOR farPlaneCenter = xmvPosition + (xmvLook * m_fFarPlaneDistance);

	// 4. 8°³ÀÇ ²ÀÁþÁ¡À» °è»êÇÕ´Ï´Ù.
	// Near Plane (Ä«¸Þ¶ó¿Í °¡±î¿î ¸é)
	XMStoreFloat3(&pCorners[0], nearPlaneCenter - (xmvRight * (nearWidth * 0.5f)) - (xmvUp * (nearHeight * 0.5f))); // ¿ÞÂÊ ¾Æ·¡
	XMStoreFloat3(&pCorners[1], nearPlaneCenter - (xmvRight * (nearWidth * 0.5f)) + (xmvUp * (nearHeight * 0.5f))); // ¿ÞÂÊ À§
	XMStoreFloat3(&pCorners[2], nearPlaneCenter + (xmvRight * (nearWidth * 0.5f)) + (xmvUp * (nearHeight * 0.5f))); // ¿À¸¥ÂÊ À§
	XMStoreFloat3(&pCorners[3], nearPlaneCenter + (xmvRight * (nearWidth * 0.5f)) - (xmvUp * (nearHeight * 0.5f))); // ¿À¸¥ÂÊ ¾Æ·¡

	// Far Plane (Ä«¸Þ¶ó¿Í ¸Õ ¸é)
	XMStoreFloat3(&pCorners[4], farPlaneCenter - (xmvRight * (farWidth * 0.5f)) - (xmvUp * (farHeight * 0.5f))); // ¿ÞÂÊ ¾Æ·¡
	XMStoreFloat3(&pCorners[5], farPlaneCenter - (xmvRight * (farWidth * 0.5f)) + (xmvUp * (farHeight * 0.5f))); // ¿ÞÂÊ À§
	XMStoreFloat3(&pCorners[6], farPlaneCenter + (xmvRight * (farWidth * 0.5f)) + (xmvUp * (farHeight * 0.5f))); // ¿À¸¥ÂÊ À§
	XMStoreFloat3(&pCorners[7], farPlaneCenter + (xmvRight * (farWidth * 0.5f)) - (xmvUp * (farHeight * 0.5f))); // ¿À¸¥ÂÊ ¾Æ·¡
}
