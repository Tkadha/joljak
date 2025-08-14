#pragma once
#include <Windows.h>
#include <string>
#include <map>

#pragma comment(lib, "winmm.lib")

class SoundManager
{
public:
    static SoundManager& GetInstance();

    void Init(HWND hWnd);
    void Play(const std::wstring& soundPath);
    void HandleMciNotify(WPARAM wParam, LPARAM lParam);

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

private:
    SoundManager();
    ~SoundManager();

    HWND m_hWnd;
    int m_aliasCounter;
    std::map<DWORD, std::wstring> m_deviceMap;
};

