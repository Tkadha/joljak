//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Scene.h"
#include "GameFramework.h"

#include <algorithm>


// ?∑‚ë¶???Í≥∏ÎãîÊø??Íæ®Îññ???¥—ä‚ÄúÔß£?(HLSL??cbGameObjectInfo ?? ??±ÌäÇ??ÅÎπû ??
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
	// ??16 + 24 + 1 = 41 DWORDS
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
		if (m_ppMaterials[nIndex]) m_ppMaterials[nIndex]->Release(); // Êπ≤Í≥ó????Ï≠???ÅÏ†£
		m_ppMaterials[nIndex] = pMaterial;
		if (m_ppMaterials[nIndex]) m_ppMaterials[nIndex]->AddRef(); // ????Ï≠?Ôß°Î™Ñ??ÔßùÏï∑?
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

// ÔßéÎ∂ø???Í≥óÏî†?Í≥ïÏ§à Ë´õÎ∂ø???Ë´õÎ∫§??ÔßçÎöÆÎ±æÊπ≤?
void CGameObject::SetOBB()
{
	if (m_pMesh) {
		// ÔßéÎ∂ø???Í≥óÏî†?Í≥ïÏ§à OBB ÔßçÎöÆÎ±æÊπ≤?
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
		m_localOBB.Orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);  // ?•ÎçáÎ¶????üæ ??ÅÏì¨
	}

	if (m_pSibling) m_pSibling->SetOBB();
	if (m_pChild) m_pChild->SetOBB();
}

void CGameObject::SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CShader* shader)
{
	if (m_pMesh) {
		// ÔßéÎ∂ø???Í≥óÏî†?Í≥ïÏ§à OBB ÔßçÎöÆÎ±æÊπ≤?
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

	if (m_pSibling) m_pSibling->SetOBB();
	if (m_pChild) m_pChild->SetOBB();
}

void CGameObject::RenderOBB(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//m_OBBShader.Render(pd3dCommandList, NULL);
	m_OBBMaterial->m_pShader->Render(pd3dCommandList, NULL);

	// OBB ?Ï¢éÏì£ Ê¥πÎ™É?ÅÊπ≤??Íæ™Î∏≥ ??ºÏ†ô
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	pd3dCommandList->IASetVertexBuffers(0, 1, &m_OBBVertexBufferView);
	pd3dCommandList->IASetIndexBuffer(&m_OBBIndexBufferView);

	// ??Line) OBB Ê¥πÎ™É?ÅÊπ≤?
	pd3dCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0); // 12Â™???= 24Â™??Î™ÉÎú≥??
}

void CGameObject::RenderOBB(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	// Ïπ¥Î©î???†Ìö®??Í≤Ä?¨Îäî ?ÑÏàò
	if (!pCamera) return;

	// 1. ?ÑÏû¨ ?§Î∏å?ùÌä∏??OBBÎ•?Í∑∏Î¶¥ ???àÎäîÏßÄ ?ïÏù∏?òÍ≥†, Í∑∏Î¶¥ ???àÎã§Î©?Í∑∏Î¶Ω?àÎã§.
	bool bCanRenderCurrentObjectOBB = m_pOBBVertexBuffer &&
		m_pOBBIndexBuffer &&
		m_pd3dcbOBBTransform &&
		m_pcbMappedOBBTransform;

	if (bCanRenderCurrentObjectOBB) {

		// 1. OBB??WVP(World * View * Projection) ?âÎ†¨ Í≥ÑÏÇ∞
		XMMATRIX world = XMLoadFloat4x4(&m_xmf4x4World);
		XMMATRIX view = XMLoadFloat4x4(&pCamera->GetViewMatrix());
		XMMATRIX proj = XMLoadFloat4x4(&pCamera->GetProjectionMatrix());
		XMFLOAT4X4 wvpMatrix;
		// HLSL?Ä row-major Í∏∞Î≥∏, C++??row-major -> HLSL?êÏÑú transpose ???òÎ†§Î©??¨Í∏∞??transpose
		XMStoreFloat4x4(&wvpMatrix, XMMatrixTranspose(world * view * proj));

		// 2. OBB ?ÅÏàò Î≤ÑÌçº ?ÖÎç∞?¥Ìä∏ (b0)
		memcpy(m_pcbMappedOBBTransform, &wvpMatrix, sizeof(XMFLOAT4X4));

		// 3. ?ÅÏàò Î≤ÑÌçº Î∞îÏù∏??(OBB Î£®Ìä∏ ?úÎ™Ö???åÎùºÎØ∏ÌÑ∞ ?∏Îç±??0Î≤?
		pd3dCommandList->SetGraphicsRootConstantBufferView(0, m_pd3dcbOBBTransform->GetGPUVirtualAddress());

		// 4. IA(Input Assembler) ?§Ï†ï
		pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST); // ?ºÏù∏ Î¶¨Ïä§??
		pd3dCommandList->IASetVertexBuffers(0, 1, &m_OBBVertexBufferView);        // ?ïÏ†ê Î≤ÑÌçº
		pd3dCommandList->IASetIndexBuffer(&m_OBBIndexBufferView);              // ?∏Îç±??Î≤ÑÌçº

		// 5. Í∑∏Î¶¨Í∏?
		pd3dCommandList->DrawIndexedInstanced(24, 1, 0, 0, 0); // ?∏Îç±??24Í∞?(??12Í∞?
	}

	if (m_pSibling) {
        //if (m_pSibling->ShouldRenderOBB()) { // ?ïÏ†úÍ∞Ä OBB ?åÎçîÎß??Ä?ÅÏù∏ÏßÄ ?ïÏù∏
            m_pSibling->RenderOBB(pd3dCommandList, pCamera);
        //}
    }
    if (m_pChild) {
        //if (m_pChild->ShouldRenderOBB()) { // ?êÏãù??OBB ?åÎçîÎß??Ä?ÅÏù∏ÏßÄ ?ïÏù∏
            m_pChild->RenderOBB(pd3dCommandList, pCamera);
        //}
    }
}

	void CGameObject::InitializeOBBResources(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	{
		// Î©îÏâ¨ ?†Ìö®??Í≤Ä????Ï∂îÍ? Í∞Ä??
		if (m_pMesh)
		{
			// OBB Î™®ÏÑúÎ¶??∞Ïù¥??
			XMFLOAT3 corners[8];
			m_localOBB.GetCorners(corners); // m_worldOBBÍ∞Ä ?†Ìö®?úÏ? Î®ºÏ? ?ïÏù∏ ?ÑÏöî

			// 2. OBB ?ïÏ†ê Î≤ÑÌçº ?ùÏÑ± (+ HRESULT ?ïÏù∏)
			ID3D12Resource* pVertexUploadBuffer = nullptr; // ?ÑÏãú ?ÖÎ°ú??Î≤ÑÌçº ?¨Ïù∏??
			m_pOBBVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, corners, sizeof(XMFLOAT3) * 8, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &pVertexUploadBuffer);
			if (!m_pOBBVertexBuffer) {
				OutputDebugString(L"!!!!!!!! ERROR: Failed to create OBB Vertex Buffer! !!!!!!!!\n");
				// ?§Ìå® ???¥ÌõÑ Î¶¨ÏÜå???ùÏÑ± Ï§ëÎã® ?êÎäî ?§Î•∏ Ï≤òÎ¶¨
			}
			else {
				m_OBBVertexBufferView.BufferLocation = m_pOBBVertexBuffer->GetGPUVirtualAddress();
				m_OBBVertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_OBBVertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * 8;
			}

			// 3. OBB ?∏Îç±???∞Ïù¥???ïÏùò (Î≥ÄÍ≤??ÜÏùå)
			UINT indices[] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7 };
			UINT indices_test[] = { 0, 1, 2, 0, 2, 3 };

			// 4. OBB ?∏Îç±??Î≤ÑÌçº ?ùÏÑ± (+ HRESULT ?ïÏù∏)
			ID3D12Resource* pIndexUploadBuffer = nullptr; // ?ÑÏãú ?ÖÎ°ú??Î≤ÑÌçº ?¨Ïù∏??
			m_pOBBIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, indices, sizeof(UINT) * 24, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &pIndexUploadBuffer);
			if (!m_pOBBIndexBuffer) {
				OutputDebugString(L"!!!!!!!! ERROR: Failed to create OBB Index Buffer! !!!!!!!!\n");
			}
			else {
				m_OBBIndexBufferView.BufferLocation = m_pOBBIndexBuffer->GetGPUVirtualAddress();
				m_OBBIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
				m_OBBIndexBufferView.SizeInBytes = sizeof(UINT) * 24;
			}

			// 5. OBB Î≥Ä???âÎ†¨???ÅÏàò Î≤ÑÌçº ?ùÏÑ± (+ HRESULT ?ïÏù∏)
			UINT ncbElementBytes = (((sizeof(XMFLOAT4X4)) + 255) & ~255);
			m_pd3dcbOBBTransform = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
			if (!m_pd3dcbOBBTransform) {
				OutputDebugString(L"!!!!!!!! ERROR: Failed to create OBB Transform CBV! !!!!!!!!\n");
				m_pcbMappedOBBTransform = nullptr; // ÎßµÌïë ?¨Ïù∏?∞ÎèÑ null Ï≤òÎ¶¨
			}
			else {
				// ÎßµÌïë???¨Ïù∏???Ä??(+ HRESULT ?ïÏù∏)
				HRESULT hr = m_pd3dcbOBBTransform->Map(0, NULL, (void**)&m_pcbMappedOBBTransform);
				if (FAILED(hr) || !m_pcbMappedOBBTransform) {
					OutputDebugString(L"!!!!!!!! ERROR: Failed to map OBB Transform CBV! !!!!!!!!\n");
					m_pcbMappedOBBTransform = nullptr; // ?§Ìå® ??null Ï≤òÎ¶¨
					// ?ÑÏöî??m_pd3dcbOBBTransform Release Í≥†Î†§
				}
			}
		}

	// ?Î®?ñá/?Î∫§Ï†£ Â™õÏïπÍª???? ?Î™ÑÌÖß (Êπ≤Í≥ó???ÑÎ∂æÎ±??Ï¢?)
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
	rotationMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);  // ??ÄÎ£??ÍπÖÌÖá ??ìÍµÖ
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
	if (!pScene) return; // ????ÅÏëùÔß????úëÔß??∫Îçá?

	if (!isRender) return;

	// ??Â™õÏïπÍªúÂ™õ? ÔßûÍ≥∏?????úëÔßçÍ≥πÎ∏?ÔßéÎ∂ø??? Ôß?Ë∏∞Îçâ????Ï≠??Í≥óÏî†?Î∂? Â™õ¬ÄÔßû¬Ä????àÎíóÔßû¬Ä ?Î∫§Ïî§
	CMaterial* pPrimaryMaterial = GetMaterial(0); // ?Í≥πÍπ≠ ??ºÏ†ô Êπ≤Í≥ó???∞Ï§à Ôß?Ë∏∞Îçâ????Ï≠?????


	if (m_pMesh && pPrimaryMaterial && pPrimaryMaterial->m_pShader)
	{
		pScene->SetGraphicsState(pd3dCommandList, pPrimaryMaterial->m_pShader);


		// --- ?®ÎìØ??CBV Ë´õÎ∂ø???---
		// ÁßªÎ?Ï∞??CBV (b1 @ ?Î™ÉÎú≥??0)
		if (pCamera && pCamera->GetCameraConstantBuffer()) {
			pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());
		}
		// Ë≠∞Í≥ïÏ±?CBV (b4 @ ?Î™ÉÎú≥??2) - Standard/Skinned/Instancing ??ÂØÉÏéå??Î®?≠î Ë´õÎ∂ø???
		CShader* pCurrentShader = pPrimaryMaterial->m_pShader; // ?Î™ÑÏìΩ??
		std::string shaderType = pCurrentShader->GetShaderType(); // GetShaderType() ??•Îãî ?Íæ©ÏäÇ
		if (shaderType == "Standard" || shaderType == "Skinned" /* || shaderType == "Instancing" */) {
			ID3D12Resource* pLightBuffer = pScene->GetLightsConstantBuffer(); // CScene ??•Îãî ???πê ?Î¨éÎ†ê
			if (pLightBuffer) {
				pd3dCommandList->SetGraphicsRootConstantBufferView(2, pLightBuffer->GetGPUVirtualAddress());
			}
		}


		// ??GameObject????ÅÎ∏≥ Ôßè‚ë§Î±?ÔßéÎ∂ø????Ï≠???øÎøâ ????Ë´õÏÑé??
		for (int i = 0; i < m_nMaterials; i++)
		{
			CMaterial* pMaterial = GetMaterial(i); // ?Íæ©Ïò± ??Ï≠?
			// ?Íæ©Ïò± ??Ï≠èÊÄ?Primary ??Ï≠???Í≥óÏî†?Î∂? ??ª‚Ö§Ôß?SetGraphicsState ??ºÎñÜ ?Î™ÑÌÖß? (ËπÇÎì≠???ÔßùÏï∑?, ??∞Îñí ??∏ÏôÇ)
			if (pMaterial && pMaterial->m_pShader == pCurrentShader) // Â™õÏàà? ?Í≥óÏî†?Î∂? ?????éÎíó ??Ï≠èÔßç?Ê¥πÎ™É??(??•Îãö??
			{
				// --- ?±—äÎÉº??Ë´õÎ∂ø???---
				// ?Íæ©Ïò± CShader (Ë´?RootSignature)??CScene::Render?Î®?Ωå ??Ä? ??ºÏ†ô??
				// 1. Â™õÏïπÍªúËπÇ??Í≥∏Îãî ??ÖÎú≤??ÑÎìÉ (?∑‚ë¶???Í≥∏Îãî b2 ????
				cbGameObjectInfo gameObjectInfo; // C++ ?¥—ä‚ÄúÔß£??Î™ÑÎí™??ÅÎí™

				// 1.1. ?Î∂æÎ±∂ ËπÇ¬Ä????∞Ï†π ??ºÏ†ô
				XMStoreFloat4x4(&gameObjectInfo.gmtxGameObject, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

				// 1.2. ??Ï≠??Î∫£ÎÇ´ Ôß?æ©??π≤?
				gameObjectInfo.gMaterialInfo.AmbientColor = pMaterial->m_xmf4AmbientColor;
				gameObjectInfo.gMaterialInfo.DiffuseColor = pMaterial->m_xmf4AlbedoColor;
				gameObjectInfo.gMaterialInfo.SpecularColor = pMaterial->m_xmf4SpecularColor;
				// ?? Specular Power??Alpha?????ŒΩÎª??ª„àÉ gameObjectInfo.gMaterialInfo.SpecularColor.w = pMaterial->m_fGlossiness;
				gameObjectInfo.gMaterialInfo.EmissiveColor = pMaterial->m_xmf4EmissiveColor;
				gameObjectInfo.gMaterialInfo.Glossiness = pMaterial->m_fGlossiness;
				gameObjectInfo.gMaterialInfo.Smoothness = pMaterial->m_fSmoothness;
				gameObjectInfo.gMaterialInfo.SpecularHighlight = pMaterial->m_fSpecularHighlight;
				gameObjectInfo.gMaterialInfo.Metallic = pMaterial->m_fMetallic;
				gameObjectInfo.gMaterialInfo.GlossyReflection = pMaterial->m_fGlossyReflection;
				// Padding?? ?•ÎçáÎ¶???Íæ©ÏäÇ ??ÅÏì¨

				// 1.3. ??øÎí™Ôß?ÔßçÎçâ?????ºÏ†ô
				gameObjectInfo.gnTexturesMask = 0;
				for (int texIdx = 0; texIdx < pMaterial->GetTextureCount(); ++texIdx) {
					// GetTexture ??•ÎãîÂ™õ¬Ä shared_ptr Ë∏∞‚â´ÍΩ£Áëú??Î∫§Ïî§??çÌÄ?raw ?????Ë´õÏÑë??
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
				// ... ??ª‚Ö® ??øÎí™Ôß?????ÜÏî† ??àÎñéÔß??∞Î∂Ω? ...

				// 1.4. ?∑‚ë¶???Í≥∏Îãî Ë´õÎ∂ø???(Standard/Skinned ?∑‚ë¶????ïÏ±∏?????î™Ë™òÎ™ÖÍΩ??Î™ÉÎú≥??1Ë∏?
				pd3dCommandList->SetGraphicsRoot32BitConstants(1, 41, &gameObjectInfo, 0);

				// 2. ??Ï≠???øÎí™Ôß?Ë´õÎ∂ø???(Descriptor Table ????Â™õ¬Ä??
				// Standard/Skinned ?∑‚ë¶????ïÏ±∏?????î™Ë™òÎ™ÖÍΩ??Î™ÉÎú≥??3Ë∏∞Îçâ??t6-t12 ??øÎí™Ôß????î†?âÎ∂ø???âÏì¨
				D3D12_GPU_DESCRIPTOR_HANDLE textureTableHandle = pMaterial->GetTextureTableGpuHandle(); // ??Ï≠???Î®?ñä????øÎí™Ôß????î†????ñÏòâ ?Î™ÉÎ±æ?????∏ò????(CTexture Êø°ÏíïÎµ??ø¬Ä??????ºÏ†ô ?Íæ©ÏäÇ)
				if (textureTableHandle.ptr != 0) {
					// ?∑‚ë¶?????î™Ë™òÎ™ÖÍΩ??Î™ÉÎú≥??3Ë∏∞ÎçâÎø???øÎí™Ôß????î†????ñÏòâ ?Î™ÉÎ±æ Ë´õÎ∂ø???
					pd3dCommandList->SetGraphicsRootDescriptorTable(3, textureTableHandle);
				}

				// 3. ??ΩÍ∂é???ø¬Ä??CBV Ë´õÎ∂ø???(Skinned ?∑‚ë¶????ïÏ±∏ ??????
		   // ?Íæ©Ïò± ?Í≥óÏî†?Î∂? Skinned ?Î™? ?Î∫§Ïî§??éÎíó Êø°ÏíñÏ≠????âÏëùÔß????´Îó≠??(?? shader->GetType())
				 // --- ??ΩÍ∂é??CBV Ë´õÎ∂ø???(?Í≥óÏî†??????"Skinned" ?Î∫§Ïî§ ?? ---
				if (shaderType == "Skinned") {
					CSkinnedMesh* pSkinnedMesh = dynamic_cast<CSkinnedMesh*>(m_pMesh);
					if (pSkinnedMesh) {
						// 3.1. Ëπ???ΩÎ¥Ω??Ë∏∞Íæ™??Ë´õÎ∂ø???(b7, ???î™Ë™òÎ™ÖÍΩ??Î™ÉÎú≥??4)
						ID3D12Resource* pOffsetBuffer = pSkinnedMesh->m_pd3dcbBindPoseBoneOffsets;
                        if (pOffsetBuffer) {
                            pd3dCommandList->SetGraphicsRootConstantBufferView(4, pOffsetBuffer->GetGPUVirtualAddress());
                        }
						// 3.2. Ëπ?ËπÇ¬Ä??Ë∏∞Íæ™??Ë´õÎ∂ø???(b8, ???î™Ë™òÎ™ÖÍΩ??Î™ÉÎú≥??5)
						if (m_pSharedAnimController && // ???Î™ÉÎ±∂?????ŒªÎß??å‚ë¶?ÉÊø°?ªÏú≠ ??????Î∫§Ïî§
							m_pSharedAnimController->m_ppd3dcbSkinningBoneTransforms &&
							m_pSharedAnimController->m_ppd3dcbSkinningBoneTransforms[0]) {
							pd3dCommandList->SetGraphicsRootConstantBufferView(5, m_pSharedAnimController->m_ppd3dcbSkinningBoneTransforms[0]->GetGPUVirtualAddress());
						}
						else {
							// Êø°Ïíì???∞Ïíï?? ?å‚ë¶?ÉÊø°?ªÏú≠ ????Í≥? null ?Î™?, ?Íæ®Îï≤Ôß???Ä? Ë∏∞Íæ™?ÅÂ™õ? null ?Î™? ?Î∫§Ïî§
							OutputDebugStringW(L"!!! Render: Skinned - Failed to get valid Bone Transform buffer (b8) via m_pSharedAnimController!\n");
							wchar_t dbgMsg[128];
							swprintf_s(dbgMsg, L"    m_pSharedAnimController = %p\n", (void*)m_pSharedAnimController); // ?????Â™?Êø°Ïíì??
							OutputDebugStringW(dbgMsg);
							// ?Íæ©ÏäÇ??m_pSharedAnimController ??Ä? ????Í≥ïÎ±æ???Î∫§Ïî§??éÎíó Êø°Ïíì???∞Î∂Ω?
						}
					}
				}



				// --- Ê¥πÎ™É?ÅÊπ≤?---
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
	std::shared_ptr<CTexture> pTexture = nullptr; // shared_ptrÊø?ËπÇ¬ÄÂØ?
	
	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) // CMaterial ?????Ë´õÍ≥óÎø??Ï¢? Â™õ¬Ä??
		{
			// m_ppMaterials[i]->m_vTextures Â™õ¬Ä shared_ptr Ë∏∞‚â´ÍΩ??®ÌÄ?Â™õ¬Ä??
			for (int j = 0; j < m_ppMaterials[i]->GetTextureCount(); j++) // GetTextureCount ????
			{
				// ??øÎí™Ôß???ÄÏ´???æß??(m_ppstrTextureNames ?????Ï¢?)
				if (!_tcsncmp(m_ppMaterials[i]->m_ppstrTextureNames[j], pstrTextureName, _tcslen(pstrTextureName)))
				{
					// CMaterial??m_vTextures?Î®?Ωå shared_ptr Â™õ¬Ä?Î™ÑÏÇ§Êπ?
					if (j < m_ppMaterials[i]->m_vTextures.size()) {
						return m_ppMaterials[i]->m_vTextures[j]; // shared_ptr ËπÇÎì≠Í∂??èÎø¨ Ë´õÏÑë??(Ôß°Î™Ñ??ÁßªÎåÅ???ÔßùÏï∑?)
					}
				}
			}
		}
	}

	// ?Î®?ñá/?Î∫§Ï†£ ?Î™ÉÎ±∂?Î®?Ωå Ôß°ÏñòÎ¶?(??? ?Î™ÑÌÖß)
	if (m_pSibling) {
		pTexture = m_pSibling->FindReplicatedTexture(pstrTextureName);
		if (pTexture) return pTexture; // Ôß°Ïñ†?ùÔßé?Ë´õÎ∂æÏ§?Ë´õÏÑë??
	}
	if (m_pChild) {
		pTexture = m_pChild->FindReplicatedTexture(pstrTextureName);
		if (pTexture) return pTexture; // Ôß°Ïñ†?ùÔßé?Ë´õÎ∂æÏ§?Ë´õÏÑë??
	}

	return nullptr; // Ôß?Ôß°Ïñ†?ùÔßé?nullptr (??shared_ptr) Ë´õÏÑë??
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
	// ShaderManager Ë´?ResourceManager Â™õ¬Ä?Î™ÑÏÇ§Êπ?
	assert(pGameFramework != nullptr && "GameFramework pointer is needed!");
	ShaderManager* pShaderManager = pGameFramework->GetShaderManager();
	ResourceManager* pResourceManager = pGameFramework->GetResourceManager();
	assert(pShaderManager != nullptr && "ShaderManager is not available!");
	assert(pResourceManager != nullptr && "ResourceManager is not available!");

	char pstrToken[64] = { '\0' };
	int nMaterial = 0;
	UINT nReads = 0;

	m_nMaterials = ReadIntegerFromFile(pInFile);

	wchar_t buffer[128];// Êø°Ïíì???Ë∏∞Íæ™??
	swprintf_s(buffer, L"LoadMaterialsFromFile: Expecting %d materials.\n", m_nMaterials);
	OutputDebugStringW(buffer);

	if (m_nMaterials <= 0) return; // ??Ï≠???ÅÏëùÔß??´ÎÇÖÏ¶?

	if (m_ppMaterials) delete[] m_ppMaterials; // ??Ä? ??àÎñéÔß???ÅÏ†£ (??Î∏??Ë´õ‚ëπ?)
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

			if (!pMaterial) continue; // Material ??πÍΩ¶ ??ΩÎô£ ????ºÏì¨ ?Ï¢èÍ≤ô??∞Ï§à

			// --- ?Í≥óÏî†????ºÏ†ô Êø°ÏíñÏ≠?ËπÇ¬ÄÂØ?---
			UINT nMeshType = GetMeshType();
			std::string shaderName = "Standard"; // Êπ≤Í≥ï??™õ?

			// ÔßéÎ∂ø??????ÜÎøâ ?Í≥ïÏî™ ?Íæ©ÏäÇ???Í≥óÏî†????ÄÏ´?ÂØÉÍ≥ó??
			if (nMeshType & VERTEXT_NORMAL_TANGENT_TEXTURE) { // Êπ≤Í≥ï????øÎí™Ôß??Î™?/?Íæ©Ï†®????Î∏???
				if (nMeshType & VERTEXT_BONE_INDEX_WEIGHT) { // Ëπ?Â™õ¬Ä‰ª•Î¨í????Î∏???
					shaderName = "Skinned";
				}
				else {
					shaderName = "Standard";
				}
			}
			else {
				// ??ª‚Ö® ÔßéÎ∂ø??????ÜÎøâ ????Ôß£ÏÑé??(?? ??±Í∏ΩÔß???àÎíó ÔßéÎ∂ø????
				// ?Íæ©ÏäÇ??éÎñéÔß???Î¶????ª‚Ö® ?Í≥óÏî†????ÄÏ´??Ï¢äÎñ¶ Êø°ÏíñÏ≠??∞Î∂Ω?
			}

			// ShaderManagerÊø°Ïíï????Í≥óÏî†??Â™õ¬Ä?Î™ÑÏÇ§Êπ?
			CShader* pMatShader = pShaderManager->GetShader(shaderName, pd3dCommandList);
			if (pMatShader) {
				pMaterial->SetShader(pMatShader); // CMaterial???Í≥óÏî†????ºÏ†ô
				// GetShader???Î™ÑÌÖß?Î®? ?Íæ™Îπê AddRef ??âÏëùË™ò¬ÄÊø? SetShader?Î®?Ωå AddRef ??????Î¶??Release
				pMatShader->Release();
			}
			else {
				OutputDebugStringA(("Error: Could not get shader '" + shaderName + "' from ShaderManager! Assigning default Standard shader.\n").c_str());
				// ??âÏáÖ Ôß£ÏÑé?? Standard ?Í≥óÏî†?Î∂æÏî™????ºÎñÜ ??ïÎ£Ñ
				pMatShader = pShaderManager->GetShader("Standard", pd3dCommandList);
				if (pMatShader) {
					pMaterial->SetShader(pMatShader);
					pMatShader->Release();
				}
			}
			// --- ?Í≥óÏî†????ºÏ†ô Êø°ÏíñÏ≠???---

			SetMaterial(nMaterial, pMaterial); // ??Ï≠???ºÏ†ô
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
			// LoadTextureFromFile ?Î™ÑÌÖß ???Î™ÉÎú≥??0)?? ????MATERIAL_ALBEDO_MAP) ?Íæ®Îññ
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

	// ??•Îãî ?´ÎÇÖÏ¶????Î∫§Ïî§ (?Î∂æÏæ≠Ê∫êÎÇÜ??
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
		if (nReads != 1 || nStrLength >= sizeof(pstrToken))  // <-- ?†Ïå®Î™åÏòô ?†Ïèô?ôÌò∏ ?†Ïå©Í≥§Ïòô
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
		pstrToken[nStrLength] = '\0';  // ?†Ïèô?ôÂç†?ôÏòô ?¨Âç†?ôÏòô ?†Ïã≠Í≥§Ïòô ?†Ïèô?ôÂç†?ôÏòô

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
     CAnimationController* controllerToUse = m_pSkinnedAnimationController ? m_pSkinnedAnimationController : controller; // ?Î®?ñä????âÏëùÔß??Î®?ñä ?Í≥óÍΩë
     if (m_pMesh && dynamic_cast<CSkinnedMesh*>(m_pMesh)) {
         m_pSharedAnimController = controllerToUse; // ??ΩÍ∂é??ÔßéÎ∂ø?©Ôßé??å‚ë¶?ÉÊø°?ªÏú≠ ????
     }
     if (m_pChild) m_pChild->PropagateAnimController(controllerToUse); // ?Î®?ñá?Î®?æ∂ ?Íæ™ÎôÜ
     if (m_pSibling) m_pSibling->PropagateAnimController(controller);    // ?Î∫§Ï†£???∫¬ÄÔßè‚ë£? ‰ª•¬Ä ÂØ??Íæ™ÎôÜ
 }



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework) // Material ????1Â™?
{
	assert(pGameFramework != nullptr && "GameFramework pointer is needed for CHeightMapTerrain!");
	ResourceManager* pResourceManager = pGameFramework->GetResourceManager();
	ShaderManager* pShaderManager = pGameFramework->GetShaderManager(); // ShaderManager Â™õ¬Ä?Î™ÑÏÇ§Êπ?
	assert(pResourceManager != nullptr && pShaderManager != nullptr);

	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	CHeightMapGridMesh *pMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, nWidth, nLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
	SetMesh(pMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// ??Ï≠???πÍΩ¶
	CMaterial* pTerrainMaterial = new CMaterial(2, pGameFramework);

	// ??øÎí™Ôß?Êø°ÏíïÎ±?
	std::shared_ptr<CTexture> pTerrainBaseTexture = pResourceManager->GetTexture(L"Terrain/DemoTerrain3.dds", pd3dCommandList);
	std::shared_ptr<CTexture> pTerrainDetailTexture = pResourceManager->GetTexture(L"Terrain/TerrainGrass_basecolor.dds", pd3dCommandList);
	
	// ??Ï≠????øÎí™Ôß??Ï¢äÎñ¶ Ë´?SRV ??πÍΩ¶ ?Î∂øÍªå
	if (pTerrainBaseTexture) {
		pTerrainMaterial->AssignTexture(0, pTerrainBaseTexture, pd3dDevice); // 0Ë∏?????
	}
	if (pTerrainDetailTexture) {
		pTerrainMaterial->AssignTexture(1, pTerrainDetailTexture, pd3dDevice); // 1Ë∏?????
	}

	// ?Í≥óÏî†??Â™õ¬Ä?Î™ÑÏÇ§Êπ?Ë´???ºÏ†ô
	CShader* pTerrainShader = pShaderManager->GetShader("Terrain", pd3dCommandList); 
	if (pTerrainShader) {
		pTerrainMaterial->SetShader(pTerrainShader); 
		pTerrainShader->Release(); // GetShaderÊø???? Ôß°Î™Ñ????ÅÏ†£
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


	CMaterial* pMaterial = GetMaterial(0); // Ôßû¬Ä?Î∫? ??Ï≠???éÍµπ Â™õ¬Ä??
	if (m_pMesh && pMaterial && pMaterial->m_pShader)
	{
		// --- ?Í≥πÍπ≠ ??ºÏ†ô ---
		pScene->SetGraphicsState(pd3dCommandList, pMaterial->m_pShader);

		// --- ?®ÎìØ??CBV Ë´õÎ∂ø???---
		// ÁßªÎ?Ï∞??CBV (b1 @ ?Î™ÉÎú≥??0)
		if (pCamera && pCamera->GetCameraConstantBuffer()) {
			pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());
		}

		UpdateTransform(NULL); // Ôßû¬Ä???Î∂æÎ±∂ ??∞Ï†π ??ÖÎú≤??ÑÎìÉ

		// 1. Ôßû¬Ä??Â™õÏïπÍª??Í≥∏Îãî Ë´õÎ∂ø???(b2 @ Param 1 - ?Î∂æÎ±∂ ??∞Ï†πÔß?
		XMFLOAT4X4 gmtxGameObject; // ?Î∂æÎ±∂ ??∞Ï†πÔß??Íæ©ÏäÇ
		XMStoreFloat4x4(&gmtxGameObject, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
		// ?∑‚ë¶?????î™Ë™òÎ™ÖÍΩ?1Ë∏∞ÎçâÎø?16 DWORDS (??∞Ï†π ??Î¶? ??ºÏ†ô
		pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &gmtxGameObject, 0);

		// 2. Ôßû¬Ä????øÎí™Ôß????î†??Ë´õÎ∂ø???(t1, t2 @ Param 2)
		D3D12_GPU_DESCRIPTOR_HANDLE textureTableHandle = pMaterial->GetTextureTableGpuHandle();
		if (textureTableHandle.ptr != 0) {
			// ?∑‚ë¶?????î™Ë™òÎ™ÖÍΩ??Î™ÉÎú≥??2Ë∏∞ÎçâÎø?Ë´õÎ∂ø???
			pd3dCommandList->SetGraphicsRootDescriptorTable(2, textureTableHandle);
		}
		else {
			OutputDebugString(L"Warning: Terrain material has null texture handle for binding.\n");
		}

		// --- Ê¥πÎ™É?ÅÊπ≤?---
		m_pMesh->Render(pd3dCommandList, 0);

	}
	// Ôßû¬Ä?Î∫? ?Î®?ñá/?Î∫§Ï†£ ??ÅÏì¨
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	assert(pGameFramework != nullptr && "GameFramework pointer is needed for CSkyBox!");
	ResourceManager* pResourceManager = pGameFramework->GetResourceManager();
	ShaderManager* pShaderManager = pGameFramework->GetShaderManager(); // ShaderManager Â™õ¬Ä?Î™ÑÏÇ§Êπ?
	assert(pResourceManager != nullptr && pShaderManager != nullptr);

	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// ??Ï≠???πÍΩ¶
	CMaterial* pSkyBoxMaterial = new CMaterial(1, pGameFramework);

	// ??øÎí™Ôß?Êø°ÏíïÎ±?
	std::shared_ptr<CTexture> pSkyBoxTexture = pResourceManager->GetTexture(L"SkyBox/SkyBox_1.dds", pd3dCommandList);
	if (pSkyBoxTexture) {
		pSkyBoxMaterial->AssignTexture(0, pSkyBoxTexture, pd3dDevice); // 0Ë∏??Î™ÉÎú≥??ºÎøâ ?Ï¢äÎñ¶
	}
	else {
		// ??øÎí™Ôß?Êø°ÏíïÎµ???ΩÎô£ Ôß£ÏÑé??
		OutputDebugString(L"Error: Failed to load SkyBox texture using ResourceManager.\n");
	}

	// 5. ?Í≥óÏî†??Â™õ¬Ä?Î™ÑÏÇ§Êπ?Ë´???ºÏ†ô
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

	CMaterial* pMaterial = GetMaterial(0); // ??ºÎ≠Ö??ÄÏª??ªÎíó ??Ï≠???éÍµπ Â™õ¬Ä??
	if (m_pMesh && pMaterial && pMaterial->m_pShader)
	{
		pScene->SetGraphicsState(pd3dCommandList, pMaterial->m_pShader);

		// --- ?®ÎìØ??CBV Ë´õÎ∂ø???---
		// ÁßªÎ?Ï∞??CBV (b1 @ ?Î™ÉÎú≥??0)
		if (pCamera->GetCameraConstantBuffer()) { // pCamera??null ?Íæ®ÎñÇ
			pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());
		}

		// --- ??ºÎ≠Ö??ÄÏª???±—äÎÉº??Ë´õÎ∂ø???---
		XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
		SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);
		UpdateTransform(NULL); // ?Î∂æÎ±∂ ??∞Ï†π ??ÖÎú≤??ÑÎìÉ

		// ??ºÎ≠Ö??ÄÏª????øÎí™Ôß????î†??Ë´õÎ∂ø???(t13 @ Param 1)
		D3D12_GPU_DESCRIPTOR_HANDLE textureTableHandle = pMaterial->GetTextureTableGpuHandle();
		if (textureTableHandle.ptr != 0) {
			// ?∑‚ë¶?????î™Ë™òÎ™ÖÍΩ??Î™ÉÎú≥??1Ë∏∞ÎçâÎø?Ë´õÎ∂ø???
			pd3dCommandList->SetGraphicsRootDescriptorTable(1, textureTableHandle);
		}
		else {
			OutputDebugString(L"Warning: Skybox material has null texture handle for binding.\n");
		}

		// --- Ê¥πÎ™É?ÅÊπ≤?---
		m_pMesh->Render(pd3dCommandList, 0); // ÔßéÎ∂ø?????úëÔß?

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

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
}


// ------------------ ??é–?------------------
void CTreeObject::StartFalling(const XMFLOAT3& hitDirection) {
	if (m_bIsFalling || m_bHasFallen) return; // ??Ä? ?Í≥ïÏú≠Ôßû¬Ä????áÍµÖ???Í≥ïÏú≠Ë≠∞ÎöØ?ùÔßé?‰ª•Î¨ê????ΩÎªæ Ë´õ‚ëπ?

	m_bIsFalling = true;
	m_fFallingTimer = 0.0f;
	m_fCurrentFallAngle = 0.0f;
	m_xmf4x4InitialToParent = m_xmf4x4ToParent; // ?Íæ©Ïò± ?Í≥? ËπÇ¬Ä????∞Ï†π ????

	// ?Í≥ïÏú≠Ôßû¬Ä????ÂØÉÍ≥ó??
	// ??âÎñÜ: hitDirection (???†Ö??ÅÎº±->??é–?Ë∏∞‚â´ÍΩ??Î®?íó ???†Ö??ÅÎº± Look Ë∏∞‚â´ÍΩ? ????èÏ≠Ö???∞Î∫§?ùÊø°???ºÏ†ô
	// ??Î¶??ïÎíó ??•Îãö??çÏæ∂ X???Î®?íó Z??‰ª???éÍµπÊø???ïÎú°??çÏæ∂ ?Î®?íó ?®Ï¢é???Â™õÎ??ùÊø°???ºÏ†ô
	XMFLOAT3 worldUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3FallingAxis = Vector3::CrossProduct(worldUp, hitDirection); // hitDirection????èÏ≠Ö??øÌÄ?Ë´õÎ∂æ?????≤Îªæ????
	if (Vector3::LengthSq(m_xmf3FallingAxis) < 0.001f) { // hitDirection?????Íæ®Ïòí Ë´õ‚ë∫Îº??ÂØÉÏéå??????
		m_xmf3FallingAxis = XMFLOAT3(1.0f, 0.0f, 0.0f); // Êπ≤Í≥ï???∞Î∫§?ùÊø°???ºÏ†ô
	}
	m_xmf3FallingAxis = Vector3::Normalize(m_xmf3FallingAxis);

	// ????ÅÍ∏Ω ?®Îì¶Í∫????Í≥∏Ïî† ?Íæ®Îï≤?Íæ®Ï§â ??ºÏ†ô (?Ï¢èÍπÆ??
	// isRender = false; // ?Íæ©Ï≠Ö?? ???úëÔß???èÎº±????
	// ?Î®?íó ?∞‚ë∏Î£éÔß£???æ™??ÍπäÏÜï ??
}

void CTreeObject::Animate(float fTimeElapsed) {
	// ÔßçÎöØÎπ?CGameObject ??m_pSkinnedAnimationController Â™õ¬Ä ??áÌÄ???Ä? ?????ïÎñéÔß??íÏá±? ?Î™ÑÌÖß
	// if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

	if (m_bIsFalling && !m_bHasFallen) {
		m_fFallingTimer += fTimeElapsed;
		float normalizedTime = std::min(m_fFallingTimer / m_fFallingDuration, 1.0f);

		// ??ìÏªô???Í≥ïÏî™ ???üæ Â™õÍ≥∑Î£?ËπÇÎãøÏª?(Ease-Out ??£ÎÇµ ?ÍπÜÏì£ ‰∫åÏá∞?????Î®?ø∞??ªÏú≠??)
		m_fCurrentFallAngle = m_fTargetFallAngle * normalizedTime; // ?Ï¢èÏÇé ËπÇÎãøÏª?

		// ???üæ ËπÇ¬Ä????πÍΩ¶
		// 1. ??∞ÌÅ∏??∞Ï§à ??ÄÎ£?
		XMMATRIX R = XMMatrixIdentity();
		if (Vector3::LengthSq(m_xmf3RotationPivot) > 0.001f) { // ??∞ÌÅ∏???Î®?†è???Íæ®Îï≤Ôß?
			R = XMMatrixTranslation(-m_xmf3RotationPivot.x, -m_xmf3RotationPivot.y, -m_xmf3RotationPivot.z);
		}
		// 2. ???üæ
		R = XMMatrixMultiply(R, XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3FallingAxis), m_fCurrentFallAngle));
		// 3. ??ºÎñÜ ?Î®?òí ??∞ÌÅ∏ ?Íæ©ÌäÇÊø?
		if (Vector3::LengthSq(m_xmf3RotationPivot) > 0.001f) {
			R = XMMatrixMultiply(R, XMMatrixTranslation(m_xmf3RotationPivot.x, m_xmf3RotationPivot.y, m_xmf3RotationPivot.z));
		}


		// ?•ÎçáÎ¶?ËπÇ¬Ä????∞Ï†π?????üæ ?Í≥∏Ïäú
		XMStoreFloat4x4(&m_xmf4x4ToParent, XMMatrixMultiply(R, XMLoadFloat4x4(&m_xmf4x4InitialToParent)));

		if (normalizedTime >= 1.0f) {
			m_bHasFallen = true;
			m_bIsFalling = false;

			CScene* pScene = m_pGameFramework->GetScene(); // CGameObjectÂ™õ¬Ä m_pGameFramework Ôßé„ÖªÏæ?ëú?Â™õ¬Ä?Î™ÑÎπû ??
			if (pScene) {
				int numBranchesToSpawn = 3 + (rand() % 2); // 3 ?Î®?íó 4Â™?
				for (int i = 0; i < numBranchesToSpawn; ++i) {
					XMFLOAT3 fallenTreePos = GetPosition(); 
					XMFLOAT3 spawnOffsetLocal = XMFLOAT3(
						((float)(rand() % 200) - 100.0f) * 0.1f, // X -10 ~ +10
						(rand() % 10) + 10.0f,                     // Y 10~19
						((float)(rand() % 200) - 100.0f) * 0.1f  // Z -10 ~ +10
					);

					XMFLOAT3 spawnPos = Vector3::Add(fallenTreePos, spawnOffsetLocal);
					if (pScene->m_pTerrain) { // Ôßû¬Ä???Íæ©Îøâ ??ΩÎ£ø??éÎ£ÑÊø??ÎØ™Ïî† ËπÇÎåÅ??
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
			isRender = false; // Ë´õÎ∂æÏ§????™Ôßû?ÂØ???çÍµÖ?? ??±Ï†ô ??ìÏªô ?????™Ôßû??Íæ®Ï§â CBranchObject ?Î®?Ωå Ôß£ÏÑé??
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
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework); // ÔßçÎçâ?Ôß??Î™ÑÏòÑ ?∞Î∂Ω?
	SetChild(pGameObject);

	m_objectType = GameObjectType::Tree;

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
}

CBirchObject::CBirchObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/Tree/FAE_Birch_A_LOD0.bin", "rb");
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework); // ÔßçÎçâ?Ôß??Î™ÑÏòÑ ?∞Î∂Ω?
	SetChild(pGameObject);

	m_objectType = GameObjectType::Tree;

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
}

CWillowObject::CWillowObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/Tree/FAE_Willow_A_LOD0.bin", "rb");
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework); // ÔßçÎçâ?Ôß??Î™ÑÏòÑ ?∞Î∂Ω?
	SetChild(pGameObject);

	m_objectType = GameObjectType::Tree;

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
}

CBranchObject::CBranchObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework, CHeightMapTerrain* pTerrain)
	: CGameObject(1, pGameFramework) { // ??Ï≠?1Â™?Â™õ¬Ä?? ?∫¬ÄÔß???πÍΩ¶???Î™ÑÌÖß
	m_pTerrainRef = pTerrain;

	CLoadedModelInfo* pBranchModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/Branch_A.bin", pGameFramework);
	if (pBranchModel && pBranchModel->m_pModelRootObject) {
		if (pBranchModel->m_pModelRootObject->m_pMesh)
			SetMesh(pBranchModel->m_pModelRootObject->m_pMesh);
		if (pBranchModel->m_pModelRootObject->m_nMaterials > 0 && pBranchModel->m_pModelRootObject->m_ppMaterials[0])
			SetMaterial(0, pBranchModel->m_pModelRootObject->m_ppMaterials[0]); 
		delete pBranchModel;
	}
	else {
		OutputDebugStringA("Error: Failed to load Branch.bin model.\n");
	}
	SetScale(5.0f, 5.0f, 5.0f);
}

void CItemObject::Animate(float fTimeElapsed) {
	if (!isRender) return; // ???úëÔß???àÎ¶∫Ôß???ÖÎú≤??ÑÎìÉ????äÎ∏ø

	if (m_bOnGround) {
		m_fElapsedAfterLanding += fTimeElapsed;
		if (m_fElapsedAfterLanding > m_fLifeTime) {
			isRender = false; // ??±Ï†ô ??ìÏªô ?????™Ôßû?(??Îø????ºÏ†£Êø???ìÍµÖ??éÎíó Êø°ÏíñÏ≠??Íæ©ÏäÇ)
		}
		return;
	}

	// ‰ª•Î¨ê???Í≥∏Ïäú
    XMFLOAT3 gravityForceThisFrame = Vector3::ScalarProduct(m_xmf3Gravity, 10.0f);
    m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, gravityForceThisFrame);

	// ?Íæ©ÌäÇ ??ÖÎú≤??ÑÎìÉ (??ÄÎ£?
	XMFLOAT3 xmf3Shift = Vector3::ScalarProduct(m_xmf3Velocity, 0.5f);
	XMFLOAT3 oldPos = GetPosition();
	XMFLOAT3 newPos = Vector3::Add(oldPos, xmf3Shift);;
	SetPosition(newPos);


	// ??ÉÎÇµ???∞‚ë∏Î£?Ôß£ÎåÑÍ≤?
	if (m_pTerrainRef) {
		XMFLOAT3 currentPos = GetPosition();
		// ??éÏ∂™Â™õ¬ÄÔßû¬Ä Ôßè‚ë§???Ë´õÎ∂æ???∫¬Ä?∫Íæ©??Êπ≤Í≥ó???∞Ï§à Ôßû¬Ä???ÎØ™Ïî†?? ??æß??
		float branchHeightOffset = (m_localOBB.Extents.y > 0) ? m_localOBB.Extents.y : 0.5f; // Ôßè‚ë§??Ë´õÎ∂ø???Ë´õÎ∫§???ÎØ™Ïî†????àÏª≤ ?Î®?íó Êπ≤Í≥ï??™õ?
		float terrainHeight = m_pTerrainRef->GetHeight(currentPos.x, currentPos.z) + branchHeightOffset;

		if (currentPos.y <= terrainHeight) {
			currentPos.y = terrainHeight;
			SetPosition(currentPos); // Ôßû¬Ä???ÎØ™Ïî†??ÔßçÏöé??
			m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f); // ??ÜÎøâ ??∞ÏëùÔß???æÎ£Ñ 0
			m_bOnGround = true;
			m_fElapsedAfterLanding = 0.0f; // ??éÏ±∏ ????Ä????ñÏòâ
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

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
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

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
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

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
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

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
}

CRockDropObject::CRockDropObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework, CHeightMapTerrain* pTerrain)
	: CGameObject(1, pGameFramework) { // ??Ï≠?1Â™?Â™õ¬Ä?? ?∫¬ÄÔß???πÍΩ¶???Î™ÑÌÖß
	m_pTerrainRef = pTerrain;

	CLoadedModelInfo* pBranchModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/RockCluster_B_LOD0.bin", pGameFramework);
	if (pBranchModel && pBranchModel->m_pModelRootObject) {
		if (pBranchModel->m_pModelRootObject->m_pMesh)
			SetMesh(pBranchModel->m_pModelRootObject->m_pMesh);
		if (pBranchModel->m_pModelRootObject->m_nMaterials > 0 && pBranchModel->m_pModelRootObject->m_ppMaterials[0])
			SetMaterial(0, pBranchModel->m_pModelRootObject->m_ppMaterials[0]);
		delete pBranchModel;
	}
	else {
		OutputDebugStringA("Error: Failed to load Branch.bin model.\n");
	}
	SetScale(50.0f, 50.0f, 50.0f);
}

void CRockObject::EraseRock()
{
	CScene* pScene = m_pGameFramework->GetScene(); // CGameObjectÂ™õ¬Ä m_pGameFramework Ôßé„ÖªÏæ?ëú?Â™õ¬Ä?Î™ÑÎπû ??
	if (pScene) {
		int numBranchesToSpawn = 3 + (rand() % 2); // 3 ?Î®?íó 4Â™?
		for (int i = 0; i < numBranchesToSpawn; ++i) {
			XMFLOAT3 fallenTreePos = GetPosition();
			XMFLOAT3 spawnOffsetLocal = XMFLOAT3(
				((float)(rand() % 200) - 100.0f) * 0.1f, // X -10 ~ +10
				(rand() % 10) + 10.0f,                     // Y 10~19
				((float)(rand() % 200) - 100.0f) * 0.1f  // Z -10 ~ +10
			);

			XMFLOAT3 spawnPos = Vector3::Add(fallenTreePos, spawnOffsetLocal);
			if (pScene->m_pTerrain) { // Ôßû¬Ä???Íæ©Îøâ ??ΩÎ£ø??éÎ£ÑÊø??ÎØ™Ïî† ËπÇÎåÅ??
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



// ------------------ ?? ?? ------------------
CBushAObject::CBushAObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, "Model/Vegetation/Bush_A_LOD0.bin", "rb");
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework); // ÔßçÎçâ?Ôß??Î™ÑÏòÑ ?∞Î∂Ω?
	SetChild(pGameObject);

	m_objectType = GameObjectType::Vegetation;

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
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

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
}

CStaticObject::CStaticObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* modelname, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, modelname, "rb");
	::rewind(pInFile);

	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(
		pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
	SetChild(pGameObject);

	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
}

UserObject::UserObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CLoadedModelInfo* pModel, int nAnimationTracks, CGameFramework* pGameFramework) : CGameObject(1, pGameFramework)
{
	CLoadedModelInfo* pUserModel = pModel;
	if (!pUserModel) pUserModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, "Model/SK_Hu_M_FullBody.bin", pGameFramework);
	SetChild(pUserModel->m_pModelRootObject, true);

	AddObject(pd3dDevice, pd3dCommandList, "thumb_02_r", "Model/Sword_01.bin", pGameFramework, XMFLOAT3(0.05, 0.00, -0.05), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	AddObject(pd3dDevice, pd3dCommandList, "Helmet", "Model/Hair_01.bin", pGameFramework, XMFLOAT3(0, 0.1, 0), XMFLOAT3(0,0,0), XMFLOAT3(1, 1, 1));
	//AddWeapon(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Boots_Peasant_Armor", "Model/Boots_Peasant_Armor.bin");
	AddObject(pd3dDevice, pd3dCommandList, "spine_01", "Model/Torso_Peasant_03_Armor.bin", pGameFramework, XMFLOAT3(-0.25, 0.1, 0), XMFLOAT3(90, 0, 90), XMFLOAT3(1, 1, 1));

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
		UpdateTransform(nullptr); // ËπÇ¬Ä????∞Ï†π ÔßùÎê±??Â™õÍπÜ??
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
		UpdateTransform(nullptr); // ËπÇ¬Ä????∞Ï†π ÔßùÎê±??Â™õÍπÜ??
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
	CGameObject* pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework); // ÔßçÎçâ?Ôß??Î™ÑÏòÑ ?∞Î∂Ω?
	SetChild(pGameObject);

	
	if (pInFile) fclose(pInFile); // ???î™ ??™Î¶∞ ?∞Î∂Ω?
}