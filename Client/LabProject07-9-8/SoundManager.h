#pragma once
#include <Windows.h>
#include <string>
#include <map>

#pragma comment(lib, "winmm.lib")

class SoundManager
{
private:
    struct SoundInstance {
        std::wstring alias; // ���� ��Ī (��: "snd_0")
        std::wstring path;  // ���� ���� ��� (��: L"sounds/Stamp land.wav")
    };

public:
    static SoundManager& GetInstance();

    void Init(HWND hWnd);
    void Play(const std::wstring& soundPath);
    void HandleMciNotify(WPARAM wParam, LPARAM lParam);
    bool IsPlaying(const std::wstring& soundPath) const;

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

private:
    SoundManager();
    ~SoundManager();

    HWND m_hWnd;
    int m_aliasCounter;
    std::map<DWORD, SoundInstance> m_deviceMap;
};

