//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Scene.h"
#include "GameFramework.h"
#include "NetworkManager.h"
#include <algorithm>



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
	
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	m_OBBMaterial = new CMaterial(1);
}


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
	if (m_ppMaterials) {
		delete[] m_ppMaterials;
		m_ppMaterials = nullptr;
	}

	if (m_pSkinnedAnimationController) {
		delete m_pSkinnedAnimationController;
		m_pSkinnedAnimationController = nullptr;
	}
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
void CGameObject::Check_attack()
{

	switch (m_objectType)
	{
	case GameObjectType::Spider:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
	case GameObjectType::Pig:
	case GameObjectType::Snake:
	case GameObjectType::Raptor:
		if (m_anitype != 11) return;
		break;
	case GameObjectType::Snail:
	case GameObjectType::Wasp:
		if (m_anitype != 7) return;
		break;
	case GameObjectType::Wolf:
	case GameObjectType::Cow:
		if (m_anitype != 10) return;
		break;
	case GameObjectType::Toad:
		if (m_anitype != 9) return;
		break;
	default:
		break;
	}
	if (GameObjectType::Toad == m_objectType) {
		CAnimationSet* pAnimationSet = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_pSkinnedAnimationController->m_pAnimationTracks[m_anitype].m_nAnimationSet];
		auto animation_pos = m_pSkinnedAnimationController->m_pAnimationTracks[m_anitype].m_fPosition;
		if (animation_pos < pAnimationSet->m_fLength / 4) return;
	}
	else {
		CAnimationSet* pAnimationSet = m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_pSkinnedAnimationController->m_pAnimationTracks[m_anitype].m_nAnimationSet];
		auto animation_pos = m_pSkinnedAnimationController->m_pAnimationTracks[m_anitype].m_fPosition;
		if (animation_pos < pAnimationSet->m_fLength / 2) return;
	}
	// if attack animation
	// check hit player
	auto p_info = m_pScene->GetPlayerInfo();
	if (p_info) {
		if (m_pScene->CollisionCheck(this, p_info)) {
			if (false == p_info->invincibility) {
				auto obj = dynamic_cast<CMonsterObject*> (this);
				p_info->DecreaseHp(obj->GetAtk());
				p_info->SetInvincibility();

				// 밀려나기
				XMFLOAT3 monsterlook = obj->GetLook();
				const float KnockBackDistance = 10.f;
				XMFLOAT3 playerPos = p_info->GetPosition();

				XMFLOAT3 newPlayerPos;
				newPlayerPos.x = playerPos.x + monsterlook.x * KnockBackDistance;
				newPlayerPos.y = playerPos.y + monsterlook.y * KnockBackDistance;
				newPlayerPos.z = playerPos.z + monsterlook.z * KnockBackDistance;
				p_info->SetPosition(newPlayerPos);

				PlayerStateID currentState = p_info->m_pStateMachine->GetCurrentStateID();
				if (currentState != PlayerStateID::HitReaction && currentState != PlayerStateID::Dead) {
					p_info->m_pStateMachine->PerformStateChange(PlayerStateID::HitReaction, true);
				}

				auto& nwManager = NetworkManager::GetInstance();
				{
				auto pos = p_info->GetPosition();
					POSITION_PACKET p;
					p.position.x = pos.x;
					p.position.y = pos.y;
					p.position.z = pos.z;
					nwManager.PushSendQueue(p, p.size);
				}
				{
					SET_HP_HIT_OBJ_PACKET p;
					p.hit_obj_id = obj->m_id;
					p.hp = p_info->getHp();
					p.size = sizeof(SET_HP_HIT_OBJ_PACKET);
					p.type = static_cast<char>(E_PACKET::E_P_SETHP);
					nwManager.PushSendQueue(p, p.size);
				}
			}
		}
	}


}

void CGameObject::ChangeAnimation(ANIMATION_TYPE type)
{
	m_pSkinnedAnimationController->SetTrackEnable(m_anitype, false);

	switch (m_objectType)
	{
	case GameObjectType::Spider:
	case GameObjectType::Bat:
	case GameObjectType::Turtle:
	case GameObjectType::Pig:
	case GameObjectType::Snake:
	case GameObjectType::Raptor:
		switch (type)
		{
		case ANIMATION_TYPE::IDLE:
			m_anitype = 0;
			break;
		case ANIMATION_TYPE::WALK:
			m_anitype = 2;
			break;
		case ANIMATION_TYPE::RUN:
			m_anitype = 6;
			break;
		case ANIMATION_TYPE::DIE:
			m_anitype = 9;
			break;
		case ANIMATION_TYPE::HIT:
			m_anitype = 10;
			break;
		case ANIMATION_TYPE::ATTACK:
			m_anitype = 11;
			break;
		}
		break;
	case GameObjectType::Snail:
	case GameObjectType::Wasp:
		switch (type)
		{
		case ANIMATION_TYPE::IDLE:
			m_anitype = 0;
			break;
		case ANIMATION_TYPE::WALK:
			m_anitype = 2;
			break;
		case ANIMATION_TYPE::RUN:
			m_anitype = 2;
			break;
		case ANIMATION_TYPE::DIE:
			m_anitype = 5;
			break;
		case ANIMATION_TYPE::HIT:
			m_anitype = 6;
			break;
		case ANIMATION_TYPE::ATTACK:
			m_anitype = 7;
			break;
		}
		break;
	case GameObjectType::Wolf:
	case GameObjectType::Cow:
		switch (type)
		{
		case ANIMATION_TYPE::IDLE:
			m_anitype = 0;
			break;
		case ANIMATION_TYPE::WALK:
			m_anitype = 2;
			break;
		case ANIMATION_TYPE::RUN:
			m_anitype = 5;
			break;
		case ANIMATION_TYPE::DIE:
			m_anitype = 8;
			break;
		case ANIMATION_TYPE::HIT:
			m_anitype = 9;
			break;
		case ANIMATION_TYPE::ATTACK:
			m_anitype = 10;
			break;
		}
		break;
	case GameObjectType::Toad:
		switch (type)
		{
		case ANIMATION_TYPE::IDLE:
			m_anitype = 0;
			break;
		case ANIMATION_TYPE::WALK:
			m_anitype = 1;
			break;
		case ANIMATION_TYPE::RUN:
			m_anitype = 4;
			break;
		case ANIMATION_TYPE::DIE:
			m_anitype = 7;
			break;
		case ANIMATION_TYPE::HIT:
			m_anitype = 8;
			break;
		case ANIMATION_TYPE::ATTACK:
			m_anitype = 9;
			break;
		}
		break;
	default:	
		break;
	}
	
	m_pSkinnedAnimationController->m_pAnimationTracks[m_anitype].SetPosition(-ANIMATION_CALLBACK_EPSILON);
	m_pSkinnedAnimationController->SetTrackEnable(m_anitype, true);
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
	/*swprintf_s(buffer, L"SetMaterial: Index=%d, pMaterial=%p\n", nIndex, (void*)pMaterial);
	OutputDebugStringW(buffer);*/

	if (m_ppMaterials && (nIndex < m_nMaterials))
	{
		if (m_ppMaterials[nIndex]) m_ppMaterials[nIndex]->Release(); 
		m_ppMaterials[nIndex] = pMaterial;
		if (m_ppMaterials[nIndex]) m_ppMaterials[nIndex]->AddRef(); 
	}
	else {
		//OutputDebugStringW(L"  --> SetMaterial FAILED: Invalid index or m_ppMaterials is null.\n");
	}
}

bool CGameObject::CheckCollisionOBB(CGameObject* other)
{
	return m_worldOBB.Intersects(other->m_worldOBB);
}

void CGameObject::SetOBB(const XMFLOAT3& center, const XMFLOAT3& size, const XMFLOAT4& orientation)
{
	m_xmf3Position = center;
	m_xmf3Size = size;

	XMStoreFloat3(&m_localOBB.Center, XMLoadFloat3(&m_xmf3Position));
	XMStoreFloat3(&m_localOBB.Extents, XMLoadFloat3(&m_xmf3Size));
	XMStoreFloat4(&m_localOBB.Orientation, XMLoadFloat4(&orientation));


	if (m_pSibling) m_pSibling->SetOBB(center, size, orientation);
	if (m_pChild) m_pChild->SetOBB(center, size, orientation);
}

void CGameObject::SetOBB(float scalex, float scaley, float scalez, const XMFLOAT3& centerOffset)
{
	if (m_pMesh) {
		
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
			(minPos.x + maxPos.x) * 0.5f+centerOffset.x,
			(minPos.y + maxPos.y) * 0.5f+centerOffset.y,
			(minPos.z + maxPos.z) * 0.5f+centerOffset.z
		);
		m_localOBB.Extents = XMFLOAT3(
			(maxPos.x - minPos.x) * 0.5f * scalex,
			(maxPos.y - minPos.y) * 0.5f * scaley,
			(maxPos.z - minPos.z) * 0.5f * scalez
		);
		m_localOBB.Orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);  
	}

	if (m_pSibling) m_pSibling->SetOBB(scalex, scaley, scalez,centerOffset);
	if (m_pChild) m_pChild->SetOBB(scalex, scaley, scalez,centerOffset);
}

void CGameObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CShader* shader)
{
	if (m_pMesh) {
		
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
		m_localOBB.Orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		InitializeOBBResources(pd3dDevice, pd3dCommandList);
	}
	/*
	if (m_pSibling) m_pSibling->SetOBB();
	if (m_pChild) m_pChild->SetOBB();
	*/
}

BoundingOrientedBox CGameObject::GetOBB()
{
	if (m_pMesh) {
		return m_localOBB;
	}
	if (m_pSibling) return m_pSibling->GetOBB();
	if (m_pChild) return m_pChild->GetOBB();
}

void CGameObject::RenderOBB(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//m_OBBShader.Render(pd3dCommandList, NULL);
	m_OBBMaterial->m_pShader->Render(pd3dCommandList, NULL);

	
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	pd3dCommandList->IASetVertexBuffers(0, 1, &m_OBBVertexBufferView);
	pd3dCommandList->IASetIndexBuffer(&m_OBBIndexBufferView);

	
	pd3dCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0); 
}

void CGameObject::RenderOBB(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	
	if (!pCamera) return;

	
	bool bCanRenderCurrentObjectOBB = m_pOBBVertexBuffer &&
		m_pOBBIndexBuffer &&
		m_pd3dcbOBBTransform &&
		m_pcbMappedOBBTransform;

	if (bCanRenderCurrentObjectOBB) {

		
		XMMATRIX world = XMLoadFloat4x4(&m_xmf4x4World);
		XMMATRIX view = XMLoadFloat4x4(&pCamera->GetViewMatrix());
		XMMATRIX proj = XMLoadFloat4x4(&pCamera->GetProjectionMatrix());
		XMFLOAT4X4 wvpMatrix;
		
		XMStoreFloat4x4(&wvpMatrix, XMMatrixTranspose(world * view * proj));

		
		memcpy(m_pcbMappedOBBTransform, &wvpMatrix, sizeof(XMFLOAT4X4));

		pd3dCommandList->SetGraphicsRootConstantBufferView(0, m_pd3dcbOBBTransform->GetGPUVirtualAddress());

		
		pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST); 
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_OBBVertexBufferView);        
		pd3dCommandList->IASetIndexBuffer(&m_OBBIndexBufferView);              

		
		pd3dCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0); 
	}

	if (m_pSibling) {
        
            m_pSibling->RenderOBB(pd3dCommandList, pCamera);
        
    }
    if (m_pChild) {
       
            m_pChild->RenderOBB(pd3dCommandList, pCamera);
     
    }
}

void CGameObject::InitializeOBBResources(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
		
	if (m_pMesh)
	{
			
		XMFLOAT3 corners[8];
		m_localOBB.GetCorners(corners);

			
		ID3D12Resource* pVertexUploadBuffer = nullptr;
		m_pOBBVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, corners, sizeof(XMFLOAT3) * 8, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &pVertexUploadBuffer);
		if (!m_pOBBVertexBuffer) {
			//OutputDebugString(L"!!!!!!!! ERROR: Failed to create OBB Vertex Buffer! !!!!!!!!\n");
				
		}
		else {
			m_OBBVertexBufferView.BufferLocation = m_pOBBVertexBuffer->GetGPUVirtualAddress();
			m_OBBVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
			m_OBBVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * 8;
		}

			
		UINT indices[] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };
		UINT indices_test[] = { 0, 1, 2, 0, 2, 3 };

			
		ID3D12Resource* pIndexUploadBuffer = nullptr; 
		m_pOBBIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, indices, sizeof(UINT) * 24, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &pIndexUploadBuffer);
		if (!m_pOBBIndexBuffer) {
			//OutputDebugString(L"!!!!!!!! ERROR: Failed to create OBB Index Buffer! !!!!!!!!\n");
		}
		else {
			m_OBBIndexBufferView.BufferLocation = m_pOBBIndexBuffer->GetGPUVirtualAddress();
			m_OBBIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
			m_OBBIndexBufferView.SizeInBytes = sizeof(UINT) * 24;
		}

			
		UINT ncbElementBytes = (((sizeof(XMFLOAT4X4)) + 255) & ~255);
		m_pd3dcbOBBTransform = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		if (!m_pd3dcbOBBTransform) {
			//OutputDebugString(L"!!!!!!!! ERROR: Failed to create OBB Transform CBV! !!!!!!!!\n");
			m_pcbMappedOBBTransform = nullptr; 
		}
		else {
				
			HRESULT hr = m_pd3dcbOBBTransform->Map(0, NULL, (void**)&m_pcbMappedOBBTransform);
			if (FAILED(hr) || !m_pcbMappedOBBTransform) {
				//OutputDebugString(L"!!!!!!!! ERROR: Failed to map OBB Transform CBV! !!!!!!!!\n");
				m_pcbMappedOBBTransform = nullptr;
					
			}
		}
	}

	
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

	
	XMMATRIX worldMatrix = XMLoadFloat4x4(&m_xmf4x4World);

	
	XMVECTOR localCenter = XMLoadFloat3(&m_localOBB.Center);
	XMVECTOR worldCenter = XMVector3TransformCoord(localCenter, worldMatrix);
	XMStoreFloat3(&m_worldOBB.Center, worldCenter);

	
	XMMATRIX rotationMatrix = worldMatrix;
	rotationMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);  
	XMVECTOR orientation = XMQuaternionRotationMatrix(rotationMatrix);
	XMStoreFloat4(&m_worldOBB.Orientation, orientation);

	
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
	if (!pScene) return; 

	if (!isRender) return;

	
	CMaterial* pPrimaryMaterial = GetMaterial(0); 


	if (m_pMesh && pPrimaryMaterial && pPrimaryMaterial->m_pShader)
	{
		pScene->SetGraphicsState(pd3dCommandList, pPrimaryMaterial->m_pShader);


		
		if (pCamera && pCamera->GetCameraConstantBuffer()) {
			pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());
		}
		
		CShader* pCurrentShader = pPrimaryMaterial->m_pShader; 
		std::string shaderType = pCurrentShader->GetShaderType(); 
		if (shaderType == "Standard" || shaderType == "Skinned" /* || shaderType == "Instancing" */) {
			ID3D12Resource* pLightBuffer = pScene->GetLightsConstantBuffer(); 
			if (pLightBuffer) {
				pd3dCommandList->SetGraphicsRootConstantBufferView(2, pLightBuffer->GetGPUVirtualAddress());
			}

			pd3dCommandList->SetGraphicsRootDescriptorTable(4, pScene->GetShadowMapSrv());
		}

		//if (shaderType == "Standard") {
		//	// Standard 셰이더의 경우, 4번 슬롯에 그림자 맵 바인딩
		//	pd3dCommandList->SetGraphicsRootDescriptorTable(4, pScene->GetShadowMapSrv());
		//}
		//else if (shaderType == "Skinned") {
		//	// Skinned 셰이더의 경우, 6번 슬롯에 그림자 맵 바인딩
		//	pd3dCommandList->SetGraphicsRootDescriptorTable(6, pScene->GetShadowMapSrv());
		//}

		
		for (int i = 0; i < m_nMaterials; i++)
		{
			CMaterial* pMaterial = GetMaterial(i); 
			
			if (pMaterial && pMaterial->m_pShader == pCurrentShader) 
			{
				
				cbGameObjectInfo gameObjectInfo; 

				
				XMStoreFloat4x4(&gameObjectInfo.gmtxGameObject, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

				
				gameObjectInfo.gMaterialInfo.AmbientColor = pMaterial->m_xmf4AmbientColor;
				gameObjectInfo.gMaterialInfo.DiffuseColor = pMaterial->m_xmf4AlbedoColor;
				gameObjectInfo.gMaterialInfo.SpecularColor = pMaterial->m_xmf4SpecularColor;
				
				gameObjectInfo.gMaterialInfo.EmissiveColor = pMaterial->m_xmf4EmissiveColor;
				gameObjectInfo.gMaterialInfo.Glossiness = pMaterial->m_fGlossiness;
				gameObjectInfo.gMaterialInfo.Smoothness = pMaterial->m_fSmoothness;
				gameObjectInfo.gMaterialInfo.SpecularHighlight = pMaterial->m_fSpecularHighlight;
				gameObjectInfo.gMaterialInfo.Metallic = pMaterial->m_fMetallic;
				gameObjectInfo.gMaterialInfo.GlossyReflection = pMaterial->m_fGlossyReflection;
				
				gameObjectInfo.gnTexturesMask = 0;
				for (int texIdx = 0; texIdx < pMaterial->GetTextureCount(); ++texIdx) {
					
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
				
				pd3dCommandList->SetGraphicsRoot32BitConstants(1, 41, &gameObjectInfo, 0);

			
				D3D12_GPU_DESCRIPTOR_HANDLE textureTableHandle = pMaterial->GetTextureTableGpuHandle(); 
				if (textureTableHandle.ptr != 0) {
					
					pd3dCommandList->SetGraphicsRootDescriptorTable(3, textureTableHandle);
				}

				
				if (shaderType == "Skinned") {
					CSkinnedMesh* pSkinnedMesh = dynamic_cast<CSkinnedMesh*>(m_pMesh);
					if (pSkinnedMesh) {
						
						ID3D12Resource* pOffsetBuffer = pSkinnedMesh->m_pd3dcbBindPoseBoneOffsets;
                        if (pOffsetBuffer) {
                            pd3dCommandList->SetGraphicsRootConstantBufferView(5, pOffsetBuffer->GetGPUVirtualAddress());
                        }
						
						if (m_pSharedAnimController && 
							m_pSharedAnimController->m_ppd3dcbSkinningBoneTransforms &&
							m_pSharedAnimController->m_ppd3dcbSkinningBoneTransforms[0]) {
							pd3dCommandList->SetGraphicsRootConstantBufferView(6, m_pSharedAnimController->m_ppd3dcbSkinningBoneTransforms[0]->GetGPUVirtualAddress());
						}
						else {
							
							//OutputDebugStringW(L"!!! Render: Skinned - Failed to get valid Bone Transform buffer (b8) via m_pSharedAnimController!\n");
							//wchar_t dbgMsg[128];
							//swprintf_s(dbgMsg, L"    m_pSharedAnimController = %p\n", (void*)m_pSharedAnimController); 
							//OutputDebugStringW(dbgMsg);
							
						}
					}
				}



				
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

void CGameObject::RenderShadow(ID3D12GraphicsCommandList* pd3dCommandList)
{
	cbGameObjectInfo gameObjectInfo;
	XMStoreFloat4x4(&gameObjectInfo.gmtxGameObject, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 41, &gameObjectInfo, 0);

	if (m_pMesh)
	{
		if (m_pMesh->m_nSubMeshes > 0)
		{
			for (int i = 0; i < m_pMesh->m_nSubMeshes; i++)
			{
				m_pMesh->Render(pd3dCommandList, i);
			}
		}
		else
		{
			m_pMesh->Render(pd3dCommandList, 0);
		}
	}

	if (m_pSibling) m_pSibling->RenderShadow(pd3dCommandList);
	if (m_pChild) m_pChild->RenderShadow(pd3dCommandList);
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
	std::shared_ptr<CTexture> pTexture = nullptr; 
	
	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) 
		{
			
			for (int j = 0; j < m_ppMaterials[i]->GetTextureCount(); j++) // GetTextureCount
			{
				
				if (!_tcsncmp(m_ppMaterials[i]->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName)))
				{
					
					if (j < m_ppMaterials[i]->m_vTextures.size()) {
						return m_ppMaterials[i]->m_vTextures[j]; 
					}
				}
			}
		}
	}


	if (m_pSibling) {
		pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName);
		if (pTexture) return pTexture;
	}
	if (m_pChild) {
		pTexture = m_pChild->FindReplicatedTexture(pstrTextureName);
		if (pTexture) return pTexture; 
	}

	return nullptr; 
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
	
	assert(pGameFramework != nullptr && "GameFramework pointer is needed!");
	ShaderManager* pShaderManager = pGameFramework->GetShaderManager();
	ResourceManager* pResourceManager = pGameFramework->GetResourceManager();
	assert(pShaderManager != nullptr && "ShaderManager is not available!");
	assert(pResourceManager != nullptr && "ResourceManager is not available!");

	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	m_nMaterials = ReadIntegerFromFile(pInFile);

	//wchar_t buffer[128];
	//swprintf_s(buffer, L"LoadMaterialsFromFile: Expecting %d materials.\n", m_nMaterials);
	//OutputDebugStringW(buffer);

	if (m_nMaterials <= 0) return; 

	if (m_ppMaterials) delete[] m_ppMaterials; 
	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	CMaterial *pMaterial = NULL;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		/*	OutputDebugStringA("LoadMaterialsFromFile: Read Token: ");
		OutputDebugStringA(pstrToken);
		OutputDebugStringA("\n");*/


		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ReadIntegerFromFile(pInFile);
			//OutputDebugStringW((L"  Processing <Material> index: " + std::to_wstring(nMaterial) + L"\n").c_str());

			pMaterial = new CMaterial(7, pGameFramework); // Assume 7 textures for now
			//OutputDebugStringW((L"    new CMaterial result: " + std::wstring(pMaterial ? L"Success" : L"FAILED!") + L"\n").c_str());

			if (!pMaterial) continue; 

			
			UINT nMeshType = GetMeshType();
			std::string shaderName = "Standard"; 

		
			if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE) { 
				if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT) {
					shaderName = "Skinned";
				}
				else {
					shaderName = "Standard";
				}
			}
			else {
				
			}

			
			CShader* pMatShader = pShaderManager->GetShader(shaderName);
			if (pMatShader) {
				pMaterial->SetShader(pMatShader);
			}
			else {
				//OutputDebugStringA(("Error: Could not get shader '" + shaderName + "' from ShaderManager! Assigning default Standard shader.\n").c_str());
				
				pMatShader = pShaderManager->GetShader("Standard");
				if (pMatShader) {
					pMaterial->SetShader(pMatShader);
				}
			}
			

			SetMaterial(nMaterial, pMaterial); 
			//OutputDebugStringW((L"    SetMaterial called for index: " + std::to_wstring(nMaterial) + L"\n").c_str());
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
			//OutputDebugStringW(L"LoadMaterialsFromFile: Found </Materials>, exiting loop.\n");
			break;
		}
	}

	
	//for (int i = 0; i < m_nMaterials; ++i) {
	//	//swprintf_s(buffer, L"LoadMaterialsFromFile: Final check - Material[%d] pointer: %p\n", i, (void*)m_ppMaterials[i]);
	//	//OutputDebugStringW(buffer);
	//}
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
			XMFLOAT3 xmf3Position, xmf3EulerRotation, xmf3Scale;
			XMFLOAT4 xmf4QuaternionRotation;

			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3EulerRotation, sizeof(float), 3, pInFile); // Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4QuaternionRotation, sizeof(float), 4, pInFile); // Quaternion

			XMMATRIX mtxScale = XMMatrixScaling(xmf3Scale.x, xmf3Scale.y, xmf3Scale.z);
			XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(&xmf4QuaternionRotation));
			XMMATRIX mtxTranslate = XMMatrixTranslation(xmf3Position.x, xmf3Position.y, xmf3Position.z);

			//XMStoreFloat4x4(&pGameObject->m_xmf4x4ToParent, XMMatrixMultiply(XMMatrixMultiply(mtxScale, mtxRotate), mtxTranslate));


		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(float), 16, pInFile);
			//XMFLOAT4X4 xmf4x4TempWorldMatrix; // 임시 변수
			//nReads = (UINT)::fread(&xmf4x4TempWorldMatrix, sizeof(float), 16, pInFile);

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

	//_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	//OutputDebugString(pstrDebug);
	
	ofstream fout("Player.txt", ios::app);
	
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


				// 스킨드 메쉬 정보 준비
				if (pLoadedModel->m_pModelRootObject && pLoadedModel->m_nSkinnedMeshes > 0) {
					pLoadedModel->FindAndCacheSkinnedMeshes();
				}
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
	if (pInFile) fclose(pInFile);
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
		if (nReads != 1 || nStrLength >= sizeof(pstrToken))  
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
		pstrToken[nStrLength] = '\0';  

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
     CAnimationController* controllerToUse = m_pSkinnedAnimationController ? m_pSkinnedAnimationController : controller; 
     if (m_pMesh && dynamic_cast<CSkinnedMesh*>(m_pMesh)) {
         m_pSharedAnimController = controllerToUse; 
     }
     if (m_pChild) m_pChild->PropagateAnimController(controllerToUse); 
     if (m_pSibling) m_pSibling->PropagateAnimController(controller);    
 }

 void CGameObject::CopyDataFrom(CGameObject* pSource)
 {
	 // 메쉬와 머티리얼 포인터 공유
	 this->SetMesh(pSource->m_pMesh);

	 if (this->m_ppMaterials) delete[] this->m_ppMaterials;
	 this->m_nMaterials = pSource->m_nMaterials;
	 this->m_ppMaterials = nullptr;
	 if (this->m_nMaterials > 0) {
		 this->m_ppMaterials = new CMaterial * [this->m_nMaterials];
		 for (int i = 0; i < this->m_nMaterials; i++) this->m_ppMaterials[i] = nullptr;
	 }

	 for (int i = 0; i < pSource->m_nMaterials; i++) {
		 this->SetMaterial(i, pSource->m_ppMaterials[i]);
	 }

	 strcpy_s(this->m_pstrFrameName, 64, pSource->m_pstrFrameName);
	 this->m_objectType = pSource->m_objectType;
	 this->m_pGameFramework = pSource->m_pGameFramework;
	 this->m_localOBB = pSource->m_localOBB; 

	 this->hp = pSource->hp;
	 this->atk = pSource->atk;
	 this->level = pSource->level;
	 this->isRender = pSource->isRender;
	 this->_invincible = pSource->_invincible;

	 // 자식 계층 구조 복제
	 if (pSource->m_pChild) {
		 CGameObject* pClonedChild = pSource->m_pChild->Clone();
		 this->SetChild(pClonedChild, true);

		 CGameObject* pCurrentSourceSibling = pSource->m_pChild->m_pSibling;
		 CGameObject* pLastClonedSibling = pClonedChild;
		 while (pCurrentSourceSibling) {
			 CGameObject* pClonedSibling = pCurrentSourceSibling->Clone();
			 pClonedSibling->m_pParent = this;
			 pLastClonedSibling->m_pSibling = pClonedSibling;

			 pLastClonedSibling = pClonedSibling;
			 pCurrentSourceSibling = pCurrentSourceSibling->m_pSibling;
		 }
	 }
 }

 CGameObject* CGameObject::Clone()
 {
	 CGameObject* pNewInstance = new CGameObject(this->m_pGameFramework);
	 pNewInstance->CopyDataFrom(this);
	 return pNewInstance;
 }




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework) 
{
	assert(pGameFramework != nullptr && "GameFramework pointer is needed for CHeightMapTerrain!");
	ResourceManager* pResourceManager = pGameFramework->GetResourceManager();
	ShaderManager* pShaderManager = pGameFramework->GetShaderManager(); 
	assert(pResourceManager != nullptr && pShaderManager != nullptr);

	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	CHeightMapGridMesh *pMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, nWidth, nLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
	SetMesh(pMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	
	CMaterial* pTerrainMaterial = new CMaterial(2, pGameFramework);

	
	std::shared_ptr<CTexture> pTerrainBaseTexture = pResourceManager->GetTexture(L"Terrain/DemoTerrain3.dds", pd3dCommandList);
	std::shared_ptr<CTexture> pTerrainDetailTexture = pResourceManager->GetTexture(L"Terrain/TerrainGrass_basecolor.dds", pd3dCommandList);
	
	
	if (pTerrainBaseTexture) {
		pTerrainMaterial->AssignTexture(0, pTerrainBaseTexture, pd3dDevice); 
	}
	if (pTerrainDetailTexture) {
		pTerrainMaterial->AssignTexture(1, pTerrainDetailTexture, pd3dDevice); 
	}

	
	CShader* pTerrainShader = pShaderManager->GetShader("Terrain"); 
	if (pTerrainShader) {
		pTerrainMaterial->SetShader(pTerrainShader); 
	}
	else {
		//OutputDebugString(L"Error: Failed to get Terrain shader. Material will not have a shader.\n");
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


	CMaterial* pMaterial = GetMaterial(0); 
	if (m_pMesh && pMaterial && pMaterial->m_pShader)
	{
		
		pScene->SetGraphicsState(pd3dCommandList, pMaterial->m_pShader);

		
		if (pCamera && pCamera->GetCameraConstantBuffer()) {
			pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());
		}

		pd3dCommandList->SetGraphicsRootDescriptorTable(3, pScene->GetShadowMapSrv());

		UpdateTransform(NULL); 

		
		XMFLOAT4X4 gmtxGameObject; 
		XMStoreFloat4x4(&gmtxGameObject, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
		
		pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &gmtxGameObject, 0);

		
		D3D12_GPU_DESCRIPTOR_HANDLE textureTableHandle = pMaterial->GetTextureTableGpuHandle();
		if (textureTableHandle.ptr != 0) {
			
			pd3dCommandList->SetGraphicsRootDescriptorTable(2, textureTableHandle);
		}

		
		m_pMesh->Render(pd3dCommandList, 0);

	}
	
}


void CHeightMapTerrain::RenderShadow(ID3D12GraphicsCommandList* pd3dCommandList)
{
	// 월드 행렬을 단위 행렬로 만듭니다.
	XMMATRIX mtxIdentity = XMMatrixIdentity();

	cbGameObjectInfo gameObjectInfo;
	XMStoreFloat4x4(&gameObjectInfo.gmtxGameObject, XMMatrixTranspose(mtxIdentity));

	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 41, &gameObjectInfo.gmtxGameObject, 0); // 월드 행렬만 전달

	// 메시 전체를 그립니다.
	if (m_pMesh && m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			m_pMesh->Render(pd3dCommandList, i);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	assert(pGameFramework != nullptr && "GameFramework pointer is needed for CSkyBox!");
	ResourceManager* pResourceManager = pGameFramework->GetResourceManager();
	ShaderManager* pShaderManager = pGameFramework->GetShaderManager(); 
	assert(pResourceManager != nullptr && pShaderManager != nullptr);

	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	
	CMaterial* pSkyBoxMaterial = new CMaterial(1, pGameFramework);

	
	std::shared_ptr<CTexture> pSkyBoxTexture = pResourceManager->GetTexture(L"SkyBox/SkyBox_0.dds", pd3dCommandList);
	if (pSkyBoxTexture) {
		pSkyBoxMaterial->AssignTexture(0, pSkyBoxTexture, pd3dDevice); 
	}

	
	CShader* pSkyBoxShader = pShaderManager->GetShader("Skybox");
	if (pSkyBoxShader) {
		pSkyBoxMaterial->SetShader(pSkyBoxShader);
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

	CMaterial* pMaterial = GetMaterial(0); 
	if (m_pMesh && pMaterial && pMaterial->m_pShader)
	{
		pScene->SetGraphicsState(pd3dCommandList, pMaterial->m_pShader);

		
		if (pCamera->GetCameraConstantBuffer()) { 
			pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());
		}

		
		XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
		SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);
		UpdateTransform(NULL); 

		
		D3D12_GPU_DESCRIPTOR_HANDLE textureTableHandle = pMaterial->GetTextureTableGpuHandle();
		if (textureTableHandle.ptr != 0) {
			
			pd3dCommandList->SetGraphicsRootDescriptorTable(1, textureTableHandle);
		}

		
		m_pMesh->Render(pd3dCommandList, 0); 

	}
}

void CSkyBox::SetSkyboxIndex(int index)
{
	if (index >= 0 && index < m_vSkyboxTextures.size())
	{
		m_nCurrentTextureIndex = index;

		
		CMaterial* pMaterial = GetMaterial(0);
		if (pMaterial)
		{
			pMaterial->AssignTexture(0, m_vSkyboxTextures[index], m_pGameFramework->GetDevice());
		}
	}
}

void CSkyBox::LoadTextures(ID3D12GraphicsCommandList* cmdList, const std::vector<std::wstring>& texturePaths)
{
	ResourceManager* pRes = m_pGameFramework->GetResourceManager();
	for (const auto& path : texturePaths)
	{
		auto tex = pRes->GetTexture(path.c_str(), cmdList);
		if (tex) m_vSkyboxTextures.push_back(tex);
	}


	if (!m_vSkyboxTextures.empty())
	{
		CMaterial* pMaterial = GetMaterial(0);
		if (pMaterial)
		{
			pMaterial->AssignTexture(0, m_vSkyboxTextures[0], m_pGameFramework->GetDevice());
		}
	}
}




CMonsterObject::CMonsterObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* pModel, int nAnimationTracks, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	CLoadedModelInfo *pMonsterModel = pModel;
	if (!pMonsterModel) pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/Monster.bin", pGameFramework);

	SetChild(pMonsterModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pMonsterModel);

#ifdef ONLINE
	FSM_manager = nullptr;
#else
	FSM_manager = std::make_shared<FSMManager<CGameObject>>(this);
#endif

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

	if (pInFile) fclose(pInFile); 
}



void CTreeObject::StartFalling(const XMFLOAT3& hitDirection) {
	if (m_bIsFalling || m_bHasFallen) return; 

	m_bIsFalling = true;
	m_fFallingTimer = 0.0f;
	m_fCurrentFallAngle = 0.0f;
	m_xmf4x4InitialToParent = m_xmf4x4ToParent; 

	
	XMFLOAT3 worldUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3FallingAxis = Vector3::CrossProduct(worldUp, hitDirection); 
	if (Vector3::LengthSq(m_xmf3FallingAxis) < 0.001f) { 
		m_xmf3FallingAxis = XMFLOAT3(1.0f, 0.0f, 0.0f); 
	}
	m_xmf3FallingAxis = Vector3::Normalize(m_xmf3FallingAxis);

	
}

void CTreeObject::Animate(float fTimeElapsed) {
	

	if (m_bIsFalling && !m_bHasFallen) {
		m_fFallingTimer += fTimeElapsed;
		float normalizedTime = std::min(m_fFallingTimer / m_fFallingDuration, 1.0f);

		
		m_fCurrentFallAngle = m_fTargetFallAngle * normalizedTime; 

		
		XMMATRIX R = XMMatrixIdentity();
		if (Vector3::LengthSq(m_xmf3RotationPivot) > 0.001f) { 
			R = XMMatrixTranslation(-m_xmf3RotationPivot.x, -m_xmf3RotationPivot.y, -m_xmf3RotationPivot.z);
		}
		
		R = XMMatrixMultiply(R, XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3FallingAxis), m_fCurrentFallAngle));
		
		if (Vector3::LengthSq(m_xmf3RotationPivot) > 0.001f) {
			R = XMMatrixMultiply(R, XMMatrixTranslation(m_xmf3RotationPivot.x, m_xmf3RotationPivot.y, m_xmf3RotationPivot.z));
		}


		
		XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixMultiply(R, XMLoadFloat4x4(&m_xmf4x4InitialToParent)));

		if (normalizedTime >= 1.0f) {
			m_bHasFallen = true;
			m_bIsFalling = false;

			CScene* pScene = m_pGameFramework->GetScene(); 
			if (pScene) {
				int numBranchesToSpawn = 3 + (rand() % 2); 
				for (int i = 0; i < numBranchesToSpawn; ++i) {
					XMFLOAT3 fallenTreePos = GetPosition(); 
					XMFLOAT3 spawnOffsetLocal = XMFLOAT3(
						((float)(rand() % 200) - 100.0f) * 0.1f, // X -10 ~ +10
						(rand() % 10) + 10.0f,                     // Y 10~19
						((float)(rand() % 200) - 100.0f) * 0.1f  // Z -10 ~ +10
					);

					XMFLOAT3 spawnPos = Vector3::Add(fallenTreePos, spawnOffsetLocal);
					if (pScene->m_pTerrain) { 
						spawnPos.y = pScene->m_pTerrain->GetHeight(spawnPos.x, spawnPos.z) + spawnOffsetLocal.y;
					}

					XMFLOAT3 ejectVelocity = XMFLOAT3(
						((float)(rand() % 100) - 50.0f), 
						((float)(rand() % 60) + 50.0f),
						((float)(rand() % 100) - 50.0f)
					);
					pScene->SpawnBranch(spawnPos, ejectVelocity);
				}
			}
			isRender = false; 
		}
	}
	UpdateTransform(NULL);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
}


CPineObject::CPineObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/Tree/FAE_Pine_A_LOD0.bin", "rb");
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework); 
	SetChild(pGameObject);

	m_objectType = GameObjectType::Tree;

	if (pInFile) fclose(pInFile);
}

CPineObject::CPineObject(CGameFramework* pGameFramework)
	: CTreeObject()
{
}

CGameObject* CPineObject::Clone()
{
	CPineObject* pNewInstance = new CPineObject(m_pGameFramework);

	pNewInstance->CopyDataFrom(this);

	return pNewInstance;
}

CBirchObject::CBirchObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/Tree/FAE_Birch_A_LOD0.bin", "rb");
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework); 
	SetChild(pGameObject);

	m_objectType = GameObjectType::Tree;

	if (pInFile) fclose(pInFile);
}

CWillowObject::CWillowObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/Tree/FAE_Willow_A_LOD0.bin", "rb");
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	m_objectType = GameObjectType::Tree;

	if (pInFile) fclose(pInFile); 
}

CBranchObject::CBranchObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework, CHeightMapTerrain* pTerrain)
	: CGameObject(1, pGameFramework) { 
	m_pTerrainRef = pTerrain;

	CLoadedModelInfo* pBranchModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/Branch_A.bin", pGameFramework);
	if (pBranchModel && pBranchModel->m_pModelRootObject) {
		if (pBranchModel->m_pModelRootObject->m_pMesh)
			SetMesh(pBranchModel->m_pModelRootObject->m_pMesh);
		if (pBranchModel->m_pModelRootObject->m_nMaterials > 0 && pBranchModel->m_pModelRootObject->m_ppMaterials[0])
			SetMaterial(0, pBranchModel->m_pModelRootObject->m_ppMaterials[0]); 
		delete pBranchModel;
	}
	SetScale(5.0f, 5.0f, 5.0f);
}

void CItemObject::Animate(float fTimeElapsed) {
	if (!isRender) return; 

	if (m_bOnGround) {
		m_fElapsedAfterLanding += fTimeElapsed;
		if (m_fElapsedAfterLanding > m_fLifeTime) {
			isRender = false; 
		}
		return;
	}

	
    XMFLOAT3 gravityForceThisFrame = Vector3::ScalarProduct(m_xmf3Gravity, 10.0f);
    m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, gravityForceThisFrame);

	
	XMFLOAT3 xmf3Shift = Vector3::ScalarProduct(m_xmf3Velocity, 0.5f);
	XMFLOAT3 oldPos = GetPosition();
	XMFLOAT3 newPos = Vector3::Add(oldPos, xmf3Shift);;
	SetPosition(newPos);


	
	if (m_pTerrainRef) {
		XMFLOAT3 currentPos = GetPosition();
		
		float branchHeightOffset = (m_localOBB.Extents.y > 0) ? m_localOBB.Extents.y : 0.5f; 
		float terrainHeight = m_pTerrainRef->GetHeight(currentPos.x, currentPos.z) + branchHeightOffset;

		if (currentPos.y <= terrainHeight) {
			currentPos.y = terrainHeight;
			SetPosition(currentPos); 
			m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f); 
			m_bOnGround = true;
			m_fElapsedAfterLanding = 0.0f; 
		}
	}
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

	if (pInFile) fclose(pInFile); 
}

// ------------------ ??------------------
CRockClusterBObject::CRockClusterBObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/RockCluster_B_LOD0.bin", "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	m_objectType = GameObjectType::Rock;

	if (pInFile) fclose(pInFile); 
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

	if (pInFile) fclose(pInFile); 
}

CCliffFObject::CCliffFObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/Cliff_A_LOD0.bin", "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	m_objectType = GameObjectType::Rock;

	if (pInFile) fclose(pInFile); 
}

CRockDropObject::CRockDropObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework, CHeightMapTerrain* pTerrain)
	: CGameObject(1, pGameFramework) { 
	m_pTerrainRef = pTerrain;

	CLoadedModelInfo* pBranchModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/RockCluster_B_LOD0.bin", pGameFramework);
	if (pBranchModel && pBranchModel->m_pModelRootObject) {
		if (pBranchModel->m_pModelRootObject->m_pMesh)
			SetMesh(pBranchModel->m_pModelRootObject->m_pMesh);
		if (pBranchModel->m_pModelRootObject->m_nMaterials > 0 && pBranchModel->m_pModelRootObject->m_ppMaterials[0])
			SetMaterial(0, pBranchModel->m_pModelRootObject->m_ppMaterials[0]);
		delete pBranchModel;
	}
	SetScale(50.0f, 50.0f, 50.0f);
}

void CRockObject::EraseRock()
{
	CScene* pScene = m_pGameFramework->GetScene(); 
	if (pScene) {
		int numBranchesToSpawn = 3 + (rand() % 2); 
		for (int i = 0; i < numBranchesToSpawn; ++i) {
			XMFLOAT3 fallenTreePos = GetPosition();
			XMFLOAT3 spawnOffsetLocal = XMFLOAT3(
				((float)(rand() % 200) - 100.0f) * 0.1f, // X -10 ~ +10
				(rand() % 10) + 10.0f,                     // Y 10~19
				((float)(rand() % 200) - 100.0f) * 0.1f  // Z -10 ~ +10
			);

			XMFLOAT3 spawnPos = Vector3::Add(fallenTreePos, spawnOffsetLocal);
			if (pScene->m_pTerrain) { 
				spawnPos.y = pScene->m_pTerrain->GetHeight(spawnPos.x, spawnPos.z) + spawnOffsetLocal.y;
			}

			XMFLOAT3 ejectVelocity = XMFLOAT3(
				((float)(rand() % 100) - 50.0f),
				((float)(rand() % 60) + 50.0f),
				((float)(rand() % 100) - 50.0f)
			);
			pScene->SpawnRock(spawnPos, ejectVelocity);
		}
	}

	isRender = false;
}




CBushAObject::CBushAObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/Vegetation/Bush_A_LOD0.bin", "rb");
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework); 
	SetChild(pGameObject);

	m_objectType = GameObjectType::Vegetation;

	if (pInFile) fclose(pInFile); 
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

	if (pInFile) fclose(pInFile); 
}

CStaticObject::CStaticObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* modelname, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, modelname, "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	if (pInFile) fclose(pInFile); 
}

UserObject::UserObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* pModel, int nAnimationTracks, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	CLoadedModelInfo* pUserModel = pModel;
	if (!pUserModel) pUserModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Hu_M_FullBody.bin", pGameFramework);
	SetChild(pUserModel->m_pModelRootObject, true);

	AddObject(pd3dDevice, pd3dCommandList, "thumb_02_r", "Model/Sword_01.bin", pGameFramework, XMFLOAT3(0.05, 0.00, -0.05), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	AddObject(pd3dDevice, pd3dCommandList, "Helmet", "Model/Hair_01.bin", pGameFramework, XMFLOAT3(0, 0.1, 0), XMFLOAT3(0,0,0), XMFLOAT3(1, 1, 1));
	//AddWeapon(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Boots_Peasant_Armor", "Model/Boots_Peasant_Armor.bin");
	//AddObject(pd3dDevice, pd3dCommandList, "spine_01", "Model/Torso_Peasant_03_Armor.bin", pGameFramework, XMFLOAT3(-0.25, 0.1, 0), XMFLOAT3(90, 0, 90), XMFLOAT3(1, 1, 1));

	XMFLOAT3 offset{ -0.230000, 0.040000, -0.010000 }, scale{ 1.10000, 1.250000, 1.150000 };
	AddObject(pd3dDevice, pd3dCommandList, "spine_01", "Model/Torso_Peasant_03_Armor.bin", pGameFramework, offset, XMFLOAT3(85, 0, 90), scale);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pUserModel);
}

UserObject::~UserObject()
{
}

void UserObject::AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* framename, char* modelname, CGameFramework* pGameFramework)
{
	CGameObject* handFrame = FindFrame(framename);
	if (handFrame) {
		CGameObject* weapon = new CStaticObject(pd3dDevice, pd3dCommandList, modelname, pGameFramework);
		weapon->SetPosition(0, 0, 0);
		weapon->SetScale(1, 1, 1);
		weapon->Rotate(0.0f, 0.0f, 0.0f);

		handFrame->SetChild(weapon);
		UpdateTransform(nullptr); 
	}
}
void UserObject::AddObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* framename, char* modelname, CGameFramework* pGameFramework, XMFLOAT3 offset, XMFLOAT3 rotate = { 0,0,0 }, XMFLOAT3 scale = { 1,1,1 })
{
	CGameObject* handFrame = FindFrame(framename);
	if (handFrame) {
		CGameObject* weapon = new CStaticObject(pd3dDevice, pd3dCommandList, modelname, pGameFramework);
		weapon->SetPosition(offset);
		weapon->SetScale(scale.x, scale.y, scale.z);
		weapon->Rotate(rotate.x, rotate.y, rotate.z);

		handFrame->SetChild(weapon);
		UpdateTransform(nullptr); 
	}
}

CGameObject* UserObject::FindFrame(char* framename)
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

#include "../../Server/Global.h"
void UserObject::ChangeAnimation(PlayerInput inputData)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackEnable(on_track, false);

	if (inputData.Attack) on_track = 12;
	else if (inputData.Run) {
		if (inputData.MoveForward) on_track = 5;
		else if (inputData.MoveBackward) on_track = 6;
		else if (inputData.WalkLeft) on_track = 7;
		else if (inputData.WalkRight) on_track = 8;
	}
	else if (inputData.MoveForward) on_track = 1;
	else if (inputData.MoveBackward) on_track = 2;
	else if (inputData.WalkLeft) on_track = 3;
	else if (inputData.WalkRight) on_track = 4;
	else if (inputData.Jump) {}
	else if (inputData.Interact) {}
	else on_track = 0;

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackEnable(on_track, true);
}


void CGameObject::SetColor(const XMFLOAT4& color)
{
	m_xmf4DebugColor = color;
}

CConstructionObject::CConstructionObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/buildobject/pannel.bin", "rb");
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework); 
	SetChild(pGameObject);

	
	if (pInFile) fclose(pInFile); 
}

CRockShardEffect::CRockShardEffect(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* framework)
	: CGameObject(1, framework)
{
	m_pGameFramework = framework;

	FILE* pInFile = nullptr;
	::fopen_s(&pInFile, "Model/Branch_A.bin", "rb");
	if (!pInFile) {
		return;
	}
	::rewind(pInFile);

	CGameObject* rockObj = CGameObject::LoadFrameHierarchyFromFile(
		device, cmdList, NULL, pInFile, NULL, framework);

	if (pInFile) fclose(pInFile);

	if (rockObj && rockObj->m_pMesh)
	{
		SetMesh(rockObj->m_pMesh); // 파편 메시에 복사
	}

	m_ppMaterials = new CMaterial * [1];
	m_ppMaterials[0] = new CMaterial(0, framework);
	m_nMaterials = 1;

	isRender = true;
}

void CRockShardEffect::Activate(const XMFLOAT3& position, const XMFLOAT3& velocity)
{

	char buf[256];
	sprintf_s(buf, "✅ Activate called! pos=(%.2f, %.2f, %.2f), vel=(%.2f, %.2f, %.2f)\n",
		position.x, position.y, position.z,
		velocity.x, velocity.y, velocity.z);
	OutputDebugStringA(buf);


	SetPosition(position);
	SetScale(1.0f, 1.0f, 1.0f);
	m_vVelocity = velocity;
	m_fElapsedTime = 0.0f;
	isRender = true;
	m_bActive = true;
}

void CRockShardEffect::Update(float deltaTime)
{
	if (!m_bActive) {
		//OutputDebugStringA("❌ Update skipped (not active)\n");
		return;
	}

	//char buf[128];
	//sprintf_s(buf, "🌀 Update: elapsed=%.2f / %.2f\n", m_fElapsedTime, m_fLifeTime);
	//OutputDebugStringA(buf);

	char buf[128];
	sprintf_s(buf, "🧭 deltaTime = %.4f\n", deltaTime);
	OutputDebugStringA(buf);
	m_fElapsedTime += deltaTime;
	/*
	if (m_fElapsedTime > m_fLifeTime)
	{
		isRender = false;
		m_bActive = false;
		return;
	}
	
	*/
	m_vVelocity.y -= 9.8f * deltaTime;

	XMFLOAT3 pos = GetPosition();
	pos.x += m_vVelocity.x * deltaTime;
	pos.y += m_vVelocity.y * deltaTime;
	pos.z += m_vVelocity.z * deltaTime;
	SetPosition(pos);

}
