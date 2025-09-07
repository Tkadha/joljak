#include "SoundManager.h"
#include <Windows.h> // CreateFileW, OutputDebugStringW 등 사용

// --- WAV 파일 파싱을 위한 헬퍼 함수 ---
HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
    HRESULT hr = S_OK;
    if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while (hr == S_OK)
    {
        DWORD dwRead;
        if (ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL) == 0)
            hr = HRESULT_FROM_WIN32(GetLastError());

        if (ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0)
            hr = HRESULT_FROM_WIN32(GetLastError());

        switch (dwChunkType)
        {
        case 'FFIR': // "RIFF"
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if (ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL) == 0)
                hr = HRESULT_FROM_WIN32(GetLastError());
            break;

        default:
            if (SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
                return HRESULT_FROM_WIN32(GetLastError());
        }

        dwOffset += sizeof(DWORD) * 2;

        if (dwChunkType == fourcc)
        {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;

        if (bytesRead >= dwRIFFDataSize) return S_FALSE;
    }
    return S_OK;
}

HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
    HRESULT hr = S_OK;
    if (SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        return HRESULT_FROM_WIN32(GetLastError());
    DWORD dwRead;
    if (ReadFile(hFile, buffer, buffersize, &dwRead, NULL) == 0)
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}
// --- 헬퍼 함수 끝 ---


SoundManager& SoundManager::GetInstance()
{
    static SoundManager instance;
    return instance;
}

SoundManager::SoundManager() : m_pXAudio2(nullptr), m_pMasteringVoice(nullptr)
{
}

SoundManager::~SoundManager()
{
    Shutdown();
}

bool SoundManager::Init()
{
    // COM 초기화
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        OutputDebugStringW(L"[SoundManager] Failed to initialize COM\n");
        return false;
    }

    // XAudio2 엔진 생성
    if (FAILED(hr = XAudio2Create(&m_pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    {
        OutputDebugStringW(L"[SoundManager] Failed to create XAudio2 engine\n");
        CoUninitialize();
        return false;
    }

    // 마스터링 보이스 생성 (스피커로 나가는 최종 출력)
    if (FAILED(hr = m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice)))
    {
        OutputDebugStringW(L"[SoundManager] Failed to create mastering voice\n");
        m_pXAudio2->Release();
        CoUninitialize();
        return false;
    }

    return true;
}

void SoundManager::Shutdown()
{
    UnloadAllSounds();

    if (m_pMasteringVoice) {
        m_pMasteringVoice->DestroyVoice();
        m_pMasteringVoice = nullptr;
    }
    if (m_pXAudio2) {
        m_pXAudio2->Release();
        m_pXAudio2 = nullptr;
    }
    CoUninitialize();
}

bool SoundManager::LoadSound(const std::wstring& soundPath, const std::wstring& key)
{
    if (m_sounds.count(key)) return true; // 이미 로드됨

    // WAV 파일 열기
    HANDLE hFile = CreateFileW(soundPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hFile) {
        OutputDebugStringW((L"[SoundManager] Failed to open sound file: " + soundPath + L"\n").c_str());
        return false;
    }
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
        CloseHandle(hFile);
        return false;
    }

    SoundData newSoundData;
    DWORD dwChunkSize;
    DWORD dwChunkPosition;

    // "RIFF" 청크 찾기
    FindChunk(hFile, 'FFIR', dwChunkSize, dwChunkPosition);
    // ... (에러 처리 생략)

    // "fmt " 청크(포맷 정보) 찾고 읽기
    FindChunk(hFile, ' tmf', dwChunkSize, dwChunkPosition);
    ReadChunkData(hFile, &newSoundData.wfx, dwChunkSize, dwChunkPosition);

    // "data" 청크(실제 오디오 데이터) 찾고 읽기
    FindChunk(hFile, 'atad', dwChunkSize, dwChunkPosition);
    newSoundData.audioData.resize(dwChunkSize);
    ReadChunkData(hFile, newSoundData.audioData.data(), dwChunkSize, dwChunkPosition);

    CloseHandle(hFile);

    m_sounds[key] = std::move(newSoundData);
    OutputDebugStringW((L"[SoundManager] Loaded sound: " + soundPath + L"\n").c_str());
    return true;
}

void SoundManager::PlayLoadedSound(const std::wstring& key)
{
    auto it = m_sounds.find(key);
    if (it == m_sounds.end())
    {
        OutputDebugStringW((L"[SoundManager] Tried to play unloaded sound: " + key + L"\n").c_str());
        return;
    }

    const SoundData& soundData = it->second;

    IXAudio2SourceVoice* pSourceVoice;
    HRESULT hr;

    // 소스 보이스(재생 채널) 생성. 재생이 끝나면 콜백이 자동으로 파괴해줍니다.
    if (FAILED(hr = m_pXAudio2->CreateSourceVoice(&pSourceVoice, &(soundData.wfx), 0, XAUDIO2_DEFAULT_FREQ_RATIO, &m_voiceCallback)))
    {
        OutputDebugStringW(L"[SoundManager] Error creating source voice\n");
        return;
    }

    XAUDIO2_BUFFER buffer = { 0 };
    buffer.AudioBytes = static_cast<UINT32>(soundData.audioData.size());
    buffer.pAudioData = soundData.audioData.data();
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    // pContext에 자기 자신을 넘겨서, 콜백 함수에서 이 포인터를 받아 DestroyVoice()를 호출하게 합니다.
    buffer.pContext = pSourceVoice;

    if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
    {
        OutputDebugStringW(L"[SoundManager] Error submitting source buffer\n");
        pSourceVoice->DestroyVoice(); // 실패 시 직접 파괴
        return;
    }

    if (FAILED(hr = pSourceVoice->Start(0)))
    {
        OutputDebugStringW(L"[SoundManager] Error starting voice\n");
        pSourceVoice->DestroyVoice(); // 실패 시 직접 파괴
    }
}

void SoundManager::UnloadAllSounds()
{
    m_sounds.clear();
}