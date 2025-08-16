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
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); // 256??배수

	// 리소???�성 ?�수 결과 ?�인
	m_pd3dcbCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	// --- 리소???�성 ?�인 ---
	if (!m_pd3dcbCamera) {
		//OutputDebugString(L"!!!!!!!! ERROR: Failed to create Camera Constant Buffer! !!!!!!!!\n");
		// ?�패 ??m_pcbMappedCamera???�연??nullptr ?�태 ?��?
		return;
	}

	// 맵핑 ?�도 �?결과 ?�인
	HRESULT hResult = m_pd3dcbCamera->Map(0, NULL, (void**)&m_pcbMappedCamera);

	// --- 맵핑 ?�인 ---
	if (FAILED(hResult) || !m_pcbMappedCamera) {
		//OutputDebugString(L"!!!!!!!! ERROR: Failed to map Camera Constant Buffer! !!!!!!!!\n");
		m_pcbMappedCamera = nullptr; // ?�전?�게 nullptr 처리
		// ?�요??m_pd3dcbCamera??Release 처리 고려
	}
}



void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	// --- 중요: 맵핑???�인???�효??검??---
	if (!m_pcbMappedCamera || !m_pd3dcbCamera) {
		//OutputDebugString(L"!!!!!!!! ERROR: Camera Constant Buffer or Mapped Pointer is NULL in UpdateShaderVariables! !!!!!!!!\n");
		return; // ?�데?�트 �?바인??불�?
	}

	// ?�이??복사 (memcpy ?�??구조�?멤버 직접 ?�?�이 ???�전?????�음)
	XMMATRIX viewMatrix = XMLoadFloat4x4(&m_xmf4x4View);
	XMMATRIX projMatrix = XMLoadFloat4x4(&m_xmf4x4Projection);

	// Transpose??HLSL?�서 ?�행?�거??C++?�서 ?�행 (?��????��?)
	// HLSL?�서 Transpose ???�다�??�기???�행
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4View, XMMatrixTranspose(viewMatrix));
	XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4Projection, XMMatrixTranspose(projMatrix));
	m_pcbMappedCamera->m_xmf3Position = m_xmf3Position;

	// ?�개 ?�용
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
	// Pitch ?�전 (카메?�의 로컬 Right 벡터 기�?)
	// 카메?��? ?�레?�어�?바라보면???�아?�로 ?�직이???�과
	if (pitchDelta != 0.0f && m_pPlayer) // ?�레?�어가 ?�을 ?�만
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(pitchDelta));

		// 카메?�의 Look, Up 벡터�??�전
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);

		// Pitch ?�전 ??카메???�치???�데?�트 (?�레?�어 중심)
		XMFLOAT3 playerPos = m_pPlayer->GetPosition();
		XMVECTOR camPosGlobal = XMLoadFloat3(&m_xmf3Position);
		XMVECTOR playerPosGlobal = XMLoadFloat3(&playerPos);

		// 1. 카메???�치�??�레?�어??로컬 공간?�로 변??(카메???�치 - ?�레?�어 ?�치)
		XMVECTOR relativeCamPos = camPosGlobal - playerPosGlobal;
		// 2. 로컬 공간?�서 ?�전 (카메?�의 Right 벡터 기�?)
		relativeCamPos = XMVector3TransformCoord(relativeCamPos, xmmtxRotate);
		// 3. ?�시 ?�드 공간?�로 변??(?�전???��? ?�치 + ?�레?�어 ?�치)
		XMStoreFloat3(&m_xmf3Position, relativeCamPos + playerPosGlobal);
	}

	// Yaw ?�전 (?�레?�어???�드 Up 벡터 ?�는 ?�레?�어??Up 벡터 기�?)
	// 카메?��? ?�레?�어 주위�??�평?�로 ?�는 ?�과
	if (yawDelta != 0.0f && m_pPlayer) // ?�레?�어가 ?�을 ?�만
	{
		XMFLOAT3 playerUp = m_pPlayer->GetUpVector(); // ?�레?�어??Up 벡터 ?�용
		// ?�는 ?�드 Up 벡터 ?�용: XMFLOAT3 playerUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&playerUp), XMConvertToRadians(yawDelta));

		// 카메?�의 Look, Up, Right 벡터�??�전
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
	// 1. FOV�� �̿��� Near/Far ����� ���̿� �ʺ� ����մϴ�.
	float halfFovY = XMConvertToRadians(m_fFovAngle * 0.5f);
	float nearHeight = 2.0f * m_fNearPlaneDistance * tanf(halfFovY);
	float nearWidth = nearHeight * m_fAspectRatio;
	float farHeight = 2.0f * m_fFarPlaneDistance * tanf(halfFovY);
	float farWidth = farHeight * m_fAspectRatio;

	// 2. ī�޶��� ���� ��(Right, Up, Look)�� ��ġ�� XMVECTOR�� �ε��մϴ�.
	XMVECTOR xmvPosition = XMLoadFloat3(&m_xmf3Position);
	XMVECTOR xmvRight = XMLoadFloat3(&m_xmf3Right);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xmf3Up);
	XMVECTOR xmvLook = XMLoadFloat3(&m_xmf3Look);

	// 3. Near/Far ����� �߽����� ����մϴ�.
	XMVECTOR nearPlaneCenter = xmvPosition + (xmvLook * m_fNearPlaneDistance);
	XMVECTOR farPlaneCenter = xmvPosition + (xmvLook * m_fFarPlaneDistance);

	// 4. 8���� �������� ����մϴ�.
	// Near Plane (ī�޶�� ����� ��)
	XMStoreFloat3(&pCorners[0], nearPlaneCenter - (xmvRight * (nearWidth * 0.5f)) - (xmvUp * (nearHeight * 0.5f))); // ���� �Ʒ�
	XMStoreFloat3(&pCorners[1], nearPlaneCenter - (xmvRight * (nearWidth * 0.5f)) + (xmvUp * (nearHeight * 0.5f))); // ���� ��
	XMStoreFloat3(&pCorners[2], nearPlaneCenter + (xmvRight * (nearWidth * 0.5f)) + (xmvUp * (nearHeight * 0.5f))); // ������ ��
	XMStoreFloat3(&pCorners[3], nearPlaneCenter + (xmvRight * (nearWidth * 0.5f)) - (xmvUp * (nearHeight * 0.5f))); // ������ �Ʒ�

	// Far Plane (ī�޶�� �� ��)
	XMStoreFloat3(&pCorners[4], farPlaneCenter - (xmvRight * (farWidth * 0.5f)) - (xmvUp * (farHeight * 0.5f))); // ���� �Ʒ�
	XMStoreFloat3(&pCorners[5], farPlaneCenter - (xmvRight * (farWidth * 0.5f)) + (xmvUp * (farHeight * 0.5f))); // ���� ��
	XMStoreFloat3(&pCorners[6], farPlaneCenter + (xmvRight * (farWidth * 0.5f)) + (xmvUp * (farHeight * 0.5f))); // ������ ��
	XMStoreFloat3(&pCorners[7], farPlaneCenter + (xmvRight * (farWidth * 0.5f)) - (xmvUp * (farHeight * 0.5f))); // ������ �Ʒ�
}
