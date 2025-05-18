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

    // �ùķ��̼� ����� ���� (CPU ������)
    const DirectX::XMFLOAT3& Position(int i) const { return mCurrSolution[i]; }
    const DirectX::XMFLOAT3& Normal(int i) const { return mNormals[i]; }
    const DirectX::XMFLOAT3& TangentX(int i) const { return mTangentX[i]; } // �ʿ��ϴٸ�

    // GPU ���ҽ� ID �Ǵ� �ڵ� (ResourceManager�� ����)
    // ����: ���� ID Ÿ���� ResourceManager ���迡 ���� �ٸ�
    size_t GetPrevSolUAV() const { return mPrevSolUAVHandle; }
    size_t GetCurrSolUAV() const { return mCurrSolUAVHandle; }
    size_t GetNextSolUAV() const { return mNextSolUAVHandle; }

    // CurrSol�� ������ �� SRV�ε� ���� �� ����
    size_t GetCurrSolSRV() const { return mCurrSolSRVHandle; }

	void Update(float dt);
	void Disturb(int i, int j, float magnitude);

    // �ʱ�ȭ (ResourceManager�� ���� GPU ���ҽ� ���� ��û)
   // �� �Լ��� ResourceManager�� Device ������ ������ ������ ȣ��Ǿ�� ��
   // bool InitResources(ID3D12Device* device, ResourceManager* resManager);
   // �Ǵ� Scene/GameFramework���� ���� ���ҽ��� �����ϰ� �ڵ��� Waves ��ü�� �Ѱ��� ���� ����
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
    size_t mCurrSolSRVHandle = -1; // CurrSol�� SRV�ε� ����� ���
};

#endif // WAVES_H