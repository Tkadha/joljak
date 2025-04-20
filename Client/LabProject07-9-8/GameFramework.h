#pragma once

#define FRAME_BUFFER_WIDTH		640
#define FRAME_BUFFER_HEIGHT		480

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "Item.h"
#include "d3dx12.h"
#include "WICTextureLoader12.h"
#include <wrl.h> // 추가
#include <vector>
#include <string>
using namespace Microsoft::WRL; // 추가
#include "ResourceManager.h"

struct CraftMaterial
{
	std::string MaterialName; // 재료 이름
	int Quantity;             // 재료 수량
};

// 제작 아이템 구조체
struct CraftItem
{
	std::string ResultItemName;        // 최종 제작 아이템 이름
	std::vector<CraftMaterial> Materials; // 필요한 재료 목록
	int ResultQuantity;                // 제작 결과 수량 (예: 2개 만들면 2)
};

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
	void CreateCbvSrvDescriptorHeap();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	std::shared_ptr<Item> CreateDummyItem();
	void AddDummyItem();
	void LoadIconTextures();
	void CreateIconDescriptorHeap();
	void InitializeCraftItems();
	bool CanCraftItem();
	void CraftSelectedItem();
	std::vector<CraftItem> m_vecCraftableItems;


private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;
	int                         m_nSelectedHotbarIndex = 0;
	bool						ShowInventory = false;
	bool						ShowCraftingUI = false; // 조합창 열기 여부
	int							selectedCraftItemIndex = -1; // 현재 선택한 아이템 인덱스
        
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


	// 상수 버퍼, 셰이더 리소스 디스크립터 힙 Scene에서 옮김
private:
	ComPtr<ID3D12DescriptorHeap>	m_pd3dCbvSrvDescriptorHeap;
	UINT							m_nCbvSrvDescriptorIncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dCbvCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGpuHandleStart;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuHandleStart;

	UINT m_nNextCbvOffset = 0; // CBV 영역 내 다음 오프셋
	UINT m_nNextSrvOffset = 0; // SRV 영역 내 다음 오프셋 (CBV 영역 이후 시작)
	UINT m_nTotalCbvDescriptors; // 생성 시 설정
	UINT m_nTotalSrvDescriptors; // 생성 시 설정

	// 리소스 매니저 추가
	std::unique_ptr<ResourceManager> m_pResourceManager;

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
	void CreateShaderResourceViews(CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);


	ID3D12Device* GetDevice() { return m_pd3dDevice; }
	ResourceManager* GetResourceManager() { return m_pResourceManager.get(); };

	ID3D12DescriptorHeap* m_pd3dSrvDescriptorHeapForImGui = nullptr;
	ID3D12DescriptorHeap* m_pd3dSrvDescriptorHeapForIcons = nullptr;

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

	CScene						*m_pScene = NULL;
	CPlayer						*m_pPlayer = NULL;
	CCamera						*m_pCamera = NULL;

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];
	int g_itemIDCounter = 0;
	struct InventorySlot {
		std::shared_ptr<Item> item;
		int quantity = 0;
		bool IsEmpty() const { return item == nullptr; }
	};

	std::vector<InventorySlot> m_inventorySlots;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pWoodTexture = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_WoodTextureHandle = {};

	BOOL obbRender = FALSE;
};

