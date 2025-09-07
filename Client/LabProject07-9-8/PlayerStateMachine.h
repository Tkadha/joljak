// PlayerStateMachine.h
#pragma once

#include "PlayerStateDefs.h"
#include <vector>
#include <memory> // unique_ptr 사용

// 전방 선언
class CTerrainPlayer;       // 실제 플레이어 클래스 타입으로 변경 필요
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
    int shardType = 0; // [수정] 파편 종류를 int로 저장 (0:없음, 1:나무, 2:돌)
};


// --- 상태 머신 클래스 ---
class PlayerStateMachine {
public:
    PlayerStateMachine(CTerrainPlayer* owner, CAnimationController* animCtrl);
    ~PlayerStateMachine();

    // 매 프레임 호출되어 상태 업데이트 및 블렌딩 처리
    void Update(float deltaTime);
    // 입력 시스템으로부터 호출되어 입력 상태 저장
    void HandleInput(const PlayerInputData& input);
    // 현재 상태 ID 반환
    PlayerStateID GetCurrentStateID() const;
    // 현재 애니메이션 트랙의 재생 위치 반환 (상태에서 종료 확인 등에 사용)
    float GetTrackPosition(int trackIndex) const;
    // 현재 애니메이션 트랙의 길이 반환
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
    // [추가] GameFramework가 마지막 공격 정보를 가져가는 public 함수
    HitInfo GetAndClearLastHitInfo() {
        HitInfo lastHit = m_LastHitInfo; // 정보를 복사
        m_LastHitInfo.hasHit = false;      // 정보 초기화 (중복 생성을 막기 위함)
        return lastHit;
    }

    // 공격 키 입력 시 호출
    PlayerStateID DetermineAttackState();


    // 상태 전환 처리 (내부 헬퍼 함수) - public 이동
    void PerformStateChange(PlayerStateID newStateID, bool forceImmediate = false);
private:
    CTerrainPlayer* m_pOwner = nullptr;                // 상태 머신을 소유한 플레이어 객체
    CAnimationController* m_pAnimController = nullptr; // 애니메이션 제어기
    IPlayerState* m_pCurrentState = nullptr;         // 현재 활성 상태 (Raw 포인터)
    PlayerStateID m_eCurrentStateID = PlayerStateID::None;

    // 모든 상태 객체 인스턴스 저장 (unique_ptr로 소유권 관리)
    std::vector<std::unique_ptr<IPlayerState>> m_vStates;

    PlayerInputData m_LastInput; // 마지막으로 받은 입력 상태 저장

    // --- 블렌딩 관련 변수 ---
    PlayerStateID m_eTargetStateID = PlayerStateID::None; // 전환 목표 상태 ID
    int m_nSourceTrack = BlendConfig::PRIMARY_TRACK;    // 현재 주 애니메이션 트랙 (블렌드 아웃될 트랙)
    int m_nTargetTrack = BlendConfig::BLENDING_TRACK;   // 새로 재생될 애니메이션 트랙 (블렌드 인될 트랙)
    float m_fBlendDuration = BlendConfig::DEFAULT_BLEND_DURATION; // 블렌딩 지속 시간
    float m_fBlendTimer = 0.0f;                       // 현재 블렌딩 진행 시간
    bool m_bIsBlending = false;                       // 블렌딩 진행 중 여부 플래그

    // 상태 객체 가져오기 (내부 헬퍼 함수)
    IPlayerState* GetState(PlayerStateID id);
    // 블렌딩 업데이트 처리 (내부 헬퍼 함수)
    void UpdateBlend(float deltaTime);

    HitInfo m_LastHitInfo;
    ObjectInfo m_ObjectInfo;
};

class CGameObject;

// --- 상태 인터페이스 ---
class IPlayerState {
public:
    virtual ~IPlayerState() = default;

    // 상태 진입 시 호출
    virtual void Enter(CTerrainPlayer* player, PlayerStateMachine* stateMachine) = 0;
    // 매 프레임 상태 로직 실행 및 다음 상태 ID 반환
    virtual PlayerStateID Update(CTerrainPlayer* player, PlayerStateMachine* stateMachine, float deltaTime) = 0;
    // 상태 종료 시 호출
    virtual void Exit(CTerrainPlayer* player, PlayerStateMachine* stateMachine) = 0;
    // 상태 ID 반환
    virtual PlayerStateID GetID() const = 0;

    void CollisionUpdate(CTerrainPlayer* player, CGameObject* hitObject);
    
};


