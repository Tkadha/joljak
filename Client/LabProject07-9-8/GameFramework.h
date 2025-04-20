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
	
	// --- ���� ����ȭ�� �潺 �� �߰� ---
	UINT64                      m_nMasterFenceValue = 0;

	// ��� ����, ���̴� ���ҽ� ��ũ���� �� Scene���� �ű�
private:
	ComPtr<ID3D12DescriptorHeap>	m_pd3dCbvSrvDescriptorHeap;
	UINT							m_nCbvSrvDescriptorIncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dCbvCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGpuHandleStart;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuHandleStart;

	// ���÷� �� ũ�⵵ �ʿ��� �� ����
	UINT m_nSamplerDescriptorIncrementSize = 0; 

	UINT m_nNextCbvOffset = 0; // CBV ���� �� ���� ������
	UINT m_nNextSrvOffset = 0; // SRV ���� �� ���� ������ (CBV ���� ���� ����)
	UINT m_nTotalCbvDescriptors; // ���� �� ����
	UINT m_nTotalSrvDescriptors; // ���� �� ����

	// ���ҽ� �Ŵ��� �߰�
	std::unique_ptr<ResourceManager> m_pResourceManager;
	//std::unique_ptr<ShaderManager> m_pShaderManager;
	ShaderManager* m_pShaderManager;

public:
	// CBV ���� �Ҵ� ��û (nDescriptors�� �Ҵ� �� ���� �ڵ� ��ȯ)
	bool AllocateCbvDescriptors(UINT nDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE& outCpuStartHandle, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuStartHandle);
	// SRV ���� �Ҵ� ��û
	bool AllocateSrvDescriptors(UINT nDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE& outCpuStartHandle, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuStartHandle);

	// ���� ���� �ڵ� ��ȯ �Լ� ��...
	ID3D12DescriptorHeap* GetCbvSrvHeap() { return m_pd3dCbvSrvDescriptorHeap.Get(); }
	UINT GetCbvSrvDescriptorSize() { return m_nCbvSrvDescriptorIncrementSize; }

	void CreateCbvSrvDescriptorHeaps(int nConstantBufferViews, int nShaderResourceViews);
	D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	
	// --- ��ũ���� ũ�� ��ȯ �Լ� �߰� ---
	UINT GetSamplerDescriptorIncrementSize() const { return m_nSamplerDescriptorIncrementSize; }


	ID3D12Device* GetDevice() { return m_pd3dDevice; }
	ResourceManager* GetResourceManager() { return m_pResourceManager.get(); };
	ShaderManager* GetShaderManager() { return m_pShaderManager; };



	void WaitForGpu(); // GPU ��� �Լ� �߰�

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

