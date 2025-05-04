#include "Animation.h"
#include "Object.h"

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
	int nSkinnedMesh = 0;
	m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nSkinnedMesh);

	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
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
	m_nAnimationTracks = nAnimationTracks;
	m_pAnimationTracks = new CAnimationTrack[nAnimationTracks];

	m_pAnimationSets = pModel->m_pAnimationSets;
	m_pAnimationSets->AddRef();

	m_pModelRootObject = pModel->m_pModelRootObject;

	m_nSkinnedMeshes = pModel->m_nSkinnedMeshes;
	m_ppSkinnedMeshes = new CSkinnedMesh * [m_nSkinnedMeshes];
	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i] = pModel->m_ppSkinnedMeshes[i];

	m_ppd3dcbSkinningBoneTransforms = new ID3D12Resource * [m_nSkinnedMeshes];
	m_ppcbxmf4x4MappedSkinningBoneTransforms = new XMFLOAT4X4 * [m_nSkinnedMeshes];

	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
		if (m_ppd3dcbSkinningBoneTransforms[i] == nullptr)
		{
			// 실패 로그 (HRESULT 없이)
			OutputDebugStringW(L"[AnimCtrl CBV Check] CreateBufferResource returned NULL for CBV index ");
			OutputDebugStringW(std::to_wstring(i).c_str());
			OutputDebugStringW(L".\n");

			// --- 실패 시 Device Removed Reason 확인 (중요!) ---
			HRESULT removedReason = pd3dDevice->GetDeviceRemovedReason();
			
			// 로그 확인용
			auto FormatHRESULT = [](HRESULT hr) -> std::wstring {
				std::wstringstream ss;
				ss << L"0x" << std::hex << std::uppercase << std::setw(8) << std::setfill(L'0') << hr;
				return ss.str();
				}; 
			std::wstring reasonMsg = L"[AnimCtrl CBV Check]   Device Removed Reason after NULL return: " + FormatHRESULT(removedReason) + L"\n";
			OutputDebugStringW(reasonMsg.c_str());
			// -----------------------------------------

			m_ppcbxmf4x4MappedSkinningBoneTransforms[i] = nullptr; // 맵핑 포인터도 null 처리
			// 여기서 중단점(breakpoint)을 설정하고 디버거로 상태를 확인하는 것이 유용합니다.
			// __debugbreak(); // 필요시 중단점 설정
		}
		else
		{
			// 성공 시 맵핑 시도 (맵핑 실패 로그는 여전히 유용)
			HRESULT mapHr = m_ppd3dcbSkinningBoneTransforms[i]->Map(0, NULL, (void**)&m_ppcbxmf4x4MappedSkinningBoneTransforms[i]);
			if (FAILED(mapHr))
			{
				// 맵핑 실패 로그
				// ...
				m_ppcbxmf4x4MappedSkinningBoneTransforms[i] = nullptr;
			}
		}
	}
}

CAnimationController::~CAnimationController()
{
	if (m_pAnimationTracks) delete[] m_pAnimationTracks;

	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i]->Unmap(0, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Release();
	}
	if (m_ppd3dcbSkinningBoneTransforms) delete[] m_ppd3dcbSkinningBoneTransforms;
	if (m_ppcbxmf4x4MappedSkinningBoneTransforms) delete[] m_ppcbxmf4x4MappedSkinningBoneTransforms;

	if (m_pAnimationSets) m_pAnimationSets->Release();

	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
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
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppSkinnedMeshes[i]->m_pd3dcbSkinningBoneTransforms = m_ppd3dcbSkinningBoneTransforms[i];
		m_ppSkinnedMeshes[i]->m_pcbxmf4x4MappedSkinningBoneTransforms = m_ppcbxmf4x4MappedSkinningBoneTransforms[i];
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
	m_fTime += fTimeElapsed;
	if (m_pAnimationTracks)
	{
		for (int j = 0; j < m_pAnimationSets->m_nBoneFrames; j++) 
			m_pAnimationSets->m_ppBoneFrameCaches[j]->m_xmf4x4ToParent = Matrix4x4::Zero();

		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable)
			{
				CAnimationSet* pAnimationSet = m_pAnimationSets->m_pAnimationSets[m_pAnimationTracks[k].m_nAnimationSet];
				float fPosition = m_pAnimationTracks[k].UpdatePosition(m_pAnimationTracks[k].m_fPosition, fTimeElapsed, pAnimationSet->m_fLength);
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
		UpdateBoneTransformCBVContents();

		OnRootMotion(pRootGameObject);
		OnAnimationIK(pRootGameObject);
	}
}


void CAnimationController::UpdateBoneTransformCBVContents()
{
	// 필요한 멤버 변수 유효성 검사
	if (!m_pAnimationSets || !m_pAnimationSets->m_ppBoneFrameCaches || !m_ppSkinnedMeshes || !m_ppcbxmf4x4MappedSkinningBoneTransforms)
		return;

	// 이 컨트롤러가 관리하는 각 스키닝 메쉬에 대해 반복 (CBV 버퍼 업데이트)
	for (int i = 0; i < m_nSkinnedMeshes; ++i)
	{
		CSkinnedMesh* pSkinnedMesh = m_ppSkinnedMeshes[i];
		XMFLOAT4X4* pMappedBuffer = m_ppcbxmf4x4MappedSkinningBoneTransforms[i]; // 해당 메쉬용 CBV 버퍼

		if (!pSkinnedMesh || !pMappedBuffer || !pSkinnedMesh->m_ppSkinningBoneFrameCaches) continue;

		// 이 메쉬에 영향을 주는 각 뼈에 대해 반복
		for (int j = 0; j < pSkinnedMesh->m_nSkinningBones; ++j)
		{
			// 메쉬의 뼈 캐시에서 해당 뼈의 GameObject 노드 찾기
			CGameObject* pBoneNode = pSkinnedMesh->m_ppSkinningBoneFrameCaches[j];
			if (pBoneNode)
			{
				// 이 뼈의 최종 월드 변환 행렬 가져오기
				XMFLOAT4X4 worldMatrix = pBoneNode->m_xmf4x4World;
				XMFLOAT4X4 transposedWorld;

				// --- 월드 행렬만 전치해서 복사 ---
				XMStoreFloat4x4(&transposedWorld, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));

				// 계산된 행렬을 CBV의 해당 본 인덱스 위치에 복사
				if (j < SKINNED_ANIMATION_BONES) { // SKINNED_ANIMATION_BONES 정의 필요
					pMappedBuffer[j] = transposedWorld;
				}
			}
			else
			{
				// 뼈 노드 못찾음 오류 처리
				if (j < SKINNED_ANIMATION_BONES) {
					pMappedBuffer[j] = Matrix4x4::Identity(); // 예: Identity 행렬 넣기
				}
			}
		}
		m_ppd3dcbSkinningBoneTransforms[i]->Unmap(0, nullptr);
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