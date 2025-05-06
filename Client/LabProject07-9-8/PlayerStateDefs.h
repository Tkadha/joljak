// PlayerStateDefs.h
#pragma once
#include <DirectXMath.h> // XMFLOAT3 등 사용

// --- 플레이어 상태 ID ---
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
    Interact,     // 일반 상호작용 (상황 따라 세분화 가능)
    HitReaction,
    Dead
    // 필요한 상태 추가
};

// --- 애니메이션 인덱스 (CAnimationSet 배열 인덱스와 일치 필요) ---
namespace AnimIndices {
    // 이 값들은 실제 로드된 애니메이션 순서에 맞게 수정해야 합니다!
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
    // ... 기타 애니메이션 ...
}

// --- 상태 머신으로 전달될 입력 정보 ---
struct PlayerInputData {
    bool MoveForward = false;
    bool MoveBackward = false;
    bool MoveLeft = false;
    bool MoveRight = false;
    bool Jump = false;
    bool Attack = false; // 예: F키 또는 마우스 클릭
    bool Interact = false; // 예: E키
    bool Run = false; // 예: Shift 키
    // 필요시 다른 키나 마우스 입력 추가

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

// --- 블렌딩 설정 ---
namespace BlendConfig {
    const float DEFAULT_BLEND_DURATION = 0.2f; // 기본 블렌딩 시간 (초)
    const int PRIMARY_TRACK = 0;              // 주 애니메이션 트랙
    const int BLENDING_TRACK = 1;             // 블렌딩용 보조 트랙
}