// PlayerStateMachine.h
#pragma once

#include "PlayerStateDefs.h"
#include <vector>
#include <memory> // unique_ptr ���

// ���� ����
class CTerrainPlayer;       // ���� �÷��̾� Ŭ���� Ÿ������ ���� �ʿ�
class CAnimationController;
class IPlayerState;

// --- ���� �ӽ� Ŭ���� ---
class PlayerStateMachine {
public:
    PlayerStateMachine(CTerrainPlayer* owner, CAnimationController* animCtrl);
    ~PlayerStateMachine();

    // �� ������ ȣ��Ǿ� ���� ������Ʈ �� ���� ó��
    void Update(float deltaTime);
    // �Է� �ý������κ��� ȣ��Ǿ� �Է� ���� ����
    void HandleInput(const PlayerInputData& input);
    // ���� ���� ID ��ȯ
    PlayerStateID GetCurrentStateID() const;
    // ���� �ִϸ��̼� Ʈ���� ��� ��ġ ��ȯ (���¿��� ���� Ȯ�� � ���)
    float GetTrackPosition(int trackIndex) const;
    // ���� �ִϸ��̼� Ʈ���� ���� ��ȯ
    float GetAnimationLength(int trackIndex) const;

    const PlayerInputData& GetLastInput() const { return m_LastInput; }

    // ���� Ű �Է� �� ȣ��
    PlayerStateID DetermineAttackState();
private:
    CTerrainPlayer* m_pOwner = nullptr;                // ���� �ӽ��� ������ �÷��̾� ��ü
    CAnimationController* m_pAnimController = nullptr; // �ִϸ��̼� �����
    IPlayerState* m_pCurrentState = nullptr;         // ���� Ȱ�� ���� (Raw ������)
    PlayerStateID m_eCurrentStateID = PlayerStateID::None;

    // ��� ���� ��ü �ν��Ͻ� ���� (unique_ptr�� ������ ����)
    std::vector<std::unique_ptr<IPlayerState>> m_vStates;

    PlayerInputData m_LastInput; // ���������� ���� �Է� ���� ����

    // --- ���� ���� ���� ---
    PlayerStateID m_eTargetStateID = PlayerStateID::None; // ��ȯ ��ǥ ���� ID
    int m_nSourceTrack = BlendConfig::PRIMARY_TRACK;    // ���� �� �ִϸ��̼� Ʈ�� (���� �ƿ��� Ʈ��)
    int m_nTargetTrack = BlendConfig::BLENDING_TRACK;   // ���� ����� �ִϸ��̼� Ʈ�� (���� �ε� Ʈ��)
    float m_fBlendDuration = BlendConfig::DEFAULT_BLEND_DURATION; // ���� ���� �ð�
    float m_fBlendTimer = 0.0f;                       // ���� ���� ���� �ð�
    bool m_bIsBlending = false;                       // ���� ���� �� ���� �÷���

    // ���� ��ü �������� (���� ���� �Լ�)
    IPlayerState* GetState(PlayerStateID id);
    // ���� ��ȯ ó�� (���� ���� �Լ�)
    void PerformStateChange(PlayerStateID newStateID, bool forceImmediate = false);
    // ���� ������Ʈ ó�� (���� ���� �Լ�)
    void UpdateBlend(float deltaTime);


};

// --- ���� �������̽� ---
class IPlayerState {
public:
    virtual ~IPlayerState() = default;

    // ���� ���� �� ȣ��
    virtual void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) = 0;
    // �� ������ ���� ���� ���� �� ���� ���� ID ��ȯ
    virtual PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) = 0;
    // ���� ���� �� ȣ��
    virtual void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) = 0;
    // ���� ID ��ȯ
    virtual PlayerStateID GetID() const = 0;
};


