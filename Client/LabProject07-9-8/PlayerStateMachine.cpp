// PlayerStateMachine.cpp
#include "stdafx.h" // 필요시 프로젝트의 stdafx.h 포함
#include "PlayerStateMachine.h"
#include "Player.h" // CTerrainPlayer 정의 포함
#include "Animation.h" // CAnimationController 정의 포함
#include <iostream>   // 디버깅 출력용 (추후 제거)
#include <algorithm>  // std::min, std::max 등 사용

// --- 구체적인 상태 클래스 구현 ---

class IdleState : public IPlayerState {
public:
    PlayerStateID GetID() const override { return PlayerStateID::Idle; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        std::cout << "Entering Idle State\n";
        // 상태 머신을 통해 애니메이션 변경 요청 (즉시 변경, 블렌딩 없음 가정)
        // 실제 애니메이션 설정은 PlayerStateMachine::PerformStateChange 에서 처리됨
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        const auto& input = player->GetStateMachineInput(); // 입력 가져오기

        // 이동 입력 체크
        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.MoveLeft) return PlayerStateID::MoveLeft;
        if (input.MoveRight) return PlayerStateID::MoveRight;
        // 기타 입력 (공격, 점프 등)
        if (input.Attack) return PlayerStateID::AttackMelee1;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { // 상호작용 키 입력 시
            // 충돌 검사
            //if () {
            //    // return PlayerStateID::Interact;
            //}
        }

        // 다른 입력이 없으면 Idle 상태 유지
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
        if (input.MoveLeft) {
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.MoveRight) {
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector); // 대각선 이동 시 속도 보정 위해 정규화
            float currentSpeed = (input.Run && input.MoveForward) ? 80.0f : 50.0f; // 달리기/걷기 속도 구분 (RunForward 상태 전환은 아래에서)
            XMFLOAT3 currentVel = player->GetVelocity(); // 현재 Y 속도 유지
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, currentSpeed, false);
            targetVel.y = currentVel.y; // Y 속도는 유지
            player->SetVelocity(targetVel); // 최종 속도 설정
        }
        else {
            // 모든 이동 키가 떨어졌으면 Idle 상태로 전환
            return PlayerStateID::Idle;
        }
        // 입력에 따른 상태 전환 체크
        if (input.Run && input.MoveForward && !input.MoveLeft && !input.MoveRight && !input.MoveBackward) {
            // 앞 + 달리기만 눌렸을 때 RunForward 상태로 (필요시)
            // return PlayerStateID::RunForward;
        }
        if (input.Attack) return PlayerStateID::AttackMelee1;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* 상호작용 체크 */ }


        // 현재 상태 유지 조건 결정 (예: 어떤 이동키든 눌려 있으면 해당 주 방향 상태 유지)
        if (input.MoveForward) return PlayerStateID::WalkForward; // 주 방향 우선
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.MoveLeft) return PlayerStateID::MoveLeft;
        if (input.MoveRight) return PlayerStateID::MoveRight;

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
        if (input.MoveLeft) { // PlayerInputData 멤버 이름에 맞게 수정 (WalkLeft -> StrafeLeft)
            moveVector = Vector3::Add(moveVector, right, -1.0f);
            isMoving = true;
        }
        if (input.MoveRight) { // PlayerInputData 멤버 이름에 맞게 수정 (WalkRight -> StrafeRight)
            moveVector = Vector3::Add(moveVector, right);
            isMoving = true;
        }

        if (isMoving) {
            moveVector = Vector3::Normalize(moveVector); // 대각선 이동 속도 보정
            // 달리기 키(Shift)는 앞으로 갈 때만 적용되도록 제한 (선택적)
            float currentSpeed = (input.Run && input.MoveForward && !input.MoveBackward && !input.MoveLeft && !input.MoveRight) ? 80.0f : 50.0f; // 예시 속도
            XMFLOAT3 currentVel = player->GetVelocity();
            XMFLOAT3 targetVel = Vector3::ScalarProduct(moveVector, currentSpeed, false);
            targetVel.y = currentVel.y; // Y축 속도 유지
            player->SetVelocity(targetVel);
        }
        else {
            // 모든 이동 키가 떨어졌으면 Idle 상태로 전환
            return PlayerStateID::Idle;
        }

        if (input.Attack) return PlayerStateID::AttackMelee1;
        if (input.Jump) return PlayerStateID::JumpStart;
        if (input.Interact) { /* 상호작용 체크 */ }

        if (input.MoveForward) return PlayerStateID::WalkForward;
        if (input.MoveBackward) return PlayerStateID::WalkBackward;
        if (input.MoveLeft) return PlayerStateID::MoveLeft;
        if (input.MoveRight) return PlayerStateID::MoveRight;

        // 여기까지 오면 안 되지만, 안전을 위해 Idle 반환
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
        if (input.Interact) { /* 상호작용 체크 */ }

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

        // --- 속도 적용 ---
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
        if (input.Interact) { /* 상호작용 체크 */ }

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
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK; // 공격 애니메이션을 재생할 트랙

public:
    PlayerStateID GetID() const override { return PlayerStateID::AttackMelee1; }

    void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        m_bAttackFinished = false;

        // 상태 머신을 통해 애니메이션 변경 요청
        // PerformStateChange 에서 실제로 트랙 설정이 이루어짐
        // (이 상태는 보통 즉시 전환될 수 있음 - 필요시 forceImmediate 사용 고려)

        // 공격 중에는 플레이어 이동 속도 0으로 설정 (선택적)
        player->SetVelocity({ 0.0f, player->GetVelocity().y, 0.0f });

        // Enter 시점에서 사용할 트랙 결정 (블렌딩 직후라면 target 트랙일 수 있음)
        // m_nAnimTrack = stateMachine->GetCurrentAnimationTrack(); // 이런 함수가 필요할수도 있음
        // 여기서는 일단 주 트랙 사용 가정
        m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
    }

    PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) override {
        // 애니메이션 재생 완료 확인
        float currentPosition = stateMachine->GetTrackPosition(m_nAnimTrack);
        float animLength = stateMachine->GetAnimationLength(m_nAnimTrack);

        // CAnimationTrack의 m_nType 이 ANIMATION_TYPE_ONCE 로 설정되었다고 가정
        // 재생 위치가 길이를 넘어서면 완료된 것으로 간주 (약간의 오차 감안)
        if (!m_bAttackFinished && currentPosition >= animLength * 0.99f) { // 거의 끝에 도달하면
            m_bAttackFinished = true;
        }

        // 애니메이션이 끝나면 Idle 상태로 전환
        if (m_bAttackFinished) {
            return PlayerStateID::Idle;
        }

        // 공격 중에는 다른 입력 무시하고 현재 상태 유지
        return PlayerStateID::AttackMelee1;
    }

    void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) override {
        // 특별히 정리할 내용 없음 (Idle 상태에서 필요한 설정할 것임)
    }
};

class JumpStartState : public IPlayerState {
private:
    bool m_bAnimFinished = false;
    int m_nAnimTrack = BlendConfig::PRIMARY_TRACK;
    float m_fJumpVelocity = 300.0f; // 점프 시 수직 속도 (조정 필요)

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

        if (input.MoveLeft) {
            currentVel = Vector3::Add(currentVel, right, -50.0f * airControlFactor);
        }
        if (input.MoveRight) {
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
    m_vStates.push_back(std::make_unique<AttackMelee1State>()); 
    m_vStates.push_back(std::make_unique<JumpStartState>());    
    m_vStates.push_back(std::make_unique<JumpLoopState>());     
    m_vStates.push_back(std::make_unique<JumpEndState>());     
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
            // ... 다른 상태들에 대한 애니메이션 인덱스 매핑 추가 ...
        default: std::cerr << "Warning: No animation index mapped for state " << (int)newStateID << std::endl; break;
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

