//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dCommandList = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	m_pPlayer = NULL;
	m_inventorySlots.resize(25);
	_tcscpy_s(m_pszFrameRate, _T("LabProject ("));
}

CGameFramework::~CGameFramework()
{
	
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	CoInitialize(NULL);

	

	BuildObjects();


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("Paperlogy-4Regular.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesKorean());
	io.Fonts->Build();
	ImGui::StyleColorsDark();

	CreateCbvSrvDescriptorHeap();
	CreateIconDescriptorHeap();
	//LoadIconTextures();
	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX12_Init(
		m_pd3dDevice,
		m_nSwapChainBuffers,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_pd3dSrvDescriptorHeapForImGui,
		m_pd3dSrvDescriptorHeapForImGui->GetCPUDescriptorHandleForHeapStart(),
		m_pd3dSrvDescriptorHeapForImGui->GetGPUDescriptorHandleForHeapStart()
	);
	InitializeCraftItems();
	return(true);
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1 **)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);
#endif
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void **)&m_pdxgiFactory);

	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice))) break;
	}

	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void **)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice);
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_pd3dCommandAllocator);

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			::SetCapture(hWnd);
			::GetCursorPos(&m_ptOldCursorPos);
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			::ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			break;
		default:
			break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_ESCAPE:
					::PostQuitMessage(0);
					break;
				case VK_RETURN:
					break;
				case VK_F1:
				case VK_F2:
				case VK_F3:
				case VK_F4:
					m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
					break;
				case VK_F9:
					ChangeSwapChainState();
					break;
				default:
					break;
			}
			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_TAB:
				if (ShowInventory == false)
					ShowInventory = true;
				else
					ShowInventory = false;
				
				break;
			case 'I':
				AddDummyItem();
				break;
			}
		default:
			break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, nMessageID, wParam, lParam))
		return true;
	switch (nMessageID)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
				m_GameTimer.Stop();
			else
				m_GameTimer.Start();
			break;
		}
		case WM_SIZE:
			break;
		case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
            break;
        case WM_KEYDOWN:
        case WM_KEYUP:
			OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
	}
	return(0);
}

std::shared_ptr<Item> CGameFramework::CreateDummyItem()
{
	return std::make_shared<Item>(g_itemIDCounter++, "더미아이템");
}

void CGameFramework::AddDummyItem()
{
	for (auto& slot : m_inventorySlots)
	{
		if (slot.IsEmpty())
		{
			slot.item = CreateDummyItem();
			OutputDebugStringA("더미 아이템 추가됨\n");
			return;
		}
	}

	OutputDebugStringA("인벤토리가 가득 찼습니다\n");
}

void CGameFramework::LoadIconTextures()
{
	// 1. 업로드 버퍼 준비
	ID3D12Resource* pUploadBuffer = nullptr;
	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA subresourceData = {};

	HRESULT hr = LoadWICTextureFromFile(
		m_pd3dDevice,
		L"ICON/wood.png",
		&m_pWoodTexture,
		decodedData,
		subresourceData
	);

	if (SUCCEEDED(hr))
	{
		// 2. 업로드 버퍼 생성
		UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_pWoodTexture.Get(), 0, 1);

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Alignment = 0;
		bufferDesc.Width = uploadBufferSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		hr = m_pd3dDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pUploadBuffer)
		);

		if (FAILED(hr))
		{
			MessageBox(NULL, L"Failed to create upload buffer!", L"Error", MB_OK);
			return;
		}

		// 3. 업로드 버퍼로 데이터 복사
		UpdateSubresources(
			m_pd3dCommandList,
			m_pWoodTexture.Get(),
			pUploadBuffer,
			0, 0, 1,
			&subresourceData
		);

		// 4. 리소스 상태 전환
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = m_pWoodTexture.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_pd3dCommandList->ResourceBarrier(1, &barrier);

		// 5. CreateShaderResourceView
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pd3dSrvDescriptorHeapForIcons->GetCPUDescriptorHandleForHeapStart();

		m_pd3dDevice->CreateShaderResourceView(m_pWoodTexture.Get(), &srvDesc, cpuHandle);

		// 6. 업로드 버퍼 해제
		if (pUploadBuffer)
		{
			pUploadBuffer->Release();
			pUploadBuffer = nullptr;
		}

		m_pd3dCommandList->Close();

		// 8. 커맨드리스트 제출
		ID3D12CommandList* ppCommandLists[] = { m_pd3dCommandList };
		m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// 9. GPU가 완료될 때까지 대기
		WaitForGpuComplete();
	}
	else
	{
		MessageBox(NULL, L"Failed to load wood texture!", L"Error", MB_OK);
	}
}



void CGameFramework::CreateIconDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 32; // 예를 들어 아이콘 32개쯤? (원하는 수)
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	HRESULT hr = m_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pd3dSrvDescriptorHeapForIcons));
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create Icon SRV Descriptor Heap!", L"Error", MB_OK);
	}
}
void CGameFramework::OnDestroy()
{
    ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
    if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

#if defined(_DEBUG)
	IDXGIDebug1	*pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void **)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

#define _WITH_TERRAIN_PLAYER

void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pScene = new CScene();
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

	

#ifdef _WITH_TERRAIN_PLAYER
	CTerrainPlayer *pPlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), m_pScene->m_pTerrain);
#else
	CAirplanePlayer *pPlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), NULL);
	pPlayer->SetPosition(XMFLOAT3(425.0f, 240.0f, 640.0f));
#endif

	m_pScene->m_pPlayer = m_pPlayer = pPlayer;
	m_pCamera = m_pPlayer->GetCamera();

	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pPlayer) m_pPlayer->Release();

	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;
	if (GetKeyboardState(pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
	if (!bProcessedByScene)
	{
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		DWORD dwDirection = 0;
		if (pKeysBuffer[VK_UP] & 0xF0 || pKeysBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeysBuffer[VK_DOWN] & 0xF0 || pKeysBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeysBuffer[VK_LEFT] & 0xF0 || pKeysBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeysBuffer[VK_RIGHT] & 0xF0 || pKeysBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeysBuffer[VK_SPACE] & 0xF0) dwDirection |= DIR_UP;
		if (pKeysBuffer[VK_SHIFT] & 0xF0) dwDirection |= DIR_DOWN;
		else m_pPlayer->keyInput(pKeysBuffer);

		
		if (m_pCamera->GetMode() == TOP_VIEW_CAMERA)
		{
			
			if (pKeysBuffer['Q'] & 0xF0)
			{
				XMFLOAT3 offset = m_pCamera->GetOffset();
				offset.y = max(20.0f, offset.y - 10.0f); 
				m_pCamera->SetOffset(offset);
			}
			if (pKeysBuffer['E'] & 0xF0)
			{
				XMFLOAT3 offset = m_pCamera->GetOffset();
				offset.y = min(200.0f, offset.y + 10.0f);  
				m_pCamera->SetOffset(offset);
			}
		}
		else if (m_pCamera->GetMode() == FIRST_PERSON_CAMERA || m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			
			if (cxDelta || cyDelta)
			{
				if (pKeysBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
		}

		if (dwDirection) m_pPlayer->Move(dwDirection, 12.25f, true);
	}
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::AnimateObjects()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	if (m_pScene) m_pScene->AnimateObjects(fTimeElapsed);

	m_pPlayer->Animate(fTimeElapsed);
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

//#define _WITH_PLAYER_TOP

void CGameFramework::FrameAdvance()
{    
	m_GameTimer.Tick(30.0f);
	
	ProcessInput();

    AnimateObjects();

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera);

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, m_pCamera);

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	///////////////////////////////////////////////////////////// 아이템 핫바
	const int HotbarCount = 5;
	const float SlotSize = 54.0f;
	const float SlotSpacing = 6.0f;

	const float ExtraPadding = 18.0f; 

	const float TotalWidth = (SlotSize * HotbarCount) + (SlotSpacing * (HotbarCount - 1));
	const float WindowWidth = TotalWidth + ExtraPadding;

	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	ImVec2 hotbarPos = ImVec2(30.0f, displaySize.y - 80.0f);

	ImGui::SetNextWindowPos(hotbarPos);
	ImGui::SetNextWindowSize(ImVec2(WindowWidth, 65));
	ImGui::Begin("Hotbar", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBackground);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

	for (int i = 0; i < HotbarCount; ++i)
	{
		if (i > 0) ImGui::SameLine();

		if (i == m_nSelectedHotbarIndex)
			ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255, 255, 0, 255));

		ImGui::PushID(i);
		ImGui::Button(std::to_string(i + 1).c_str(), ImVec2(SlotSize, SlotSize));
		ImGui::PopID();

		if (i == m_nSelectedHotbarIndex)
			ImGui::PopStyleColor();

		if (ImGui::IsItemClicked())
			m_nSelectedHotbarIndex = i;
	}
	ImGui::PopStyleVar();
	ImGui::End();


	//////////////////////////////////////////////////플레이어 UI

	const float hudWidth = 300.0f;
	const float hudHeight = 100.0f;
	const float barWidth = 100.0f;
	const float barHeight = 15.0f;

	ImVec2 hudPos = ImVec2(displaySize.x - hudWidth+10.0f, displaySize.y - hudHeight);

	ImGui::SetNextWindowPos(hudPos);
	ImGui::SetNextWindowSize(ImVec2(hudWidth, hudHeight));
	ImGui::Begin("StatusBars", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse);

	
	ImGui::BeginGroup();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("🟥"); // 체력 
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	ImGui::ProgressBar(
		(float)m_pPlayer->Playerhp / (float)m_pPlayer->Maxhp,
		ImVec2(barWidth, barHeight),
		std::to_string(m_pPlayer->Playerhp).c_str()
	);
	ImGui::PopStyleColor();
	ImGui::EndGroup();

	ImGui::SameLine(0.0f, 50.0f); 
	ImGui::BeginGroup();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("🟦"); // 스태미너
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
	ImGui::ProgressBar(
		(float)m_pPlayer->Playerstamina / (float)m_pPlayer->Maxstamina,
		ImVec2(barWidth, barHeight),
		std::to_string(m_pPlayer->Playerstamina).c_str()
	);
	ImGui::PopStyleColor();
	ImGui::EndGroup();

	
	ImGui::BeginGroup();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("🟨"); // 허기
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
	ImGui::ProgressBar(m_pPlayer->PlayerHunger, ImVec2(barWidth, barHeight));
	ImGui::PopStyleColor();
	ImGui::EndGroup();

	ImGui::SameLine(0.0f, 50.0f);

	ImGui::BeginGroup();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("🟪"); // 갈증
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.4f, 0.2f, 1.0f, 1.0f));
	ImGui::ProgressBar(m_pPlayer->PlayerThirst, ImVec2(barWidth, barHeight));
	ImGui::PopStyleColor();
	ImGui::EndGroup();

	ImGui::End();
	//////////////////////////////////////////////////////// 인벤토리
	if (ShowInventory)
	{
		const int inventoryCols = 5;
		const int inventoryRows = 5;
		const float slotSize = 50.0f;
		const float spacing = 5.0f;
		ImVec2 invSize = ImVec2(600.0f, 360.0f);

		ImVec2 invPos = ImVec2(
			displaySize.x * 0.5f - invSize.x * 0.5f,
			displaySize.y * 0.5f - invSize.y * 0.5f
		);

		ImGui::SetNextWindowPos(invPos);
		ImGui::SetNextWindowSize(invSize);
		ImGui::Begin("Inventory", nullptr,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse);

		// ▶ 2열로 나누기
		ImGui::Columns(2, nullptr, false);

		for (int i = 0; i < m_inventorySlots.size(); ++i)
		{
			ImGui::PushID(i);

			if (!m_inventorySlots[i].IsEmpty())
			{
				// 아이템이 있을 때
				ID3D12DescriptorHeap* ppIconHeaps[] = { m_pd3dSrvDescriptorHeapForIcons };
				m_pd3dCommandList->SetDescriptorHeaps(_countof(ppIconHeaps), ppIconHeaps);

				// 아이템 아이콘 출력
				ImTextureID textureID = (ImTextureID)m_pd3dSrvDescriptorHeapForIcons->GetGPUDescriptorHandleForHeapStart().ptr;
				ImVec2 iconSize = ImVec2(50, 50);

				ImGui::ImageButton("##icon", textureID, iconSize);
				
			}
			else
			{
				// 아이템이 없을 때 빈 칸
				ImGui::Button(" ", ImVec2(50, 50));
			}

			ImGui::PopID();

			if ((i + 1) % 5 != 0)
				ImGui::SameLine();
		}


		ImGui::NextColumn();

		

		// ▶ 오른쪽: 스탯 정보 출력
		ImGui::Text("플레이어 레벨: %d", m_pPlayer ->PlayerLevel);
		ImGui::Text("스테이터스:");

		ImGui::BulletText("체력: %d / %d", m_pPlayer->Playerhp, m_pPlayer->Maxhp);
		ImGui::SameLine();
		if (m_pPlayer->StatPoint > 0) {
			if (ImGui::Button("+##hp")) { m_pPlayer->Maxhp += 10; m_pPlayer->StatPoint--; m_pPlayer->Playerhp += 10; }
		}
		else {
			ImGui::BeginDisabled(); ImGui::Button("+##hp"); ImGui::EndDisabled();
		}

		ImGui::BulletText("스태미너: %d / %d", m_pPlayer->Playerstamina, m_pPlayer->Maxstamina);
		ImGui::SameLine();
		if (m_pPlayer->StatPoint > 0) {
			if (ImGui::Button("+##stamina")) { m_pPlayer->Maxstamina += 10; m_pPlayer->StatPoint--; m_pPlayer->Playerstamina += 10; }
		}
		else {
			ImGui::BeginDisabled(); ImGui::Button("+##stamina"); ImGui::EndDisabled();
		}

		ImGui::BulletText("공격력: %d", m_pPlayer->PlayerAttack);
		ImGui::SameLine();
		if (m_pPlayer->StatPoint > 0) {
			if (ImGui::Button("+##atk")) { m_pPlayer->PlayerAttack += 1; m_pPlayer->StatPoint--; }
		}
		else {
			ImGui::BeginDisabled(); ImGui::Button("+##atk"); ImGui::EndDisabled();
		}

		ImGui::BulletText("이동속도: %d", m_pPlayer->PlayerSpeed);
		ImGui::SameLine();
		if (m_pPlayer->StatPoint > 0) {
			if (ImGui::Button("+##speed")) { m_pPlayer->PlayerSpeed += 0.2; m_pPlayer->StatPoint--; }
		}
		else {
			ImGui::BeginDisabled(); ImGui::Button("+##speed"); ImGui::EndDisabled();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Text("보유 포인트: %d", m_pPlayer->StatPoint);
		ImGui::Columns(1); // 정렬 초기화
		ImGui::End();
	}
	////////////////////////////////////////////////////////////////////////////////////// 조합창
	if (ShowCraftingUI) 
	{
		const float windowWidth = 600.0f;
		const float windowHeight = 450.0f;

		ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		ImVec2 craftingPos = ImVec2(
			displaySize.x * 0.5f - windowWidth * 0.5f,
			displaySize.y * 0.5f - windowHeight * 0.5f
		);

		ImGui::SetNextWindowPos(craftingPos);
		ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

		ImGui::Begin("조합 화면", nullptr,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse);

		
		ImGui::Columns(2, nullptr, false);

		// 조합 가능한 아이템 리스트
		ImGui::Text("제작 아이템");
		ImGui::Separator();

		for (int i = 0; i < m_vecCraftableItems.size(); ++i)
		{
			const CraftItem& item = m_vecCraftableItems[i];

			if (ImGui::Selectable(item.ResultItemName.c_str(), selectedCraftItemIndex == i))
			{
				selectedCraftItemIndex = i; // 아이템 선택
			}
		}

		ImGui::NextColumn();

		// ▶ 오른쪽: 필요한 재료 출력
		ImGui::Text("필요 재료");
		ImGui::Separator();

		if (selectedCraftItemIndex >= 0 && selectedCraftItemIndex < m_vecCraftableItems.size())
		{
			const CraftItem& selectedItem = m_vecCraftableItems[selectedCraftItemIndex];

			for (const CraftMaterial& mat : selectedItem.Materials)
			{
				ImGui::Text("%s x%d", mat.MaterialName.c_str(), mat.Quantity);
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::Button("조합하기", ImVec2(200, 50)))
			{
				// 조합 버튼 클릭 시 처리할 로직
			}
		}

		ImGui::Columns(1);

		ImGui::End();
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	ImGui::Render();
	ID3D12DescriptorHeap* ppHeaps[] = { m_pd3dSrvDescriptorHeapForImGui };
	m_pd3dCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pd3dCommandList);

	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close();
	
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(%4f, %4f, %4f)"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}
void CGameFramework::CreateCbvSrvDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 2; // ImGui만 쓸 거면 1개면 충분
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	HRESULT hr = m_pd3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pd3dSrvDescriptorHeapForImGui));
	if (FAILED(hr))
		::MessageBox(NULL, _T("Failed to create SRV Descriptor Heap for ImGui"), _T("Error"), MB_OK);
}
void CGameFramework::InitializeCraftItems()
{
	m_vecCraftableItems.clear();

	// 막대기
	m_vecCraftableItems.push_back({
		"막대기",
		{ {"나무", 2} },
		2
		});

	// 제작대
	m_vecCraftableItems.push_back({
		"제작대",
		{ {"나무", 4} },
		1
		});

	// 화로
	m_vecCraftableItems.push_back({
		"화로",
		{ {"돌", 8} },
		1
		});

	// 그릇
	m_vecCraftableItems.push_back({
		"그릇",
		{ {"나무", 1} },
		1
		});

	// 횃불
	m_vecCraftableItems.push_back({
		"횃불",
		{ {"막대기", 1}, {"석탄", 1} },
		2
		});

	// 나무 곡괭이
	m_vecCraftableItems.push_back({
		"나무 곡괭이",
		{ {"막대기", 2}, {"나무", 3} },
		1
		});

	// 나무 도끼
	m_vecCraftableItems.push_back({
		"나무 도끼",
		{ {"막대기", 2}, {"나무", 3} },
		1
		});

	// 나무 검
	m_vecCraftableItems.push_back({
		"나무 검",
		{ {"막대기", 1}, {"나무", 2} },
		1
		});

	// 나무 망치
	m_vecCraftableItems.push_back({
		"나무 망치",
		{ {"막대기", 2}, {"나무", 3} },
		1
		});

	// 돌 곡괭이
	m_vecCraftableItems.push_back({
		"돌 곡괭이",
		{ {"막대기", 2}, {"돌", 3} },
		1
		});

	// 돌 도끼
	m_vecCraftableItems.push_back({
		"돌 도끼",
		{ {"막대기", 2}, {"돌", 3} },
		1
		});

	// 돌 검
	m_vecCraftableItems.push_back({
		"돌 검",
		{ {"막대기", 1}, {"돌", 2} },
		1
		});

	// 돌 망치
	m_vecCraftableItems.push_back({
		"돌 망치",
		{ {"막대기", 2}, {"돌", 3} },
		1
		});

	// 철 곡괭이
	m_vecCraftableItems.push_back({
		"철 곡괭이",
		{ {"막대기", 2}, {"철괴", 3} },
		1
		});

	// 철 도끼
	m_vecCraftableItems.push_back({
		"철 도끼",
		{ {"막대기", 2}, {"철괴", 3} },
		1
		});

	// 철 검
	m_vecCraftableItems.push_back({
		"철 검",
		{ {"막대기", 1}, {"철괴", 2} },
		1
		});

	// 철 망치
	m_vecCraftableItems.push_back({
		"철 망치",
		{ {"막대기", 2}, {"철괴", 3} },
		1
		});
}