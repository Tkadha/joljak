#include "SoundManager.h"
#include <mmsystem.h>

// GetInstance() 메서드 구현
SoundManager& SoundManager::GetInstance()
{
    // C++11 이상에서 이 방식은 스레드에 안전합니다.
    static SoundManager instance; // 이 인스턴스는 프로그램 전체에서 단 하나입니다.
    return instance;
}

// 생성자 구현
SoundManager::SoundManager()
    : m_hWnd(NULL), m_aliasCounter(0)
{
}

SoundManager::~SoundManager()
{
    // 프로그램 종료 시 혹시나 남아있는 사운드가 있다면 모두 닫습니다.
    for (auto const& [deviceID, instance] : m_deviceMap)
    {
        std::wstring command = L"close " + instance.alias;
        mciSendString(command.c_str(), NULL, 0, NULL);
    }
    m_deviceMap.clear();
}

// 초기화 함수 구현
void SoundManager::Init(HWND hWnd)
{
    m_hWnd = hWnd;
}

void SoundManager::Play(const std::wstring& soundPath)
{
    if (soundPath == L"Sound/Golem/Stamp land.wav" && IsPlaying(L"Sound/Golem/Stamp land.wav"))
    {
        return;
    }

    // 1. 고유한 별칭 생성 (예: "snd_0", "snd_1", ...)
    std::wstring alias = L"snd_" + std::to_wstring(m_aliasCounter++);

    // 2. "open" 명령으로 사운드 파일 열기
    std::wstring command = L"open \"" + soundPath + L"\" type waveaudio alias " + alias;
    if (mciSendString(command.c_str(), NULL, 0, NULL) != 0)
    {
        return;
    }

    // 3. 별칭을 이용해 장치 ID(Device ID)를 얻어옴
    DWORD deviceID = mciGetDeviceID(alias.c_str());
    if (deviceID == 0)
    {
        // 장치 ID 얻기 실패 시, 열었던 장치를 바로 닫음
        command = L"close " + alias;
        mciSendString(command.c_str(), NULL, 0, NULL);
        return;
    }

    // 4. 장치 ID와 별칭을 맵에 저장 (나중에 닫을 때 사용)
    SoundInstance newInstance;
    newInstance.alias = alias;
    newInstance.path = soundPath; // 파일 경로도 함께 저장!
    m_deviceMap[deviceID] = newInstance;

    // 5. "play" 명령에 "notify" 옵션을 추가하여 재생 요청
    command = L"play " + alias + L" notify";
    mciSendString(command.c_str(), NULL, 0, m_hWnd);
}

void SoundManager::HandleMciNotify(WPARAM wParam, LPARAM lParam)
{
    // 재생이 성공적으로 끝났을 때만 처리
    if (wParam == MCI_NOTIFY_SUCCESSFUL)
    {
        DWORD deviceID = (DWORD)lParam; // lParam으로 재생이 끝난 장치의 ID가 전달됨

        // 맵에서 해당 장치 ID를 찾음
        auto it = m_deviceMap.find(deviceID);
        if (it != m_deviceMap.end())
        {
            std::wstring alias = it->second.alias; // 맵에서 별칭을 가져옴

            // "close" 명령으로 장치를 닫아 리소스 해제
            std::wstring command = L"close " + alias;
            mciSendString(command.c_str(), NULL, 0, NULL);

            // 맵에서 해당 항목 제거
            m_deviceMap.erase(it);
        }
    }
}

bool SoundManager::IsPlaying(const std::wstring& soundPath) const
{
    // m_deviceMap에 저장된 모든 재생 중인 사운드를 순회합니다.
    for (auto const& [deviceID, instanceInfo] : m_deviceMap)
    {
        // 저장된 경로(instanceInfo.path)와 찾으려는 경로(soundPath)가 일치하는지 확인
        if (instanceInfo.path == soundPath)
        {
            // 일치하는 것을 찾았으므로 true를 반환하고 함수를 종료합니다.
            return true;
        }
    }

    // 루프를 모두 돌았는데도 찾지 못했다면, 재생 중이 아니므로 false를 반환합니다.
    return false;
}
