#include "Material.h"
#include "Shader.h"
#include "Scene.h"
#include "GameFramework.h"
#include "ResourceManager.h"

CMaterial::CMaterial(int nTextures) : m_nTextures(nTextures)
{
	m_ppTextures = new CTexture * [m_nTextures];
	m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
	for (int i = 0; i < m_nTextures; i++) m_ppTextures[i] = nullptr;
	for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';
}


CMaterial::CMaterial(int nTextures, CGameFramework* pGameFramework)
	: m_nTextures(nTextures), m_pShader(nullptr), 
	m_d3dCpuSrvStartHandle({ 0 }), m_d3dSrvGpuStartHandle({ 0 }), m_nCbvSrvDescriptorIncrementSize(0)
{
	assert(pGameFramework != nullptr && "GameFramework pointer is needed for CMaterial!");
	// Framework에서 크기 가져오기
	m_nCbvSrvDescriptorIncrementSize = pGameFramework->GetCbvSrvDescriptorSize();



	// 텍스처 이름/포인터 배열 초기화 (기존 코드)
	m_ppTextures = new CTexture * [m_nTextures];
	for (int i = 0; i < m_nTextures; i++) m_ppTextures[i] = NULL;
	m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
	for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';

	// SRV 디스크립터 블록 할당 ---
	// 이 재질이 사용할 텍스처 개수(m_nTextures)만큼 연속된 SRV 슬롯 할당 요청
	if (m_nTextures > 0) {
		// AllocateSrvDescriptors 함수는 시작 핸들(CPU/GPU)을 반환한다고 가정
		if (!pGameFramework->AllocateSrvDescriptors(m_nTextures, m_d3dCpuSrvStartHandle, m_d3dSrvGpuStartHandle))
		{
			OutputDebugString(L"Error: Failed to allocate SRV descriptors for Material!\n");
			// 실패 시 핸들 초기화
			m_d3dCpuSrvStartHandle = { 0 }; 
			m_d3dSrvGpuStartHandle = { 0 };
		}
	}
}

CMaterial::~CMaterial()
{
	if (m_pShader) m_pShader->Release();

	// 텍스처 포인터 배열 해제 (AddRef/Release 관리 가정 시 여기서 Release 호출 필요 없음)
	// ResourceManager가 텍스처 생명주기 관리
	if (m_ppTextures) delete[] m_ppTextures;
	if (m_ppstrTextureNames) delete[] m_ppstrTextureNames;
}

void CMaterial::SetShader(CShader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CMaterial::SetTexture(CTexture* pTexture, UINT nTexture)
{
	if (nTexture < 0 || nTexture >= m_nTextures) return;
	if (m_ppTextures[nTexture]) m_ppTextures[nTexture]->Release();
	m_ppTextures[nTexture] = pTexture;
	if (m_ppTextures[nTexture]) m_ppTextures[nTexture]->AddRef();
}

void CMaterial::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->ReleaseUploadBuffers();
	}
}

void CMaterial::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4AmbientColor, 16);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4AlbedoColor, 20);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4SpecularColor, 24);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4EmissiveColor, 28);

	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nType, 32);

	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->UpdateShaderVariables(pd3dCommandList);
		//if (m_ppTextures[i]) m_ppTextures[i]->UpdateShaderVariable(pd3dCommandList, 0, 0);
	}
}

void CMaterial::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, ResourceManager* pResourceManager)
{
	char pstrTextureName[64] = { '\0' };

	BYTE nStrLength = 64;
	UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrTextureName, sizeof(char), nStrLength, pInFile);
	pstrTextureName[nStrLength] = '\0';

	bool bDuplicated = false;
	if (strcmp(pstrTextureName, "null"))
	{
		SetMaterialType(nType);

		char pstrFilePath[64] = { '\0' };
		strcpy_s(pstrFilePath, 64, "Model/Textures/");

		bDuplicated = (pstrTextureName[0] == '@');
		strcpy_s(pstrFilePath + 15, 64 - 15, (bDuplicated) ? (pstrTextureName + 1) : pstrTextureName);
		strcpy_s(pstrFilePath + 15 + ((bDuplicated) ? (nStrLength - 1) : nStrLength), 64 - 15 - ((bDuplicated) ? (nStrLength - 1) : nStrLength), ".dds");

		size_t nConverted = 0;
		mbstowcs_s(&nConverted, pwstrTextureName, 64, pstrFilePath, _TRUNCATE);

		//#define _WITH_DISPLAY_TEXTURE_NAME

#ifdef _WITH_DISPLAY_TEXTURE_NAME
		static int nTextures = 0, nRepeatedTextures = 0;
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Texture Name: %d %c %s\n"), (pstrTextureName[0] == '@') ? nRepeatedTextures++ : nTextures++, (pstrTextureName[0] == '@') ? '@' : ' ', pwstrTextureName);
		OutputDebugString(pstrDebug);
#endif
		if (!bDuplicated)
		{
			// --- 리소스 매니저 사용 ---
			*ppTexture = pResourceManager->GetTexture(pwstrTextureName, pd3dCommandList);
			if (*ppTexture)
			{
				(*ppTexture)->AddRef(); // 만약 CTexture가 참조 카운팅을 사용한다면
				int nRootParamsInTexture = (*ppTexture)->GetRootParameters(); // CTexture가 자신이 바인딩될 파라미터 수를 안다면
				for (int j = 0; j < nRootParamsInTexture; ++j) {
					(*ppTexture)->SetRootParameterIndex(j, nRootParameter + j); // nRootParameter는 함수의 인자
				}
			}
			else
			{
				// 텍스처 로딩 실패 처리
				OutputDebugStringW((L"Error: Texture load failed via ResourceManager for " + std::wstring(pwstrTextureName) + L"\n").c_str());
			}
			// -----------------------

		}
		else
		{
			if (pParent)
			{
				while (pParent)
				{
					if (!pParent->m_pParent) break;
					pParent = pParent->m_pParent;
				}
				CGameObject* pRootGameObject = pParent;
				*ppTexture = pRootGameObject->FindReplicatedTexture(pwstrTextureName);
				if (*ppTexture) (*ppTexture)->AddRef();
			}
		}
	}
}

void CMaterial::LoadTextureFromFile(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, UINT nTextureIndex, UINT nTextureType, _TCHAR * pwstrTextureName, CGameObject * pParent, ResourceManager * pResourceManager)
{
	if (nTextureIndex >= m_nTextures || !pResourceManager || m_d3dCpuSrvStartHandle.ptr == 0) {
		// 인덱스 오류, 리소스 매니저 없음, 또는 SRV 슬롯 할당 실패 시 처리 안함
		return;
	}

	// 텍스처 이름 저장 (기존 코드)
	lstrcpy(m_ppstrTextureNames[nTextureIndex], pwstrTextureName);

	// ResourceManager를 통해 텍스처 리소스 로드/가져오기
	CTexture* pTexture = pResourceManager->GetTexture(pwstrTextureName, pd3dCommandList);
	m_ppTextures[nTextureIndex] = pTexture; // 포인터 저장 (AddRef 필요시 ResourceManager::GetTexture가 처리 가정)

	if (pTexture && pTexture->GetResource(0)) { // 텍스처 로딩 및 리소스 존재 확인
		// 할당받은 SRV 블록 내에서 이 텍스처의 위치 계산
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_d3dCpuSrvStartHandle);
		cpuHandle.Offset(nTextureIndex, m_nCbvSrvDescriptorIncrementSize);

		// SRV 생성
		ID3D12Resource* pShaderResource = pTexture->GetResource(0); // 텍스처 리소스
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = pTexture->GetShaderResourceViewDesc(0); // SRV 설정
		pd3dDevice->CreateShaderResourceView(pShaderResource, &srvDesc, cpuHandle);
	}

	// 재질 타입 마스크 설정 (예시)
	SetMaterialType(nTextureType); // 함수가 있다고 가정
}