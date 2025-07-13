#include "Material.h"
#include "Shader.h"
#include "Scene.h"
#include "GameFramework.h"
#include "ResourceManager.h"

CMaterial::CMaterial(int nTextures): m_nTextures(nTextures),
m_d3dCpuSrvStartHandle({ 0 }), m_d3dSrvGpuStartHandle({ 0 }), m_nCbvSrvDescriptorIncrementSize(0)
{

}

CMaterial::CMaterial(int nTextures, CGameFramework* pGameFramework)
	: m_nTextures(nTextures), m_pGameFramework(pGameFramework),
    m_d3dCpuSrvStartHandle({ 0 }), m_d3dSrvGpuStartHandle({ 0 }), m_nCbvSrvDescriptorIncrementSize(0)
{
	assert(pGameFramework != nullptr && "GameFramework pointer is needed for CMaterial!");
	// Framework���� ũ�� ��������
	m_nCbvSrvDescriptorIncrementSize = pGameFramework->GetCbvSrvDescriptorSize();


	// SRV ��ũ���� ��� �Ҵ� ---
	// �� ������ ����� �ؽ�ó ����(m_nTextures)��ŭ ���ӵ� SRV ���� �Ҵ� ��û
    if (m_nTextures > 0) {
        m_vTextures.resize(m_nTextures); // shared_ptr ���� ��������
        m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
        for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';
       
        // SRV ��� �Ҵ�
        bool bAllocated = pGameFramework->AllocateSrvDescriptors(m_nTextures, m_d3dCpuSrvStartHandle, m_d3dSrvGpuStartHandle);

        if (!bAllocated)
        {
            m_d3dCpuSrvStartHandle = { 0 };
            m_d3dSrvGpuStartHandle = { 0 };
        }
    }
    else {
        m_ppstrTextureNames = nullptr; // �ؽ�ó ������ �̸� �迭�� null
    }
}

// CMaterial �Ҹ��� ����
CMaterial::~CMaterial() {
    //if (m_pShader) m_pShader->Release();
    if (m_ppstrTextureNames) delete[] m_ppstrTextureNames;
    // SRV �ڵ� ������ ���ʿ� (�� ��ü ����)
}

//void CMaterial::SetShader(CShader* pShader)
//{
//	//if (m_pShader) m_pShader->Release();
//	//m_pShader = pShader;
//	//if (m_pShader) m_pShader->AddRef();
//}

CTexture* CMaterial::GetTexture(int index) const {
    return (index >= 0 && index < m_vTextures.size()) ? m_vTextures[index].get() : nullptr;
}

void CMaterial::ReleaseUploadBuffers() {
    for (const auto& pTexture : m_vTextures) {
        if (pTexture) {
            pTexture->ReleaseUploadBuffers();
        }
    }
}

void CMaterial::LoadTextureFromFile(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    UINT nTextureIndex,
    UINT nTextureType,
    CGameObject* pParent,
    FILE* pInFile,
    ResourceManager* pResourceManager)
{
    // 1. ��ȿ�� �˻�
    if (nTextureIndex >= m_nTextures || !pInFile || !pResourceManager || m_d3dCpuSrvStartHandle.ptr == 0) {
        OutputDebugStringW(L"!!!!!!!! ERROR: Invalid parameters or unallocated SRV slot in LoadTextureFromFile. !!!!!!!!\n");
        // ���� �����͸� �б� ��ġ�� �����־�� �� �� �����Ƿ� ���� (�Ʒ� fread �κ� ������)
        // �ӽ÷� ���̸� �а� �Ѿ�ų�, ���� ������ ��Ȯ�� �˾ƾ� ��
        BYTE nStrLength = 0;
        UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile); // ���� �б�
        if (nStrLength > 0) {
            char dummy[256];
            if (nStrLength >= 256) nStrLength = 255; // ���� �����÷ο� ����
            nReads = (UINT)::fread(dummy, sizeof(char), nStrLength, pInFile); // �̸� ������ ������
        }
        return;
    }

    // 2. ���Ͽ��� �ؽ�ó �̸�(char*) �б�
    char pstrTextureName[64] = { '\0' };
    BYTE nStrLength = 0; // ���̸� ���� ����
    UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
    if (nReads != 1 || nStrLength >= 64) { // ���� �б� ���� �Ǵ� ���� �ʰ� ��
        OutputDebugStringW(L"!!!!!!!! ERROR: Failed to read texture name length or length too long. !!!!!!!!\n");
        // ���� ó��: ���� ������ �̵� �� �ʿ��� �� ����
        if (nStrLength > 0 && nStrLength < 255) { // ����ġ ���� ���̶� �ϴ� �а� �Ѿ�� �õ�
            char dummy[256];
            nReads = (UINT)::fread(dummy, sizeof(char), nStrLength, pInFile);
        }
        return;
    }
    nReads = (UINT)::fread(pstrTextureName, sizeof(char), nStrLength, pInFile);
    if (nReads != nStrLength) {
        OutputDebugStringW(L"!!!!!!!! ERROR: Failed to read texture name string. !!!!!!!!\n");
        return;
    }
    pstrTextureName[nStrLength] = '\0'; // Null ���� ó��

    OutputDebugStringA("CMaterial::LoadTextureFromFile - Read FileName (char*): ");
    OutputDebugStringA(pstrTextureName);
    OutputDebugStringA("\n");


    // 3. "null" �ؽ�ó ó��
    if (!strcmp(pstrTextureName, "null"))
    {
        m_vTextures[nTextureIndex].reset(); // �ش� ���� shared_ptr ����
        if (m_ppstrTextureNames) m_ppstrTextureNames[nTextureIndex][0] = L'\0';
        return;
    }

    // 4. ���� ��� ���� �� Wide Char ��ȯ
    char pstrFilePath[128] = { '\0' }; // ��� ���� ���� ���� �ø�
    strcpy_s(pstrFilePath, 128, "Model/Textures/"); // �⺻ ���

    bool bDuplicated = (pstrTextureName[0] == '@'); // �ߺ� �ؽ�ó ���� Ȯ��
    // ��� + ���� �̸� ����
    strcat_s(pstrFilePath, 128, (bDuplicated) ? (pstrTextureName + 1) : pstrTextureName);
    // Ȯ����(.dds) �߰�
    strcat_s(pstrFilePath, 128, ".dds");

    // Wide Char ��ȯ
    wchar_t pwstrTextureName[128] = { L'\0' }; // Wide Char ����
    size_t nConverted = 0;
    mbstowcs_s(&nConverted, pwstrTextureName, 128, pstrFilePath, _TRUNCATE);

    if (nConverted <= 0) {
        OutputDebugStringW(L"!!!!!!!! ERROR: Failed to convert texture path to wide char. !!!!!!!!\n");
        return;
    }

    // 5. ��ȯ�� Wide Char ���� �̸� ����
    if (m_ppstrTextureNames) lstrcpy(m_ppstrTextureNames[nTextureIndex], pwstrTextureName);
    OutputDebugStringW(L"CMaterial::LoadTextureFromFile - Loading Texture: ");
    OutputDebugStringW(pwstrTextureName);
    OutputDebugStringW(L"\n");

    // 6. ���� Ÿ�� ����ũ ����
    SetMaterialType(nTextureType);

    // 7. �ؽ�ó �ε�/�������� �� SRV ����
    std::shared_ptr<CTexture> pTexture = nullptr;
    if (!bDuplicated) // �ߺ� �ؽ�ó�� �ƴϸ� ResourceManager ���
    {
        pTexture = pResourceManager->GetTexture(pwstrTextureName, pd3dCommandList);
        if (pTexture) {
            OutputDebugStringW(L"    Loaded via ResourceManager.\n");
        }
        else {
            OutputDebugStringW(L"    !!!!!!!! FAILED to load via ResourceManager !!!!!!!!\n");
        }
    }
    else // �ߺ� �ؽ�ó ó��
    {
        if (pParent)
        {
            // �ֻ��� ��Ʈ ��ü ã��
            CGameObject* pRootGameObject = pParent;
            while (pRootGameObject->m_pParent) pRootGameObject = pRootGameObject->m_pParent;
            // ��Ʈ���� �ؽ�ó ã��
            pTexture = pRootGameObject->FindReplicatedTexture(pwstrTextureName); // shared_ptr ��ȯ ����
            if (pTexture) {
                OutputDebugStringW(L"    Found replicated texture (manual AddRef).\n");
                }
            else {
                OutputDebugStringW(L"    !!!!!!!! FAILED to find replicated texture !!!!!!!!\n");
            }
        }
    }

    // 8. �ؽ�ó ������ ����
    if (nTextureIndex < m_vTextures.size()) {
        m_vTextures[nTextureIndex] = pTexture;
    }
    // --- 9. SRV ���� ---
    if (pTexture && pTexture->GetResource(0)) { // CTexture ��ü �� ���� ���ҽ� ��ȿ�� �˻�
        // SRV ���� ��ġ ���
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_d3dCpuSrvStartHandle);
        cpuHandle.Offset(nTextureIndex, m_nCbvSrvDescriptorIncrementSize);

        ID3D12Resource* pShaderResource = pTexture->GetResource(0);
        // CTexture�κ��� �ùٸ� SRV ����(Dimension ����) ��������
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = pTexture->GetShaderResourceViewDesc(0); // �ε��� 0 ��� (ť��� ���� ���� ���ҽ��� �ϳ��� �� ����)

        // SRV ����
        if (pShaderResource) {
            pd3dDevice->CreateShaderResourceView(pShaderResource, &srvDesc, cpuHandle);
            OutputDebugStringW(L"    SRV Created in CMaterial.\n");
        }

    }
    else {
        // �ؽ�ó �ε� ���� �Ǵ� ���ҽ� ����
        OutputDebugStringW(L"    Warning: Texture resource is null. Cannot create SRV in CMaterial.\n");
    }
}

bool CMaterial::AssignTexture(UINT nTextureIndex, std::shared_ptr<CTexture> pTexture, ID3D12Device* pd3dDevice)
{
    // �ε��� �� �ڵ� ��ȿ�� �˻�
    if (nTextureIndex >= (UINT)m_nTextures || m_d3dCpuSrvStartHandle.ptr == 0 || !pd3dDevice) {
        OutputDebugStringW(L"!!!!!!!! ERROR: Invalid index or unallocated SRV slot in AssignTexture. !!!!!!!!\n");
        return false;
    }

    // shared_ptr ���� (���� ���)
    m_vTextures[nTextureIndex] = pTexture;

    // SRV ����   
    if (pTexture && pTexture->GetResource(0)) {
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_d3dCpuSrvStartHandle);
        cpuHandle.Offset(nTextureIndex, m_nCbvSrvDescriptorIncrementSize);

        ID3D12Resource* pShaderResource = pTexture->GetResource(0);
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = pTexture->GetShaderResourceViewDesc(0);

        if (pShaderResource) {
            pd3dDevice->CreateShaderResourceView(pShaderResource, &srvDesc, cpuHandle);
            OutputDebugStringW(L"    SRV Created in CMaterial::AssignTexture.\n");
            return true;
        }
        else {
            OutputDebugStringW(L"    Warning: Texture resource pointer is null before SRV creation in AssignTexture.\n");
            return false; // ���ҽ� ������ ���� ����
        }
    }
    else {
        OutputDebugStringW(L"    Warning: Texture pointer is null. Cannot create SRV in AssignTexture.\n");
         return false; // �ؽ�ó ������ ���� ����
    }
}