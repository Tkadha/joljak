#pragma once

#include <xaudio2.h>
#include <string>
#include <map>
#include <vector>

#pragma comment(lib, "xaudio2.lib")

// WAV ������ �����Ϳ� ���� ������ ���� ����ü
struct SoundData {
    WAVEFORMATEX wfx = { 0 };
    std::vector<BYTE> audioData;
};

// ���� ����� ������ �� �ڵ����� �Ҹ��ڸ� ȣ���ϱ� ���� �ݹ� Ŭ����
class VoiceCallback : public IXAudio2VoiceCallback
{
public:
    VoiceCallback() {}
    ~VoiceCallback() {}

    // ��� ���� ����� ������ �� ȣ��˴ϴ�.
    STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext) {
        if (pBufferContext) {
            // ���ؽ�Ʈ�� ���޵� �ҽ� ���̽��� �ı��Ͽ� ���ҽ��� �����մϴ�.
            IXAudio2SourceVoice* pSourceVoice = static_cast<IXAudio2SourceVoice*>(pBufferContext);
            pSourceVoice->DestroyVoice();
        }
    }

    // �Ʒ� �Լ����� �̹� ���������� �ʿ� ������, �������̽� �԰ݻ� �ݵ�� �־�� �մϴ�.
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
    void PlayLoadedSound(const std::wstring& key); // PlayLoadedSound���� �̸� ����

    void UnloadAllSounds();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

private:
    SoundManager();
    ~SoundManager();

    // XAudio2 �ٽ� �������̽�
    IXAudio2* m_pXAudio2;
    IXAudio2MasteringVoice* m_pMasteringVoice;

    // �ε�� ���� �����͸� �����ϴ� ��
    std::map<std::wstring, SoundData> m_sounds;

    // ��� �ҽ� ���̽��� ������ ���� �ݹ� �ν��Ͻ�
    VoiceCallback m_voiceCallback;
};