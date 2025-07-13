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
	// Framework에서 크기 가져오기
	m_nCbvSrvDescriptorIncrementSize = pGameFramework->GetCbvSrvDescriptorSize();


	// SRV 디스크립터 블록 할당 ---
	// 이 재질이 사용할 텍스처 개수(m_nTextures)만큼 연속된 SRV 슬롯 할당 요청
    if (m_nTextures > 0) {
        m_vTextures.resize(m_nTextures); // shared_ptr 벡터 리사이즈
        m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
        for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';
       
        // SRV 블록 할당
        bool bAllocated = pGameFramework->AllocateSrvDescriptors(m_nTextures, m_d3dCpuSrvStartHandle, m_d3dSrvGpuStartHandle);

        if (!bAllocated)
        {
            m_d3dCpuSrvStartHandle = { 0 };
            m_d3dSrvGpuStartHandle = { 0 };
        }
    }
    else {
        m_ppstrTextureNames = nullptr; // 텍스처 없으면 이름 배열도 null
    }
}

// CMaterial 소멸자 수정
CMaterial::~CMaterial() {
    //if (m_pShader) m_pShader->Release();
    if (m_ppstrTextureNames) delete[] m_ppstrTextureNames;
    // SRV 핸들 해제는 불필요 (힙 자체 관리)
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
    // 1. 유효성 검사
    if (nTextureIndex >= m_nTextures || !pInFile || !pResourceManager || m_d3dCpuSrvStartHandle.ptr == 0) {
        OutputDebugStringW(L"!!!!!!!! ERROR: Invalid parameters or unallocated SRV slot in LoadTextureFromFile. !!!!!!!!\n");
        // 파일 포인터를 읽기 위치라도 맞춰주어야 할 수 있으므로 주의 (아래 fread 부분 때문에)
        // 임시로 길이만 읽고 넘어가거나, 파일 형식을 정확히 알아야 함
        BYTE nStrLength = 0;
        UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile); // 길이 읽기
        if (nStrLength > 0) {
            char dummy[256];
            if (nStrLength >= 256) nStrLength = 255; // 버퍼 오버플로우 방지
            nReads = (UINT)::fread(dummy, sizeof(char), nStrLength, pInFile); // 이름 데이터 버리기
        }
        return;
    }

    // 2. 파일에서 텍스처 이름(char*) 읽기
    char pstrTextureName[64] = { '\0' };
    BYTE nStrLength = 0; // 길이를 먼저 읽음
    UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
    if (nReads != 1 || nStrLength >= 64) { // 길이 읽기 실패 또는 버퍼 초과 시
        OutputDebugStringW(L"!!!!!!!! ERROR: Failed to read texture name length or length too long. !!!!!!!!\n");
        // 오류 처리: 파일 포인터 이동 등 필요할 수 있음
        if (nStrLength > 0 && nStrLength < 255) { // 예상치 못한 길이라도 일단 읽고 넘어가기 시도
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
    pstrTextureName[nStrLength] = '\0'; // Null 종단 처리

    OutputDebugStringA("CMaterial::LoadTextureFromFile - Read FileName (char*): ");
    OutputDebugStringA(pstrTextureName);
    OutputDebugStringA("\n");


    // 3. "null" 텍스처 처리
    if (!strcmp(pstrTextureName, "null"))
    {
        m_vTextures[nTextureIndex].reset(); // 해당 슬롯 shared_ptr 비우기
        if (m_ppstrTextureNames) m_ppstrTextureNames[nTextureIndex][0] = L'\0';
        return;
    }

    // 4. 파일 경로 조합 및 Wide Char 변환
    char pstrFilePath[128] = { '\0' }; // 경로 포함 위해 버퍼 늘림
    strcpy_s(pstrFilePath, 128, "Model/Textures/"); // 기본 경로

    bool bDuplicated = (pstrTextureName[0] == '@'); // 중복 텍스처 여부 확인
    // 경로 + 파일 이름 조합
    strcat_s(pstrFilePath, 128, (bDuplicated) ? (pstrTextureName + 1) : pstrTextureName);
    // 확장자(.dds) 추가
    strcat_s(pstrFilePath, 128, ".dds");

    // Wide Char 변환
    wchar_t pwstrTextureName[128] = { L'\0' }; // Wide Char 버퍼
    size_t nConverted = 0;
    mbstowcs_s(&nConverted, pwstrTextureName, 128, pstrFilePath, _TRUNCATE);

    if (nConverted <= 0) {
        OutputDebugStringW(L"!!!!!!!! ERROR: Failed to convert texture path to wide char. !!!!!!!!\n");
        return;
    }

    // 5. 변환된 Wide Char 파일 이름 저장
    if (m_ppstrTextureNames) lstrcpy(m_ppstrTextureNames[nTextureIndex], pwstrTextureName);
    OutputDebugStringW(L"CMaterial::LoadTextureFromFile - Loading Texture: ");
    OutputDebugStringW(pwstrTextureName);
    OutputDebugStringW(L"\n");

    // 6. 재질 타입 마스크 설정
    SetMaterialType(nTextureType);

    // 7. 텍스처 로드/가져오기 및 SRV 생성
    std::shared_ptr<CTexture> pTexture = nullptr;
    if (!bDuplicated) // 중복 텍스처가 아니면 ResourceManager 사용
    {
        pTexture = pResourceManager->GetTexture(pwstrTextureName, pd3dCommandList);
        if (pTexture) {
            OutputDebugStringW(L"    Loaded via ResourceManager.\n");
        }
        else {
            OutputDebugStringW(L"    !!!!!!!! FAILED to load via ResourceManager !!!!!!!!\n");
        }
    }
    else // 중복 텍스처 처리
    {
        if (pParent)
        {
            // 최상위 루트 객체 찾기
            CGameObject* pRootGameObject = pParent;
            while (pRootGameObject->m_pParent) pRootGameObject = pRootGameObject->m_pParent;
            // 루트에서 텍스처 찾기
            pTexture = pRootGameObject->FindReplicatedTexture(pwstrTextureName); // shared_ptr 반환 받음
            if (pTexture) {
                OutputDebugStringW(L"    Found replicated texture (manual AddRef).\n");
                }
            else {
                OutputDebugStringW(L"    !!!!!!!! FAILED to find replicated texture !!!!!!!!\n");
            }
        }
    }

    // 8. 텍스처 포인터 저장
    if (nTextureIndex < m_vTextures.size()) {
        m_vTextures[nTextureIndex] = pTexture;
    }
    // --- 9. SRV 생성 ---
    if (pTexture && pTexture->GetResource(0)) { // CTexture 객체 및 내부 리소스 유효성 검사
        // SRV 생성 위치 계산
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_d3dCpuSrvStartHandle);
        cpuHandle.Offset(nTextureIndex, m_nCbvSrvDescriptorIncrementSize);

        ID3D12Resource* pShaderResource = pTexture->GetResource(0);
        // CTexture로부터 올바른 SRV 설정(Dimension 포함) 가져오기
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = pTexture->GetShaderResourceViewDesc(0); // 인덱스 0 사용 (큐브맵 등은 내부 리소스가 하나일 수 있음)

        // SRV 생성
        if (pShaderResource) {
            pd3dDevice->CreateShaderResourceView(pShaderResource, &srvDesc, cpuHandle);
            OutputDebugStringW(L"    SRV Created in CMaterial.\n");
        }

    }
    else {
        // 텍스처 로드 실패 또는 리소스 없음
        OutputDebugStringW(L"    Warning: Texture resource is null. Cannot create SRV in CMaterial.\n");
    }
}

bool CMaterial::AssignTexture(UINT nTextureIndex, std::shared_ptr<CTexture> pTexture, ID3D12Device* pd3dDevice)
{
    // 인덱스 및 핸들 유효성 검사
    if (nTextureIndex >= (UINT)m_nTextures || m_d3dCpuSrvStartHandle.ptr == 0 || !pd3dDevice) {
        OutputDebugStringW(L"!!!!!!!! ERROR: Invalid index or unallocated SRV slot in AssignTexture. !!!!!!!!\n");
        return false;
    }

    // shared_ptr 저장 (벡터 사용)
    m_vTextures[nTextureIndex] = pTexture;

    // SRV 생성   
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
            return false; // 리소스 없으면 실패 간주
        }
    }
    else {
        OutputDebugStringW(L"    Warning: Texture pointer is null. Cannot create SRV in AssignTexture.\n");
         return false; // 텍스처 없으면 실패 간주
    }
}