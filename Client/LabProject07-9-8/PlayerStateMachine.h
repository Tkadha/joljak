// PlayerStateMachine.h
#pragma once

#include "PlayerStateDefs.h"
#include <vector>
#include <memory> // unique_ptr ���

// ���� ����
class CTerrainPlayer;       // ���� �÷��̾� Ŭ���� Ÿ������ ���� �ʿ�
class CAnimationController;
class IPlayerState;

struct ObjectInfo {
    bool hasHit = true;
    XMFLOAT3 position;
    float x;
    float y;
    float z;
};

struct HitInfo {
    bool hasHit = false;
    XMFLOAT3 position;
    int shardType = 0; // [����] ���� ������ int�� ���� (0:����, 1:����, 2:��)
};


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

    void SetLastHitInfo(const XMFLOAT3& position, int type) {
        m_LastHitInfo.hasHit = true;
        m_LastHitInfo.position = position;
        m_LastHitInfo.shardType = type;
    }
    
    void SetObjectHitInfo(const XMFLOAT3& position, float x2, float y2, float z2) {
        m_ObjectInfo.hasHit = true;
        m_ObjectInfo.position = position;;
        m_ObjectInfo.x = x2;
        m_ObjectInfo.y = y2;
        m_ObjectInfo.z = z2;
    }
    
    ObjectInfo GetLastObjectInfo() {
        ObjectInfo objecthit = m_ObjectInfo;
        m_ObjectInfo.hasHit = false;
        return objecthit;
    }
    // [�߰�] GameFramework�� ������ ���� ������ �������� public �Լ�
    HitInfo GetAndClearLastHitInfo() {
        HitInfo lastHit = m_LastHitInfo; // ������ ����
        m_LastHitInfo.hasHit = false;      // ���� �ʱ�ȭ (�ߺ� ������ ���� ����)
        return lastHit;
    }

    // ���� Ű �Է� �� ȣ��
    PlayerStateID DetermineAttackState();


    // ���� ��ȯ ó�� (���� ���� �Լ�) - public �̵�
    void PerformStateChange(PlayerStateID newStateID, bool forceImmediate = false);
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
    // ���� ������Ʈ ó�� (���� ���� �Լ�)
    void UpdateBlend(float deltaTime);

    HitInfo m_LastHitInfo;
    ObjectInfo m_ObjectInfo;
};

class CGameObject;

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

    void CollisionUpdate(CTerrainPlayer* player, CGameObject* hitObject);
    
};


