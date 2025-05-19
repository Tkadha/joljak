//***************************************************************************************
// Waves.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs the calculations for the wave simulation.  After the simulation has been
// updated, the client must copy the current solution into vertex buffers for rendering.
// This class only does the calculations, it does not do any drawing.
//***************************************************************************************

#ifndef WAVES_H
#define WAVES_H

#include <vector>
#include <DirectXMath.h>

class Waves
{
public:
    Waves(int m, int n, float dx, float dt, float speed, float damping);
    Waves(const Waves& rhs) = delete;
    Waves& operator=(const Waves& rhs) = delete;
    ~Waves();

	int RowCount()const;
	int ColumnCount()const;
	int VertexCount()const;
	int TriangleCount()const;
	float Width()const;
	float Depth()const;

    // 시뮬레이션 결과에 접근 (CPU 데이터)
    const DirectX::XMFLOAT3& Position(int i) const { return mCurrSolution[i]; }
    const DirectX::XMFLOAT3& Normal(int i) const { return mNormals[i]; }
    const DirectX::XMFLOAT3& TangentX(int i) const { return mTangentX[i]; } // 필요하다면

    // GPU 리소스 ID 또는 핸들 (ResourceManager가 관리)
    // 예시: 실제 ID 타입은 ResourceManager 설계에 따라 다름
    size_t GetPrevSolUAV() const { return mPrevSolUAVHandle; }
    size_t GetCurrSolUAV() const { return mCurrSolUAVHandle; }
    size_t GetNextSolUAV() const { return mNextSolUAVHandle; }

    // CurrSol은 렌더링 시 SRV로도 사용될 수 있음
    size_t GetCurrSolSRV() const { return mCurrSolSRVHandle; }

	void Update(float dt);
	void Disturb(int i, int j, float magnitude);

    // 초기화 (ResourceManager를 통해 GPU 리소스 생성 요청)
   // 이 함수는 ResourceManager와 Device 접근이 가능한 곳에서 호출되어야 함
   // bool InitResources(ID3D12Device* device, ResourceManager* resManager);
   // 또는 Scene/GameFramework에서 직접 리소스를 생성하고 핸들을 Waves 객체에 넘겨줄 수도 있음
    void SetResourceHandles(size_t prevSolUAV, size_t currSolUAV, size_t nextSolUAV, size_t currSolSRV);


private:
    int mNumRows = 0;
    int mNumCols = 0;

    int mVertexCount = 0;
    int mTriangleCount = 0;

    // Simulation constants we can precompute.
    float mK1 = 0.0f;
    float mK2 = 0.0f;
    float mK3 = 0.0f;

    float mTimeStep = 0.0f;
    float mSpatialStep = 0.0f;

    std::vector<DirectX::XMFLOAT3> mPrevSolution;
    std::vector<DirectX::XMFLOAT3> mCurrSolution;
    std::vector<DirectX::XMFLOAT3> mNormals;
    std::vector<DirectX::XMFLOAT3> mTangentX;


    size_t mPrevSolUAVHandle = -1;
    size_t mCurrSolUAVHandle = -1;
    size_t mNextSolUAVHandle = -1;
    size_t mCurrSolSRVHandle = -1; // CurrSol을 SRV로도 사용할 경우
};

#endif // WAVES_H