#pragma once
#include <Windows.h>
#include <string>
#include <map>

#pragma comment(lib, "winmm.lib")

class SoundManager
{
private:
    struct SoundInstance {
        std::wstring alias; // 고유 별칭 (예: "snd_0")
        std::wstring path;  // 원본 파일 경로 (예: L"sounds/Stamp land.wav")
    };

public:
    static SoundManager& GetInstance();

    void Init(HWND hWnd);
    void Play(const std::wstring& soundPath);
    void HandleMciNotify(WPARAM wParam, LPARAM lParam);
    bool IsPlaying(const std::wstring& soundPath) const;


    void LoadSound(const std::wstring& soundPath, const std::wstring& key);
    void PlayLoadedSound(const std::wstring& key);

    void UnloadAllSounds();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

private:
    SoundManager();
    ~SoundManager();

    HWND m_hWnd;
    int m_aliasCounter;
    std::map<DWORD, SoundInstance> m_deviceMap;


    struct LoadedSoundInfo {
        std::wstring alias;
        std::wstring path;
    };
    std::map<std::wstring, LoadedSoundInfo> m_preOpenedSounds;
};

