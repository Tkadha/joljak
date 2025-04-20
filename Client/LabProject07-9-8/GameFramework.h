#pragma once

#define FRAME_BUFFER_WIDTH		640
#define FRAME_BUFFER_HEIGHT		480

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "ResourceManager.h"
#include "ShaderManager.h"

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

    void BuildObjects();
    void ReleaseObjects();

    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;
        
	IDXGIFactory4				*m_pdxgiFactory = NULL;
	IDXGISwapChain3				*m_pdxgiSwapChain = NULL;
	ID3D12Device				*m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap = NULL;

	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;

	ID3D12CommandAllocator		*m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue			*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList	*m_pd3dCommandList = NULL;

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;
	
	// --- 종료 동기화용 펜스 값 추가 ---
	UINT64                      m_nMasterFenceValue = 0;

	// 상수 버퍼, 셰이더 리소스 디스크립터 힙 Scene에서 옮김
private:
	ComPtr<ID3D12DescriptorHeap>	m_pd3dCbvSrvDescriptorHeap;
	UINT							m_nCbvSrvDescriptorIncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dCbvCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGpuHandleStart;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuHandleStart;

	// 샘플러 힙 크기도 필요할 수 있음
	UINT m_nSamplerDescriptorIncrementSize = 0; 

	UINT m_nNextCbvOffset = 0; // CBV 영역 내 다음 오프셋
	UINT m_nNextSrvOffset = 0; // SRV 영역 내 다음 오프셋 (CBV 영역 이후 시작)
	UINT m_nTotalCbvDescriptors; // 생성 시 설정
	UINT m_nTotalSrvDescriptors; // 생성 시 설정

	// 리소스 매니저 추가
	std::unique_ptr<ResourceManager> m_pResourceManager;
	//std::unique_ptr<ShaderManager> m_pShaderManager;
	ShaderManager* m_pShaderManager;

public:
	// CBV 슬롯 할당 요청 (nDescriptors개 할당 후 시작 핸들 반환)
	bool AllocateCbvDescriptors(UINT nDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE& outCpuStartHandle, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuStartHandle);
	// SRV 슬롯 할당 요청
	bool AllocateSrvDescriptors(UINT nDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE& outCpuStartHandle, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuStartHandle);

	// 힙의 시작 핸들 반환 함수 등...
	ID3D12DescriptorHeap* GetCbvSrvHeap() { return m_pd3dCbvSrvDescriptorHeap.Get(); }
	UINT GetCbvSrvDescriptorSize() { return m_nCbvSrvDescriptorIncrementSize; }

	void CreateCbvSrvDescriptorHeaps(int nConstantBufferViews, int nShaderResourceViews);
	D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	
	// --- 디스크립터 크기 반환 함수 추가 ---
	UINT GetSamplerDescriptorIncrementSize() const { return m_nSamplerDescriptorIncrementSize; }


	ID3D12Device* GetDevice() { return m_pd3dDevice; }
	ResourceManager* GetResourceManager() { return m_pResourceManager.get(); };
	ShaderManager* GetShaderManager() { return m_pShaderManager; };



	void WaitForGpu(); // GPU 대기 함수 추가

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

	CScene						*m_pScene = NULL;
	CPlayer						*m_pPlayer = NULL;
	CCamera						*m_pCamera = NULL;

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];

	BOOL obbRender = FALSE;
};

