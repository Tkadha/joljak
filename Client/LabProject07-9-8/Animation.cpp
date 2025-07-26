#include "Animation.h"
#include "Object.h"
#include "GameFramework.h"

// 로그 확인용(나중에 제거)
#include <sstream>     // std::wstringstream 사용
#include <iomanip>     // std::hex, std::setw, std::setfill 사용
#include <windows.h>   // OutputDebugStringW 사용

CLoadedModelInfo::~CLoadedModelInfo()
{
	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CLoadedModelInfo::PrepareSkinning()
{
	//int nSkinnedMesh = 0;
	//m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	//m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nSkinnedMesh);

	for (int i = 0; i < m_nSkinnedMeshes; i++) 
		m_ppSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
}

void CLoadedModelInfo::FindAndCacheSkinnedMeshes()
{
	if (m_nSkinnedMeshes == 0 || !m_pModelRootObject)
	{
		if (m_ppSkinnedMeshes) { // 이미 할당되었다면 이전 것 해제
			delete[] m_ppSkinnedMeshes;
			m_ppSkinnedMeshes = nullptr;
		}
		return;
	}

	// m_ppSkinnedMeshes가 이미 할당되었는지 확인
	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;

	m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	for (int i = 0; i < m_nSkinnedMeshes; ++i) m_ppSkinnedMeshes[i] = nullptr; // 초기화

	int nFoundSkinnedMeshes = 0; // 실제로 찾은 스킨드 메쉬 개수
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nFoundSkinnedMeshes);

	if (nFoundSkinnedMeshes != m_nSkinnedMeshes)
	{
		// 예상했던 개수와 실제로 찾은 개수가 다르면 로그를 남기거나 처리할 수 있습니다.
		// 예를 들어 m_nSkinnedMeshes = nFoundSkinnedMeshes; 로 업데이트 할 수도 있습니다.
		// 여기서는 일단 m_nSkinnedMeshes가 정확한 용량이라고 가정합니다.
		// 만약 nFoundSkinnedMeshes가 더 적다면, m_ppSkinnedMeshes의 뒷부분은 nullptr로 남아있을 것입니다.
		// 또는, 실제 찾은 개수만큼만 유효하다고 간주하고 m_nSkinnedMeshes 값을 업데이트합니다.
		// 안전하게는 m_nSkinnedMeshes = nFoundSkinnedMeshes; 로 설정하는 것이 좋습니다.
		m_nSkinnedMeshes = nFoundSkinnedMeshes;
	}
}


CAnimationSet::CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrames, int nAnimatedBones, char* pstrName)
{
	m_fLength = fLength;
	m_nFramesPerSecond = nFramesPerSecond;
	m_nKeyFrames = nKeyFrames;

	strcpy_s(m_pstrAnimationSetName, 64, pstrName);

#ifdef _WITH_ANIMATION_SRT
	m_nKeyFrameTranslations = nKeyFrames;
	m_pfKeyFrameTranslationTimes = new float[m_nKeyFrameTranslations];
	m_ppxmf3KeyFrameTranslations = new XMFLOAT3 * [m_nKeyFrameTranslations];
	for (int i = 0; i < m_nKeyFrameTranslations; i++) m_ppxmf3KeyFrameTranslations[i] = new XMFLOAT4X4[nAnimatedBones];

	m_nKeyFrameScales = nKeyFrames;
	m_pfKeyFrameScaleTimes = new float[m_nKeyFrameScales];
	m_ppxmf3KeyFrameScales = new XMFLOAT3 * [m_nKeyFrameScales];
	for (int i = 0; i < m_nKeyFrameScales; i++) m_ppxmf3KeyFrameScales[i] = new XMFLOAT4X4[nAnimatedBones];

	m_nKeyFrameRotations = nKeyFrames;
	m_pfKeyFrameRotationTimes = new float[m_nKeyFrameRotations];
	m_ppxmf4KeyFrameRotations = new XMFLOAT3 * [m_nKeyFrameRotations];
	for (int i = 0; i < m_nKeyFrameRotations; i++) m_ppxmf4KeyFrameRotations[i] = new XMFLOAT4X4[nAnimatedBones];
#else
	m_pfKeyFrameTimes = new float[nKeyFrames];
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4 * [nKeyFrames];
	for (int i = 0; i < nKeyFrames; i++) m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nAnimatedBones];
#endif
}

CAnimationSet::~CAnimationSet()
{
#ifdef _WITH_ANIMATION_SRT
	if (m_pfKeyFrameTranslationTimes) delete[] m_pfKeyFrameTranslationTimes;
	for (int j = 0; j < m_nKeyFrameTranslations; j++) if (m_ppxmf3KeyFrameTranslations[j]) delete[] m_ppxmf3KeyFrameTranslations[j];
	if (m_ppxmf3KeyFrameTranslations) delete[] m_ppxmf3KeyFrameTranslations;

	if (m_pfKeyFrameScaleTimes) delete[] m_pfKeyFrameScaleTimes;
	for (int j = 0; j < m_nKeyFrameScales; j++) if (m_ppxmf3KeyFrameScales[j]) delete[] m_ppxmf3KeyFrameScales[j];
	if (m_ppxmf3KeyFrameScales) delete[] m_ppxmf3KeyFrameScales;

	if (m_pfKeyFrameRotationTimes) delete[] m_pfKeyFrameRotationTimes;
	for (int j = 0; j < m_nKeyFrameRotations; j++) if (m_ppxmf4KeyFrameRotations[j]) delete[] m_ppxmf4KeyFrameRotations[j];
	if (m_ppxmf4KeyFrameRotations) delete[] m_ppxmf4KeyFrameRotations;
#else
	if (m_pfKeyFrameTimes) delete[] m_pfKeyFrameTimes;
	for (int j = 0; j < m_nKeyFrames; j++) if (m_ppxmf4x4KeyFrameTransforms[j]) delete[] m_ppxmf4x4KeyFrameTransforms[j];
	if (m_ppxmf4x4KeyFrameTransforms) delete[] m_ppxmf4x4KeyFrameTransforms;
#endif
}

XMFLOAT4X4 CAnimationSet::GetSRT(int nBone, float fPosition)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();
#ifdef _WITH_ANIMATION_SRT
	XMVECTOR S, R, T;
	for (int i = 0; i < (m_nKeyFrameTranslations - 1); i++)
	{
		if ((m_pfKeyFrameTranslationTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameTranslationTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameTranslationTimes[i]) / (m_pfKeyFrameTranslationTimes[i + 1] - m_pfKeyFrameTranslationTimes[i]);
			T = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameTranslations[i + 1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameScales - 1); i++)
	{
		if ((m_pfKeyFrameScaleTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameScaleTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameScaleTimes[i]) / (m_pfKeyFrameScaleTimes[i + 1] - m_pfKeyFrameScaleTimes[i]);
			S = XMVectorLerp(XMLoadFloat3(&m_ppxmf3KeyFrameScales[i][nBone]), XMLoadFloat3(&m_ppxmf3KeyFrameScales[i + 1][nBone]), t);
			break;
		}
	}
	for (UINT i = 0; i < (m_nKeyFrameRotations - 1); i++)
	{
		if ((m_pfKeyFrameRotationTimes[i] <= fPosition) && (fPosition <= m_pfKeyFrameRotationTimes[i + 1]))
		{
			float t = (m_fPosition - m_pfKeyFrameRotationTimes[i]) / (m_pfKeyFrameRotationTimes[i + 1] - m_pfKeyFrameRotationTimes[i]);
			R = XMQuaternionSlerp(XMQuaternionConjugate(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i][nBone])), XMQuaternionConjugate(XMLoadFloat4(&m_ppxmf4KeyFrameRotations[i + 1][nBone])), t);
			break;
		}
	}

	XMStoreFloat4x4(&xmf4x4Transform, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
#else   
	for (int i = 0; i < (m_nKeyFrames - 1); i++)
	{
		if ((m_pfKeyFrameTimes[i] <= fPosition) && (fPosition < m_pfKeyFrameTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameTimes[i]) / (m_pfKeyFrameTimes[i + 1] - m_pfKeyFrameTimes[i]);
			xmf4x4Transform = Matrix4x4::Interpolate(m_ppxmf4x4KeyFrameTransforms[i][nBone], m_ppxmf4x4KeyFrameTransforms[i + 1][nBone], t);
			break;
		}
	}
	if (fPosition >= m_pfKeyFrameTimes[m_nKeyFrames - 1]) xmf4x4Transform = m_ppxmf4x4KeyFrameTransforms[m_nKeyFrames - 1][nBone];

#endif
	return(xmf4x4Transform);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationSets::CAnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	m_pAnimationSets = new CAnimationSet * [nAnimationSets];
}

CAnimationSets::~CAnimationSets()
{
	for (int i = 0; i < m_nAnimationSets; i++) if (m_pAnimationSets[i]) delete m_pAnimationSets[i];
	if (m_pAnimationSets) delete[] m_pAnimationSets;

	if (m_ppBoneFrameCaches) delete[] m_ppBoneFrameCaches;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationTrack::~CAnimationTrack()
{
	if (m_pCallbackKeys) delete[] m_pCallbackKeys;
	if (m_pAnimationCallbackHandler) delete m_pAnimationCallbackHandler;
}

void CAnimationTrack::SetCallbackKeys(int nCallbackKeys)
{
	m_nCallbackKeys = nCallbackKeys;
	m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationTrack::SetCallbackKey(int nKeyIndex, float fKeyTime, void* pData)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void CAnimationTrack::SetAnimationCallbackHandler(CAnimationCallbackHandler* pCallbackHandler)
{
	m_pAnimationCallbackHandler = pCallbackHandler;
}

void CAnimationTrack::HandleCallback()
{
	if (m_pAnimationCallbackHandler)
	{
		for (int i = 0; i < m_nCallbackKeys; i++)
		{
			if (::IsEqual(m_pCallbackKeys[i].m_fTime, m_fPosition, ANIMATION_CALLBACK_EPSILON))
			{
				if (m_pCallbackKeys[i].m_pCallbackData) m_pAnimationCallbackHandler->HandleCallback(m_pCallbackKeys[i].m_pCallbackData, m_fPosition);
				break;
			}
		}
	}
}

float CAnimationTrack::UpdatePosition(float fTrackPosition, float fElapsedTime, float fAnimationLength)
{
	float fTrackElapsedTime = fElapsedTime * m_fSpeed;
	switch (m_nType)
	{
	case ANIMATION_TYPE_LOOP:
	{
		if (m_fPosition < 0.0f) m_fPosition = 0.0f;
		else
		{
			m_fPosition = fTrackPosition + fTrackElapsedTime;
			if (m_fPosition > fAnimationLength)
			{
				m_fPosition = -ANIMATION_CALLBACK_EPSILON;
				return(fAnimationLength);
			}
		}
		//			m_fPosition = fmod(fTrackPosition, m_pfKeyFrameTimes[m_nKeyFrames-1]); // m_fPosition = fTrackPosition - int(fTrackPosition / m_pfKeyFrameTimes[m_nKeyFrames-1]) * m_pfKeyFrameTimes[m_nKeyFrames-1];
		//			m_fPosition = fmod(fTrackPosition, m_fLength); //if (m_fPosition < 0) m_fPosition += m_fLength;
		//			m_fPosition = fTrackPosition - int(fTrackPosition / m_fLength) * m_fLength;
		break;
	}
	case ANIMATION_TYPE_ONCE:
		m_fPosition = fTrackPosition + fTrackElapsedTime;
		if (m_fPosition > fAnimationLength) m_fPosition = fAnimationLength;
		break;
	case ANIMATION_TYPE_PINGPONG:
		break;
	}

	return(m_fPosition);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationController::CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel)
{
	m_pGameFramework = pModel->m_pModelRootObject->m_pGameFramework; // GameFramework 포인터 저장
	m_nAnimationTracks = nAnimationTracks; //
	m_pAnimationTracks = new CAnimationTrack[nAnimationTracks]; //

	m_pAnimationSets = pModel->m_pAnimationSets; //
	m_pAnimationSets->AddRef(); //

	m_pModelRootObject = pModel->m_pModelRootObject; //

	// 기존 m_nSkinnedMeshes와 m_ppSkinnedMeshes 대신 m_vSkinnedMeshes 사용
	// 초기 모델의 스킨드 메쉬들을 동적 배열에 추가
	if (pModel->m_nSkinnedMeshes > 0 && pModel->m_ppSkinnedMeshes) //
	{
		for (int i = 0; i < pModel->m_nSkinnedMeshes; ++i) //
		{
			AddSkinnedMesh(pModel->m_ppSkinnedMeshes[i], pd3dDevice, pd3dCommandList); //
		}
	}
}

void CAnimationController::AddSkinnedMesh(CSkinnedMesh* pSkinnedMesh, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (!pSkinnedMesh) return;

	for (CSkinnedMesh* existingMesh : m_vSkinnedMeshes) {
		if (existingMesh == pSkinnedMesh) {
			OutputDebugStringW(L"Warning: Attempted to add an already existing SkinnedMesh to CAnimationController.\n");
			return;
		}
	}

	m_vSkinnedMeshes.push_back(pSkinnedMesh);

	// 해당 SkinnedMesh를 위한 CBV 생성
	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255);
	ID3D12Resource* pNewCB = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
	XMFLOAT4X4* pMappedCB = nullptr;

	if (pNewCB)
	{
		pNewCB->Map(0, NULL, (void**)&pMappedCB);
		m_vpd3dcbSkinningBoneTransforms.push_back(pNewCB);
		m_vppcbxmf4x4MappedSkinningBoneTransforms.push_back(pMappedCB);

		pSkinnedMesh->m_pd3dcbSkinningBoneTransforms = pNewCB;
		pSkinnedMesh->m_pcbxmf4x4MappedSkinningBoneTransforms = pMappedCB;
	}
	else
	{
		OutputDebugStringW(L"ERROR: Failed to create CBV for new SkinnedMesh in CAnimationController.\n");
		m_vpd3dcbSkinningBoneTransforms.push_back(nullptr);
		m_vppcbxmf4x4MappedSkinningBoneTransforms.push_back(nullptr);
	}
}

void CAnimationController::RemoveSkinnedMesh(CSkinnedMesh* pSkinnedMesh)
{
	if (!pSkinnedMesh) return;

	for (size_t i = 0; i < m_vSkinnedMeshes.size(); ++i)
	{
		if (m_vSkinnedMeshes[i] == pSkinnedMesh)
		{
			if (m_vpd3dcbSkinningBoneTransforms[i])
			{
				m_vpd3dcbSkinningBoneTransforms[i]->Unmap(0, NULL);
				m_vpd3dcbSkinningBoneTransforms[i]->Release();
			}
			m_vSkinnedMeshes.erase(m_vSkinnedMeshes.begin() + i);
			m_vpd3dcbSkinningBoneTransforms.erase(m_vpd3dcbSkinningBoneTransforms.begin() + i);
			m_vppcbxmf4x4MappedSkinningBoneTransforms.erase(m_vppcbxmf4x4MappedSkinningBoneTransforms.begin() + i);

			pSkinnedMesh->m_pd3dcbSkinningBoneTransforms = nullptr;
			pSkinnedMesh->m_pcbxmf4x4MappedSkinningBoneTransforms = nullptr;
			return;
		}
	}
}



CAnimationController::~CAnimationController()
{
	if (m_pAnimationTracks) delete[] m_pAnimationTracks;

	for (size_t i = 0; i < m_vpd3dcbSkinningBoneTransforms.size(); ++i)
	{
		if (m_vpd3dcbSkinningBoneTransforms[i])
		{
			m_vpd3dcbSkinningBoneTransforms[i]->Unmap(0, NULL);
			m_vpd3dcbSkinningBoneTransforms[i]->Release();
		}
	}
	m_vpd3dcbSkinningBoneTransforms.clear();
	m_vppcbxmf4x4MappedSkinningBoneTransforms.clear();
	m_vSkinnedMeshes.clear();

	if (m_pAnimationSets) m_pAnimationSets->Release();
}

void CAnimationController::SetCallbackKeys(int nAnimationTrack, int nCallbackKeys)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetCallbackKeys(nCallbackKeys);
}

void CAnimationController::SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fKeyTime, void* pData)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetCallbackKey(nKeyIndex, fKeyTime, pData);
}

void CAnimationController::SetAnimationCallbackHandler(int nAnimationTrack, CAnimationCallbackHandler* pCallbackHandler)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetAnimationCallbackHandler(pCallbackHandler);
}

void CAnimationController::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].m_nAnimationSet = nAnimationSet;
}

void CAnimationController::SetTrackEnable(int nAnimationTrack, bool bEnable)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetEnable(bEnable);
}

void CAnimationController::SetTrackPosition(int nAnimationTrack, float fPosition)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetPosition(fPosition);
}

void CAnimationController::SetTrackSpeed(int nAnimationTrack, float fSpeed)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
}

void CAnimationController::SetTrackWeight(int nAnimationTrack, float fWeight)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetWeight(fWeight);
}

void CAnimationController::SetTrackType(int nAnimationTrack, int type)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetType(type);
}

void CAnimationController::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{	
	for (CSkinnedMesh* pSkinnedMesh : m_vSkinnedMeshes)
	{
		if (pSkinnedMesh) // 유효성 검사
		{
			pSkinnedMesh->UpdateShaderVariables(pd3dCommandList);
		}
	}
}
/*
void CAnimationController::AdvanceTime(float fTimeElapsed, CGameObject *pRootGameObject)
{
	m_fTime += fTimeElapsed;
	if (m_pAnimationTracks)
	{
//		for (int k = 0; k < m_nAnimationTracks; k++) m_pAnimationTracks[k].m_fPosition += (fTimeElapsed * m_pAnimationTracks[k].m_fSpeed);
		for (int k = 0; k < m_nAnimationTracks; k++) m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet]->UpdatePosition(fTimeElapsed * m_pAnimationTracks[k].m_fSpeed);

		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
		{
			XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Zero();
			for (int k = 0; k < m_nAnimationTracks; k++)
			{
				if (m_pAnimationTracks[k].m_bEnable)
				{
					CAnimationSet *pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet];
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j);
					xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, m_pAnimationTracks[k].m_fWeight));
				}
			}
			m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
		}

		pRootGameObject->UpdateTransform(NULL);

		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable) m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet]->HandleCallback();
		}
	}
}
//*/
//*
void CAnimationController::AdvanceTime(float fTimeElapsed, CGameObject* pRootGameObject)
{
	float scaledTimeElapsed = fTimeElapsed * m_fAnimationSpeed;
	if (m_pAnimationTracks)
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++) 
			m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = Matrix4x4::Zero();

		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable)
			{
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet];
				float fPosition = m_pAnimationTracks[k].UpdatePosition(m_pAnimationTracks[k].m_fPosition, scaledTimeElapsed, pAnimationSet->m_fLength);
				for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++)
				{
					//XMFLOAT4X4 xmf4x4Transform = m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent;
					XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Zero();
					XMFLOAT4X4 xmf4x4TrackTransform = pAnimationSet->GetSRT(j, fPosition);
					xmf4x4Transform = Matrix4x4::Add(xmf4x4Transform, Matrix4x4::Scale(xmf4x4TrackTransform, m_pAnimationTracks[k].m_fWeight));
					m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = xmf4x4Transform;
				}
				m_pAnimationTracks[k].HandleCallback();
			}
		}

		pRootGameObject->UpdateTransform(NULL);

		// CBV 내용 업데이트
		//UpdateBoneTransformCBVContents();

		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}


void CAnimationController::UpdateBoneTransformCBVContents()
{
	// 필요한 멤버 변수 유효성 검사
	if (!m_pAnimationSets) return;

	// 이 컨트롤러가 관리하는 각 스키닝 메쉬에 대해 CBV 버퍼 업데이트
	for (size_t i = 0; i < m_vSkinnedMeshes.size(); ++i)
	{
		CSkinnedMesh* pSkinnedMesh = m_vSkinnedMeshes[i];
		XMFLOAT4X4* pMappedBuffer = m_vppcbxmf4x4MappedSkinningBoneTransforms[i]; // 해당 메쉬용 CBV 버퍼

		if (!pSkinnedMesh || !pMappedBuffer || !pSkinnedMesh->m_ppSkinningBoneFrameCaches) continue;

		// 이 메쉬에 영향을 주는 각 뼈에 대해 반복
		for (int j = 0; j < pSkinnedMesh->m_nSkinningBones; ++j)
		{
			CGameObject* pBoneNode = pSkinnedMesh->m_ppSkinningBoneFrameCaches[j];
			if (pBoneNode)
			{
				XMFLOAT4X4 worldMatrix = pBoneNode->m_xmf4x4World;
				XMFLOAT4X4 transposedWorld;
				XMStoreFloat4x4(&transposedWorld, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));

				if (j < SKINNED_ANIMATION_BONES) {
					pMappedBuffer[j] = transposedWorld;
				}
			}
			else
			{
				if (j < SKINNED_ANIMATION_BONES) {
					pMappedBuffer[j] = Matrix4x4::Identity();
				}
			}
		}
	}
}

// 예: void CAnimationController::UpdateBoneLocalTransformCBV()
void CAnimationController::UpdateBoneLocalTransformCBV()
{
	// 필요한 멤버 변수 유효성 검사
	if (!m_pAnimationSets || !m_ppSkinnedMeshes || !m_ppcbxmf4x4MappedSkinningBoneTransforms)
		return;

	// 이 컨트롤러가 관리하는 각 스키닝 메쉬에 대해 반복 (CBV 버퍼 업데이트)
	for (int i = 0; i < m_nSkinnedMeshes; ++i)
	{
		CSkinnedMesh* pSkinnedMesh = m_ppSkinnedMeshes[i];
		XMFLOAT4X4* pMappedBuffer = m_ppcbxmf4x4MappedSkinningBoneTransforms[i]; // 해당 메쉬용 CBV 버퍼

		// 메쉬 및 해당 뼈 캐시 유효성 검사
		if (!pSkinnedMesh || !pMappedBuffer || !pSkinnedMesh->m_ppSkinningBoneFrameCaches) {
			continue;
		}

		// 이 메쉬에 영향을 주는 각 뼈에 대해 반복
		for (int j = 0; j < pSkinnedMesh->m_nSkinningBones; ++j)
		{
			// 메쉬의 뼈 캐시에서 해당 뼈의 GameObject 노드 찾기
			CGameObject* pBoneNode = pSkinnedMesh->m_ppSkinningBoneFrameCaches[j];
			if (pBoneNode)
			{
				// --- 이 뼈의 애니메이션 적용된 '로컬' 변환 행렬 가져오기 ---
				XMFLOAT4X4 localMatrix = pBoneNode->m_xmf4x4ToParent; // 월드 행렬 대신 로컬 행렬 사용!
				XMFLOAT4X4 transposedLocal;

				// --- 로컬 행렬 전치해서 복사 ---
				XMStoreFloat4x4(&transposedLocal, XMMatrixTranspose(XMLoadFloat4x4(&localMatrix)));

				// 계산된 행렬을 CBV의 해당 본 인덱스 위치에 복사
				if (j < SKINNED_ANIMATION_BONES) { // SKINNED_ANIMATION_BONES 정의 필요
					pMappedBuffer[j] = transposedLocal; // 전치된 '로컬' 행렬 복사
				}
			}
			else
			{
				// 뼈 노드 못찾음 오류 처리
				if (j < SKINNED_ANIMATION_BONES) {
					pMappedBuffer[j] = Matrix4x4::Identity();
				}
				// 로그 추가 권장
			}
		}
	}
}



CAnimationController::CAnimationController(const CAnimationController& other, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pGameFramework = other.m_pGameFramework; 

	m_nAnimationTracks = other.m_nAnimationTracks; 
	m_pAnimationTracks = new CAnimationTrack[m_nAnimationTracks]; 
	for (int i = 0; i < m_nAnimationTracks; ++i) { 
		m_pAnimationTracks[i] = other.m_pAnimationTracks[i]; 
	}

	m_pAnimationSets = other.m_pAnimationSets; 
	if (m_pAnimationSets) m_pAnimationSets->AddRef(); 

	m_pModelRootObject = other.m_pModelRootObject; 

	m_vSkinnedMeshes.reserve(other.m_vSkinnedMeshes.size()); 
	m_vpd3dcbSkinningBoneTransforms.reserve(other.m_vSkinnedMeshes.size()); 
	m_vppcbxmf4x4MappedSkinningBoneTransforms.reserve(other.m_vSkinnedMeshes.size()); 

	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); 
	for (size_t i = 0; i < other.m_vSkinnedMeshes.size(); ++i) 
	{
		ID3D12Resource* pNewCB = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL); 
		XMFLOAT4X4* pMappedCB = nullptr; 
		if (pNewCB) { 
			HRESULT mapHr = pNewCB->Map(0, NULL, (void**)&pMappedCB); 
			if (FAILED(mapHr)) { 
				OutputDebugStringW(L"[AnimCtrl Clone CBV Check] Map failed in Clone constructor.\n"); 
				pMappedCB = nullptr; 
			}
		}
		else { 
			OutputDebugStringW(L"[AnimCtrl Clone CBV Check] CreateBufferResource returned NULL in Clone constructor.\n"); 
		}
		m_vpd3dcbSkinningBoneTransforms.push_back(pNewCB); 
		m_vppcbxmf4x4MappedSkinningBoneTransforms.push_back(pMappedCB); 
		m_vSkinnedMeshes.push_back(nullptr); 
	}
}


CAnimationController* CAnimationController::Clone(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	return new CAnimationController(*this, pd3dDevice, pd3dCommandList);
}