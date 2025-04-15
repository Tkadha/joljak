#include "ResourceManager.h"
#include "Texture.h"       // 실제 CTexture 헤더 경로로 수정
#include "GameFramework.h" // 실제 CGameFramework 헤더 경로로 수정
#include <iostream>        // 오류 로깅 등

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    // unique_ptr가 자동으로 메모리 해제
}

bool ResourceManager::Initialize(CGameFramework* pFramework)
{
    if (!pFramework) return false;
    m_pFramework = pFramework;
    return true;
}

CTexture* ResourceManager::GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList)
{
    // 1. 캐시 확인
    auto findIter = m_TextureCache.find(filename);
    if (findIter != m_TextureCache.end())
    {
        // 캐시에 존재하면 바로 반환
        return findIter->second.get();
    }

    // 2. 캐시에 없음 -> 새로 로드
    OutputDebugStringW((L"Loading Texture: " + filename + L"\n").c_str()); // 로딩 로그

    // 새 CTexture 객체 생성 (unique_ptr 사용)
    // CTexture 생성자 파라미터는 실제 정의에 맞게 조정 필요
    auto newTexture = std::make_unique<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
    if (!newTexture)
    {
        OutputDebugStringW((L"Error: Failed to create CTexture object for " + filename + L"\n").c_str());
        return nullptr;
    }

    // 3. DDS 파일 로드하여 ID3D12Resource 생성
    // LoadTextureFromDDSFile 함수는 성공 시 true 또는 HRESULT S_OK 반환 가정
    newTexture->LoadTextureFromDDSFile(m_pFramework->GetDevice(), cmdList, filename.c_str(), RESOURCE_TEXTURE2D, 0);
    //if (!newTexture->LoadTextureFromDDSFile(m_pFramework->GetDevice(), cmdList, filename.c_str(), RESOURCE_TEXTURE2D, 0))
    //{
    //    OutputDebugStringW((L"Error: Failed to load DDS file: " + filename + L"\n").c_str());
    //    return nullptr; // 로드 실패 시 nullptr 반환
    //}

    // 4. SRV 생성
    int nTextures = newTexture->GetTextures(); // 보통 DDS는 1개, 텍스처 배열/큐브맵은 여러 개
    if (nTextures > 0)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpuStartHandle = { 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE gpuStartHandle = { 0 };

        // 4a. 프레임워크에서 SRV 디스크립터 슬롯 할당 요청
        if (!m_pFramework->AllocateSrvDescriptors(nTextures, cpuStartHandle, gpuStartHandle))
        {
            OutputDebugStringW((L"Error: Failed to allocate SRV descriptors for " + filename + L"\n").c_str());
            // 리소스는 로드했지만 SRV 생성을 못하는 상황. 로드된 리소스 해제 필요할 수 있음.
            return nullptr; // 실패 처리
        }

        // 4b. 할당된 슬롯에 SRV 생성
        D3D12_CPU_DESCRIPTOR_HANDLE currentCpuHandle = cpuStartHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE currentGpuHandle = gpuStartHandle; // GPU 핸들도 저장하기 위해

        for (int i = 0; i < nTextures; ++i)
        {
            ID3D12Resource* pShaderResource = newTexture->GetResource(i);
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = newTexture->GetShaderResourceViewDesc(i);

            if (pShaderResource)
            {
                m_pFramework->GetDevice()->CreateShaderResourceView(pShaderResource, &srvDesc, currentCpuHandle);
            }
            else
            {
                // 리소스가 없는 경우 처리 (Null SRV 생성 등)
                m_pFramework->GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, currentCpuHandle); // 예: Null SRV
            }


            // CTexture 객체에 해당 SRV의 GPU 핸들 저장
            newTexture->SetGpuDescriptorHandle(i, currentGpuHandle);

            // 다음 슬롯으로 이동
            currentCpuHandle.ptr += m_pFramework->GetCbvSrvDescriptorSize();
            currentGpuHandle.ptr += m_pFramework->GetCbvSrvDescriptorSize();
        }
        // CTexture 객체에 시작 GPU 핸들만 저장하고 싶다면 아래처럼 수정
        // newTexture->SetBaseGpuDescriptorHandle(gpuStartHandle); // 이런 함수가 있다고 가정
    }

    // 5. 로드 및 SRV 생성 완료된 텍스처를 캐시에 추가하고 포인터 반환
    // newTexture의 소유권을 캐시(m_TextureCache)로 이동(move)
    auto [iter, success] = m_TextureCache.emplace(filename, std::move(newTexture));
    if (!success) {
        OutputDebugStringW((L"Error: Failed to insert texture into cache: " + filename + L"\n").c_str());
        return nullptr; // 캐시 삽입 실패? (이론상 발생하기 어려움)
    }

    // 캐시에 저장된 객체의 포인터 반환
    return iter->second.get();
}

void ResourceManager::ReleaseAll()
{
    // (선택적) 멀티스레드 로딩 시 Lock
    // std::lock_guard<std::mutex> lock(m_Mutex);
    m_TextureCache.clear(); // unique_ptr가 자동으로 메모리 해제
}