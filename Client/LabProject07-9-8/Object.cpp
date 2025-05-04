//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Scene.h"
#include "GameFramework.h"

#include <algorithm>


// 루트 상수로 전달할 구조체 (HLSL의 cbGameObjectInfo 와 일치해야 함)
struct cbGameObjectInfo {
	XMFLOAT4X4    gmtxGameObject;     // 16 DWORDS
	struct MaterialInfoCpp {
		XMFLOAT4   AmbientColor;  // 4
		XMFLOAT4   DiffuseColor;  // 4
		XMFLOAT4   SpecularColor; // 4
		XMFLOAT4   EmissiveColor; // 4
		float      Glossiness;        // 1
		float      Smoothness;        // 1
		float      SpecularHighlight; // 1
		float      Metallic;          // 1
		float      GlossyReflection;  // 1
		XMFLOAT3   Padding;           // 3 => MaterialInfoCpp = 24 DWORDS
	} gMaterialInfo;
	UINT          gnTexturesMask;     // 1 DWORD
	// 총 16 + 24 + 1 = 41 DWORDS
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject(CGameFramework* pGameFramework) : m_pGameFramework(pGameFramework)
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	m_OBBMaterial = new CMaterial(1, pGameFramework);


}

CGameObject::CGameObject(int nMaterials, CGameFramework* pGameFramework) : CGameObject(pGameFramework)
{
	m_nMaterials = nMaterials;
	if (m_nMaterials > 0)
	{
		m_ppMaterials = new CMaterial*[m_nMaterials];
		for(int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = nullptr;
	}
}

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();
	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		}
	}
	if (m_ppMaterials) delete[] m_ppMaterials;

	if (m_pSkinnedAnimationController) delete m_pSkinnedAnimationController;
	//if (FSM_manager) delete FSM_manager;
}

void CGameObject::AddRef() 
{ 
	m_nReferences++; 

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void CGameObject::Release() 
{ 
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();

	if (--m_nReferences <= 0) delete this; 
}

void CGameObject::SetChild(CGameObject *pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}

void CGameObject::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

//void CGameObject::SetShader(CShader *pShader)
//{
//	m_nMaterials = 1;
//	m_ppMaterials = new CMaterial*[m_nMaterials];
//	m_ppMaterials[0] = new CMaterial(0);
//	m_ppMaterials[0]->SetShader(pShader);
//}

void CGameObject::SetShader(int nMaterial, CShader *pShader)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->SetShader(pShader);
}

void CGameObject::SetMaterial(int nIndex, CMaterial *pMaterial)
{
	wchar_t buffer[128];
	swprintf_s(buffer, L"SetMaterial: Index=%d, pMaterial=%p\n", nIndex, (void*)pMaterial);
	OutputDebugStringW(buffer);

	if (m_ppMaterials && (nIndex < m_nMaterials))
	{
		if (m_ppMaterials[nIndex]) m_ppMaterials[nIndex]->Release(); // 기존 재질 해제
		m_ppMaterials[nIndex] = pMaterial;
		if (m_ppMaterials[nIndex]) m_ppMaterials[nIndex]->AddRef(); // 새 재질 참조 증가
	}
	else {
		OutputDebugStringW(L"  --> SetMaterial FAILED: Invalid index or m_ppMaterials is null.\n");
	}
}

bool CGameObject::CheckCollisionOBB(CGameObject* other)
{
	return m_localOBB.Intersects(other->m_localOBB);
}

void CGameObject::SetOBB(const XMFLOAT3& center, const XMFLOAT3& size, const XMFLOAT4& orientation)
{
	m_xmf3Position = center;
	m_xmf3Size = size;

	XMStoreFloat3(&m_localOBB.Center, XMLoadFloat3(&m_xmf3Position));
	XMStoreFloat3(&m_localOBB.Extents, XMLoadFloat3(&m_xmf3Size));
	XMStoreFloat4(&m_localOBB.Orientation, XMLoadFloat4(&orientation));
}

// 메쉬 데이터로 바운딩 박스 만들기
void CGameObject::SetOBB()
{
	if (m_pMesh) {
		// 메시 데이터로 OBB 만들기
		XMFLOAT3 minPos = m_pMesh->m_pxmf3Positions[0];
		XMFLOAT3 maxPos = m_pMesh->m_pxmf3Positions[0];
		for (int i = 1; i < m_pMesh->m_nPositions; ++i) {
			minPos.x = min(minPos.x, m_pMesh->m_pxmf3Positions[i].x);
			minPos.y = min(minPos.y, m_pMesh->m_pxmf3Positions[i].y);
			minPos.z = min(minPos.z, m_pMesh->m_pxmf3Positions[i].z);
			maxPos.x = max(maxPos.x, m_pMesh->m_pxmf3Positions[i].x);
			maxPos.y = max(maxPos.y, m_pMesh->m_pxmf3Positions[i].y);
			maxPos.z = max(maxPos.z, m_pMesh->m_pxmf3Positions[i].z);
		}
		m_localOBB.Center = XMFLOAT3(
			(minPos.x + maxPos.x) * 0.5f,
			(minPos.y + maxPos.y) * 0.5f,
			(minPos.z + maxPos.z) * 0.5f
		);
		m_localOBB.Extents = XMFLOAT3(
			(maxPos.x - minPos.x) * 0.5f,
			(maxPos.y - minPos.y) * 0.5f,
			(maxPos.z - minPos.z) * 0.5f
		);
		m_localOBB.Orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);  // 초기 회전 없음
	}

	if (m_pSibling) m_pSibling->SetOBB();
	if (m_pChild) m_pChild->SetOBB();
}

void CGameObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CShader* shader)
{
	if (m_pMesh) {
		// 메시 데이터로 OBB 만들기
		XMFLOAT3 minPos = m_pMesh->m_pxmf3Positions[0];
		XMFLOAT3 maxPos = m_pMesh->m_pxmf3Positions[0];
		for (int i = 1; i < m_pMesh->m_nPositions; ++i) {
			minPos.x = min(minPos.x, m_pMesh->m_pxmf3Positions[i].x);
			minPos.y = min(minPos.y, m_pMesh->m_pxmf3Positions[i].y);
			minPos.z = min(minPos.z, m_pMesh->m_pxmf3Positions[i].z);
			maxPos.x = max(maxPos.x, m_pMesh->m_pxmf3Positions[i].x);
			maxPos.y = max(maxPos.y, m_pMesh->m_pxmf3Positions[i].y);
			maxPos.z = max(maxPos.z, m_pMesh->m_pxmf3Positions[i].z);
		}
		m_localOBB.Center = XMFLOAT3(
			(minPos.x + maxPos.x) * 0.5f,
			(minPos.y + maxPos.y) * 0.5f,
			(minPos.z + maxPos.z) * 0.5f
		);
		m_localOBB.Extents = XMFLOAT3(
			(maxPos.x - minPos.x) * 0.5f,
			(maxPos.y - minPos.y) * 0.5f,
			(maxPos.z - minPos.z) * 0.5f
		);
		m_localOBB.Orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);  // 초기 회전 없음

		InitializeOBBResources(pd3dDevice, pd3dCommandList);
	}

	if (m_pSibling) m_pSibling->SetOBB();
	if (m_pChild) m_pChild->SetOBB();
}

void CGameObject::RenderOBB(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//m_OBBShader.Render(pd3dCommandList, NULL);
	m_OBBMaterial->m_pShader->Render(pd3dCommandList, NULL);

	// OBB 선을 그리기 위한 설정
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	pd3dCommandList->IASetVertexBuffers(0, 1, &m_OBBVertexBufferView);
	pd3dCommandList->IASetIndexBuffer(&m_OBBIndexBufferView);

	// 선(Line) OBB 그리기
	pd3dCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0); // 12개 선 = 24개 인덱스
}

void CGameObject::RenderOBB(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	// OBB 렌더링에 필요한 리소스(버텍스/인덱스/상수 버퍼)가 생성되었는지 확인
	if (!m_pOBBVertexBuffer || !m_pOBBIndexBuffer || !m_pd3dcbOBBTransform || !m_pcbMappedOBBTransform) return;

	// 1. OBB의 WVP(World * View * Projection) 행렬 계산
	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX view = XMLoadFloat4x4(&pCamera->GetViewMatrix());
	XMMATRIX proj = XMLoadFloat4x4(&pCamera->GetProjectionMatrix());
	XMFLOAT4X4 wvpMatrix;
	// HLSL은 row-major 기본, C++는 row-major -> HLSL에서 transpose 안 하려면 여기서 transpose
	XMStoreFloat4x4(&wvpMatrix, XMMatrixTranspose(world * view * proj));

	// 2. OBB 상수 버퍼 업데이트 (b0)
	memcpy(m_pcbMappedOBBTransform, &wvpMatrix, sizeof(XMFLOAT4X4));

	// 3. 상수 버퍼 바인딩 (OBB 루트 서명의 파라미터 인덱스 0번)
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, m_pd3dcbOBBTransform->GetGPUVirtualAddress());

	// 4. IA(Input Assembler) 설정
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST); // 라인 리스트
	pd3dCommandList->IASetVertexBuffers(0, 1, &m_OBBVertexBufferView);        // 정점 버퍼
	pd3dCommandList->IASetIndexBuffer(&m_OBBIndexBufferView);              // 인덱스 버퍼

	// 5. 그리기
	pd3dCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0); // 인덱스 24개 (선 12개)
}

void CGameObject::InitializeOBBResources(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	// 메쉬 유효성 검사 등 추가 가능
	if (m_pMesh)
	{
		// OBB 모서리 데이터
		XMFLOAT3 corners[8];
		m_worldOBB.GetCorners(corners); // m_worldOBB가 유효한지 먼저 확인 필요

		// 2. OBB 정점 버퍼 생성 (+ HRESULT 확인)
		m_pOBBVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, corners, sizeof(XMFLOAT3) * 8, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
		if (!m_pOBBVertexBuffer) {
			OutputDebugString(L"!!!!!!!! ERROR: Failed to create OBB Vertex Buffer! !!!!!!!!\n");
			// 실패 시 이후 리소스 생성 중단 또는 다른 처리
		}
		else {
			m_OBBVertexBufferView.BufferLocation = m_pOBBVertexBuffer->GetGPUVirtualAddress();
			m_OBBVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
			m_OBBVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * 8;
		}

		// 3. OBB 인덱스 데이터 정의 (변경 없음)
		UINT indices[] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };

		// 4. OBB 인덱스 버퍼 생성 (+ HRESULT 확인)
		m_pOBBIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, indices, sizeof(UINT) * 24, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr);
		if (!m_pOBBIndexBuffer) {
			OutputDebugString(L"!!!!!!!! ERROR: Failed to create OBB Index Buffer! !!!!!!!!\n");
		}
		else {
			m_OBBIndexBufferView.BufferLocation = m_pOBBIndexBuffer->GetGPUVirtualAddress();
			m_OBBIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
			m_OBBIndexBufferView.SizeInBytes = sizeof(UINT) * 24;
		}

		// 5. OBB 변환 행렬용 상수 버퍼 생성 (+ HRESULT 확인)
		UINT ncbElementBytes = (((sizeof(XMFLOAT4X4)) + 255) & ~255);
		m_pd3dcbOBBTransform = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		if (!m_pd3dcbOBBTransform) {
			OutputDebugString(L"!!!!!!!! ERROR: Failed to create OBB Transform CBV! !!!!!!!!\n");
			m_pcbMappedOBBTransform = nullptr; // 맵핑 포인터도 null 처리
		}
		else {
			// 맵핑된 포인터 저장 (+ HRESULT 확인)
			HRESULT hr = m_pd3dcbOBBTransform->Map(0, NULL, (void**)&m_pcbMappedOBBTransform);
			if (FAILED(hr) || !m_pcbMappedOBBTransform) {
				OutputDebugString(L"!!!!!!!! ERROR: Failed to map OBB Transform CBV! !!!!!!!!\n");
				m_pcbMappedOBBTransform = nullptr; // 실패 시 null 처리
				// 필요시 m_pd3dcbOBBTransform Release 고려
			}
		}
	}

	// 자식/형제 객체 재귀 호출 (기존 코드 유지)
	if (m_pSibling) m_pSibling->InitializeOBBResources(pd3dDevice, pd3dCommandList);
	if (m_pChild) m_pChild->InitializeOBBResources(pd3dDevice, pd3dCommandList);

}




void CGameObject::FindAndSetSkinnedMesh(CSkinnedMesh **ppSkinnedMeshes, int *pnSkinnedMesh)
{
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) ppSkinnedMeshes[(*pnSkinnedMesh)++] = (CSkinnedMesh *)m_pMesh;

	if (m_pSibling) m_pSibling->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
	if (m_pChild) m_pChild->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
}

CGameObject *CGameObject::FindFrame(char *pstrFrameName)
{
	CGameObject *pFrameObject = NULL;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void CGameObject::UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	// 월드 OBB 계산
	XMMATRIX worldMatrix = XMLoadFloat4x4(&m_xmf4x4World);

	// 중심점 변환
	XMVECTOR localCenter = XMLoadFloat3(&m_localOBB.Center);
	XMVECTOR worldCenter = XMVector3TransformCoord(localCenter, worldMatrix);
	XMStoreFloat3(&m_worldOBB.Center, worldCenter);

	// 방향 변환 (회전)
	XMMATRIX rotationMatrix = worldMatrix;
	rotationMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);  // 이동 성분 제거
	XMVECTOR orientation = XMQuaternionRotationMatrix(rotationMatrix);
	XMStoreFloat4(&m_worldOBB.Orientation, orientation);

	// 크기 변환 (스케일)
	XMFLOAT3 scale;
	scale.x = XMVectorGetX(XMVector3Length(worldMatrix.r[0]));
	scale.y = XMVectorGetX(XMVector3Length(worldMatrix.r[1]));
	scale.z = XMVectorGetX(XMVector3Length(worldMatrix.r[2]));
	m_worldOBB.Extents.x = m_localOBB.Extents.x * scale.x;
	m_worldOBB.Extents.y = m_localOBB.Extents.y * scale.y;
	m_worldOBB.Extents.z = m_localOBB.Extents.z * scale.z;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void CGameObject::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackAnimationSet(nAnimationTrack, nAnimationSet);
}

void CGameObject::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
}

void CGameObject::Animate(float fTimeElapsed)
{
	OnPrepareRender();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene* pScene = m_pGameFramework ? m_pGameFramework->GetScene() : nullptr;
	if (!pScene) return; // 씬 없으면 렌더링 불가

	// 이 객체가 직접 렌더링할 메쉬와 첫 번째 재질/셰이더를 가지고 있는지 확인
	CMaterial* pPrimaryMaterial = GetMaterial(0); // 상태 설정 기준으로 첫 번째 재질 사용


	if (m_pMesh && pPrimaryMaterial && pPrimaryMaterial->m_pShader)
	{
		pScene->SetGraphicsState(pd3dCommandList, pPrimaryMaterial->m_pShader);


		// --- 공통 CBV 바인딩 ---
		// 카메라 CBV (b1 @ 인덱스 0)
		if (pCamera && pCamera->GetCameraConstantBuffer()) {
			pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());
		}
		// 조명 CBV (b4 @ 인덱스 2) - Standard/Skinned/Instancing 인 경우에만 바인딩
		CShader* pCurrentShader = pPrimaryMaterial->m_pShader; // 편의상
		std::string shaderType = pCurrentShader->GetShaderType(); // GetShaderType() 함수 필요
		if (shaderType == "Standard" || shaderType == "Skinned" /* || shaderType == "Instancing" */) {
			ID3D12Resource* pLightBuffer = pScene->GetLightsConstantBuffer(); // CScene 함수 통해 접근
			if (pLightBuffer) {
				pd3dCommandList->SetGraphicsRootConstantBufferView(2, pLightBuffer->GetGPUVirtualAddress());
			}
		}


		// 이 GameObject에 속한 모든 메쉬/재질 쌍에 대해 반복
		for (int i = 0; i < m_nMaterials; i++)
		{
			CMaterial* pMaterial = GetMaterial(i); // 현재 재질
			// 현재 재질과 Primary 재질의 셰이더가 다르면 SetGraphicsState 다시 호출? (복잡도 증가, 일단 생략)
			if (pMaterial && pMaterial->m_pShader == pCurrentShader) // 같은 셰이더를 사용하는 재질만 그림 (단순화)
			{
				// --- 리소스 바인딩 ---
				// 현재 CShader (및 RootSignature)는 CScene::Render에서 이미 설정됨
				// 1. 객체별 상수 업데이트 (루트 상수 b2 사용)
				cbGameObjectInfo gameObjectInfo; // C++ 구조체 인스턴스

				// 1.1. 월드 변환 행렬 설정
				XMStoreFloat4x4(&gameObjectInfo.gmtxGameObject, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

				// 1.2. 재질 정보 채우기
				gameObjectInfo.gMaterialInfo.AmbientColor = pMaterial->m_xmf4AmbientColor;
				gameObjectInfo.gMaterialInfo.DiffuseColor = pMaterial->m_xmf4AlbedoColor;
				gameObjectInfo.gMaterialInfo.SpecularColor = pMaterial->m_xmf4SpecularColor;
				// 예: Specular Power를 Alpha에 저장했다면 gameObjectInfo.gMaterialInfo.SpecularColor.w = pMaterial->m_fGlossiness;
				gameObjectInfo.gMaterialInfo.EmissiveColor = pMaterial->m_xmf4EmissiveColor;
				gameObjectInfo.gMaterialInfo.Glossiness = pMaterial->m_fGlossiness;
				gameObjectInfo.gMaterialInfo.Smoothness = pMaterial->m_fSmoothness;
				gameObjectInfo.gMaterialInfo.SpecularHighlight = pMaterial->m_fSpecularHighlight;
				gameObjectInfo.gMaterialInfo.Metallic = pMaterial->m_fMetallic;
				gameObjectInfo.gMaterialInfo.GlossyReflection = pMaterial->m_fGlossyReflection;
				// Padding은 초기화 필요 없음

				// 1.3. 텍스처 마스크 설정
				gameObjectInfo.gnTexturesMask = 0;
				for (int texIdx = 0; texIdx < pMaterial->GetTextureCount(); ++texIdx) {
					// GetTexture 함수가 shared_ptr 벡터를 확인하고 raw 포인터 반환
					if (pMaterial->GetTexture(texIdx)) {
						if (texIdx == 0) gameObjectInfo.gnTexturesMask |= MATERIAL_ALBEDO_MAP;
						else if (texIdx == 1) gameObjectInfo.gnTexturesMask |= MATERIAL_SPECULAR_MAP;
						else if (texIdx == 2) gameObjectInfo.gnTexturesMask |= MATERIAL_NORMAL_MAP;
						else if (texIdx == 3) gameObjectInfo.gnTexturesMask |= MATERIAL_METALLIC_MAP;
						else if (texIdx == 4) gameObjectInfo.gnTexturesMask |= MATERIAL_EMISSION_MAP;
						else if (texIdx == 5) gameObjectInfo.gnTexturesMask |= MATERIAL_DETAIL_ALBEDO_MAP;
						else if (texIdx == 6) gameObjectInfo.gnTexturesMask |= MATERIAL_DETAIL_NORMAL_MAP;
					}
				}
				// ... 다른 텍스처 타입이 있다면 추가 ...

				// 1.4. 루트 상수 바인딩 (Standard/Skinned 루트 서명의 파라미터 인덱스 1번)
				pd3dCommandList->SetGraphicsRoot32BitConstants(1, 41, &gameObjectInfo, 0);

				// 2. 재질 텍스처 바인딩 (Descriptor Table 사용 가정)
				// Standard/Skinned 루트 서명의 파라미터 인덱스 3번이 t6-t12 텍스처 테이블이었음
				D3D12_GPU_DESCRIPTOR_HANDLE textureTableHandle = pMaterial->GetTextureTableGpuHandle(); // 재질이 자신의 텍스처 테이블 시작 핸들을 알아야 함 (CTexture 로딩/관리 시 설정 필요)
				if (textureTableHandle.ptr != 0) {
					// 루트 파라미터 인덱스 3번에 텍스처 테이블 시작 핸들 바인딩
					pd3dCommandList->SetGraphicsRootDescriptorTable(3, textureTableHandle);
				}

				// 3. 스키닝 관련 CBV 바인딩 (Skinned 루트 서명 사용 시)
		   // 현재 셰이더가 Skinned 인지 확인하는 로직이 있으면 더 좋음 (예: shader->GetType())
				 // --- 스키닝 CBV 바인딩 (셰이더 타입 "Skinned" 확인 후) ---
				if (shaderType == "Skinned") {
					CSkinnedMesh* pSkinnedMesh = dynamic_cast<CSkinnedMesh*>(m_pMesh);
					if (pSkinnedMesh) {
						// 3.1. 본 오프셋 버퍼 바인딩 (b7, 파라미터 인덱스 4)
						ID3D12Resource* pOffsetBuffer = pSkinnedMesh->m_pd3dcbBindPoseBoneOffsets;
                        if (pOffsetBuffer) {
                            pd3dCommandList->SetGraphicsRootConstantBufferView(4, pOffsetBuffer->GetGPUVirtualAddress());
                        }
						// 3.2. 본 변환 버퍼 바인딩 (b8, 파라미터 인덱스 5)
						if (m_pSharedAnimController && // 이 노드에 저장된 컨트롤러 포인터 확인
							m_pSharedAnimController->m_ppd3dcbSkinningBoneTransforms &&
							m_pSharedAnimController->m_ppd3dcbSkinningBoneTransforms[0]) {
							pd3dCommandList->SetGraphicsRootConstantBufferView(5, m_pSharedAnimController->m_ppd3dcbSkinningBoneTransforms[0]->GetGPUVirtualAddress());
						}
						else {
							// 로그 출력: 컨트롤러 포인터가 null 인지, 아니면 내부 버퍼가 null 인지 확인
							OutputDebugStringW(L"!!! Render: Skinned - Failed to get valid Bone Transform buffer (b8) via m_pSharedAnimController!\n");
							wchar_t dbgMsg[128];
							swprintf_s(dbgMsg, L"    m_pSharedAnimController = %p\n", (void*)m_pSharedAnimController); // 포인터 값 로깅
							OutputDebugStringW(dbgMsg);
							// 필요시 m_pSharedAnimController 내부 포인터들도 확인하는 로그 추가
						}
					}
				}



				// --- 그리기 ---
				m_pMesh->Render(pd3dCommandList, i); 
			}
		}
	}
	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera);
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView)
{
	OnPrepareRender();

	if (m_pMesh) m_pMesh->Render(pd3dCommandList, nInstances, d3dInstancingBufferView);
}

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial)
{
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	}

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParent._41 = x;
	m_xmf4x4ToParent._42 = y;
	m_xmf4x4ToParent._43 = z;

	UpdateTransform(NULL);
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::Move(XMFLOAT3 xmf3Offset)
{
	m_xmf4x4ToParent._41 += xmf3Offset.x;
	m_xmf4x4ToParent._42 += xmf3Offset.y;
	m_xmf4x4ToParent._43 += xmf3Offset.z;

	UpdateTransform(NULL);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetToParentPosition()
{
	return(XMFLOAT3(m_xmf4x4ToParent._41, m_xmf4x4ToParent._42, m_xmf4x4ToParent._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::SetLook(XMFLOAT3 xmf3Look)
{
	XMVECTOR vLook = XMLoadFloat3(&xmf3Look);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&m_xmf4x4ToParent._31), vLook);

}

void CGameObject::SetUp(XMFLOAT3 xmf3Up)
{
	XMVECTOR vUp = XMLoadFloat3(&xmf3Up);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&m_xmf4x4ToParent._21), vUp);

}

void CGameObject::SetRight(XMFLOAT3 xmf3Right)
{
	XMVECTOR vRight = XMLoadFloat3(&xmf3Right);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&m_xmf4x4ToParent._11), vRight);
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)terraindata;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	int z = (int)(xmf3Position.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z, bReverseQuad) + 0.0f;
	xmf3Position.y = fHeight;
	CGameObject::SetPosition(xmf3Position);

}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

//#define _WITH_DEBUG_FRAME_HIERARCHY

std::shared_ptr<CTexture> CGameObject::FindReplicatedTexture(_TCHAR *pstrTextureName)
{
	std::shared_ptr<CTexture> pTexture = nullptr; // shared_ptr로 변경
	
	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) // CMaterial 포인터 배열 유지 가정
		{
			// m_ppMaterials[i]->m_vTextures 가 shared_ptr 벡터라고 가정
			for (int j = 0; j < m_ppMaterials[i]->GetTextureCount(); j++) // GetTextureCount 사용
			{
				// 텍스처 이름 비교 (m_ppstrTextureNames 사용 유지)
				if (!_tcsncmp(m_ppMaterials[i]->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName)))
				{
					// CMaterial의 m_vTextures에서 shared_ptr 가져오기
					if (j < m_ppMaterials[i]->m_vTextures.size()) {
						return m_ppMaterials[i]->m_vTextures[j]; // shared_ptr 복사하여 반환 (참조 카운트 증가)
					}
				}
			}
		}
	}

	// 자식/형제 노드에서 찾기 (재귀 호출)
	if (m_pSibling) {
		pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName);
		if (pTexture) return pTexture; // 찾으면 바로 반환
	}
	if (m_pChild) {
		pTexture = m_pChild->FindReplicatedTexture(pstrTextureName);
		if (pTexture) return pTexture; // 찾으면 바로 반환
	}

	return nullptr; // 못 찾으면 nullptr (빈 shared_ptr) 반환
}

int ReadIntegerFromFile(FILE *pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile); 
	return(nValue);
}

float ReadFloatFromFile(FILE *pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile); 
	return(fValue);
}

BYTE ReadStringFromFile(FILE *pInFile, char *pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile); 
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

void CGameObject::LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CGameFramework* pGameFramework)
{
	// ShaderManager 및 ResourceManager 가져오기
	assert(pGameFramework != nullptr && "GameFramework pointer is needed!");
	ShaderManager* pShaderManager = pGameFramework->GetShaderManager();
	ResourceManager* pResourceManager = pGameFramework->GetResourceManager();
	assert(pShaderManager != nullptr && "ShaderManager is not available!");
	assert(pResourceManager != nullptr && "ResourceManager is not available!");

	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	m_nMaterials = ReadIntegerFromFile(pInFile);

	wchar_t buffer[128];// 로그용 버퍼
	swprintf_s(buffer, L"LoadMaterialsFromFile: Expecting %d materials.\n", m_nMaterials);
	OutputDebugStringW(buffer);

	if (m_nMaterials <= 0) return; // 재질 없으면 종료

	if (m_ppMaterials) delete[] m_ppMaterials; // 이미 있다면 해제 (재할당 방지)
	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	CMaterial *pMaterial = NULL;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
			OutputDebugStringA("LoadMaterialsFromFile: Read Token: ");
		OutputDebugStringA(pstrToken);
		OutputDebugStringA("\n");


		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ReadIntegerFromFile(pInFile);
			OutputDebugStringW((L"  Processing <Material> index: " + std::to_wstring(nMaterial) + L"\n").c_str());

			pMaterial = new CMaterial(7, pGameFramework); // Assume 7 textures for now
			OutputDebugStringW((L"    new CMaterial result: " + std::wstring(pMaterial ? L"Success" : L"FAILED!") + L"\n").c_str());

			if (!pMaterial) continue; // Material 생성 실패 시 다음 토큰으로

			// --- 셰이더 설정 로직 변경 ---
			UINT nMeshType = GetMeshType();
			std::string shaderName = "Standard"; // 기본값

			// 메쉬 타입에 따라 필요한 셰이더 이름 결정
			if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE) { // 기본 텍스처/노멀/탄젠트 포함 시
				if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT) { // 본 가중치 포함 시
					shaderName = "Skinned";
				}
				else {
					shaderName = "Standard";
				}
			}
			else {
				// 다른 메쉬 타입에 대한 처리 (예: 색상만 있는 메쉬 등)
				// 필요하다면 여기에 다른 셰이더 이름 할당 로직 추가
			}

			// ShaderManager로부터 셰이더 가져오기
			CShader* pMatShader = pShaderManager->GetShader(shaderName, pd3dCommandList);
			if (pMatShader) {
				pMaterial->SetShader(pMatShader); // CMaterial에 셰이더 설정
				// GetShader는 호출자를 위해 AddRef 했으므로, SetShader에서 AddRef 한 후 여기서 Release
				pMatShader->Release();
			}
			else {
				OutputDebugStringA(("Error: Could not get shader '" + shaderName + "' from ShaderManager! Assigning default Standard shader.\n").c_str());
				// 예외 처리: Standard 셰이더라도 다시 시도
				pMatShader = pShaderManager->GetShader("Standard", pd3dCommandList);
				if (pMatShader) {
					pMaterial->SetShader(pMatShader);
					pMatShader->Release();
				}
			}
			// --- 셰이더 설정 로직 끝 ---

			SetMaterial(nMaterial, pMaterial); // 재질 설정
			OutputDebugStringW((L"    SetMaterial called for index: " + std::to_wstring(nMaterial) + L"\n").c_str());
		}
		else if (!strcmp(pstrToken, "<AlbedoColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<EmissiveColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularColor>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		}
		else if (!strcmp(pstrToken, "<Glossiness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossiness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Smoothness>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSmoothness), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Metallic>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fSpecularHighlight), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<SpecularHighlight>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<GlossyReflection>:"))
		{
			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<AlbedoMap>:"))
		{
			// LoadTextureFromFile 호출 시 인덱스(0)와 타입(MATERIAL_ALBEDO_MAP) 전달
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 0, MATERIAL_ALBEDO_MAP, pParent, pInFile, pResourceManager);
		}
		else if (!strcmp(pstrToken, "<SpecularMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 1, MATERIAL_ALBEDO_MAP, pParent, pInFile, pResourceManager);
		}
		else if (!strcmp(pstrToken, "<NormalMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 2, MATERIAL_ALBEDO_MAP, pParent, pInFile, pResourceManager);
		}
		else if (!strcmp(pstrToken, "<MetallicMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 3, MATERIAL_ALBEDO_MAP, pParent, pInFile, pResourceManager);
		}
		else if (!strcmp(pstrToken, "<EmissionMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 4, MATERIAL_ALBEDO_MAP, pParent, pInFile, pResourceManager);
		}
		else if (!strcmp(pstrToken, "<DetailAlbedoMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 5, MATERIAL_ALBEDO_MAP, pParent, pInFile, pResourceManager);
		}
		else if (!strcmp(pstrToken, "<DetailNormalMap>:"))
		{
			pMaterial->LoadTextureFromFile(pd3dDevice, pd3dCommandList, 6, MATERIAL_ALBEDO_MAP, pParent, pInFile, pResourceManager);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			OutputDebugStringW(L"LoadMaterialsFromFile: Found </Materials>, exiting loop.\n");
			break;
		}
	}

	// 함수 종료 전 확인 (디버깅용)
	for (int i = 0; i < m_nMaterials; ++i) {
		swprintf_s(buffer, L"LoadMaterialsFromFile: Final check - Material[%d] pointer: %p\n", i, (void*)m_ppMaterials[i]);
		OutputDebugStringW(buffer);
	}
}

CGameObject *CGameObject::LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pParent, FILE *pInFile, int *pnSkinnedMeshes, CGameFramework* pGameFramework)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	CGameObject *pGameObject = new CGameObject(pGameFramework);

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			CStandardMesh *pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			CSkinnedMesh *pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinInfoFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			pGameObject->SetMesh(pSkinnedMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pGameFramework);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					CGameObject *pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pGameObject, pInFile, pnSkinnedMeshes, pGameFramework);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, "(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

void CGameObject::PrintFrameInfo(CGameObject *pGameObject, CGameObject *pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);
	
	ofstream fout("Player_weapon_Frame.txt", ios::app);
	
	if (pGameObject)
		fout << pGameObject->m_pstrFrameName << " ";
	if (pParent)
		fout << pParent->m_pstrFrameName;
	fout << std::endl;

	if (pGameObject->m_pSibling) CGameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) CGameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}

void CGameObject::LoadAnimationFromFile(FILE *pInFile, CLoadedModelInfo *pLoadedModel)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nAnimationSets = 0;

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nAnimationSets = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets = new CAnimationSets(nAnimationSets);
		}
		else if (!strcmp(pstrToken, "<FrameNames>:"))
		{
			pLoadedModel->m_pAnimationSets->m_nBoneFrames = ::ReadIntegerFromFile(pInFile); 
			pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches = new CGameObject*[pLoadedModel->m_pAnimationSets->m_nBoneFrames];

			for (int j = 0; j < pLoadedModel->m_pAnimationSets->m_nBoneFrames; j++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches[j] = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);

//#define _WITH_DEBUG_SKINNING_BONE
#ifdef _WITH_DEBUG_SKINNING_BONE
				TCHAR pstrDebug[256] = { 0 };
				TCHAR pwstrAnimationBoneName[64] = { 0 };
				TCHAR pwstrBoneCacheName[64] = { 0 };
				size_t nConverted = 0;
				mbstowcs_s(&nConverted, pwstrAnimationBoneName, 64, pstrToken, _TRUNCATE);
				mbstowcs_s(&nConverted, pwstrBoneCacheName, 64, pLoadedModel->m_pAnimationSets->m_ppBoneFrameCaches[j]->m_pstrFrameName, _TRUNCATE);
				_stprintf_s(pstrDebug, 256, _T("AnimationBoneFrame:: Cache(%s) AnimationBone(%s)\n"), pwstrBoneCacheName, pwstrAnimationBoneName);
				OutputDebugString(pstrDebug);
#endif
			}
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken); //Animation Set Name

			float fLength = ::ReadFloatFromFile(pInFile);
			int nFramesPerSecond = ::ReadIntegerFromFile(pInFile);
			int nKeyFrames = ::ReadIntegerFromFile(pInFile);

			pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet] = new CAnimationSet(fLength, nFramesPerSecond, nKeyFrames, pLoadedModel->m_pAnimationSets->m_nBoneFrames, pstrToken);

			for (int i = 0; i < nKeyFrames; i++)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				if (!strcmp(pstrToken, "<Transforms>:"))
				{
					CAnimationSet *pAnimationSet = pLoadedModel->m_pAnimationSets->m_pAnimationSets[nAnimationSet];

					int nKey = ::ReadIntegerFromFile(pInFile); //i
					float fKeyTime = ::ReadFloatFromFile(pInFile);

#ifdef _WITH_ANIMATION_SRT
					m_pfKeyFrameScaleTimes[i] = fKeyTime;
					m_pfKeyFrameRotationTimes[i] = fKeyTime;
					m_pfKeyFrameTranslationTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameScales[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4KeyFrameRotations[i], sizeof(XMFLOAT4), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf3KeyFrameTranslations[i], sizeof(XMFLOAT3), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
#else
					pAnimationSet->m_pfKeyFrameTimes[i] = fKeyTime;
					nReads = (UINT)::fread(pAnimationSet->m_ppxmf4x4KeyFrameTransforms[i], sizeof(XMFLOAT4X4), pLoadedModel->m_pAnimationSets->m_nBoneFrames, pInFile);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}

CLoadedModelInfo *CGameObject::LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, CGameFramework* pGameFramework)
{
	FILE *pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CLoadedModelInfo *pLoadedModel = new CLoadedModelInfo();

	char pstrToken[64] = { '\0' };

	for ( ; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>:"))
			{
				pLoadedModel->m_pModelRootObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, &pLoadedModel->m_nSkinnedMeshes, pGameFramework);
				::ReadStringFromFile(pInFile, pstrToken); //"</Hierarchy>"
			}
			else if (!strcmp(pstrToken, "<Animation>:"))
			{
				CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
				pLoadedModel->PrepareSkinning();
			}
			else if (!strcmp(pstrToken, "</Animation>:"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

//#define _WITH_DEBUG_FRAME_HIERARCHY
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, "Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pLoadedModel);
}

CGameObject* CGameObject::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CGameFramework* pGameFramework)
{
	char pstrToken[64] = { '\0' };

	BYTE nStrLength = 0;
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	CGameObject* pGameObject = NULL;

	for (; ; )
	{
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
		if (nReads != 1 || nStrLength >= sizeof(pstrToken))  // <-- �޸� ��ȣ �߰�
		{
			printf("Error: Invalid string length read (%d)\n", nStrLength);
			return nullptr;
		}
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		if (nReads != nStrLength)
		{
			printf("Error: Failed to read token string\n");
			return nullptr;
		}
		pstrToken[nStrLength] = '\0';  // ���� ũ�� �ʰ� ����

		if (!strcmp(pstrToken, "<Frame>:"))
		{
			pGameObject = new CGameObject(pGameFramework);

			nReads = (UINT)::fread(&nFrame, sizeof(int), 1, pInFile);
			nReads = (UINT)::fread(&nTextures, sizeof(int), 1, pInFile);

			nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
			nReads = (UINT)::fread(pGameObject->m_pstrFrameName, sizeof(char), nStrLength, pInFile);
			pGameObject->m_pstrFrameName[nStrLength] = '\0';
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			CStandardMesh* pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pGameFramework);
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = 0;
			nReads = (UINT)::fread(&nChilds, sizeof(int), 1, pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pGameObject, pInFile, pGameFramework);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

CGameObject* CGameObject::LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName, CGameFramework* pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = NULL;
	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Hierarchy>:"))
		{
			pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, pGameFramework);
		}
		else if (!strcmp(pstrToken, "</Hierarchy>"))
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pGameObject);
}

 void CGameObject::PropagateAnimController(CAnimationController* controller) {
     CAnimationController* controllerToUse = m_pSkinnedAnimationController ? m_pSkinnedAnimationController : controller; // 자신이 있으면 자신 우선
     if (m_pMesh && dynamic_cast<CSkinnedMesh*>(m_pMesh)) {
         m_pSharedAnimController = controllerToUse; // 스키드 메쉬면 컨트롤러 저장
     }
     if (m_pChild) m_pChild->PropagateAnimController(controllerToUse); // 자식에게 전파
     if (m_pSibling) m_pSibling->PropagateAnimController(controller);    // 형제는 부모가 준 것 전파
 }



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework) // Material 슬롯 1개
{
	assert(pGameFramework != nullptr && "GameFramework pointer is needed for CHeightMapTerrain!");
	ResourceManager* pResourceManager = pGameFramework->GetResourceManager();
	ShaderManager* pShaderManager = pGameFramework->GetShaderManager(); // ShaderManager 가져오기
	assert(pResourceManager != nullptr && pShaderManager != nullptr);

	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	CHeightMapGridMesh *pMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, nWidth, nLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
	SetMesh(pMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// 재질 생성
	CMaterial* pTerrainMaterial = new CMaterial(2, pGameFramework);

	// 텍스처 로드 
	std::shared_ptr<CTexture> pTerrainBaseTexture = pResourceManager->GetTexture(L"Terrain/DemoTerrain3.dds", pd3dCommandList);
	std::shared_ptr<CTexture> pTerrainDetailTexture = pResourceManager->GetTexture(L"Terrain/TerrainGrass_basecolor.dds", pd3dCommandList);
	
	// 재질에 텍스처 할당 및 SRV 생성 요청
	if (pTerrainBaseTexture) {
		pTerrainMaterial->AssignTexture(0, pTerrainBaseTexture, pd3dDevice); // 0번 슬롯
	}
	if (pTerrainDetailTexture) {
		pTerrainMaterial->AssignTexture(1, pTerrainDetailTexture, pd3dDevice); // 1번 슬롯
	}

	// 셰이더 가져오기 및 설정
	CShader* pTerrainShader = pShaderManager->GetShader("Terrain", pd3dCommandList); 
	if (pTerrainShader) {
		pTerrainMaterial->SetShader(pTerrainShader); 
		pTerrainShader->Release(); // GetShader로 얻은 참조 해제
	}
	else {
		OutputDebugString(L"Error: Failed to get Terrain shader. Material will not have a shader.\n");
	}

	SetMaterial(0, pTerrainMaterial);

}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

void CHeightMapTerrain::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene* pScene = m_pGameFramework ? m_pGameFramework->GetScene() : nullptr;
	if (!pScene) return;


	CMaterial* pMaterial = GetMaterial(0); // 지형은 재질 하나 가정
	if (m_pMesh && pMaterial && pMaterial->m_pShader)
	{
		// --- 상태 설정 ---
		pScene->SetGraphicsState(pd3dCommandList, pMaterial->m_pShader);

		// --- 공통 CBV 바인딩 ---
		// 카메라 CBV (b1 @ 인덱스 0)
		if (pCamera && pCamera->GetCameraConstantBuffer()) {
			pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());
		}

		UpdateTransform(NULL); // 지형 월드 행렬 업데이트

		// 1. 지형 객체 상수 바인딩 (b2 @ Param 1 - 월드 행렬만)
		XMFLOAT4X4 gmtxGameObject; // 월드 행렬만 필요
		XMStoreFloat4x4(&gmtxGameObject, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
		// 루트 파라미터 1번에 16 DWORDS (행렬 크기) 설정
		pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &gmtxGameObject, 0);

		// 2. 지형 텍스처 테이블 바인딩 (t1, t2 @ Param 2)
		D3D12_GPU_DESCRIPTOR_HANDLE textureTableHandle = pMaterial->GetTextureTableGpuHandle();
		if (textureTableHandle.ptr != 0) {
			// 루트 파라미터 인덱스 2번에 바인딩!
			pd3dCommandList->SetGraphicsRootDescriptorTable(2, textureTableHandle);
		}
		else {
			OutputDebugString(L"Warning: Terrain material has null texture handle for binding.\n");
		}

		// --- 그리기 ---
		m_pMesh->Render(pd3dCommandList, 0);

	}
	// 지형은 자식/형제 없음
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	assert(pGameFramework != nullptr && "GameFramework pointer is needed for CSkyBox!");
	ResourceManager* pResourceManager = pGameFramework->GetResourceManager();
	ShaderManager* pShaderManager = pGameFramework->GetShaderManager(); // ShaderManager 가져오기
	assert(pResourceManager != nullptr && pShaderManager != nullptr);

	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// 재질 생성
	CMaterial* pSkyBoxMaterial = new CMaterial(1, pGameFramework);

	// 텍스처 로드 
	std::shared_ptr<CTexture> pSkyBoxTexture = pResourceManager->GetTexture(L"SkyBox/SkyBox_1.dds", pd3dCommandList);
	if (pSkyBoxTexture) {
		pSkyBoxMaterial->AssignTexture(0, pSkyBoxTexture, pd3dDevice); // 0번 인덱스에 할당
	}
	else {
		// 텍스처 로딩 실패 처리!
		OutputDebugString(L"Error: Failed to load SkyBox texture using ResourceManager.\n");
	}

	// 5. 셰이더 가져오기 및 설정
	CShader* pSkyBoxShader = pShaderManager->GetShader("Skybox", pd3dCommandList);
	if (pSkyBoxShader) {
		pSkyBoxMaterial->SetShader(pSkyBoxShader);
		pSkyBoxShader->Release();
	}

	SetMaterial(0, pSkyBoxMaterial);
	
}


CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CScene* pScene = m_pGameFramework ? m_pGameFramework->GetScene() : nullptr;
	if (!pScene || !pCamera) return;

	CMaterial* pMaterial = GetMaterial(0); // 스카이박스는 재질 하나 가정
	if (m_pMesh && pMaterial && pMaterial->m_pShader)
	{
		pScene->SetGraphicsState(pd3dCommandList, pMaterial->m_pShader);

		// --- 공통 CBV 바인딩 ---
		// 카메라 CBV (b1 @ 인덱스 0)
		if (pCamera->GetCameraConstantBuffer()) { // pCamera는 null 아님
			pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());
		}

		// --- 스카이박스 리소스 바인딩 ---
		XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
		SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);
		UpdateTransform(NULL); // 월드 행렬 업데이트

		// 스카이박스 텍스처 테이블 바인딩 (t13 @ Param 1)
		D3D12_GPU_DESCRIPTOR_HANDLE textureTableHandle = pMaterial->GetTextureTableGpuHandle();
		if (textureTableHandle.ptr != 0) {
			// 루트 파라미터 인덱스 1번에 바인딩!
			pd3dCommandList->SetGraphicsRootDescriptorTable(1, textureTableHandle);
		}
		else {
			OutputDebugString(L"Warning: Skybox material has null texture handle for binding.\n");
		}

		// --- 그리기 ---
		m_pMesh->Render(pd3dCommandList, 0); // 메쉬 렌더링

	}
}




CMonsterObject::CMonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* pModel, int nAnimationTracks, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	CLoadedModelInfo *pMonsterModel = pModel;
	if (!pMonsterModel) pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/Monster.bin", pGameFramework);

	SetChild(pMonsterModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pMonsterModel);

	FSM_manager = std::make_shared<FSMManager<CGameObject>>(this);
}

CMonsterObject::~CMonsterObject()
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//CEagleAnimationController::CEagleAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel) : CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pModel)
//{
//}
//
//CEagleAnimationController::~CEagleAnimationController()
//{
//}
//
//void CEagleAnimationController::OnRootMotion(CGameObject* pRootGameObject)
//{
//	if (m_bRootMotion)
//	{
//		pRootGameObject->MoveForward(0.55f);
//	}
//}

//CEagleObject::CEagleObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, int nAnimationTracks)
//{
//	CLoadedModelInfo *pEagleModel = pModel;
//	if (!pEagleModel) pEagleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Eagle.bin", NULL);
//
//	SetChild(pEagleModel->m_pModelRootObject, true);
//	m_pSkinnedAnimationController = new CEagleAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pEagleModel);
//}
//
//CEagleObject::~CEagleObject()
//{
//}
//
//void CEagleObject::SetPosition(float x, float y, float z)
//{
//	CGameObject::SetPosition(x, y, z);
//
//	XMFLOAT3 xmf3Position = XMFLOAT3(x, y, z);
//	if (Vector3::Distance(m_xmf3StartPosition, xmf3Position) > 150.0f)
//	{
//		Rotate(0.0f, 180.0f, 0.0f);
//		m_xmf3StartPosition = xmf3Position;
//	}
//}


CHairObject::CHairObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/SK_Hu_M_Hair_01.bin", "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	//CLoadedModelInfo* pGameObject = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/SK_Hu_M_Hair_01_skin.bin", NULL);

	SetChild(pGameObject);

	if (pInFile) fclose(pInFile); // 파일 닫기 추가
}


CPineObject::CPineObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/FAE_Pine_A_LOD0.bin", "rb");
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework); // 마지막 인자 추가
	SetChild(pGameObject);

	m_objectType = GameObjectType::Tree;

	if (pInFile) fclose(pInFile); // 파일 닫기 추가
}

CRockClusterAObject::CRockClusterAObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/RockCluster_A_LOD0.bin", "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	m_objectType = GameObjectType::Rock;

	if (pInFile) fclose(pInFile); // 파일 닫기 추가
}

CRockClusterBObject::CRockClusterBObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/RockCluster_B_LOD0.bin", "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	m_objectType = GameObjectType::Rock;

	if (pInFile) fclose(pInFile); // 파일 닫기 추가
}

CRockClusterCObject::CRockClusterCObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/RockCluster_C_LOD0.bin", "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	m_objectType = GameObjectType::Rock;

	if (pInFile) fclose(pInFile); // 파일 닫기 추가
}

CCliffFObject::CCliffFObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/Cliff_F_LOD0.bin", "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	m_objectType = GameObjectType::Rock;

	if (pInFile) fclose(pInFile); // 파일 닫기 추가
}

CSwordObject::CSwordObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/Sword_01.bin", "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	//m_objectType = GameObjectType::Rock;

	if (pInFile) fclose(pInFile); // 파일 닫기 추가
}

CStaticObject::CStaticObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* modelname, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, modelname, "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	if (pInFile) fclose(pInFile); // 파일 닫기 추가
}

UserObject::UserObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* pModel, int nAnimationTracks, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	CLoadedModelInfo* pUserModel = pModel;
	if (!pUserModel) pUserModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Hu_M_FullBody.bin", pGameFramework);

	SetChild(pUserModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pUserModel);
}

UserObject::~UserObject()
{
}
