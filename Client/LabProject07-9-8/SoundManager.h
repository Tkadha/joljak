#pragma once

#include <xaudio2.h>
#include <string>
#include <map>
#include <vector>

#pragma comment(lib, "xaudio2.lib")

// WAV 파일의 데이터와 포맷 정보를 담을 구조체
struct SoundData {
    WAVEFORMATEX wfx = { 0 };
    std::vector<BYTE> audioData;
};

// 사운드 재생이 끝났을 때 자동으로 소멸자를 호출하기 위한 콜백 클래스
class VoiceCallback : public IXAudio2VoiceCallback
{
public:
    VoiceCallback() {}
    ~VoiceCallback() {}

    // 재생 버퍼 사용이 끝났을 때 호출됩니다.
    STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext) {
        if (pBufferContext) {
            // 컨텍스트로 전달된 소스 보이스를 파괴하여 리소스를 해제합니다.
            IXAudio2SourceVoice* pSourceVoice = static_cast<IXAudio2SourceVoice*>(pBufferContext);
            pSourceVoice->DestroyVoice();
        }
    }

    // 아래 함수들은 이번 예제에서는 필요 없지만, 인터페이스 규격상 반드시 있어야 합니다.
    STDMETHOD_(void, OnStreamEnd) (THIS) {}
    STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) {}
    STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) {}
    STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext) {}
    STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext) {}
    STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) {}
};


class SoundManager
{
public:
    static SoundManager& GetInstance();

    bool Init();
    void Shutdown();

    bool LoadSound(const std::wstring& soundPath, const std::wstring& key);
    void PlayLoadedSound(const std::wstring& key); // PlayLoadedSound에서 이름 변경

    void UnloadAllSounds();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

private:
    SoundManager();
    ~SoundManager();

    // XAudio2 핵심 인터페이스
    IXAudio2* m_pXAudio2;
    IXAudio2MasteringVoice* m_pMasteringVoice;

    // 로드된 사운드 데이터를 저장하는 맵
    std::map<std::wstring, SoundData> m_sounds;

    // 모든 소스 보이스가 공유할 단일 콜백 인스턴스
    VoiceCallback m_voiceCallback;
};