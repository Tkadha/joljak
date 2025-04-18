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
	// Framework���� ũ�� ��������
	m_nCbvSrvDescriptorIncrementSize = pGameFramework->GetCbvSrvDescriptorSize();



	// �ؽ�ó �̸�/������ �迭 �ʱ�ȭ (���� �ڵ�)
	m_ppTextures = new CTexture * [m_nTextures];
	for (int i = 0; i < m_nTextures; i++) m_ppTextures[i] = NULL;
	m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
	for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';

	// SRV ��ũ���� ��� �Ҵ� ---
	// �� ������ ����� �ؽ�ó ����(m_nTextures)��ŭ ���ӵ� SRV ���� �Ҵ� ��û
	if (m_nTextures > 0) {
		// AllocateSrvDescriptors �Լ��� ���� �ڵ�(CPU/GPU)�� ��ȯ�Ѵٰ� ����
		if (!pGameFramework->AllocateSrvDescriptors(m_nTextures, m_d3dCpuSrvStartHandle, m_d3dSrvGpuStartHandle))
		{
			OutputDebugString(L"Error: Failed to allocate SRV descriptors for Material!\n");
			// ���� �� �ڵ� �ʱ�ȭ
			m_d3dCpuSrvStartHandle = { 0 }; 
			m_d3dSrvGpuStartHandle = { 0 };
		}
	}
}

CMaterial::~CMaterial()
{
	if (m_pShader) m_pShader->Release();

	// �ؽ�ó ������ �迭 ���� (AddRef/Release ���� ���� �� ���⼭ Release ȣ�� �ʿ� ����)
	// ResourceManager�� �ؽ�ó �����ֱ� ����
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
			// --- ���ҽ� �Ŵ��� ��� ---
			*ppTexture = pResourceManager->GetTexture(pwstrTextureName, pd3dCommandList);
			if (*ppTexture)
			{
				(*ppTexture)->AddRef(); // ���� CTexture�� ���� ī������ ����Ѵٸ�
				int nRootParamsInTexture = (*ppTexture)->GetRootParameters(); // CTexture�� �ڽ��� ���ε��� �Ķ���� ���� �ȴٸ�
				for (int j = 0; j < nRootParamsInTexture; ++j) {
					(*ppTexture)->SetRootParameterIndex(j, nRootParameter + j); // nRootParameter�� �Լ��� ����
				}
			}
			else
			{
				// �ؽ�ó �ε� ���� ó��
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
		// �ε��� ����, ���ҽ� �Ŵ��� ����, �Ǵ� SRV ���� �Ҵ� ���� �� ó�� ����
		return;
	}

	// �ؽ�ó �̸� ���� (���� �ڵ�)
	lstrcpy(m_ppstrTextureNames[nTextureIndex], pwstrTextureName);

	// ResourceManager�� ���� �ؽ�ó ���ҽ� �ε�/��������
	CTexture* pTexture = pResourceManager->GetTexture(pwstrTextureName, pd3dCommandList);
	m_ppTextures[nTextureIndex] = pTexture; // ������ ���� (AddRef �ʿ�� ResourceManager::GetTexture�� ó�� ����)

	if (pTexture && pTexture->GetResource(0)) { // �ؽ�ó �ε� �� ���ҽ� ���� Ȯ��
		// �Ҵ���� SRV ��� ������ �� �ؽ�ó�� ��ġ ���
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_d3dCpuSrvStartHandle);
		cpuHandle.Offset(nTextureIndex, m_nCbvSrvDescriptorIncrementSize);

		// SRV ����
		ID3D12Resource* pShaderResource = pTexture->GetResource(0); // �ؽ�ó ���ҽ�
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = pTexture->GetShaderResourceViewDesc(0); // SRV ����
		pd3dDevice->CreateShaderResourceView(pShaderResource, &srvDesc, cpuHandle);
	}

	// ���� Ÿ�� ����ũ ���� (����)
	SetMaterialType(nTextureType); // �Լ��� �ִٰ� ����
}