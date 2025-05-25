//***************************************************************************************
// Waves.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Waves.h"
#include <ppl.h>
#include <algorithm>
#include <vector>
#include <cassert>

using namespace DirectX;

Waves::Waves(int m, int n, float dx, float dt, float speed, float damping)
{
    mNumRows = m;
    mNumCols = n;

    mVertexCount = m*n;
    mTriangleCount = (m - 1)*(n - 1) * 2;

    mTimeStep = dt;
    mSpatialStep = dx;

    float d = damping*dt + 2.0f;
    float e = (speed*speed)*(dt*dt) / (dx*dx);
    mK1 = (damping*dt - 2.0f) / d;
    mK2 = (4.0f - 8.0f*e) / d;
    mK3 = (2.0f*e) / d;

    mPrevSolution.resize(m*n);
    mCurrSolution.resize(m*n);
    mNormals.resize(m*n);
    mTangentX.resize(m*n);

    // Generate grid vertices in system memory.

    float halfWidth = (n - 1)*dx*0.5f;
    float halfDepth = (m - 1)*dx*0.5f;
    for(int i = 0; i < m; ++i)
    {
        float z = halfDepth - i*dx;
        for(int j = 0; j < n; ++j)
        {
            float x = -halfWidth + j*dx;

            mPrevSolution[i*n + j] = XMFLOAT3(x, 1800.0f, z);
            mCurrSolution[i*n + j] = XMFLOAT3(x, 1800.0f, z);
            mNormals[i*n + j] = XMFLOAT3(0.0f, 1.0f, 0.0f);
            mTangentX[i*n + j] = XMFLOAT3(1.0f, 0.0f, 0.0f);
        }
    }
}

Waves::~Waves()
{
}

int Waves::RowCount()const
{
	return mNumRows;
}

int Waves::ColumnCount()const
{
	return mNumCols;
}

int Waves::VertexCount()const
{
	return mVertexCount;
}

int Waves::TriangleCount()const
{
	return mTriangleCount;
}

float Waves::Width()const
{
	return mNumCols*mSpatialStep;
}

float Waves::Depth()const
{
	return mNumRows*mSpatialStep;
}

void Waves::Update(float dt)
{
	static float t = 0;

	// Accumulate time.
	t += dt;

	// Only update the simulation at the specified time step.
	if( t >= mTimeStep )
	{
		// Only update interior points; we use zero boundary conditions.
		concurrency::parallel_for(1, mNumRows - 1, [this](int i)
		//for(int i = 1; i < mNumRows-1; ++i)
		{
			for(int j = 1; j < mNumCols-1; ++j)
			{
				// After this update we will be discarding the old previous
				// buffer, so overwrite that buffer with the new update.
				// Note how we can do this inplace (read/write to same element) 
				// because we won't need prev_ij again and the assignment happens last.

				// Note j indexes x and i indexes z: h(x_j, z_i, t_k)
				// Moreover, our +z axis goes "down"; this is just to 
				// keep consistent with our row indices going down.

				mPrevSolution[i*mNumCols+j].y = 
					mK1*mPrevSolution[i*mNumCols+j].y +
					mK2*mCurrSolution[i*mNumCols+j].y +
					mK3*(mCurrSolution[(i+1)*mNumCols+j].y + 
					     mCurrSolution[(i-1)*mNumCols+j].y + 
					     mCurrSolution[i*mNumCols+j+1].y + 
						 mCurrSolution[i*mNumCols+j-1].y);
			}
		});

		// We just overwrote the previous buffer with the new data, so
		// this data needs to become the current solution and the old
		// current solution becomes the new previous solution.
		std::swap(mPrevSolution, mCurrSolution);

		t = 0.0f; // reset time

		//
		// Compute normals using finite difference scheme.
		//
		concurrency::parallel_for(1, mNumRows - 1, [this](int i)
		//for(int i = 1; i < mNumRows - 1; ++i)
		{
			for(int j = 1; j < mNumCols-1; ++j)
			{
				float l = mCurrSolution[i*mNumCols+j-1].y;
				float r = mCurrSolution[i*mNumCols+j+1].y;
				float t = mCurrSolution[(i-1)*mNumCols+j].y;
				float b = mCurrSolution[(i+1)*mNumCols+j].y;
				mNormals[i*mNumCols+j].x = -r+l;
				mNormals[i*mNumCols+j].y = 2.0f*mSpatialStep;
				mNormals[i*mNumCols+j].z = b-t;

				XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&mNormals[i*mNumCols+j]));
				XMStoreFloat3(&mNormals[i*mNumCols+j], n);

				mTangentX[i*mNumCols+j] = XMFLOAT3(2.0f*mSpatialStep, r-l, 0.0f);
				XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&mTangentX[i*mNumCols+j]));
				XMStoreFloat3(&mTangentX[i*mNumCols+j], T);
			}
		});
	}
}

void Waves::Disturb(int i, int j, float magnitude)
{
	// Don't disturb boundaries.
	assert(i > 1 && i < mNumRows-2);
	assert(j > 1 && j < mNumCols-2);

	float halfMag = 0.5f*magnitude;

	// Disturb the ijth vertex height and its neighbors.
	mCurrSolution[i*mNumCols+j].y     += magnitude;
	mCurrSolution[i*mNumCols+j+1].y   += halfMag;
	mCurrSolution[i*mNumCols+j-1].y   += halfMag;
	mCurrSolution[(i+1)*mNumCols+j].y += halfMag;
	mCurrSolution[(i-1)*mNumCols+j].y += halfMag;
}
	
void Waves::InitializeGpuResourcesAndHandles(
	ID3D12Device* device,
	DXGI_FORMAT format,
	Microsoft::WRL::ComPtr<ID3D12Resource> pPrevTex,
	Microsoft::WRL::ComPtr<ID3D12Resource> pCurrTex,
	Microsoft::WRL::ComPtr<ID3D12Resource> pNextTex,
	D3D12_CPU_DESCRIPTOR_HANDLE hPrevUavCPU, D3D12_GPU_DESCRIPTOR_HANDLE hPrevUavGPU,
	D3D12_CPU_DESCRIPTOR_HANDLE hCurrUavCPU, D3D12_GPU_DESCRIPTOR_HANDLE hCurrUavGPU,
	D3D12_CPU_DESCRIPTOR_HANDLE hNextUavCPU, D3D12_GPU_DESCRIPTOR_HANDLE hNextUavGPU,
	D3D12_CPU_DESCRIPTOR_HANDLE hCurrSrvCPU, D3D12_GPU_DESCRIPTOR_HANDLE hCurrSrvGPU)
{
	m_pd3dDevice = device; // SRV 업데이트 시 필요
	m_Format = format;     // SRV 업데이트 시 필요

	m_pPrevSolTexture = pPrevTex;
	m_pCurrSolTexture = pCurrTex;
	m_pNextSolTexture = pNextTex;

	m_hPrevSolUavCPU = hPrevUavCPU;
	m_hPrevSolUavGPU = hPrevUavGPU;
	m_hCurrSolUavCPU = hCurrUavCPU;
	m_hCurrSolUavGPU = hCurrUavGPU;
	m_hNextSolUavCPU = hNextUavCPU;
	m_hNextSolUavGPU = hNextUavGPU;

	m_hCurrSolSrvCPU = hCurrSrvCPU; // 현재 CurrSol 텍스처에 대한 SRV
	m_hCurrSolSrvGPU = hCurrSrvGPU;
}

// --- 현재 시뮬레이션 단계에 맞는 리소스 반환 ---
ID3D12Resource* Waves::GetCurrentPrevSolTexture() const
{
	// 현재 m_pPrevSolTexture가 이전 스텝 결과를 가리킴
	return m_pPrevSolTexture.Get();
}

ID3D12Resource* Waves::GetCurrentCurrSolTexture() const
{
	// 현재 m_pCurrSolTexture가 현재 스텝 결과를 가리킴 (CS 입력용)
	return m_pCurrSolTexture.Get();
}

ID3D12Resource* Waves::GetCurrentNextSolTexture() const // 컴퓨트 셰이더의 출력 대상
{
	// 현재 m_pNextSolTexture가 다음 스텝 결과를 저장할 버퍼를 가리킴
	return m_pNextSolTexture.Get();
}

ID3D12Resource* Waves::GetCurrentCurrSolTextureForRendering() const // 렌더링 시 SRV로 사용될 현재 결과
{
	// 현재 m_pCurrSolTexture가 렌더링될 데이터를 담고 있음
	return m_pCurrSolTexture.Get();
}

// --- 현재 시뮬레이션 단계에 맞는 GPU 디스크립터 핸들 반환 ---
D3D12_GPU_DESCRIPTOR_HANDLE Waves::GetCurrentPrevSolUAV_GPU() const
{
	return m_hPrevSolUavGPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE Waves::GetCurrentCurrSolUAV_GPU() const
{
	return m_hCurrSolUavGPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE Waves::GetCurrentNextSolUAV_GPU() const
{
	return m_hNextSolUavGPU;
}



// 핑퐁을 위한 버퍼 역할 및 관련 디스크립터 교체 로직
void Waves::SwapSimBuffersAndDescriptors()
{
	// 리소스 포인터 스왑:
	// Prev = Curr
	// Curr = Next
	// Next = (이전 Prev)
	Microsoft::WRL::ComPtr<ID3D12Resource> tempTexture = m_pPrevSolTexture;
	m_pPrevSolTexture = m_pCurrSolTexture;
	m_pCurrSolTexture = m_pNextSolTexture;
	m_pNextSolTexture = tempTexture;

	// UAV 디스크립터 핸들 스왑 (CPU 및 GPU 모두)
	D3D12_CPU_DESCRIPTOR_HANDLE tempUavCPU = m_hPrevSolUavCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE tempUavGPU = m_hPrevSolUavGPU;

	m_hPrevSolUavCPU = m_hCurrSolUavCPU;
	m_hPrevSolUavGPU = m_hCurrSolUavGPU;

	m_hCurrSolUavCPU = m_hNextSolUavCPU;
	m_hCurrSolUavGPU = m_hNextSolUavGPU;

	m_hNextSolUavCPU = tempUavCPU;
	m_hNextSolUavGPU = tempUavGPU;

	// 현재 CurrSol 텍스처에 대한 SRV 디스크립터 업데이트
	// m_hCurrSolSrvCPU는 고정된 디스크립터 힙 슬롯을 가리키고,
	// 그 슬롯의 내용을 새로운 m_pCurrSolTexture를 가리키도록 덮어씁니다.
	if (m_pd3dDevice && m_hCurrSolSrvCPU.ptr != 0 && m_pCurrSolTexture)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = m_Format; // InitializeGpuResourcesAndHandles에서 설정된 포맷
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		m_pd3dDevice->CreateShaderResourceView(m_pCurrSolTexture.Get(), &srvDesc, m_hCurrSolSrvCPU);
	}
	else
	{
		OutputDebugStringA("Warning: SRV for current wave solution could not be updated in SwapSimBuffersAndDescriptors. Device, CPU handle, or Current Texture is null.\n");
	}
}
