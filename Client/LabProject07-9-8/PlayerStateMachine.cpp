// PlayerStateMachine.cpp
#include "stdafx.h" // 필요시 프로젝트의 stdafx.h 포함
#include "PlayerStateMachine.h"
#include "Player.h" // CTerrainPlayer 정의 포함
#include "Animation.h" // CAnimationController 정의 포함
#include <iostream>   // 디버깅 출력용 (추후 제거)
#include <algorithm>  // std::min, std::max 등 사용
#include "GameFramework.h"
#include "Object.h"

// --- 구체적인 상태 클래스 구현 ---

#define WalkSpeed 50
#define RunSpeed 80

class IdleState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::Idle; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput(); // 입력 가져오기

        // 이동 입력 체크
        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.WalkLeft) return PlayerStateID::WalkLeft;
        if (input.WalkRight) return PlayerStateID::WalkRight;
        // 기타 입력 (공격, 점프 등)
        if (input.Attack)  //return PlayerStateID::AttackMelee;
            return stateMachine->DetermineAttackState();
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { 
        }

        // 다른 입력이 없으면 Idle 상태 유지
        return PlayerStateID::Idle;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }
};
// Walk
class WalkForwardState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::WalkForward; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // 애니메이션 설정은 PlayerStateMachine::PerformStateChange 에서 처리
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput();

        XMFLOAT3 moveVector = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 look = player->GetLookVector();
        XMFLOAT3 right = player->GetRightVector();
        bool isMoving = false;

        if (input.MoveForward) {
            moveVector = Vector3::Add(moveVector, look);
            isMoving = true;
        }
        // WalkForward 상태라도 다른 키 입력을 확인하여 벡터에 더함
        if (input.MoveBackward) { // 앞뒤 동시 입력 시 상쇄 또는 다른 처리 필요 시 추가
            moveVector = Vector3::Add(moveVector, look, -1.0f);
            isMoving = true; // 뒤로 가는 키도 눌렸음
        }
        if (input.WalkLeft) {
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.WalkRight) {
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector); // 대각선 이동 시 속도 보정 위해 정규화
            XMFLOAT3 currentVel = player->GetVelocity(); // 현재 Y 속도 유지
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, WalkSpeed, false);
            targetVel.y = currentVel.y; // Y 속도는 유지
            player->SetVelocity(targetVel); // 최종 속도 설정
        }
        else {
            // 모든 이동 키가 떨어졌으면 Idle 상태로 전환
            return PlayerStateID::Idle;
        }
        // 입력에 따른 상태 전환 체크
        if (input.Run && input.MoveForward && !input.WalkLeft && !input.WalkRight && !input.MoveBackward) {
            return PlayerStateID::RunForward;
        }
        if (input.Attack) return PlayerStateID::AttackMelee;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* 상호작용 체크 */ }


        // 현재 상태 유지 조건 결정 (예: 어떤 이동키든 눌려 있으면 해당 주 방향 상태 유지)
        if (input.MoveForward) return PlayerStateID::WalkForward; // 주 방향 우선
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.WalkLeft) return PlayerStateID::WalkLeft;
        if (input.WalkRight) return PlayerStateID::WalkRight;

        // 여기까지 오면 안되지만, 만약을 위해 Idle 반환
        return PlayerStateID::Idle;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // 필요하다면 속도 초기화 등
        // player->SetVelocity({0.0f, player->GetVelocity().y, 0.0f}); // Y축 속도 유지하며 XZ만 0으로
    }
};
class WalkBackwardState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::WalkBackward; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput();

        XMFLOAT3 moveVector = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 look = player->GetLookVector();
        XMFLOAT3 right = player->GetRightVector();
        bool isMoving = false;

        if (input.MoveForward) {
            moveVector = Vector3::Add(moveVector, look);
            isMoving = true;
        }
        if (input.MoveBackward) {
            moveVector = Vector3::Add(moveVector, look, -1.0f);
            isMoving = true;
        }
        if (input.WalkLeft) { // PlayerInputData 멤버 이름에 맞게 수정 (WalkLeft -> StrafeLeft)
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.WalkRight) { // PlayerInputData 멤버 이름에 맞게 수정 (WalkRight -> StrafeRight)
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector); // 대각선 이동 속도 보정
            XMFLOAT3 currentVel = player->GetVelocity();
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, WalkSpeed, false);
            targetVel.y = currentVel.y;
            player->SetVelocity(targetVel);
        }
        else {
            return PlayerStateID::Idle;
        }

        if (input.Run && input.MoveBackward && !input.MoveForward && !input.WalkLeft && !input.WalkRight) {
            return PlayerStateID::RunBackward; // RunBackward 상태로 전환 요청
        }

        if (input.Attack) return PlayerStateID::AttackMelee;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* 상호작용 체크 */ }

        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.WalkLeft) return PlayerStateID::WalkLeft;
        if (input.WalkRight) return PlayerStateID::WalkRight;

        // 여기까지 오면 안 되지만, 안전을 위해 Idle 반환
        return PlayerStateID::Idle;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // player->SetVelocity({0.0f, player->GetVelocity().y, 0.0f});
    }
};
class WalkLeftState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::WalkLeft; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput();

        XMFLOAT3 moveVector = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 look = player->GetLookVector();
        XMFLOAT3 right = player->GetRightVector();
        bool isMoving = false;

        if (input.MoveForward) {
            moveVector = Vector3::Add(moveVector, look);
            isMoving = true;
        }
        if (input.MoveBackward) {
            moveVector = Vector3::Add(moveVector, look, -1.0f);
            isMoving = true;
        }
        if (input.WalkLeft) {
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.WalkRight) {
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector);
            XMFLOAT3 currentVel = player->GetVelocity();
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, WalkSpeed, false);
            targetVel.y = currentVel.y;
            player->SetVelocity(targetVel);
        }
        else {
            return PlayerStateID::Idle;
        }

        if (input.Run && input.WalkLeft && !input.MoveForward && !input.MoveBackward && !input.WalkRight) {
            return PlayerStateID::RunLeft; 
        }

        if (input.Attack) return PlayerStateID::AttackMelee;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* 상호작용 체크 */ }

        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.WalkLeft) return PlayerStateID::WalkLeft;
        if (input.WalkRight) return PlayerStateID::WalkRight;

        return PlayerStateID::Idle;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // player->SetVelocity({0.0f, player->GetVelocity().y, 0.0f});
    }
};
class WalkRightState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::WalkRight; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput();

        XMFLOAT3 moveVector = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 look = player->GetLookVector();
        XMFLOAT3 right = player->GetRightVector();
        bool isMoving = false;

        if (input.MoveForward) {
            moveVector = Vector3::Add(moveVector, look);
            isMoving = true;
        }
        if (input.MoveBackward) {
            moveVector = Vector3::Add(moveVector, look, -1.0f);
            isMoving = true;
        }
        if (input.WalkLeft) {
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.WalkRight) {
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        // --- 속도 적용 ---
        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector);
            XMFLOAT3 currentVel = player->GetVelocity();
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, WalkSpeed, false);
            targetVel.y = currentVel.y;
            player->SetVelocity(targetVel);
        }
        else {
            return PlayerStateID::Idle;
        }
         
        if (input.Run && input.WalkRight && !input.MoveForward && !input.MoveBackward && !input.WalkLeft ) {
            return PlayerStateID::RunRight;
        }

        if (input.Attack) return PlayerStateID::AttackMelee;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* 상호작용 체크 */ }

        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.WalkLeft) return PlayerStateID::WalkLeft;
        if (input.WalkRight) return PlayerStateID::WalkRight;

        return PlayerStateID::Idle;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // player->SetVelocity({0.0f, player->GetVelocity().y, 0.0f});
    }
};
// Run
class RunForwardState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::RunForward; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput();

        if (!input.Run || !input.MoveForward || input.MoveBackward || input.WalkLeft || input.WalkRight) {
            if (input.MoveForward) return PlayerStateID::WalkForward;
            if (input.WalkLeft) return PlayerStateID::WalkLeft;
            if (input.WalkRight) return PlayerStateID::WalkRight;
            if (input.MoveBackward) return PlayerStateID::WalkBackward;
            return PlayerStateID::Idle;
        }

        // 달리기 적용
        XMFLOAT3 look = player->GetLookVector();
        XMFLOAT3 currentVel = player->GetVelocity();
        XMFLOAT3 targetVel = Vector3::ScalarProduct(look, RunSpeed, false);
        targetVel.y = currentVel.y;
        player->SetVelocity(targetVel);

        // 다른 상태로 전환 체크
        if (input.Attack) return PlayerStateID::AttackMelee; // 달리는 중 공격 가능?
        if (input.Jump) return PlayerStateID::JumpStart;   // 달리는 중 점프 가능?
        if (input.Interact) { /* 상호작용 체크 */ }

        return PlayerStateID::RunForward;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }
};
class RunBackwardState : public IPlayerState {
public:
    // PlayerStateID::RunBackward ID 가 정의되었다고 가정
    PlayerStateID GetID() const override { return PlayerStateID::RunBackward; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Entering RunBackward State\n";
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput();

        // --- 달리기 중단/방향 전환 조건 확인 ---
        if (!input.Run || !input.MoveBackward || input.MoveForward || input.WalkLeft || input.WalkRight) {
            // 달리기 키를 뗐거나, 뒤로가기 키를 뗐거나, 다른 방향키가 눌리면 상태 전환
            if (input.MoveBackward) return PlayerStateID::WalkBackward; // 달리기는 멈추고 뒤로 걷기
            if (input.MoveForward) return PlayerStateID::WalkForward; // 앞으로 걷기
            if (input.WalkLeft) return PlayerStateID::WalkLeft;   // 왼쪽 걷기
            if (input.WalkRight) return PlayerStateID::WalkRight;  // 오른쪽 걷기
            return PlayerStateID::Idle; // 모든 이동 키 뗐으면 Idle
        }

        // --- 뒤로 달리기 속도 적용 ---
        XMFLOAT3 look = player->GetLookVector();
        XMFLOAT3 currentVel = player->GetVelocity();
        XMFLOAT3 targetVel = Vector3::ScalarProduct(look, -RunSpeed, false); // 뒤로 달리기
        targetVel.y = currentVel.y; // Y 속도 유지
        player->SetVelocity(targetVel);

        // --- 다른 상태로 전환 체크 ---
        if (input.Attack) return PlayerStateID::AttackMelee;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* 상호작용 체크 */ }

        return PlayerStateID::RunBackward; // 뒤로 달리기 상태 유지
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Exiting RunBackward State\n";
    }
};
class RunLeftState : public IPlayerState {
public:
    // PlayerStateID::RunLeft ID 가 정의되었다고 가정 (또는 PlayerStateID::WalkLeft 사용 시 조정 필요)
    PlayerStateID GetID() const override { return PlayerStateID::RunLeft; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Entering RunLeft State\n";
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput();

        if (!input.Run || !input.WalkLeft || input.MoveForward || input.MoveBackward || input.WalkRight) {
            if (input.WalkLeft) return PlayerStateID::WalkLeft;    
            if (input.MoveForward) return PlayerStateID::WalkForward; 
            if (input.MoveBackward) return PlayerStateID::WalkBackward; 
            if (input.WalkRight) return PlayerStateID::WalkRight;    
            return PlayerStateID::Idle;
        }

        XMFLOAT3 right = player->GetRightVector();
        XMFLOAT3 currentVel = player->GetVelocity();
        XMFLOAT3 targetVel = Vector3::ScalarProduct(right, -RunSpeed, false); // 왼쪽으로 달리기
        targetVel.y = currentVel.y;
        player->SetVelocity(targetVel);

        if (input.Attack) return PlayerStateID::AttackMelee;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* 상호작용 체크 */ }

        return PlayerStateID::RunLeft; // 왼쪽 달리기 상태 유지
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Exiting RunLeft State\n";
    }
};
class RunRightState : public IPlayerState {
public:
    // PlayerStateID::RunRight ID 가 정의되었다고 가정
    PlayerStateID GetID() const override { return PlayerStateID::RunRight; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Entering RunRight State\n";
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput();


        if (!input.Run || !input.WalkRight || input.MoveForward || input.MoveBackward || input.WalkLeft) {
            if (input.WalkRight) return PlayerStateID::WalkRight;    
            if (input.MoveForward) return PlayerStateID::WalkForward;   
            if (input.MoveBackward) return PlayerStateID::WalkBackward; 
            if (input.WalkLeft) return PlayerStateID::WalkLeft;    
            return PlayerStateID::Idle;
        }

        XMFLOAT3 right = player->GetRightVector();
        XMFLOAT3 currentVel = player->GetVelocity();
        XMFLOAT3 targetVel = Vector3::ScalarProduct(right, RunSpeed, false); // 오른쪽으로 달리기
        targetVel.y = currentVel.y;
        player->SetVelocity(targetVel);

        // 다른 상태로 전환 체크
        if (input.Attack) return PlayerStateID::AttackMelee;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* 상호작용 체크 */ }

        return PlayerStateID::RunRight;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Exiting RunRight State\n";
    }
};

// Attack
#include "NonAtkState.h"
#include "AtkState.h"
class AttackMeleeState : public IPlayerState {
private:
    bool m_bAttackFinished = false;
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK; // 공격 애니메이션을 재생할 트랙

public:
    PlayerStateID GetID() const override { return PlayerStateID::AttackMelee; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAttackFinished = false;
        player->SetVelocity({ 0.0f, player->GetVelocity().y, 0.0f });
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        if (!m_bAttackFinished && currentPosition >= animLength * 0.95f) { 
            m_bAttackFinished = true;

            CGameObject* hitObject = player->FindObjectHitByAttack();
            CollisionUpdate(player, hitObject);
        }

        // 애니메이션이 끝나면 Idle 상태로 전환
        if (m_bAttackFinished) {
            return PlayerStateID::Idle;
        }

        // 공격 중에는 다른 입력 무시하고 현재 상태 유지
        return PlayerStateID::AttackMelee;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }
};
class AttackAxeState : public IPlayerState {
private:
    bool m_bAttackFinished = false;
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
public:
    PlayerStateID GetID() const override { return PlayerStateID::AttackAxe; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAttackFinished = false;
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK; // 즉시 전환 시 주 트랙 사용
        player->SetVelocity({ 0.0f, player->GetVelocity().y, 0.0f }); // 공격 중 이동 정지
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        if (!m_bAttackFinished && currentPosition >= animLength * 0.95f) {
            m_bAttackFinished = true;

            CGameObject* hitObject = player->FindObjectHitByAttack(); // 공격 판정 함수 필요
            CollisionUpdate(player, hitObject);
        }
        if (m_bAttackFinished) {
            return PlayerStateID::Idle;
        }
        return PlayerStateID::AttackAxe;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Exiting AttackTree State\n";
    }
};
class AttackPickState : public IPlayerState { 
private:
    bool m_bAttackFinished = false;
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
public:
    PlayerStateID GetID() const override { return PlayerStateID::AttackPick; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAttackFinished = false;
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK; // 즉시 전환 시 주 트랙 사용
        player->SetVelocity({ 0.0f, player->GetVelocity().y, 0.0f }); // 공격 중 이동 정지
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        if (!m_bAttackFinished && currentPosition >= animLength * 0.95f) { 
            m_bAttackFinished = true;

            CGameObject* hitObject = player->FindObjectHitByAttack(); // 공격 판정 함수 필요
            CollisionUpdate(player, hitObject);
        }
        if (m_bAttackFinished) {
            return PlayerStateID::Idle;
        }
        return PlayerStateID::AttackPick;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Exiting AttackTree State\n";
    }
};




class JumpStartState : public IPlayerState {
private:
    bool m_bAnimFinished = false;
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
    float m_fJumpVelocity = 30.0f; // 점프 시 수직 속도 (조정 필요)

public:
    PlayerStateID GetID() const override { return PlayerStateID::JumpStart; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAnimFinished = false;
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK; // 주 트랙 사용 가정

        // 위쪽으로 속도 적용
        XMFLOAT3 currentVel = player->GetVelocity();
        currentVel.y = m_fJumpVelocity;
        player->SetVelocity(currentVel);
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        // 점프 시작 애니메이션 재생 완료 확인
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        if (!m_bAnimFinished && currentPosition >= animLength * 0.99f) {
            m_bAnimFinished = true;
        }

        // 애니메이션 끝나면 JumpLoop 상태로 전환
        if (m_bAnimFinished) {
            return PlayerStateID::JumpLoop;
        }

        return PlayerStateID::JumpStart; // 애니메이션 재생 중에는 상태 유지
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }
};
class JumpLoopState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::JumpLoop; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // 공중 애니메이션 설정 (PerformStateChange에서 처리됨)
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        // 공중에서 좌우 이동 등 처리 (선택적)
        const auto& input = player->GetStateMachineInput();
        float airControlFactor = 0.1f; // 공중 제어력 (조정 필요)
        XMFLOAT3 currentVel = player->GetVelocity();
        XMFLOAT3 right = player->GetRightVector();

        if (input.WalkLeft) {
            currentVel = Vector3::Add(currentVel, right, -50.0f * airControlFactor);
        }
        if (input.WalkRight) {
            currentVel = Vector3::Add(currentVel, right, 50.0f * airControlFactor);
        }
        // 필요하다면 앞뒤 이동도 약간 허용
        player->SetVelocity(currentVel);


        // 착지 조건 확인 (예: 플레이어 발밑 지형 체크 또는 Y 속도 체크)
        // 여기서는 단순하게 Y 속도가 음수가 되면 착지 시도한다고 가정
        if (player->GetVelocity().y <= 0.0f) {
            // 실제로는 레이캐스트 등으로 바닥 충돌 확인 필요
            // CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)player->GetPlayerUpdatedContext();
            // if (pTerrain && player->GetPosition().y <= pTerrain->GetHeight(player->GetPosition().x, player->GetPosition().z) + 1.0f) { // 땅 근처 1 유닛 이내
            return PlayerStateID::JumpEnd;
            // }
        }

        return PlayerStateID::JumpLoop; // 공중에서는 계속 Loop 상태
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }
};
class JumpEndState : public IPlayerState {
private:
    bool m_bAnimFinished = false;
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
public:
    PlayerStateID GetID() const override { return PlayerStateID::JumpEnd; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAnimFinished = false;
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
        // 착지 애니메이션 설정 (PerformStateChange에서 처리됨)

        // 착지 시 Y 속도 0으로 (선택적, 땅과의 충돌 처리에서 이미 될 수도 있음)
        // XMFLOAT3 currentVel = player->GetVelocity();
        // currentVel.y = 0.0f;
        // player->SetVelocity(currentVel);
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        // 착지 애니메이션 재생 완료 확인
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        if (!m_bAnimFinished && currentPosition >= animLength * 0.99f) {
            m_bAnimFinished = true;
        }

        // 애니메이션 끝나면 Idle 상태로 전환
        if (m_bAnimFinished) {
            return PlayerStateID::Idle;
        }

        return PlayerStateID::JumpEnd; // 애니메이션 재생 중에는 상태 유지
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }
};




void IPlayerState::CollisionUpdate(CTerrainPlayer* player, CGameObject* hitObject)
{
    if (!hitObject) return;
    if (hitObject->m_objectType == GameObjectType::Tree) {
        auto tree = dynamic_cast<CTreeObject*>(hitObject);
        if (tree && !tree->IsFalling() && !tree->HasFallen()) {
            int hp = tree->getHp();
            if (hp > 0) {
                switch (player->weaponType)
                {
                case WeaponType::Sword:
                    hp -= 8;
                    break;
                case WeaponType::Axe:
                    hp -= 10;
                    break;
                case WeaponType::Pick:
                    hp -= 5;
                    break;
                default:
                    break;
                }
                tree->setHp(hp);

                CScene* pScene = player->m_pGameFramework->GetScene();

                if (pScene) {
                    XMFLOAT3 treePos = tree->GetPosition();

                    XMFLOAT3 spawnOffsetLocal = XMFLOAT3(
                        ((float)(rand() % 200) - 100.0f) * 0.1f, // X -10 ~ +10
                        (rand() % 10) + 10.0f,                     // Y 10~19
                        ((float)(rand() % 200) - 100.0f) * 0.1f  // Z -10 ~ +10
                    );
                    
                    XMFLOAT3 spawnPos = Vector3::Add(spawnPos, spawnOffsetLocal);
                    if (pScene->m_pTerrain) { // 지형 위에 스폰되도록 높이 보정
                        spawnPos.y = pScene->m_pTerrain->GetHeight(spawnPos.x, spawnPos.z) + spawnOffsetLocal.y;
                    }

                    XMFLOAT3 ejectVelocity = XMFLOAT3(
                        ((float)(rand() % 100) - 50.0f),
                        ((float)(rand() % 60) + 50.0f),
                        ((float)(rand() % 100) - 50.0f)
                    );
                    pScene->SpawnRock(spawnPos, ejectVelocity);
                }
            }
            if (hp <= 0) {
                tree->StartFalling(player->GetLookVector()); // 플레이어가 바라보는 방향으로 쓰러지도록 (또는 다른 방향)
            }
        }
    }
    else if (hitObject->m_objectType == GameObjectType::Rock) {
        auto rock = dynamic_cast<CRockObject*>(hitObject);
        if (rock) {
            int hp = rock->getHp();
            if (hp > 0) {
                switch (player->weaponType)
                {
                case WeaponType::Sword:
                    hp -= 8;
                    break;
                case WeaponType::Axe:
                    hp -= 10;
                    break;
                case WeaponType::Pick:
                    hp -= 5;
                    break;
                default:
                    break;
                }
                rock->setHp(hp);

                rock->SetScale(0.9f, 0.9f, 0.9f);

                CScene* pScene = player->m_pGameFramework->GetScene();

                if (pScene) {
                    XMFLOAT3 treePos = rock->GetPosition();

                    XMFLOAT3 spawnOffsetLocal = XMFLOAT3(
                        ((float)(rand() % 200) - 100.0f) * 0.1f, // X -10 ~ +10
                        (rand() % 10) + 10.0f,                     // Y 10~19
                        ((float)(rand() % 200) - 100.0f) * 0.1f  // Z -10 ~ +10
                    );

                    XMFLOAT3 spawnPos = Vector3::Add(treePos, spawnOffsetLocal);
                    if (pScene->m_pTerrain) { // 지형 위에 스폰되도록 높이 보정
                        spawnPos.y = pScene->m_pTerrain->GetHeight(spawnPos.x, spawnPos.z) + spawnOffsetLocal.y+20;
                    }

                    XMFLOAT3 ejectVelocity = XMFLOAT3(
                        ((float)(rand() % 100) - 50.0f),
                        ((float)(rand() % 60) + 50.0f),
                        ((float)(rand() % 100) - 50.0f)
                    );

                    player->m_pGameFramework->AddItem("stone", 1);                   
                    //pScene->SpawnRock(spawnPos, ejectVelocity);
                }
            }
            if (hp <= 0) {

                int randValue = rand() % 100; // 0 ~ 99
                if (randValue < 50) {
                    player->m_pGameFramework->AddItem("stone", 3);
                }
                else if (randValue < 75) {
                    player->m_pGameFramework->AddItem("coal", 1);
                }
                else {
                    player->m_pGameFramework->AddItem("iron_material", 1);
                }
                //rock->EraseRock();
                //player->m_pGameFramework->AddItem("rock", 5); // 예시: 쓰러뜨리면 많이 획득
            }
        }
    }
    else if (hitObject->m_objectType == GameObjectType::Cow || hitObject->m_objectType == GameObjectType::Pig) {
        auto npc = dynamic_cast<CMonsterObject*>(hitObject);
        if (npc->Gethp() <= 0) return;
        if (npc->FSM_manager->GetInvincible()) return;

        switch (player->weaponType)
        {
        case WeaponType::Sword:
            npc->Decreasehp(10);
            break;
        case WeaponType::Axe:
            npc->Decreasehp(8);
            break;
        case WeaponType::Pick:
            npc->Decreasehp(6);
            break;
        default:
            break;
        }

        if (npc->Gethp() <= 0) {
            player->m_pGameFramework->AddItem("pork", 2);
        }
        if (hitObject->FSM_manager) {
            if (npc->Gethp() > 0) hitObject->FSM_manager->ChangeState(std::make_shared<NonAtkNPCRunAwayState>());
            else hitObject->FSM_manager->ChangeState(std::make_shared<NonAtkNPCDieState>());
            hitObject->FSM_manager->SetInvincible();
        }
    }

    else if (hitObject->m_objectType != GameObjectType::Unknown && hitObject->m_objectType != GameObjectType::Cow && hitObject->m_objectType != GameObjectType::Pig &&
        hitObject->m_objectType != GameObjectType::Rock && hitObject->m_objectType != GameObjectType::Tree && hitObject->m_objectType != GameObjectType::Player) {
        auto npc = dynamic_cast<CMonsterObject*>(hitObject);
        if (npc->Gethp() <= 0) return;
        if (npc->FSM_manager->GetInvincible()) return;
        switch (player->weaponType)
        {
        case WeaponType::Sword:
            npc->Decreasehp(10);
            break;
        case WeaponType::Axe:
            npc->Decreasehp(8);
            break;
        case WeaponType::Pick:
            npc->Decreasehp(6);
            break;
        default:
            break;
        }


        if (npc->Gethp() <= 0) {
            player->Playerxp += 20;
            if (player->Playerxp >= player->Totalxp) {
                player->PlayerLevel++;
                player->Playerxp = player->Playerxp - player->Totalxp;
                player->Totalxp *= 2;
                player->StatPoint += 5;
            }
        }


        if (hitObject->FSM_manager) {
            if (npc->Gethp() > 0) hitObject->FSM_manager->ChangeState(std::make_shared<AtkNPCHitState>());
            else hitObject->FSM_manager->ChangeState(std::make_shared<AtkNPCDieState>());
            hitObject->FSM_manager->SetInvincible();
        }
    }
}

// --- PlayerStateMachine 클래스 구현 ---

PlayerStateMachine::PlayerStateMachine(CTerrainPlayer* owner, CAnimationController* animCtrl)
    : m_pOwner(owner), m_pAnimController(animCtrl), m_eCurrentStateID(PlayerStateID::None), m_pCurrentState(nullptr) {
    if (!m_pOwner || !m_pAnimController) {
        // 필수 객체 없음 오류 처리
        throw std::runtime_error("PlayerStateMachine requires a valid owner and animation controller!");
    }
    if (m_pAnimController->m_nAnimationTracks < 2) {
        throw std::runtime_error("PlayerStateMachine requires CAnimationController with at least 2 tracks for blending!");
    }

    // 모든 상태 객체 생성 및 벡터에 추가
    m_vStates.push_back(std::make_unique<IdleState>());

    m_vStates.push_back(std::make_unique<WalkForwardState>());
    m_vStates.push_back(std::make_unique<WalkBackwardState>()); 
    m_vStates.push_back(std::make_unique<WalkLeftState>());  
    m_vStates.push_back(std::make_unique<WalkRightState>());  

    m_vStates.push_back(std::make_unique<AttackMeleeState>());
    m_vStates.push_back(std::make_unique<AttackAxeState>());
    m_vStates.push_back(std::make_unique<AttackPickState>());

    m_vStates.push_back(std::make_unique<JumpStartState>());    
    m_vStates.push_back(std::make_unique<JumpLoopState>());     
    m_vStates.push_back(std::make_unique<JumpEndState>());

    m_vStates.push_back(std::make_unique<RunForwardState>());
    m_vStates.push_back(std::make_unique<RunBackwardState>()); 
    m_vStates.push_back(std::make_unique<RunLeftState>());     
    m_vStates.push_back(std::make_unique<RunRightState>());    
    // ... 다른 모든 상태들도 여기에 추가 ...

    // 초기 상태 설정
    PerformStateChange(PlayerStateID::Idle, true);
}

PlayerStateMachine::~PlayerStateMachine() {
    // unique_ptr 가 자동으로 메모리 해제
}

void PlayerStateMachine::Update(float deltaTime) {
    if (!m_pCurrentState) return;

    if (m_bIsBlending) {
        UpdateBlend(deltaTime);
    }
    else {
        // 현재 상태 업데이트 및 다음 상태 ID 받기
        PlayerStateID nextStateID = m_pCurrentState->Update(m_pOwner, this, deltaTime);

        // 상태 전환 요청이 있으면 처리
        if (nextStateID != m_eCurrentStateID) {
            PerformStateChange(nextStateID); // 기본적으로 블렌딩 시도
        }
    }
}

void PlayerStateMachine::HandleInput(const PlayerInputData& input) {
    m_LastInput = input; // 받은 입력 정보 저장
}

PlayerStateID PlayerStateMachine::GetCurrentStateID() const {
    return m_eCurrentStateID;
}

IPlayerState* PlayerStateMachine::GetState(PlayerStateID id) {
    for (const auto& state : m_vStates) {
        if (state->GetID() == id) {
            return state.get(); // unique_ptr에서 관리하는 raw 포인터 반환
        }
    }
    return nullptr; // 해당 ID의 상태를 찾지 못함
}


void PlayerStateMachine::PerformStateChange(PlayerStateID newStateID, bool forceImmediate) {
    
    // 블렌딩 해제(임시)
    forceImmediate = true;

    if (newStateID == m_eCurrentStateID || !m_pAnimController) return; // 같은 상태거나 컨트롤러 없으면 무시

    IPlayerState* pNewState = GetState(newStateID);
    if (!pNewState) {
        std::cerr << "Error: Cannot find state with ID: " << static_cast<int>(newStateID) << std::endl;
        return;
    }

    // 현재 상태 종료 처리
    if (m_pCurrentState) {
        m_pCurrentState->Exit(m_pOwner, this);
    }

    // --- 블렌딩 또는 즉시 전환 처리 ---
    if (!forceImmediate && m_pCurrentState) { // 이전 상태가 있고, 즉시 전환이 아니면 블렌딩 시작
        m_bIsBlending = true;
        m_fBlendTimer = 0.0f;
        m_eTargetStateID = newStateID; // 목표 상태 저장

        // 다음 블렌딩을 위해 트랙 인덱스 교체
        std::swap(m_nSourceTrack, m_nTargetTrack);

        // 목표 상태 진입
        pNewState->Enter(m_pOwner, this);

        // m_nTargetTrack에 새 애니메이션 설정
        int animIndex = AnimIndices::IDLE; 
        int trackType = ANIMATION_TYPE_LOOP;

        const auto& input = m_pOwner->GetStateMachineInput();

        switch (newStateID) { // 목표 상태에 맞는 애니메이션 인덱스 찾기
        case PlayerStateID::Idle:
            animIndex = AnimIndices::IDLE;
            trackType = ANIMATION_TYPE_LOOP; 
            break;
        case PlayerStateID::WalkForward:
            if(input.Run)
                animIndex = AnimIndices::RUN_FORWARD;
            else
                animIndex = AnimIndices::WALK_FORWARD;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::WalkBackward:
            if (input.Run)
                animIndex = AnimIndices::RUN_BACKWARD;
            else
                animIndex = AnimIndices::WALK_BACKWARD;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::WalkLeft:
            if (input.Run)
                animIndex = AnimIndices::RUN_LEFT;
            else
                animIndex = AnimIndices::WALK_LEFT;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::WalkRight:
            if (input.Run)
                animIndex = AnimIndices::RUN_RIGHT;
            else
                animIndex = AnimIndices::WALK_RIGHT;
            trackType = ANIMATION_TYPE_LOOP;
            break;
            

        case PlayerStateID::AttackMelee:
            animIndex = AnimIndices::ATTACK_MELEE1;
            trackType = ANIMATION_TYPE_ONCE;
            break;
        case PlayerStateID::AttackAxe:
            animIndex = AnimIndices::ATTACK_AXE;
            trackType = ANIMATION_TYPE_ONCE;
            break;
        case PlayerStateID::AttackPick:
            animIndex = AnimIndices::ATTACK_PICK;
            trackType = ANIMATION_TYPE_ONCE;
            break;


        case PlayerStateID::JumpStart:
            animIndex = AnimIndices::JUMP_START;
            trackType = ANIMATION_TYPE_ONCE;
            break;
        case PlayerStateID::JumpLoop:
            animIndex = AnimIndices::JUMP_LOOP;
            trackType = ANIMATION_TYPE_LOOP; 
            break;
        case PlayerStateID::JumpEnd:
            animIndex = AnimIndices::JUMP_END;
            trackType = ANIMATION_TYPE_ONCE;
            break;

        case PlayerStateID::RunForward:
            animIndex = AnimIndices::RUN_FORWARD;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::RunBackward:
            animIndex = AnimIndices::RUN_BACKWARD;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::RunLeft:
            animIndex = AnimIndices::RUN_LEFT;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::RunRight:
            animIndex = AnimIndices::RUN_RIGHT;
            trackType = ANIMATION_TYPE_LOOP;
            break;
            // ... 다른 상태들에 대한 애니메이션 인덱스 매핑 추가 ...
        default: std::cerr << "Warning: No animation index mapped for state " << (int)newStateID << std::endl; break;
        }

        // 새 애니메이션 설정: 가중치 0
        m_pAnimController->SetTrackAnimationSet(m_nTargetTrack, animIndex);
        m_pAnimController->SetTrackType(m_nTargetTrack, trackType);
        m_pAnimController->SetTrackPosition(m_nTargetTrack, 0.0f);
        m_pAnimController->SetTrackWeight(m_nTargetTrack, 0.0f); // 블렌드 인 시작은 가중치 0
        m_pAnimController->SetTrackEnable(m_nTargetTrack, true); // 트랙 활성화

        // 이전 애니메이션 계속 활성화 상태 유지 (블렌드 아웃)
        m_pAnimController->SetTrackWeight(m_nSourceTrack, 1.0f);
        m_pAnimController->SetTrackEnable(m_nSourceTrack, true);

        // m_pCurrentState 포인터는 블렌딩 완료 시 변경
        m_eCurrentStateID = PlayerStateID::None; // 전환 중임을 표시 (선택적)


    }
    else { // 즉시 전환 (초기 상태 설정 또는 강제 전환)
        m_bIsBlending = false;
        m_fBlendTimer = 0.0f;

        // 현재 상태 포인터 및 ID 즉시 변경
        m_pCurrentState = pNewState;
        m_eCurrentStateID = newStateID;

        // 새 상태 진입 처리
        m_pCurrentState->Enter(m_pOwner, this);

        // 주 트랙(m_nSourceTrack)에 애니메이션 설정 및 활성화
        int animIndex = AnimIndices::IDLE;
        int trackType = ANIMATION_TYPE_LOOP; // 기본값은 루프

        switch (newStateID) { // 목표 상태에 맞는 애니메이션 인덱스 찾기
        case PlayerStateID::Idle:
            animIndex = AnimIndices::IDLE;
            trackType = ANIMATION_TYPE_LOOP;
            break;

        case PlayerStateID::WalkForward:
            animIndex = AnimIndices::WALK_FORWARD;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::WalkBackward:
            animIndex = AnimIndices::WALK_BACKWARD;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::WalkLeft:
            animIndex = AnimIndices::WALK_LEFT;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::WalkRight:
            animIndex = AnimIndices::WALK_RIGHT;
            trackType = ANIMATION_TYPE_LOOP;
            break;

        case PlayerStateID::AttackMelee:
            animIndex = AnimIndices::ATTACK_MELEE1;
            trackType = ANIMATION_TYPE_ONCE;
            break;
        case PlayerStateID::AttackAxe:
            animIndex = AnimIndices::ATTACK_AXE;
            trackType = ANIMATION_TYPE_ONCE;
            break;
        case PlayerStateID::AttackPick:
            animIndex = AnimIndices::ATTACK_PICK;
            trackType = ANIMATION_TYPE_ONCE;
            break;

        case PlayerStateID::JumpStart:
            animIndex = AnimIndices::JUMP_START;
            trackType = ANIMATION_TYPE_ONCE;
            break;
        case PlayerStateID::JumpLoop:
            animIndex = AnimIndices::JUMP_LOOP;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::JumpEnd:
            animIndex = AnimIndices::JUMP_END;
            trackType = ANIMATION_TYPE_ONCE;
            break;

        case PlayerStateID::RunForward:
            animIndex = AnimIndices::RUN_FORWARD;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::RunBackward:
            animIndex = AnimIndices::RUN_BACKWARD;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::RunLeft:
            animIndex = AnimIndices::RUN_LEFT;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::RunRight:
            animIndex = AnimIndices::RUN_RIGHT;
            trackType = ANIMATION_TYPE_LOOP;
            break;
            // ... 다른 상태들에 대한 애니메이션 인덱스 매핑 추가 ...
        default: OutputDebugString(L"Warning: No animation index mapped for state ");
        }
        // 새 애니메이션
        m_pAnimController->SetTrackAnimationSet(m_nSourceTrack, animIndex);
        m_pAnimController->SetTrackType(m_nSourceTrack, trackType);
        m_pAnimController->SetTrackPosition(m_nSourceTrack, 0.0f);
        m_pAnimController->SetTrackWeight(m_nSourceTrack, 1.0f); // 즉시 전환
        m_pAnimController->SetTrackEnable(m_nSourceTrack, true);

        // 이전 애니메이션 비활성화
        m_pAnimController->SetTrackWeight(m_nTargetTrack, 0.0f);
        m_pAnimController->SetTrackEnable(m_nTargetTrack, false);

        std::cout << "Immediate change complete. Current state: " << static_cast<int>(m_eCurrentStateID) << std::endl;
    }
}

void PlayerStateMachine::UpdateBlend(float deltaTime) {
    if (!m_bIsBlending || !m_pAnimController) return;

    m_fBlendTimer += deltaTime;
    float blendFactor = std::min(m_fBlendTimer / m_fBlendDuration, 1.0f);

    // 가중치 업데이트
    m_pAnimController->SetTrackWeight(m_nSourceTrack, 1.0f - blendFactor);
    m_pAnimController->SetTrackWeight(m_nTargetTrack, blendFactor);

    // 블렌딩 완료 체크
    if (blendFactor >= 1.0f) {
        m_bIsBlending = false;
        m_fBlendTimer = 0.0f;

        // 소스 트랙 비활성화
        m_pAnimController->SetTrackEnable(m_nSourceTrack, false);

        // 현재 상태 포인터 및 ID 업데이트
        m_pCurrentState = GetState(m_eTargetStateID);
        m_eCurrentStateID = m_eTargetStateID;
        m_eTargetStateID = PlayerStateID::None; // 목표 상태 초기화

        // 이제 Target 트랙이 주 트랙이 되었으므로, 다음 전환을 위해 인덱스 교체는 이미 PerformStateChange에서 처리됨.

        std::cout << "Blend complete. Current state: " << static_cast<int>(m_eCurrentStateID) << std::endl;
    }
}


float PlayerStateMachine::GetTrackPosition(int trackIndex) const {
    if (m_pAnimController && trackIndex >= 0 && trackIndex < m_pAnimController->m_nAnimationTracks) {
        return m_pAnimController->m_pAnimationTracks[trackIndex].m_fPosition;
    }
    return 0.0f;
}

float PlayerStateMachine::GetAnimationLength(int trackIndex) const {
    if (m_pAnimController && trackIndex >= 0 && trackIndex < m_pAnimController->m_nAnimationTracks) {
        int animSetIndex = m_pAnimController->m_pAnimationTracks[trackIndex].m_nAnimationSet;
        if (m_pAnimController->m_pAnimationSets && animSetIndex >= 0 && animSetIndex < m_pAnimController->m_pAnimationSets->m_nAnimationSets) {
            CAnimationSet* pSet = m_pAnimController->m_pAnimationSets->m_pAnimationSets[animSetIndex];
            if (pSet) {
                return pSet->m_fLength;
            }
        }
    }
    return 0.0f;
}



#include "Scene.h" // CScene 클래스 접근 위해 포함
#include "Object.h" // CGameObject, CMonsterObject 등 접근 위해 포함
#include "GameFramework.h"

PlayerStateID PlayerStateMachine::DetermineAttackState() {
    if (!m_pOwner || !m_pOwner->m_pGameFramework) {
        return PlayerStateID::AttackMelee; // 기본 공격 (오류 상황)
    }
    CScene* pScene = m_pOwner->m_pGameFramework->GetScene();
    if (!pScene) {
        return PlayerStateID::AttackMelee; // 기본 공격 (씬 없음)
    }

    CGameObject* targetObject = nullptr;
    float interactionRangeSq = 100.0f * 100.0f; // 예시: 상호작용/공격 범위 (반경 100 유닛) - 조정 필요

    // 1. 일반 객체들 (m_vGameObjects) 검사 (최적화 필요!)
    for (auto& pOtherObject : pScene->m_vGameObjects) {
        if (!pOtherObject || !pOtherObject->isRender) continue;

        // 거리 체크
        //float distSq = Vector3::LengthSq(Vector3::Subtract(pOtherObject->GetPosition(), m_pOwner->GetPosition()));
        //if (distSq > interactionRangeSq) continue;

        // 충돌 체크 (OBB 사용)
        if (m_pOwner->m_worldOBB.Intersects(pOtherObject->m_worldOBB)) {
            // 타입별 처리
            switch (pOtherObject->m_objectType) { // m_eObjectType 멤버가 있다고 가정
            case GameObjectType::Tree:
                return PlayerStateID::AttackAxe; // 나무 발견 시 즉시 반환
            case GameObjectType::Rock:
                return PlayerStateID::AttackPick; // 바위 발견 시 즉시 반환
            case GameObjectType::Cow:
            case GameObjectType::Pig:
                // 살아있는 몬스터인지 확인 (HP > 0)
                return PlayerStateID::AttackMelee;
            //{
            //    auto npc = dynamic_cast<CMonsterObject*>(pOtherObject);
            //    if (npc && npc->Gethp() > 0) {
            //        return PlayerStateID::AttackMelee; // 몬스터 발견 시 즉시 반환
            //    }
            //}
            break;
            // 다른 공격 가능한 몬스터 타입들 추가...
            default:
                // 공격 불가능하거나 특별한 처리 없는 타입은 일단 무시하고 계속 탐색
                break;
            }
            // 만약 여러 타입이 겹쳐있을 때 우선순위를 정하려면 여기서 break하지 않고 계속 탐색하며 가장 중요한 대상을 저장
        }
    }

    // 특별히 상호작용할 대상을 찾지 못했으면 기본 공격 상태 반환
    return PlayerStateID::AttackMelee;
}