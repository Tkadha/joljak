#include "SoundManager.h"
#include <mmsystem.h>

// GetInstance() �޼��� ����
SoundManager& SoundManager::GetInstance()
{
    // C++11 �̻󿡼� �� ����� �����忡 �����մϴ�.
    static SoundManager instance; // �� �ν��Ͻ��� ���α׷� ��ü���� �� �ϳ��Դϴ�.
    return instance;
}

// ������ ����
SoundManager::SoundManager()
    : m_hWnd(NULL), m_aliasCounter(0)
{
}

SoundManager::~SoundManager()
{
    // ���α׷� ���� �� Ȥ�ó� �����ִ� ���尡 �ִٸ� ��� �ݽ��ϴ�.
    for (auto const& [deviceID, instance] : m_deviceMap)
    {
        std::wstring command = L"close " + instance.alias;
        mciSendString(command.c_str(), NULL, 0, NULL);
    }
    m_deviceMap.clear();
}

// �ʱ�ȭ �Լ� ����
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

    // 1. ������ ��Ī ���� (��: "snd_0", "snd_1", ...)
    std::wstring alias = L"snd_" + std::to_wstring(m_aliasCounter++);

    // 2. "open" ������� ���� ���� ����
    std::wstring command = L"open \"" + soundPath + L"\" type waveaudio alias " + alias;
    if (mciSendString(command.c_str(), NULL, 0, NULL) != 0)
    {
        return;
    }

    // 3. ��Ī�� �̿��� ��ġ ID(Device ID)�� ����
    DWORD deviceID = mciGetDeviceID(alias.c_str());
    if (deviceID == 0)
    {
        // ��ġ ID ��� ���� ��, ������ ��ġ�� �ٷ� ����
        command = L"close " + alias;
        mciSendString(command.c_str(), NULL, 0, NULL);
        return;
    }

    // 4. ��ġ ID�� ��Ī�� �ʿ� ���� (���߿� ���� �� ���)
    SoundInstance newInstance;
    newInstance.alias = alias;
    newInstance.path = soundPath; // ���� ��ε� �Բ� ����!
    m_deviceMap[deviceID] = newInstance;

    // 5. "play" ��ɿ� "notify" �ɼ��� �߰��Ͽ� ��� ��û
    command = L"play " + alias + L" notify";
    mciSendString(command.c_str(), NULL, 0, m_hWnd);
}

void SoundManager::HandleMciNotify(WPARAM wParam, LPARAM lParam)
{
    // ����� ���������� ������ ���� ó��
    if (wParam == MCI_NOTIFY_SUCCESSFUL)
    {
        DWORD deviceID = (DWORD)lParam;
        auto it = m_deviceMap.find(deviceID);
        if (it != m_deviceMap.end())
        {
            const SoundInstance& instance = it->second;

            // ��Ī�� "pre_"�� �����ϴ��� Ȯ��
            if (instance.alias.rfind(L"pre_", 0) != 0)
            {
                // "pre_"�� �������� �ʴ� ���� ����("snd_")�� close �մϴ�.
                std::wstring command = L"close " + instance.alias;
                mciSendString(command.c_str(), NULL, 0, NULL);
            }
            // "pre_"�� �����ϴ� ����� close���� �ʰ� ����� ���¸� �����մϴ�!

            // ����� �������Ƿ� ���� ��� ��Ͽ����� �����մϴ�.
            m_deviceMap.erase(it);
        }
    }
}

bool SoundManager::IsPlaying(const std::wstring& soundPath) const
{
    // m_deviceMap�� ����� ��� ��� ���� ���带 ��ȸ�մϴ�.
    for (auto const& [deviceID, instanceInfo] : m_deviceMap)
    {
        // ����� ���(instanceInfo.path)�� ã������ ���(soundPath)�� ��ġ�ϴ��� Ȯ��
        if (instanceInfo.path == soundPath)
        {
            // ��ġ�ϴ� ���� ã�����Ƿ� true�� ��ȯ�ϰ� �Լ��� �����մϴ�.
            return true;
        }
    }

    // ������ ��� ���Ҵµ��� ã�� ���ߴٸ�, ��� ���� �ƴϹǷ� false�� ��ȯ�մϴ�.
    return false;
}


void SoundManager::LoadSound(const std::wstring& soundPath, const std::wstring& key)
{
    // �̹� �ε�� Ű��� �ߺ� �ε� ����
    if (m_preOpenedSounds.count(key)) return;

    LoadedSoundInfo info;
    info.alias = L"pre_" + key; // "pre_jump", "pre_hit"
    info.path = soundPath;

    std::wstring command = L"open \"" + info.path + L"\" type waveaudio alias " + info.alias;
    if (mciSendString(command.c_str(), NULL, 0, NULL) == 0)
    {
        m_preOpenedSounds[key] = info;
    }
}

void SoundManager::PlayLoadedSound(const std::wstring& key)
{
    auto it = m_preOpenedSounds.find(key);
    if (it == m_preOpenedSounds.end()) return; // �ε���� ���� ����

    if (key == L"Sound/Golem/Stamp land.wav" && IsPlaying(L"Sound/Golem/Stamp land.wav"))
    {
        return;
    }

    const LoadedSoundInfo& info = it->second; // �ε�� ���� ���� ��������

    // 1. ��ġ ID ��������
    DWORD deviceID = mciGetDeviceID(info.alias.c_str());
    if (deviceID == 0) return; // ����

    // 2. ���� ��� ���(m_deviceMap)�� ���� ���
    SoundInstance instance;
    instance.alias = info.alias;
    instance.path = info.path;
    m_deviceMap[deviceID] = instance;

    // 3. "from 0" �ɼ����� ó������ ��� + "notify"�� ���� �˸� �ޱ�
    std::wstring command = L"play " + info.alias + L" from 0 notify";
    mciSendString(command.c_str(), NULL, 0, m_hWnd);
}

void SoundManager::UnloadAllSounds()
{
    // �̸� ����ξ��� ��� ���带 �ݽ��ϴ�.
    for (auto const& [key, info] : m_preOpenedSounds)
    {
        std::wstring command = L"close " + info.alias;
        mciSendString(command.c_str(), NULL, 0, NULL);
    }
    // ���� ���ϴ�.
    m_preOpenedSounds.clear();
}
