// PlayerStateMachine.cpp
#include "stdafx.h" // �ʿ�� ������Ʈ�� stdafx.h ����
#include "PlayerStateMachine.h"
#include "Player.h" // CTerrainPlayer ���� ����
#include "Animation.h" // CAnimationController ���� ����
#include <iostream>   // ����� ��¿� (���� ����)
#include <algorithm>  // std::min, std::max �� ���
#include "GameFramework.h"
#include "Object.h"

// --- ��ü���� ���� Ŭ���� ���� ---

#define WalkSpeed 50
#define RunSpeed 80

void PlayWavSound(const _TCHAR* soundPath)
{
    PlaySound(soundPath, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
}



class IdleState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::Idle; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput(); // �Է� ��������

        // �̵� �Է� üũ
        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.WalkLeft) return PlayerStateID::WalkLeft;
        if (input.WalkRight) return PlayerStateID::WalkRight;
        // ��Ÿ �Է� (����, ���� ��)
        if (input.Attack) return stateMachine->DetermineAttackState();
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { 
        }

        // �ٸ� �Է��� ������ Idle ���� ����
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
        // �ִϸ��̼� ������ PlayerStateMachine::PerformStateChange ���� ó��
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
        // WalkForward ���¶� �ٸ� Ű �Է��� Ȯ���Ͽ� ���Ϳ� ����
        if (input.MoveBackward) { // �յ� ���� �Է� �� ��� �Ǵ� �ٸ� ó�� �ʿ� �� �߰�
            moveVector = Vector3::Add(moveVector, look, -1.0f);
            isMoving = true; // �ڷ� ���� Ű�� ������
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
            moveVector = Vector3::Normalize(moveVector); // �밢�� �̵� �� �ӵ� ���� ���� ����ȭ
            XMFLOAT3 currentVel = player->GetVelocity(); // ���� Y �ӵ� ����
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, WalkSpeed, false);
            targetVel.y = currentVel.y; // Y �ӵ��� ����
            player->SetVelocity(targetVel); // ���� �ӵ� ����
        }
        else {
            // ��� �̵� Ű�� ���������� Idle ���·� ��ȯ
            return PlayerStateID::Idle;
        }
        // �Է¿� ���� ���� ��ȯ üũ
        if (input.Run && input.MoveForward && !input.WalkLeft && !input.WalkRight && !input.MoveBackward) {
            return PlayerStateID::RunForward;
        }
        if (input.Attack) return stateMachine->DetermineAttackState();
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }


        // ���� ���� ���� ���� ���� (��: � �̵�Ű�� ���� ������ �ش� �� ���� ���� ����)
        if (input.MoveForward) return PlayerStateID::WalkForward; // �� ���� �켱
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.WalkLeft) return PlayerStateID::WalkLeft;
        if (input.WalkRight) return PlayerStateID::WalkRight;

        // ������� ���� �ȵ�����, ������ ���� Idle ��ȯ
        return PlayerStateID::Idle;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // �ʿ��ϴٸ� �ӵ� �ʱ�ȭ ��
        // player->SetVelocity({0.0f, player->GetVelocity().y, 0.0f}); // Y�� �ӵ� �����ϸ� XZ�� 0����
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
        if (input.WalkLeft) { // PlayerInputData ��� �̸��� �°� ���� (WalkLeft -> StrafeLeft)
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.WalkRight) { // PlayerInputData ��� �̸��� �°� ���� (WalkRight -> StrafeRight)
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector); // �밢�� �̵� �ӵ� ����
            XMFLOAT3 currentVel = player->GetVelocity();
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, WalkSpeed, false);
            targetVel.y = currentVel.y;
            player->SetVelocity(targetVel);
        }
        else {
            return PlayerStateID::Idle;
        }

        if (input.Run && input.MoveBackward && !input.MoveForward && !input.WalkLeft && !input.WalkRight) {
            return PlayerStateID::RunBackward; // RunBackward ���·� ��ȯ ��û
        }

        if (input.Attack) return stateMachine->DetermineAttackState();
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }

        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.WalkLeft) return PlayerStateID::WalkLeft;
        if (input.WalkRight) return PlayerStateID::WalkRight;

        // ������� ���� �� ������, ������ ���� Idle ��ȯ
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

        if (input.Attack) return stateMachine->DetermineAttackState();
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }

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

        // --- �ӵ� ���� ---
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

        if (input.Attack) return stateMachine->DetermineAttackState();
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }

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

        // �޸��� ����
        XMFLOAT3 look = player->GetLookVector();
        XMFLOAT3 currentVel = player->GetVelocity();
        XMFLOAT3 targetVel = Vector3::ScalarProduct(look, RunSpeed, false);
        targetVel.y = currentVel.y;
        player->SetVelocity(targetVel);

        // �ٸ� ���·� ��ȯ üũ
        if (input.Attack) return stateMachine->DetermineAttackState();
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }

        return PlayerStateID::RunForward;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }
};
class RunBackwardState : public IPlayerState {
public:
    // PlayerStateID::RunBackward ID �� ���ǵǾ��ٰ� ����
    PlayerStateID GetID() const override { return PlayerStateID::RunBackward; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Entering RunBackward State\n";
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput();

        // --- �޸��� �ߴ�/���� ��ȯ ���� Ȯ�� ---
        if (!input.Run || !input.MoveBackward || input.MoveForward || input.WalkLeft || input.WalkRight) {
            // �޸��� Ű�� �ðų�, �ڷΰ��� Ű�� �ðų�, �ٸ� ����Ű�� ������ ���� ��ȯ
            if (input.MoveBackward) return PlayerStateID::WalkBackward; // �޸���� ���߰� �ڷ� �ȱ�
            if (input.MoveForward) return PlayerStateID::WalkForward; // ������ �ȱ�
            if (input.WalkLeft) return PlayerStateID::WalkLeft;   // ���� �ȱ�
            if (input.WalkRight) return PlayerStateID::WalkRight;  // ������ �ȱ�
            return PlayerStateID::Idle; // ��� �̵� Ű ������ Idle
        }

        // --- �ڷ� �޸��� �ӵ� ���� ---
        XMFLOAT3 look = player->GetLookVector();
        XMFLOAT3 currentVel = player->GetVelocity();
        XMFLOAT3 targetVel = Vector3::ScalarProduct(look, -RunSpeed, false); // �ڷ� �޸���
        targetVel.y = currentVel.y; // Y �ӵ� ����
        player->SetVelocity(targetVel);

        // --- �ٸ� ���·� ��ȯ üũ ---
        if (input.Attack) return stateMachine->DetermineAttackState();
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }

        return PlayerStateID::RunBackward; // �ڷ� �޸��� ���� ����
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Exiting RunBackward State\n";
    }
};
class RunLeftState : public IPlayerState {
public:
    // PlayerStateID::RunLeft ID �� ���ǵǾ��ٰ� ���� (�Ǵ� PlayerStateID::WalkLeft ��� �� ���� �ʿ�)
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
        XMFLOAT3 targetVel = Vector3::ScalarProduct(right, -RunSpeed, false); // �������� �޸���
        targetVel.y = currentVel.y;
        player->SetVelocity(targetVel);

        if (input.Attack) return stateMachine->DetermineAttackState();
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }

        return PlayerStateID::RunLeft; // ���� �޸��� ���� ����
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Exiting RunLeft State\n";
    }
};
class RunRightState : public IPlayerState {
public:
    // PlayerStateID::RunRight ID �� ���ǵǾ��ٰ� ����
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
        XMFLOAT3 targetVel = Vector3::ScalarProduct(right, RunSpeed, false); // ���������� �޸���
        targetVel.y = currentVel.y;
        player->SetVelocity(targetVel);

        // �ٸ� ���·� ��ȯ üũ
        if (input.Attack) return stateMachine->DetermineAttackState();
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }

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
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK; // ���� �ִϸ��̼��� ����� Ʈ��
    bool m_bHasAppliedHit = false; 

public:
    PlayerStateID GetID() const override { return PlayerStateID::AttackMelee; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAttackFinished = false;
        m_bHasAppliedHit = false;
        player->SetVelocity({ 0.0f, player->GetVelocity().y, 0.0f });
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
        PlayWavSound(_T("Sound/sword.wav"));
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        if (!m_bHasAppliedHit && currentPosition >= animLength * 0.66f) {
            m_bHasAppliedHit = true; // ������ �� ��

            std::vector<CGameObject*> hitObjects = player->FindObjectHitByAttack();

            if (!hitObjects.empty()) {
                for (CGameObject* hitObject : hitObjects) {
                    CollisionUpdate(player, hitObject);
                }
            }
        }

        if (!m_bAttackFinished && currentPosition >= animLength * 0.95f) { 
            m_bAttackFinished = true;
        }

        // �ִϸ��̼��� ������ Idle ���·� ��ȯ
        if (m_bAttackFinished) {
            return PlayerStateID::Idle;
        }

        // ���� �߿��� �ٸ� �Է� �����ϰ� ���� ���� ����
        return PlayerStateID::AttackMelee;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }
};
class AttackAxeState : public IPlayerState {
private:
    bool m_bAttackFinished = false;
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
    bool m_bHasAppliedHit = false; 
public:
    PlayerStateID GetID() const override { return PlayerStateID::AttackAxe; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAttackFinished = false;
        m_bHasAppliedHit = false;
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
        player->SetVelocity({ 0.0f, player->GetVelocity().y, 0.0f }); 
        PlayWavSound(_T("Sound/axe.wav"));
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        if (!m_bHasAppliedHit && currentPosition >= animLength * 0.66f) {

            std::vector<CGameObject*> hitObjects = player->FindObjectHitByAttack();
            m_bHasAppliedHit = true; 

            if (!hitObjects.empty()) {
                for (CGameObject* hitObject : hitObjects) {
                    CollisionUpdate(player, hitObject);
                }
            }
        }

        if (!m_bAttackFinished && currentPosition >= animLength * 0.95f) {
            m_bAttackFinished = true;
        }

        // �ִϸ��̼��� ������ Idle ���·� ��ȯ
        if (m_bAttackFinished) {
            return PlayerStateID::Idle;
        }

        // ���� �߿��� �ٸ� �Է� �����ϰ� ���� ���� ����
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
    bool m_bHasAppliedHit = false;
public:
    PlayerStateID GetID() const override { return PlayerStateID::AttackPick; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAttackFinished = false;
        m_bHasAppliedHit = false;
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK; 
        player->SetVelocity({ 0.0f, player->GetVelocity().y, 0.0f }); 
        PlayWavSound(_T("Sound/pickaxe.wav"));
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        if (!m_bHasAppliedHit && currentPosition >= animLength * 0.66f) {
            m_bHasAppliedHit = true; // ������ �� ��

            std::vector<CGameObject*> hitObjects = player->FindObjectHitByAttack();

            if (!hitObjects.empty()) {
                for (CGameObject* hitObject : hitObjects) {
                    CollisionUpdate(player, hitObject);
                }
            }
        }

        if (!m_bAttackFinished && currentPosition >= animLength * 0.95f) {
            m_bAttackFinished = true;
        }

        // �ִϸ��̼��� ������ Idle ���·� ��ȯ
        if (m_bAttackFinished) {
            return PlayerStateID::Idle;
        }

        // ���� �߿��� �ٸ� �Է� �����ϰ� ���� ���� ����
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
    float m_fJumpVelocity = 30.0f; // ���� �� ���� �ӵ� (���� �ʿ�)

public:
    PlayerStateID GetID() const override { return PlayerStateID::JumpStart; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAnimFinished = false;
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK; // �� Ʈ�� ��� ����

        // �������� �ӵ� ����
        XMFLOAT3 currentVel = player->GetVelocity();
        currentVel.y = m_fJumpVelocity;
        player->SetVelocity(currentVel);
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        // ���� ���� �ִϸ��̼� ��� �Ϸ� Ȯ��
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        if (!m_bAnimFinished && currentPosition >= animLength * 0.99f) {
            m_bAnimFinished = true;
        }

        // �ִϸ��̼� ������ JumpLoop ���·� ��ȯ
        if (m_bAnimFinished) {
            return PlayerStateID::JumpLoop;
        }

        return PlayerStateID::JumpStart; // �ִϸ��̼� ��� �߿��� ���� ����
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }
};
class JumpLoopState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::JumpLoop; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // ���� �ִϸ��̼� ���� (PerformStateChange���� ó����)
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        // ���߿��� �¿� �̵� �� ó�� (������)
        const auto& input = player->GetStateMachineInput();
        float airControlFactor = 0.1f; // ���� ����� (���� �ʿ�)
        XMFLOAT3 currentVel = player->GetVelocity();
        XMFLOAT3 right = player->GetRightVector();

        if (input.WalkLeft) {
            currentVel = Vector3::Add(currentVel, right, -50.0f * airControlFactor);
        }
        if (input.WalkRight) {
            currentVel = Vector3::Add(currentVel, right, 50.0f * airControlFactor);
        }
        // �ʿ��ϴٸ� �յ� �̵��� �ణ ���
        player->SetVelocity(currentVel);


        // ���� ���� Ȯ�� (��: �÷��̾� �߹� ���� üũ �Ǵ� Y �ӵ� üũ)
        // ���⼭�� �ܼ��ϰ� Y �ӵ��� ������ �Ǹ� ���� �õ��Ѵٰ� ����
        if (player->GetVelocity().y <= 0.0f) {
            // �����δ� ����ĳ��Ʈ ������ �ٴ� �浹 Ȯ�� �ʿ�
            // CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)player->GetPlayerUpdatedContext();
            // if (pTerrain && player->GetPosition().y <= pTerrain->GetHeight(player->GetPosition().x, player->GetPosition().z) + 1.0f) { // �� ��ó 1 ���� �̳�
            return PlayerStateID::JumpEnd;
            // }
        }

        return PlayerStateID::JumpLoop; // ���߿����� ��� Loop ����
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
        // ���� �ִϸ��̼� ���� (PerformStateChange���� ó����)

        // ���� �� Y �ӵ� 0���� (������, ������ �浹 ó������ �̹� �� ���� ����)
        // XMFLOAT3 currentVel = player->GetVelocity();
        // currentVel.y = 0.0f;
        // player->SetVelocity(currentVel);
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        // ���� �ִϸ��̼� ��� �Ϸ� Ȯ��
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        if (!m_bAnimFinished && currentPosition >= animLength * 0.99f) {
            m_bAnimFinished = true;
        }

        // �ִϸ��̼� ������ Idle ���·� ��ȯ
        if (m_bAnimFinished) {
            return PlayerStateID::Idle;
        }

        return PlayerStateID::JumpEnd; // �ִϸ��̼� ��� �߿��� ���� ����
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
    }
};

class HitReactionState : public IPlayerState {
private:
    bool m_bAnimFinished = false;
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK; // �ǰ� �ִϸ��̼� ��� Ʈ�� (��� ��ȯ �� �� Ʈ��)
    float m_fStateDuration = 0.0f; // ���� ���ӽð� (�ִϸ��̼� ���̿� ���� ����)
    float m_fElapsedTimeInState = 0.0f; // �� ���¿� �ӹ� �ð�

public:
    PlayerStateID GetID() const override { return PlayerStateID::HitReaction; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Entering HitReaction State\n";
        m_bAnimFinished = false;
        m_fElapsedTimeInState = 0.0f;
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK; // ��� ��ȯ�̹Ƿ� �� Ʈ�� ���

        // �ִϸ��̼� ���̸� �����ͼ� ���� ���ӽð����� ��� (�Ǵ� ������ ���)
        m_fStateDuration = stateMachine->GetAnimationLength(m_nAnimTrack);
        if (m_fStateDuration <= 0.0f) { // �ִϸ��̼� ���� ���������� �⺻ ���ӽð�
            m_fStateDuration = 0.5f; // ��: 0.5��
        }


        // �ǰ� �� �̵� �ӵ� 0����
        player->SetVelocity({0.0f, player->GetVelocity().y, 0.0f}); // Y�� �ӵ��� ������ �� ���� (���� �ǰ� ��)
        // �Ǵ� ���� ª�� �˹� ȿ���� �� ���� �ֽ��ϴ�.

    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        m_fElapsedTimeInState += deltaTime;

        // �ִϸ��̼� ��� �Ϸ� Ȯ�� (�ð� ��� �Ǵ� GetTrackPosition ���)
        // GetTrackPosition �� ANIMATION_TYPE_ONCE ���� �Ŀ��� ��� �������� �ʴ´ٴ� ������ �ִٸ� ��� ����
        // ���⼭�� �ð� ������� ���� ó�� (�� �������� �� ����)
        if (!m_bAnimFinished && m_fElapsedTimeInState >= m_fStateDuration*0.45f) {
                m_bAnimFinished = true;
        }

        // �Ǵ� Ʈ�� ��ġ ��� (�ּ� ó���� �κ��� ���� ���)
        
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        // animLength�� Enter���� m_fStateDuration ���� �̹� ���������Ƿ� ��Ȱ�� ����
        //if (!m_bAnimFinished && currentPosition >= m_fStateDuration * 0.90f) {
        //    m_bAnimFinished = true;
        //}
        

        // TODO: �÷��̾� HP üũ�Ͽ� 0 ���ϸ� Dead ���·� ��ȯ�ϴ� ���� �߰�
        // if (player->GetCurrentHP() <= 0) { // �÷��̾� HP �������� �Լ� �ʿ�
        //     return PlayerStateID::Dead;
        // }

        // �ִϸ��̼��� ������ Idle ���·� ��ȯ
        if (m_bAnimFinished) {
            return PlayerStateID::Idle;
        }

        // �ǰ� �ִϸ��̼� �߿��� �ٸ� �Է� �����ϰ� ���� ���� ����
        return PlayerStateID::HitReaction;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Exiting HitReaction State\n";
    }
};


#include "NetworkManager.h"

struct ToolStats
{
    int damageVsTree = 1;
    int damageVsRock = 1;
    int damageVsNpc = 1;
};

// ���� �̸�(m_equippedToolName)�� Ű�� ����ϴ� ������ ���� ���̺�
const std::map<std::string, ToolStats> G_TOOL_STATS_TABLE = {
    //               { Tree, Rock, NPC }
    { "Sword_Wood",   { 2,    2,    5  } },
    { "Sword_Stone",  { 3,    3,    8  } },
    { "Sword_Metal",  { 4,    4,    12 } },

    { "Axe_Wood",     { 8,    3,    4  } },
    { "Axe_Stone",    { 12,   4,    6  } },
    { "Axe_Metal",    { 18,   5,    9  } },

    { "Pickaxe_Wood", { 3,    8,    3  } },
    { "Pickaxe_Stone",{ 4,    12,   4  } },
    { "Pickaxe_Metal",{ 5,    18,   6  } },

    { "Hammer_Wood",  { 5,    5,    5  } },
    { "Hammer_Stone", { 6,    6,    8  } },
    { "Hammer_Metal", { 7,    7,    10 } }
};

void IPlayerState::CollisionUpdate(CTerrainPlayer* player, CGameObject* hitObject)
{
    if (!hitObject) return;

    const std::string& equippedToolName = player->m_equippedToolName;

    ToolStats currentToolStats; // �⺻(�Ǽ�) ������
    if (G_TOOL_STATS_TABLE.count(equippedToolName)) {
        currentToolStats = G_TOOL_STATS_TABLE.at(equippedToolName);
    }

    if (hitObject->m_objectType == GameObjectType::Tree) {
        auto tree = dynamic_cast<CTreeObject*>(hitObject);
        if (tree && !tree->IsFalling() && !tree->HasFallen()) {
            int hp = tree->getHp();
            if (hp > 0) {
                hp -= currentToolStats.damageVsTree; 
                tree->setHp(hp);
            }
            if (hp <= 0) {
                tree->StartFalling(player->GetLookVector()); // �÷��̾ �ٶ󺸴� �������� ����������
            }
#ifdef ONLINE
            auto& nwManager = NetworkManager::GetInstance();
            OBJ_HP_PACKET p;
            p.oid = tree->m_id;
            p.hp = tree->getHp();
            nwManager.PushSendQueue(p, p.size);
#endif
        }
    }
    else if (hitObject->m_objectType == GameObjectType::Rock) {
        auto rock = dynamic_cast<CRockObject*>(hitObject);
        if (rock) {
            int hp = rock->getHp();
            if (hp > 0) {
                if (hp > 0) {
                    hp -= currentToolStats.damageVsRock;
                    rock->setHp(hp);
                    rock->SetScale(0.9f, 0.9f, 0.9f);
                    player->m_pGameFramework->AddItem("stone", 1);
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
                rock->isRender = false;
                //rock->EraseRock();
                //player->m_pGameFramework->AddItem("rock", 5); // ����: �����߸��� ���� ȹ��
            }
#ifdef ONLINE
            auto& nwManager = NetworkManager::GetInstance();
            OBJ_HP_PACKET p;
            p.oid = rock->m_id;
            p.hp = rock->getHp();
            nwManager.PushSendQueue(p, p.size);
#endif
        }
    }
    else if (hitObject->m_objectType == GameObjectType::Cow || hitObject->m_objectType == GameObjectType::Pig) {
        auto npc = dynamic_cast<CMonsterObject*>(hitObject);
        if (npc->_invincible) return;
        if (npc->Gethp() <= 0) return;
        if (npc->FSM_manager) if (npc->FSM_manager->GetInvincible()) return;

        auto& nwManager = NetworkManager::GetInstance();
        OBJ_HIT_PACKET packet;
        packet.oid = npc->m_id;

        int damage = currentToolStats.damageVsNpc; 
        npc->Decreasehp(damage);
        packet.damage = damage;
        nwManager.PushSendQueue(packet, packet.size);

        npc->SetInvincible(true); // set invincible
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
        if (npc->_invincible) return;
        if (npc->Gethp() <= 0) return;
        if (npc->FSM_manager) if (npc->FSM_manager->GetInvincible()) return;

        auto& nwManager = NetworkManager::GetInstance();
        OBJ_HIT_PACKET packet;
        packet.oid = npc->m_id;

        int damage = currentToolStats.damageVsNpc; 
        npc->Decreasehp(damage);
        packet.damage = damage;

        nwManager.PushSendQueue(packet, packet.size);

        npc->SetInvincible(true); // set invincible
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

// --- PlayerStateMachine Ŭ���� ���� ---

PlayerStateMachine::PlayerStateMachine(CTerrainPlayer* owner, CAnimationController* animCtrl)
    : m_pOwner(owner), m_pAnimController(animCtrl), m_eCurrentStateID(PlayerStateID::None), m_pCurrentState(nullptr) {
    if (!m_pOwner || !m_pAnimController) {
        // �ʼ� ��ü ���� ���� ó��
        throw std::runtime_error("PlayerStateMachine requires a valid owner and animation controller!");
    }
    if (m_pAnimController->m_nAnimationTracks < 2) {
        throw std::runtime_error("PlayerStateMachine requires CAnimationController with at least 2 tracks for blending!");
    }

    // ��� ���� ��ü ���� �� ���Ϳ� �߰�
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

    m_vStates.push_back(std::make_unique<HitReactionState>());
    // ... �ٸ� ��� ���µ鵵 ���⿡ �߰� ...

    // �ʱ� ���� ����
    PerformStateChange(PlayerStateID::Idle, true);
}

PlayerStateMachine::~PlayerStateMachine() {
    // unique_ptr �� �ڵ����� �޸� ����
}

void PlayerStateMachine::Update(float deltaTime) {
    if (!m_pCurrentState) return;

    if (m_bIsBlending) {
        UpdateBlend(deltaTime);
    }
    else {
        // ���� ���� ������Ʈ �� ���� ���� ID �ޱ�
        PlayerStateID nextStateID = m_pCurrentState->Update(m_pOwner, this, deltaTime);

        // ���� ��ȯ ��û�� ������ ó��
        if (nextStateID != m_eCurrentStateID) {
            PerformStateChange(nextStateID); // �⺻������ ���� �õ�
        }
    }
}

void PlayerStateMachine::HandleInput(const PlayerInputData& input) {
    m_LastInput = input; // ���� �Է� ���� ����
}

PlayerStateID PlayerStateMachine::GetCurrentStateID() const {
    return m_eCurrentStateID;
}

IPlayerState* PlayerStateMachine::GetState(PlayerStateID id) {
    for (const auto& state : m_vStates) {
        if (state->GetID() == id) {
            return state.get(); // unique_ptr���� �����ϴ� raw ������ ��ȯ
        }
    }
    return nullptr; // �ش� ID�� ���¸� ã�� ����
}


void PlayerStateMachine::PerformStateChange(PlayerStateID newStateID, bool forceImmediate) {
    
    // ���� ����
    forceImmediate = true;

    if (newStateID == m_eCurrentStateID || !m_pAnimController) return; // ���� ���°ų� ��Ʈ�ѷ� ������ ����

    IPlayerState* pNewState = GetState(newStateID);
    if (!pNewState) {
        std::cerr << "Error: Cannot find state with ID: " << static_cast<int>(newStateID) << std::endl;
        return;
    }

    // ���� ���� ���� ó��
    if (m_pCurrentState) {
        m_pCurrentState->Exit(m_pOwner, this);
    }

    // --- ���� �Ǵ� ��� ��ȯ ó�� ---
    if (!forceImmediate && m_pCurrentState) { // ���� ���°� �ְ�, ��� ��ȯ�� �ƴϸ� ���� ����
        m_bIsBlending = true;
        m_fBlendTimer = 0.0f;
        m_eTargetStateID = newStateID; // ��ǥ ���� ����

        // ���� ������ ���� Ʈ�� �ε��� ��ü
        std::swap(m_nSourceTrack, m_nTargetTrack);

        // ��ǥ ���� ����
        pNewState->Enter(m_pOwner, this);

        // m_nTargetTrack�� �� �ִϸ��̼� ����
        int animIndex = AnimIndices::IDLE; 
        int trackType = ANIMATION_TYPE_LOOP;

        const auto& input = m_pOwner->GetStateMachineInput();

        switch (newStateID) { // ��ǥ ���¿� �´� �ִϸ��̼� �ε��� ã��
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


        case PlayerStateID::HitReaction:
            animIndex = AnimIndices::HIT_REACTION;
            trackType = ANIMATION_TYPE_ONCE; // �ǰ� �ִϸ��̼��� �� �� ���
            break;
            // ... �ٸ� ���µ鿡 ���� �ִϸ��̼� �ε��� ���� �߰� ...
        default: std::cerr << "Warning: No animation index mapped for state " << (int)newStateID << std::endl; break;
        }

        // �� �ִϸ��̼� ����: ����ġ 0
        m_pAnimController->SetTrackAnimationSet(m_nTargetTrack, animIndex);
        m_pAnimController->SetTrackType(m_nTargetTrack, trackType);
        m_pAnimController->SetTrackPosition(m_nTargetTrack, 0.0f);
        m_pAnimController->SetTrackWeight(m_nTargetTrack, 0.0f); // ���� �� ������ ����ġ 0
        m_pAnimController->SetTrackEnable(m_nTargetTrack, true); // Ʈ�� Ȱ��ȭ

        // ���� �ִϸ��̼� ��� Ȱ��ȭ ���� ���� (���� �ƿ�)
        m_pAnimController->SetTrackWeight(m_nSourceTrack, 1.0f);
        m_pAnimController->SetTrackEnable(m_nSourceTrack, true);

        // m_pCurrentState �����ʹ� ���� �Ϸ� �� ����
        m_eCurrentStateID = PlayerStateID::None; // ��ȯ ������ ǥ�� (������)


    }
    else { // ��� ��ȯ (�ʱ� ���� ���� �Ǵ� ���� ��ȯ)
        m_bIsBlending = false;
        m_fBlendTimer = 0.0f;

        // ���� ���� ������ �� ID ��� ����
        m_pCurrentState = pNewState;
        m_eCurrentStateID = newStateID;

        // �� ���� ���� ó��
        m_pCurrentState->Enter(m_pOwner, this);

        // �� Ʈ��(m_nSourceTrack)�� �ִϸ��̼� ���� �� Ȱ��ȭ
        int animIndex = AnimIndices::IDLE;
        int trackType = ANIMATION_TYPE_LOOP; // �⺻���� ����

        switch (newStateID) { // ��ǥ ���¿� �´� �ִϸ��̼� �ε��� ã��
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


        case PlayerStateID::HitReaction:
            animIndex = AnimIndices::HIT_REACTION;
            trackType = ANIMATION_TYPE_ONCE; // �ǰ� �ִϸ��̼��� �� �� ���
            break;
            // ... �ٸ� ���µ鿡 ���� �ִϸ��̼� �ε��� ���� �߰� ...
        default: OutputDebugString(L"Warning: No animation index mapped for state ");
        }
        // �� �ִϸ��̼�
        m_pAnimController->SetTrackAnimationSet(m_nSourceTrack, animIndex);
        m_pAnimController->SetTrackType(m_nSourceTrack, trackType);
        m_pAnimController->SetTrackPosition(m_nSourceTrack, 0.0f);
        m_pAnimController->SetTrackWeight(m_nSourceTrack, 1.0f); // ��� ��ȯ
        m_pAnimController->SetTrackEnable(m_nSourceTrack, true);

        // ���� �ִϸ��̼� ��Ȱ��ȭ
        m_pAnimController->SetTrackWeight(m_nTargetTrack, 0.0f);
        m_pAnimController->SetTrackEnable(m_nTargetTrack, false);

        std::cout << "Immediate change complete. Current state: " << static_cast<int>(m_eCurrentStateID) << std::endl;
    }
}

void PlayerStateMachine::UpdateBlend(float deltaTime) {
    if (!m_bIsBlending || !m_pAnimController) return;

    m_fBlendTimer += deltaTime;
    float blendFactor = std::min(m_fBlendTimer / m_fBlendDuration, 1.0f);

    // ����ġ ������Ʈ
    m_pAnimController->SetTrackWeight(m_nSourceTrack, 1.0f - blendFactor);
    m_pAnimController->SetTrackWeight(m_nTargetTrack, blendFactor);

    // ���� �Ϸ� üũ
    if (blendFactor >= 1.0f) {
        m_bIsBlending = false;
        m_fBlendTimer = 0.0f;

        // �ҽ� Ʈ�� ��Ȱ��ȭ
        m_pAnimController->SetTrackEnable(m_nSourceTrack, false);

        // ���� ���� ������ �� ID ������Ʈ
        m_pCurrentState = GetState(m_eTargetStateID);
        m_eCurrentStateID = m_eTargetStateID;
        m_eTargetStateID = PlayerStateID::None; // ��ǥ ���� �ʱ�ȭ

        // ���� Target Ʈ���� �� Ʈ���� �Ǿ����Ƿ�, ���� ��ȯ�� ���� �ε��� ��ü�� �̹� PerformStateChange���� ó����.

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



#include "Scene.h" // CScene Ŭ���� ���� ���� ����
#include "Object.h" // CGameObject, CMonsterObject �� ���� ���� ����
#include "GameFramework.h"

PlayerStateID PlayerStateMachine::DetermineAttackState() {
    if (!m_pOwner) {
        return PlayerStateID::AttackMelee;
    }

    const std::string& equippedToolName = m_pOwner->m_equippedToolName;

    if (equippedToolName.empty()) {
        return PlayerStateID::AttackMelee;
    }

    // 3. ���� �̸��� ���Ե� Ű���带 �������� ���¸� �����մϴ�.
    if (equippedToolName.find("Sword") != std::string::npos) {
        return PlayerStateID::AttackMelee;
    }
    else if (equippedToolName.find("Axe") != std::string::npos) {
        return PlayerStateID::AttackAxe;
    }
    else if (equippedToolName.find("Pickaxe") != std::string::npos) {
        return PlayerStateID::AttackPick;
    }
    else if (equippedToolName.find("Hammer") != std::string::npos) {
        return PlayerStateID::AttackMelee; // ��ġ�� Į�� ���� ���� ���� ���
    }

    // ������ �ƴ� ������(���� ��)�� ��� ���� ��� �⺻ ����
    return PlayerStateID::AttackMelee;
}