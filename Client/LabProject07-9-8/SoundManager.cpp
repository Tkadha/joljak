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
    for (auto const& [deviceID, alias] : m_deviceMap)
    {
        std::wstring command = L"close " + alias;
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
    m_deviceMap[deviceID] = alias;

    // 5. "play" ��ɿ� "notify" �ɼ��� �߰��Ͽ� ��� ��û
    command = L"play " + alias + L" notify";
    mciSendString(command.c_str(), NULL, 0, m_hWnd);
}

void SoundManager::HandleMciNotify(WPARAM wParam, LPARAM lParam)
{
    // ����� ���������� ������ ���� ó��
    if (wParam == MCI_NOTIFY_SUCCESSFUL)
    {
        DWORD deviceID = (DWORD)lParam; // lParam���� ����� ���� ��ġ�� ID�� ���޵�

        // �ʿ��� �ش� ��ġ ID�� ã��
        auto it = m_deviceMap.find(deviceID);
        if (it != m_deviceMap.end())
        {
            std::wstring alias = it->second; // �ʿ��� ��Ī�� ������

            // "close" ������� ��ġ�� �ݾ� ���ҽ� ����
            std::wstring command = L"close " + alias;
            mciSendString(command.c_str(), NULL, 0, NULL);

            // �ʿ��� �ش� �׸� ����
            m_deviceMap.erase(it);
        }
    }
}