// PlayerStateDefs.h
#pragma once
#include <DirectXMath.h> // XMFLOAT3 �� ���

// --- �÷��̾� ���� ID ---
enum class PlayerStateID {
    None = -1,
    Idle,
    WalkForward,
    WalkBackward,
    MoveLeft,
    MoveRight,
    RunForward,
    AttackMelee1,
    JumpStart,
    JumpLoop,
    JumpEnd,
    Interact,     // �Ϲ� ��ȣ�ۿ� (��Ȳ ���� ����ȭ ����)
    HitReaction,
    Dead
    // �ʿ��� ���� �߰�
};

// --- �ִϸ��̼� �ε��� (CAnimationSet �迭 �ε����� ��ġ �ʿ�) ---
namespace AnimIndices {
    // �� ������ ���� �ε�� �ִϸ��̼� ������ �°� �����ؾ� �մϴ�!
    const int IDLE = 0;
    const int WALK_FORWARD = 1;
    const int WALK_BACKWARD = 2; 
    const int WALK_LEFT = 3;
    const int WALK_RIGHT = 4;
    const int RUN_FORWARD = 5;
    const int RUN_BACKWARD = 6;
    const int RUN_LEFT = 7;
    const int RUN_RIGHT = 8;
    const int JUMP_START = 9;    
    const int JUMP_LOOP = 10;     
    const int JUMP_END = 11;     
    const int ATTACK_MELEE1 = 12;
    const int ATTACK_AXE = 13;
    const int ATTACK_PICK = 14;
    const int HIT_REACTION = 15;
    // ... ��Ÿ �ִϸ��̼� ...
}

// --- ���� �ӽ����� ���޵� �Է� ���� ---
struct PlayerInputData {
    bool MoveForward = false;
    bool MoveBackward = false;
    bool MoveLeft = false;
    bool MoveRight = false;
    bool Jump = false;
    bool Attack = false; // ��: FŰ �Ǵ� ���콺 Ŭ��
    bool Interact = false; // ��: EŰ
    bool Run = false; // ��: Shift Ű
    // �ʿ�� �ٸ� Ű�� ���콺 �Է� �߰�

    bool operator!=(const PlayerInputData& other) const {
        return MoveForward != other.MoveForward ||
            MoveBackward != other.MoveBackward ||
            MoveLeft != other.MoveLeft ||
            MoveRight != other.MoveRight ||
            Jump != other.Jump ||
            Attack != other.Attack ||
            Interact != other.Interact ||
            Run != other.Run;
    }
};

// --- ���� ���� ---
namespace BlendConfig {
    const float DEFAULT_BLEND_DURATION = 0.2f; // �⺻ ���� �ð� (��)
    const int PRIMARY_TRACK = 0;              // �� �ִϸ��̼� Ʈ��
    const int BLENDING_TRACK = 1;             // ������ ���� Ʈ��
}