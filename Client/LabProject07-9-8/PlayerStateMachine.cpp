// PlayerStateMachine.cpp
#include "stdafx.h" // �ʿ�� ������Ʈ�� stdafx.h ����
#include "PlayerStateMachine.h"
#include "Player.h" // CTerrainPlayer ���� ����
#include "Animation.h" // CAnimationController ���� ����
#include <iostream>   // ����� ��¿� (���� ����)
#include <algorithm>  // std::min, std::max �� ���

// --- ��ü���� ���� Ŭ���� ���� ---

class IdleState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::Idle; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Entering Idle State\n";
        // ���� �ӽ��� ���� �ִϸ��̼� ���� ��û (��� ����, ���� ���� ����)
        // ���� �ִϸ��̼� ������ PlayerStateMachine::PerformStateChange ���� ó����
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput(); // �Է� ��������

        // �̵� �Է� üũ
        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.MoveLeft) return PlayerStateID::MoveLeft;
        if (input.MoveRight) return PlayerStateID::MoveRight;
        // ��Ÿ �Է� (����, ���� ��)
        if (input.Attack) return PlayerStateID::AttackMelee1;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { // ��ȣ�ۿ� Ű �Է� ��
            // �浹 �˻�
            //if () {
            //    // return PlayerStateID::Interact;
            //}
        }

        // �ٸ� �Է��� ������ Idle ���� ����
        return PlayerStateID::Idle;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Exiting Idle State\n";
    }
};

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
        if (input.MoveLeft) {
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.MoveRight) {
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector); // �밢�� �̵� �� �ӵ� ���� ���� ����ȭ
            float currentSpeed = (input.Run && input.MoveForward) ? 80.0f : 50.0f; // �޸���/�ȱ� �ӵ� ���� (RunForward ���� ��ȯ�� �Ʒ�����)
            XMFLOAT3 currentVel = player->GetVelocity(); // ���� Y �ӵ� ����
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, currentSpeed, false);
            targetVel.y = currentVel.y; // Y �ӵ��� ����
            player->SetVelocity(targetVel); // ���� �ӵ� ����
        }
        else {
            // ��� �̵� Ű�� ���������� Idle ���·� ��ȯ
            return PlayerStateID::Idle;
        }
        // �Է¿� ���� ���� ��ȯ üũ
        if (input.Run && input.MoveForward && !input.MoveLeft && !input.MoveRight && !input.MoveBackward) {
            // �� + �޸��⸸ ������ �� RunForward ���·� (�ʿ��)
            // return PlayerStateID::RunForward;
        }
        if (input.Attack) return PlayerStateID::AttackMelee1;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }


        // ���� ���� ���� ���� ���� (��: � �̵�Ű�� ���� ������ �ش� �� ���� ���� ����)
        if (input.MoveForward) return PlayerStateID::WalkForward; // �� ���� �켱
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.MoveLeft) return PlayerStateID::MoveLeft;
        if (input.MoveRight) return PlayerStateID::MoveRight;

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
        if (input.MoveLeft) { // PlayerInputData ��� �̸��� �°� ���� (WalkLeft -> StrafeLeft)
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.MoveRight) { // PlayerInputData ��� �̸��� �°� ���� (WalkRight -> StrafeRight)
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector); // �밢�� �̵� �ӵ� ����
            // �޸��� Ű(Shift)�� ������ �� ���� ����ǵ��� ���� (������)
            float currentSpeed = (input.Run && input.MoveForward && !input.MoveBackward && !input.MoveLeft && !input.MoveRight) ? 80.0f : 50.0f; // ���� �ӵ�
            XMFLOAT3 currentVel = player->GetVelocity();
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, currentSpeed, false);
            targetVel.y = currentVel.y; // Y�� �ӵ� ����
            player->SetVelocity(targetVel);
        }
        else {
            // ��� �̵� Ű�� ���������� Idle ���·� ��ȯ
            return PlayerStateID::Idle;
        }

        if (input.Attack) return PlayerStateID::AttackMelee1;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }

        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.MoveLeft) return PlayerStateID::MoveLeft;
        if (input.MoveRight) return PlayerStateID::MoveRight;

        // ������� ���� �� ������, ������ ���� Idle ��ȯ
        return PlayerStateID::Idle;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // player->SetVelocity({0.0f, player->GetVelocity().y, 0.0f});
    }
};
class WalkLeftState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::MoveLeft; }

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
        if (input.MoveLeft) {
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.MoveRight) {
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector);
            float currentSpeed = (input.Run && input.MoveForward && !input.MoveBackward && !input.MoveLeft && !input.MoveRight) ? 80.0f : 50.0f;
            XMFLOAT3 currentVel = player->GetVelocity();
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, currentSpeed, false);
            targetVel.y = currentVel.y;
            player->SetVelocity(targetVel);
        }
        else {
            return PlayerStateID::Idle;
        }

        if (input.Attack) return PlayerStateID::AttackMelee1;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }

        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.MoveLeft) return PlayerStateID::MoveLeft;
        if (input.MoveRight) return PlayerStateID::MoveRight;

        return PlayerStateID::Idle;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // player->SetVelocity({0.0f, player->GetVelocity().y, 0.0f});
    }
};
class WalkRightState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::MoveRight; }

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
        if (input.MoveLeft) {
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.MoveRight) {
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        // --- �ӵ� ���� ---
        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector);
            float currentSpeed = (input.Run && input.MoveForward && !input.MoveBackward && !input.MoveLeft && !input.MoveRight) ? 80.0f : 50.0f;
            XMFLOAT3 currentVel = player->GetVelocity();
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, currentSpeed, false);
            targetVel.y = currentVel.y;
            player->SetVelocity(targetVel);
        }
        else {
            return PlayerStateID::Idle;
        }
            
        if (input.Attack) return PlayerStateID::AttackMelee1;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* ��ȣ�ۿ� üũ */ }

        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.MoveLeft) return PlayerStateID::MoveLeft;
        if (input.MoveRight) return PlayerStateID::MoveRight;

        return PlayerStateID::Idle;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // player->SetVelocity({0.0f, player->GetVelocity().y, 0.0f});
    }
};

class AttackMelee1State : public IPlayerState {
private:
    bool m_bAttackFinished = false;
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK; // ���� �ִϸ��̼��� ����� Ʈ��

public:
    PlayerStateID GetID() const override { return PlayerStateID::AttackMelee1; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAttackFinished = false;

        // ���� �ӽ��� ���� �ִϸ��̼� ���� ��û
        // PerformStateChange ���� ������ Ʈ�� ������ �̷����
        // (�� ���´� ���� ��� ��ȯ�� �� ���� - �ʿ�� forceImmediate ��� ���)

        // ���� �߿��� �÷��̾� �̵� �ӵ� 0���� ���� (������)
        player->SetVelocity({ 0.0f, player->GetVelocity().y, 0.0f });

        // Enter �������� ����� Ʈ�� ���� (���� ���Ķ�� target Ʈ���� �� ����)
        // m_nAnimTrack = stateMachine->GetCurrentAnimationTrack(); // �̷� �Լ��� �ʿ��Ҽ��� ����
        // ���⼭�� �ϴ� �� Ʈ�� ��� ����
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        // �ִϸ��̼� ��� �Ϸ� Ȯ��
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        // CAnimationTrack�� m_nType �� ANIMATION_TYPE_ONCE �� �����Ǿ��ٰ� ����
        // ��� ��ġ�� ���̸� �Ѿ�� �Ϸ�� ������ ���� (�ణ�� ���� ����)
        if (!m_bAttackFinished && currentPosition >= animLength * 0.99f) { // ���� ���� �����ϸ�
            m_bAttackFinished = true;
        }

        // �ִϸ��̼��� ������ Idle ���·� ��ȯ
        if (m_bAttackFinished) {
            return PlayerStateID::Idle;
        }

        // ���� �߿��� �ٸ� �Է� �����ϰ� ���� ���� ����
        return PlayerStateID::AttackMelee1;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // Ư���� ������ ���� ���� (Idle ���¿��� �ʿ��� ������ ����)
    }
};

class JumpStartState : public IPlayerState {
private:
    bool m_bAnimFinished = false;
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
    float m_fJumpVelocity = 300.0f; // ���� �� ���� �ӵ� (���� �ʿ�)

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

        if (input.MoveLeft) {
            currentVel = Vector3::Add(currentVel, right, -50.0f * airControlFactor);
        }
        if (input.MoveRight) {
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
    m_vStates.push_back(std::make_unique<AttackMelee1State>()); 
    m_vStates.push_back(std::make_unique<JumpStartState>());    
    m_vStates.push_back(std::make_unique<JumpLoopState>());     
    m_vStates.push_back(std::make_unique<JumpEndState>());     
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
    
    // ���� ����(�ӽ�)
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
        case PlayerStateID::MoveLeft: 
            animIndex = AnimIndices::WALK_LEFT;  
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::MoveRight: 
            animIndex = AnimIndices::WALK_RIGHT; 
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::AttackMelee1:
            animIndex = AnimIndices::ATTACK_MELEE1;
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
        case PlayerStateID::MoveLeft:
            animIndex = AnimIndices::WALK_LEFT;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::MoveRight:
            animIndex = AnimIndices::WALK_RIGHT;
            trackType = ANIMATION_TYPE_LOOP;
            break;
        case PlayerStateID::AttackMelee1:
            animIndex = AnimIndices::ATTACK_MELEE1;
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
            // ... �ٸ� ���µ鿡 ���� �ִϸ��̼� �ε��� ���� �߰� ...
        default: std::cerr << "Warning: No animation index mapped for state " << (int)newStateID << std::endl; break;
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

