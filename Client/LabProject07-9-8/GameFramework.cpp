﻿//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include <thread>
#include "GameFramework.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "NetworkManager.h"
#include "PlayerStateDefs.h"
#include "Player.h"


void CGameFramework::NerworkThread()
{
	auto& nwManager = NetworkManager::GetInstance();
	nwManager.do_recv();
	while (true)
	{

		while (!nwManager.send_queue.empty())
		{
			auto packet = nwManager.PopSendQueue();

			nwManager.do_send(packet.first.get(), packet.second);
			SleepEx(1, TRUE);
		}

		while (!nwManager.recv_queue.empty())
		{
			auto packet = nwManager.PopRecvQueue();
			ProcessPacket(packet.first.get());

		}
		SleepEx(10, TRUE);
	}
}
void CGameFramework::ProcessPacket(char* packet)
{
	E_PACKET type = static_cast<E_PACKET>(packet[1]);
	switch (type)
	{
	case E_PACKET::E_P_POSITION:
	{
		POSITION_PACKET* recv_p = reinterpret_cast<POSITION_PACKET*>(packet);
		if (recv_p->uid == _MyID) {
			m_pPlayer->SetPosition(XMFLOAT3{ recv_p->position.x, recv_p->position.y, recv_p->position.z});
		}
		else if (m_pScene->PlayerList.find(recv_p->uid) != m_pScene->PlayerList.end()) {
			m_pScene->PlayerList[recv_p->uid]->SetPosition(XMFLOAT3{ recv_p->position.x, recv_p->position.y, recv_p->position.z });
		}
	}
	break;
	case E_PACKET::E_P_ROTATE:
	{
		ROTATE_PACKET* recv_p = reinterpret_cast<ROTATE_PACKET*>(packet);
		if (m_pScene->PlayerList.find(recv_p->uid) != m_pScene->PlayerList.end()) {
			m_pScene->PlayerList[recv_p->uid]->SetLook(XMFLOAT3{ recv_p->look.x, recv_p->look.y, recv_p->look.z });
			m_pScene->PlayerList[recv_p->uid]->SetUp(XMFLOAT3{ recv_p->up.x, recv_p->up.y, recv_p->up.z });
			m_pScene->PlayerList[recv_p->uid]->SetRight(XMFLOAT3{ recv_p->right.x, recv_p->right.y, recv_p->right.z });
			m_pScene->PlayerList[recv_p->uid]->SetScale(10.f, 10.f, 10.f);
		}
	}
	break;
	case E_PACKET::E_P_INPUT:
	{
		INPUT_PACKET* recv_p = reinterpret_cast<INPUT_PACKET*>(packet);
		if (m_pScene->PlayerList.find(recv_p->uid) != m_pScene->PlayerList.end()) {
			m_pScene->PlayerList[recv_p->uid]->ChangeAnimation(recv_p->inputData);
		}
	}
		break;
	case E_PACKET::E_P_LOGIN:
	{
		LOGIN_PACKET* recv_p = reinterpret_cast<LOGIN_PACKET*>(packet);
		if (_MyID == -1) _MyID = recv_p->uid;
		else if (m_pScene->PlayerList.find(recv_p->uid) == m_pScene->PlayerList.end()) {		
			m_logQueue.push(log_inout{ E_PACKET::E_P_LOGIN ,recv_p->uid });
		}
	}
	break;
	case E_PACKET::E_P_LOGOUT:
	{
		LOGOUT_PACKET* recv_p = reinterpret_cast<LOGOUT_PACKET*>(packet);
		m_logQueue.push(log_inout{ E_PACKET::E_P_LOGOUT ,recv_p->uid });
	}
	break;
	default:
		break;
	}
}
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
	_MyID = -1;
	_tcscpy_s(m_pszFrameRate, _T("Survival Odyssey ("));
}

CGameFramework::~CGameFramework()
{
	
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;
	m_nIconCount = 0;
	

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateCbvSrvDescriptorHeaps(200, 30000);
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
	ItemManager::Initialize();
	InitializeItemIcons();


	m_pConstructionSystem = new CConstructionSystem();
	m_pConstructionSystem->Init(m_pd3dDevice, m_pd3dCommandList, this, m_pScene);
	
	
	auto& nwManager = NetworkManager::GetInstance();
	nwManager.Init();
	std::thread t(&CGameFramework::NerworkThread, this);
	t.detach();
	

	//ChangeSwapChainState();

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

	// --- 디바이스 생성 직후 디스크립터 크기 조회 및 저장 ---
	if (m_pd3dDevice) { // 디바이스가 성공적으로 생성되었다면
		m_nCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_nSamplerDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}
	else {
		// 디바이스 생성 실패 시 오류 처리
		OutputDebugString(L"Error: Failed to create D3D12 Device. Cannot get descriptor sizes.\n");
		// 필요하다면 프로그램 종료 또는 예외 처리
	}
	m_pShaderManager = new ShaderManager(m_pd3dDevice);

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
	
	// 마스터 펜스 값 초기화
	m_nMasterFenceValue = 0; // 0부터 시작

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
				ShowInventory = !ShowInventory;
				break;
			case 'I':
				
				break;
			case 'O':
				ShowCraftingUI = !ShowCraftingUI;
				break;
			case 'B':
				BuildMode = !BuildMode;
				break;
			case 'K':
				ShowFurnaceUI = !ShowFurnaceUI;
				break;
			case 'R':
				BuildMode = false;
				bPrevBuildMode = BuildMode;
				m_pConstructionSystem->ConfirmPlacement();

				break;
			}
			break;
		default:
			break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, nMessageID, wParam, lParam)) return true;
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
	if (m_inventorySlots.size() >= 5) // 인벤토리 5칸 이상이면
	{
		if (m_inventorySlots[0].IsEmpty())
		{
			auto tempItem = ItemManager::GetItemByName("wood");
			if (tempItem)
			{
				m_inventorySlots[0].item = tempItem;
				m_inventorySlots[0].quantity = 5;
			}
		}

		if (m_inventorySlots[1].IsEmpty())
		{
			auto tempItem2 = ItemManager::GetItemByName("stone");
			if (tempItem2)
			{
				m_inventorySlots[1].item = tempItem2;
				m_inventorySlots[1].quantity = 3;
			}
		}
	}
}

void CGameFramework::AddItem(const std::string& name, int quantity)
{
	auto newItem = ItemManager::GetItemByName(name);
	if (!newItem) return;

	// 1. 이미 존재하는 아이템이면 수량 증가
	for (auto& slot : m_inventorySlots) {
		if (!slot.IsEmpty() && slot.item->GetName() == name) {
			slot.quantity += quantity;
			return;
		}
	}

	// 2. 빈 슬롯이 있으면 새로운 아이템 추가
	for (auto& slot : m_inventorySlots) {
		if (slot.IsEmpty()) {
			slot.item = newItem;
			slot.quantity = quantity;
			return;
		}
	}

	// 3. 인벤토리가 가득 찼을 경우
	OutputDebugStringA("인벤토리가 가득 찼습니다.\n");
}


ImTextureID CGameFramework::LoadIconTexture(const std::wstring& filename)
{
	ID3D12Resource* pTexture = nullptr;
	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA subresourceData = {};

	HRESULT hr = LoadWICTextureFromFile(
		m_pd3dDevice,
		filename.c_str(),
		&pTexture,
		decodedData,
		subresourceData
	);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"LoadWICTextureFromFile 실패!", L"Error", MB_OK);
		return (ImTextureID)nullptr;
	}

	// ⭐⭐ CommandList Reset ⭐⭐
	m_pd3dCommandAllocator->Reset();
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	// 📦 업로드용 임시 버퍼 생성
	D3D12_RESOURCE_DESC textureDesc = pTexture->GetDesc();
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(pTexture, 0, 1);

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = uploadBufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* pUploadBuffer = nullptr;
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
		MessageBox(NULL, L"UploadBuffer 생성 실패!", L"Error", MB_OK);
		return (ImTextureID)nullptr;
	}

	// 📦 서브리소스 업데이트
	UpdateSubresources(m_pd3dCommandList, pTexture, pUploadBuffer, 0, 0, 1, &subresourceData);

	// 📦 텍스처 barrier
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = pTexture;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_pd3dCommandList->ResourceBarrier(1, &barrier);

	// ⭐⭐ CommandList Close + Execute ⭐⭐
	m_pd3dCommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// ⭐⭐ GPU 작업 기다리기
	WaitForGpuComplete();

	// ➡️ SRV 만들기
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pd3dSrvDescriptorHeapForIcons->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += (m_nIconCount * m_nCbvSrvDescriptorIncrementSize);

	m_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, cpuHandle);

	// ➡️ 텍스처 핸들 리턴
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_pd3dSrvDescriptorHeapForIcons->GetGPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += (m_nIconCount * m_nCbvSrvDescriptorIncrementSize);

	m_nIconCount++;

	return reinterpret_cast<ImTextureID>(reinterpret_cast<void*>(gpuHandle.ptr));
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
	WaitForGpu();
	::CloseHandle(m_hFenceEvent);

    ReleaseObjects();

	m_pResourceManager->ReleaseAll();


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
	m_pResourceManager = std::make_unique<ResourceManager>();
	m_pResourceManager->Initialize(this);


	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pScene = new CScene(this);
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

	

#ifdef _WITH_TERRAIN_PLAYER
	CTerrainPlayer *pPlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->m_pTerrain, this);
#else
	CAirplanePlayer *pPlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), NULL);
	pPlayer->SetPosition(XMFLOAT3(425.0f, 240.0f, 640.0f));
#endif

	m_pScene->m_pPlayer = m_pPlayer = pPlayer;
	m_pCamera = m_pPlayer->GetCamera();
	m_pPlayer->SetOwningScene(m_pScene);

	pPlayer->SetOBB();
	pPlayer->InitializeOBBResources(m_pd3dDevice, m_pd3dCommandList);


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
	//for( auto& player : PlayerList)
	//{
	//	player.second->Release();
	//}
	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

#include <map>

void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	static std::map<UCHAR, bool> keyPressed; // 키별로 눌림 상태를 저장하는 맵
	static std::map<UCHAR, bool> toggleStates; // 키별로 토글 상태를 저장하는 맵
	bool bProcessedByScene = false;

	if (GetKeyboardState(pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
	if (!bProcessedByScene && m_pPlayer)
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

		PlayerInputData inputData;

		inputData.MoveForward = (pKeysBuffer[VK_UP] & 0xF0 || pKeysBuffer['W'] & 0xF0);
		inputData.MoveBackward = (pKeysBuffer[VK_DOWN] & 0xF0 || pKeysBuffer['S'] & 0xF0);
		inputData.WalkLeft = (pKeysBuffer[VK_LEFT] & 0xF0 || pKeysBuffer['A'] & 0xF0);
		inputData.WalkRight = (pKeysBuffer[VK_RIGHT] & 0xF0 || pKeysBuffer['D'] & 0xF0);
		inputData.Jump = (pKeysBuffer[VK_SPACE] & 0xF0);
		inputData.Attack = (pKeysBuffer['F'] & 0xF0); // 'F' 키를 Attack 으로 매핑 (예시)
		// inputData.Interact = (pKeysBuffer['E'] & 0xF0); // 'E' 키를 Interact 로 매핑 (필요시)
		inputData.Run = (pKeysBuffer[VK_SHIFT] & 0xF0); // Shift 키를 Run 으로 매핑

		if (m_pPlayer && m_pPlayer->m_pStateMachine) // 플레이어와 상태머신 유효성 검사
		{
			m_pPlayer->m_pStateMachine->HandleInput(inputData);
		}

		
		DWORD dwDirection = 0;
		if (pKeysBuffer[VK_UP] & 0xF0 || pKeysBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeysBuffer[VK_DOWN] & 0xF0 || pKeysBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeysBuffer[VK_LEFT] & 0xF0 || pKeysBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeysBuffer[VK_RIGHT] & 0xF0 || pKeysBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeysBuffer[VK_SPACE] & 0xF0) dwDirection |= DIR_UP;
		if (pKeysBuffer[VK_SHIFT] & 0xF0) dwDirection |= DIR_DOWN;
		else m_pPlayer->keyInput(pKeysBuffer);
		

		// 토글 처리할 키들을 배열 또는 다른 컨테이너에 저장
		UCHAR toggleKeys[] = { 'R','1','2','3' /*, 다른 키들 */};
		for (UCHAR key : toggleKeys)
		{
			if (pKeysBuffer[key] & 0xF0)
			{
				if (!keyPressed[key])
				{
					toggleStates[key] = !toggleStates[key];
					keyPressed[key] = true;
					// 토글된 상태에 따른 동작 수행
					if (key == 'R')
					{
						obbRender = toggleStates[key];
					}

					if (key == '1')
					{
						m_pPlayer->m_pSword->isRender = true;
						m_pPlayer->m_pAxe->isRender = false;
						m_pPlayer->weaponType = WeaponType::Sword;
					}

					if (key == '2')
					{
						m_pPlayer->m_pSword->isRender = false;
						m_pPlayer->m_pAxe->isRender = true;
						m_pPlayer->weaponType = WeaponType::Axe;
					}

					// 다른 키에 대한 처리 추가
				}
			}
			else
			{
				keyPressed[key] = false;
			}
		}

		for (int i = 0; i < 5; ++i)
		{
			if (GetAsyncKeyState('1' + i) & 0x8000)
			{
				m_SelectedHotbarIndex = i;
				break;
			}
		}
		// 카메라 모드에 따른 입력 처리 (기존 코드와 동일)
		if (m_pCamera->GetMode() == TOP_VIEW_CAMERA)
		{
			// 탑뷰: 마우스 휠로 줌인/줌아웃
			// 실제로는 마우스 휠 이벤트를 처리하려면 별도의 메시지 처리가 필요할 수 있음
			// 여기서는 예시로 키 입력으로 대체 (Q: 줌인, E: 줌아웃)
			if (pKeysBuffer['Q'] & 0xF0) {
				XMFLOAT3 offset = m_pCamera->GetOffset();
				offset.y = max(20.0f, offset.y - 10.0f);
				m_pCamera->SetOffset(offset);
			}
			if (pKeysBuffer['E'] & 0xF0) {
				XMFLOAT3 offset = m_pCamera->GetOffset();
				offset.y = min(200.0f, offset.y + 10.0f);  // 줌아웃, 최대 높이 200
				m_pCamera->SetOffset(offset);
			}
		}
		else if (m_pCamera->GetMode() == FIRST_PERSON_CAMERA || m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			// 자유 시점: 마우스로 회전
			if (beforeInput != inputData)
			{
				beforeInput = inputData;
				auto& nwManager = NetworkManager::GetInstance();
				INPUT_PACKET p;
				// 변경
				p.inputData.Attack = inputData.Attack;
				p.inputData.Jump = inputData.Jump;
				p.inputData.MoveBackward = inputData.MoveBackward;
				p.inputData.MoveForward = inputData.MoveForward;
				p.inputData.WalkLeft = inputData.WalkLeft;
				p.inputData.WalkRight = inputData.WalkRight;
				p.inputData.Run = inputData.Run;
				p.inputData.Interact = inputData.Interact;
				p.size = sizeof(INPUT_PACKET);
				p.type = static_cast<char>(E_PACKET::E_P_INPUT);
				nwManager.PushSendQueue(p, p.size);
			}

			if ( (cxDelta != 0.0f) || (cyDelta != 0.0f))
			{
				auto& nwManager = NetworkManager::GetInstance();

				if (cxDelta || cyDelta)
				{
					if (pKeysBuffer[VK_RBUTTON] & 0xF0)
						m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
					else
						m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
					{
						ROTATE_PACKET p;
						auto& lookv = m_pPlayer->GetLookVector();
						p.look.x = lookv.x;
						p.look.y = lookv.y;
						p.look.z = lookv.z;
						auto& rightv = m_pPlayer->GetRightVector();
						p.right.x = rightv.x;
						p.right.y = rightv.y;
						p.right.z = rightv.z;
						auto& upv = m_pPlayer->GetUpVector();
						p.up.x = upv.x;
						p.up.y = upv.y;
						p.up.z = upv.z;
						p.size = sizeof(ROTATE_PACKET);
						p.type = static_cast<char>(E_PACKET::E_P_ROTATE);

						nwManager.PushSendQueue(p, p.size);
					}

				}
			}
		}
	}
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
	if (m_logQueue.size() > 0) {
		m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
		while (m_logQueue.size() > 0) {
			auto log = m_logQueue.front();
			m_logQueue.pop();
			switch (log.packetType)
			{
			case E_PACKET::E_P_LOGIN:
			{
				CLoadedModelInfo* pUserModel = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, "Model/SK_Hu_M_FullBody.bin", this);
				int animate_count = 15;
				m_pScene->PlayerList[log.ID] = std::make_unique<UserObject>(m_pd3dDevice, m_pd3dCommandList, pUserModel, animate_count, this);
				m_pScene->PlayerList[log.ID]->m_objectType = GameObjectType::Player;
				m_pScene->PlayerList[log.ID]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
				for (int j = 1; j < animate_count; ++j) {
					m_pScene->PlayerList[log.ID]->m_pSkinnedAnimationController->SetTrackAnimationSet(j, j);
					m_pScene->PlayerList[log.ID]->m_pSkinnedAnimationController->SetTrackEnable(j, false);
				}
				m_pScene->PlayerList[log.ID]->on_track = 0;
				m_pScene->PlayerList[log.ID]->SetPosition(XMFLOAT3{ 1500.f,m_pScene->m_pTerrain->GetHeight(1500,1500) ,1500.f });
				m_pScene->PlayerList[log.ID]->SetScale(10.0f, 10.0f, 10.0f);
				m_pScene->PlayerList[log.ID]->SetTerraindata(m_pScene->m_pTerrain);
				if (m_pScene->PlayerList[log.ID]->m_pSkinnedAnimationController) m_pScene->PlayerList[log.ID]->PropagateAnimController(m_pScene->PlayerList[log.ID]->m_pSkinnedAnimationController);
				if (pUserModel) delete(pUserModel);
			}
			break;
			case E_PACKET::E_P_LOGOUT:
			{
				if (m_pScene->PlayerList.find(log.ID) != m_pScene->PlayerList.end()) {
					//m_pScene->PlayerList[log.ID]->Release();
					m_pScene->PlayerList.erase(log.ID);
				}
			}
			break;
			default:
				break;
			}
		}
		m_pd3dCommandList->Close();
		ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
		m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

		WaitForGpuComplete();

		for(auto& player : m_pScene->PlayerList)
		{
			if (player.second) player.second->ReleaseUploadBuffers();
		}
	}

	m_GameTimer.Tick(60.0f);

	ProcessInput();
	UpdateFurnace(m_GameTimer.GetTimeElapsed());
    AnimateObjects();

	if (m_pConstructionSystem->IsBuildMode()) {
		m_pConstructionSystem->UpdatePreviewPosition(m_pCamera);
	}
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());

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

		ImGui::PushID(i);

		if (i == m_SelectedHotbarIndex)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 0.5f)); // 노란색 반투명

		if (!m_inventorySlots[i].IsEmpty())
		{
			// 버튼 먼저 생성 (테두리 유지)
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImGui::Button(" ", ImVec2(SlotSize, SlotSize));

			// 버튼 위에 아이콘을 따로 그리기
			ImTextureID icon = m_inventorySlots[i].item->GetIconHandle();
			if (icon)
			{
				ImGui::GetWindowDrawList()->AddImage(
					icon,
					pos,
					ImVec2(pos.x + SlotSize, pos.y + SlotSize)
				);
			}
		}
		else
		{
			ImGui::Button(" ", ImVec2(SlotSize, SlotSize)); // 빈 슬롯은 그냥 테두리만
		}
		if (i == m_SelectedHotbarIndex)
			ImGui::PopStyleColor();

		ImGui::PopID();
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
	ImGui::Text("Hp"); // 체력 
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
	ImGui::Text("Stamina"); // 스태미너
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
	ImGui::Text("Hunger"); // 허기
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
	ImGui::ProgressBar(m_pPlayer->PlayerHunger, ImVec2(barWidth, barHeight));
	ImGui::PopStyleColor();
	ImGui::EndGroup();

	ImGui::SameLine(0.0f, 50.0f);

	ImGui::BeginGroup();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Thirst"); // 갈증
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

		ImGui::SetNextWindowPos(invPos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(invSize, ImGuiCond_FirstUseEver);
		ImGui::Begin("Inventory", nullptr,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse);

		// ▶ 2열로 나누기
		ImGui::Columns(2, nullptr, false);

		// 왼쪽: 인벤토리 슬롯
		{
			static int selectedSlotIndex = -1;

			for (int i = 0; i < m_inventorySlots.size(); ++i)
			{
				ImGui::PushID(i);

				bool isClicked = false;

				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImGui::Button(" ", ImVec2(slotSize, slotSize)); // 버튼만 깔아줌 (배경 유지용)

				if (!m_inventorySlots[i].IsEmpty())
				{
					// 아이콘 출력
					Item* item = m_inventorySlots[i].item.get();
					ImTextureID icon = item->GetIconHandle();
					if (icon)
					{
						ImGui::GetWindowDrawList()->AddImage(
							icon,
							pos,
							ImVec2(pos.x + slotSize, pos.y + slotSize)
						);
					}

					// 수량 텍스트 표시
					ImVec2 min = pos;
					ImVec2 max = ImVec2(pos.x + slotSize, pos.y + slotSize);
					ImVec2 textPos = ImVec2(min.x + 2, max.y - 18);
					ImGui::GetWindowDrawList()->AddText(
						textPos,
						IM_COL32_WHITE,
						std::to_string(m_inventorySlots[i].quantity).c_str()
					);
				}

				// ✅ 선택 테두리 강조
				if (i == selectedSlotIndex)
				{
					ImGui::GetWindowDrawList()->AddRect(
						pos,
						ImVec2(pos.x + slotSize, pos.y + slotSize),
						IM_COL32(255, 255, 0, 255), // 노란색
						0.0f,
						0,
						2.0f
					);
				}

				// 🔁 클릭 시 교환 처리
				if (ImGui::IsItemClicked())
				{
					if (ShowFurnaceUI)  // 🔥 화로 UI가 열려 있는 경우
					{
						Item* item = m_inventorySlots[i].item.get();
						if (!item) return;

						std::string name = item->GetName();

						if (name == "coal" || name == "wood") {
							furnaceSlot.fuelAmount += 25.0f;
							if (furnaceSlot.fuelAmount > 100.0f) furnaceSlot.fuelAmount = 100.0f;
							m_inventorySlots[i].quantity--;
							if (m_inventorySlots[i].quantity <= 0) m_inventorySlots[i].item = nullptr;
						}
						else if (name == "pork" || name == "iron_material") {
							furnaceSlot.material = item;
							m_inventorySlots[i].quantity--;
							if (m_inventorySlots[i].quantity <= 0) m_inventorySlots[i].item = nullptr;
						}
					}
					else  // 🔄 화로 UI가 닫혀 있는 경우: 인벤토리 슬롯 교환
					{
						if (selectedSlotIndex == -1) {
							selectedSlotIndex = i;
						}
						else {
							std::swap(m_inventorySlots[selectedSlotIndex], m_inventorySlots[i]);
							selectedSlotIndex = -1;
						}
					}
				}

				ImGui::PopID();

				if ((i + 1) % inventoryCols != 0)
					ImGui::SameLine(0.0f, spacing);
			}


		}

		ImGui::NextColumn();

		// 오른쪽: 플레이어 스탯
		{
			ImGui::Text("LV: %d", m_pPlayer->PlayerLevel);
			ImGui::Text("STATUS:");

			ImGui::BulletText("HP: %d / %d", m_pPlayer->Playerhp, m_pPlayer->Maxhp);
			ImGui::SameLine();
			if (m_pPlayer->StatPoint > 0) {
				if (ImGui::Button("+##hp")) { m_pPlayer->Maxhp += 10; m_pPlayer->StatPoint--; m_pPlayer->Playerhp += 10; }
			}
			else {
				ImGui::BeginDisabled(); ImGui::Button("+##hp"); ImGui::EndDisabled();
			}

			ImGui::BulletText("STAMINA: %d / %d", m_pPlayer->Playerstamina, m_pPlayer->Maxstamina);
			ImGui::SameLine();
			if (m_pPlayer->StatPoint > 0) {
				if (ImGui::Button("+##stamina")) { m_pPlayer->Maxstamina += 10; m_pPlayer->StatPoint--; m_pPlayer->Playerstamina += 10; }
			}
			else {
				ImGui::BeginDisabled(); ImGui::Button("+##stamina"); ImGui::EndDisabled();
			}

			ImGui::BulletText("ATK: %d", m_pPlayer->PlayerAttack);
			ImGui::SameLine();
			if (m_pPlayer->StatPoint > 0) {
				if (ImGui::Button("+##atk")) { m_pPlayer->PlayerAttack += 1; m_pPlayer->StatPoint--; }
			}
			else {
				ImGui::BeginDisabled(); ImGui::Button("+##atk"); ImGui::EndDisabled();
			}

			ImGui::BulletText("SPEED: %.1f", m_pPlayer->PlayerSpeed);
			ImGui::SameLine();
			if (m_pPlayer->StatPoint > 0) {
				if (ImGui::Button("+##speed")) { m_pPlayer->PlayerSpeed += 0.2f; m_pPlayer->StatPoint--; }
			}
			else {
				ImGui::BeginDisabled(); ImGui::Button("+##speed"); ImGui::EndDisabled();
			}
			ImGui::BulletText("XP: %d / %d", m_pPlayer->Playerxp, m_pPlayer->Totalxp);
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text("STAT POINT: %d", m_pPlayer->StatPoint);
		}

		ImGui::Columns(1); // 열 정리
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

		ImGui::Begin("Crafting", nullptr,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse);

		
		ImGui::Columns(2, nullptr, false);

		// 조합 가능한 아이템 리스트
		ImGui::Text("CRAFT ITEM");
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
		ImGui::Text("MATERIAL");
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

			if (ImGui::Button("CRAFT", ImVec2(200, 50)))
			{
				if (CanCraftItem())
				{
					CraftSelectedItem();
				}
			}
		}

		ImGui::Columns(1);

		ImGui::End();
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////// 건축 UI
	if (BuildMode)
	{
		ImGui::SetNextWindowPos(ImVec2(100, 100));
		ImGui::SetNextWindowSize(ImVec2(200, 200));
		ImGui::Begin("Build Mode", nullptr,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

		static int selected = -1;
		const char* buildings[] = { "나무 벽", "나무 문", "나무 바닥", "계단" };
		//static bool bPrevBuildMode = false;

		if (BuildMode && !bPrevBuildMode)
		{
			m_pConstructionSystem->EnterBuildMode(); // 상태 전환 시 한 번만 실행
		}
		bPrevBuildMode = BuildMode;
		/*
		for (int i = 0; i < IM_ARRAYSIZE(buildings); i++)
		{
			if (ImGui::Selectable(buildings[i], selected == i))
			{
				selected = i;

				// 1. 문자열 매핑
				std::string buildingKey;
				if (selected == 0) buildingKey = "pine";
				else if (selected == 1) buildingKey = "door";
				else if (selected == 2) buildingKey = "floor";
				else if (selected == 3) buildingKey = "stair";


				// 3. 미리보기 재생성
			}
		}
		*/

		if (m_pConstructionSystem->IsBuildMode())
		{
			XMFLOAT3 previewPos = m_pConstructionSystem->GetPreviewPosition(); // ★ getter 함수 필요
			ImGui::Text("PreviewPos: %.2f, % .2f, % .2f", previewPos.x, previewPos.y, previewPos.z);
		}
		if (ImGui::Button("Build End"))
		{
			BuildMode = false;
			bPrevBuildMode = BuildMode;
			m_pConstructionSystem->ExitBuildMode();
		}

		ImGui::End();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	if (ShowFurnaceUI)
	{
		const float slotSize = 72.0f;
		const ImVec2 slotVec = ImVec2(slotSize, slotSize);

		ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		ImVec2 windowSize = ImVec2(360, 330);
		ImVec2 centerPos = ImVec2(
			(displaySize.x - windowSize.x) * 0.5f,
			(displaySize.y - windowSize.y) * 0.5f
		);

		ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

		ImGui::Begin("Furnace", &ShowFurnaceUI, ImGuiWindowFlags_NoResize);

		ImGui::Text("FURANCE");
		ImGui::Separator();

		ImGui::SetCursorPos(ImVec2(60, 80));
		ImGui::Button(" ", slotVec); // 항상 배경 존재

		if (furnaceSlot.material)
		{
			ImTextureID matIcon = furnaceSlot.material->GetIconHandle();
			ImVec2 pos = ImGui::GetItemRectMin();

			ImGui::GetWindowDrawList()->AddImage(
				matIcon,
				pos,
				ImVec2(pos.x + 70, pos.y + 70)
			);
		}

		// 🔥 연료 아이콘
		ImGui::SetCursorPos(ImVec2(60, 200));
		ImGui::Text("FUEL");

		ImGui::SetCursorPos(ImVec2(60, 220));
		ImGui::ProgressBar(furnaceSlot.fuelAmount / 100.0f, ImVec2(150, 20));
		
		auto fireItem = ItemManager::GetItemByName("fire");
		if (fireItem)
		{
			ImTextureID fireIcon = fireItem->GetIconHandle();
			ImGui::SetCursorPos(ImVec2(72, 150)); // 중앙 위치
			ImGui::GetWindowDrawList()->AddImage(
				fireIcon,
				ImGui::GetCursorScreenPos(),
				ImVec2(ImGui::GetCursorScreenPos().x + 48, ImGui::GetCursorScreenPos().y + 48)
			);
		}

		
		auto directionItem = ItemManager::GetItemByName("direction");
		if (directionItem)
		{
			ImTextureID arrowIcon = directionItem->GetIconHandle();
			ImGui::SetCursorPos(ImVec2(190, 150)); 
			ImGui::GetWindowDrawList()->AddImage(
				arrowIcon,
				ImGui::GetCursorScreenPos(),
				ImVec2(ImGui::GetCursorScreenPos().x + 60, ImGui::GetCursorScreenPos().y + 60)
			);
		}

		if (furnaceSlot.result)
		{
			ImTextureID resultIcon = furnaceSlot.result->GetIconHandle();
			string resultitem = furnaceSlot.result->GetName();
			ImGui::SetCursorPos(ImVec2(260, 150)); // 결과 슬롯 위치
			ImGui::Image(resultIcon, slotVec);

			if (ImGui::IsItemClicked())
			{
				AddItem(resultitem, 1);
				furnaceSlot.result = nullptr;
			}
		}
		else
		{
			ImGui::SetCursorPos(ImVec2(260, 150));
			ImGui::Button("RESULT", slotVec);
		}

		ImGui::End();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
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

	m_GameTimer.GetFrameRate(m_pszFrameRate + 18, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	//_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(%4f, %4f, %4f)"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
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
		"stick",
		{ {"wood", 2} },
		2
		});

	// 제작대
	m_vecCraftableItems.push_back({
		"crafting_table",
		{ {"wood", 4} },
		1
		});

	// 화로
	m_vecCraftableItems.push_back({
		"furance",
		{ {"stone", 8} },
		1
		});

	// 그릇
	m_vecCraftableItems.push_back({
		"bowl",
		{ {"wood", 1} },
		1
		});

	// 횃불
	m_vecCraftableItems.push_back({
		"torch",
		{ {"stick", 1}, {"coal", 1} },
		2
		});

	// 나무 곡괭이
	m_vecCraftableItems.push_back({
		"wooden_pickaxe",
		{ {"stick", 2}, {"wood", 3} },
		1
		});

	// 나무 도끼
	m_vecCraftableItems.push_back({
		"wooden_axe",
		{ {"stick", 2}, {"wood", 3} },
		1
		});

	// 나무 검
	m_vecCraftableItems.push_back({
		"wooden_sword",
		{ {"stick", 1}, {"wood", 2} },
		1
		});

	// 나무 망치
	m_vecCraftableItems.push_back({
		"wooden_hammer",
		{ {"stick", 2}, {"wood", 3} },
		1
		});

	// 돌 곡괭이
	m_vecCraftableItems.push_back({
		"stone_pickaxe",
		{ {"stick", 2}, {"stone", 3} },
		1
		});

	// 돌 도끼
	m_vecCraftableItems.push_back({
		"stone_axe",
		{ {"stick", 2}, {"stone", 3} },
		1
		});

	// 돌 검
	m_vecCraftableItems.push_back({
		"stone_sword",
		{ {"stick", 1}, {"stone", 2} },
		1
		});

	// 돌 망치
	m_vecCraftableItems.push_back({
		"stone_hammer",
		{ {"stick", 2}, {"stone", 3} },
		1
		});

	// 철 곡괭이
	m_vecCraftableItems.push_back({
		"iron_pickaxe",
		{ {"stick", 2}, {"iron", 3} },
		1
		});

	// 철 도끼
	m_vecCraftableItems.push_back({
		"iron_axe",
		{ {"stick", 2}, {"iron", 3} },
		1
		});

	// 철 검
	m_vecCraftableItems.push_back({
		"iron_sword",
		{ {"stick", 1}, {"iron", 2} },
		1
		});

	// 철 망치
	m_vecCraftableItems.push_back({
		"iron_hammer",
		{ {"stick", 2}, {"iron", 3} },
		1
		});
}
bool CGameFramework::CanCraftItem()
{
	if (selectedCraftItemIndex < 0 || selectedCraftItemIndex >= m_vecCraftableItems.size())
		return false; // 잘못된 인덱스

	const CraftItem& selectedItem = m_vecCraftableItems[selectedCraftItemIndex];

	// 필요한 재료들 다 검사
	for (const CraftMaterial& material : selectedItem.Materials)
	{
		int requiredQuantity = material.Quantity;
		int playerQuantity = 0;

		// 인벤토리를 돌면서 같은 재료 찾기
		for (const InventorySlot& slot : m_inventorySlots)
		{
			if (!slot.IsEmpty() && slot.item->GetName() == material.MaterialName)
			{
				playerQuantity += slot.quantity;
			}
		}

		// 한 가지라도 부족하면 바로 실패
		if (playerQuantity < requiredQuantity)
			return false;
	}

	// 모든 재료가 충분하면 성공
	return true;
}

void CGameFramework::CraftSelectedItem()
{
	if (selectedCraftItemIndex < 0 || selectedCraftItemIndex >= m_vecCraftableItems.size())
		return;

	const CraftItem& selectedItem = m_vecCraftableItems[selectedCraftItemIndex];

	// 1. 필요한 재료 차감
	for (const CraftMaterial& material : selectedItem.Materials)
	{
		int remaining = material.Quantity;

		for (InventorySlot& slot : m_inventorySlots)
		{
			if (!slot.IsEmpty() && slot.item->GetName() == material.MaterialName)
			{
				if (slot.quantity >= remaining)
				{
					slot.quantity -= remaining;
					if (slot.quantity == 0)
						slot.item = nullptr;
					break;
				}
				else
				{
					remaining -= slot.quantity;
					slot.item = nullptr;
					slot.quantity = 0;
				}
			}
		}
	}

	// 2. 결과 아이템 추가
	const std::string& itemName = selectedItem.ResultItemName;
	std::shared_ptr<Item> newItem = ItemManager::GetItemByName(itemName);
	if (!newItem) return;

	// 먼저 동일한 아이템이 있는 슬롯 찾기 → 수량만 증가
	for (InventorySlot& slot : m_inventorySlots)
	{
		if (!slot.IsEmpty() && slot.item->GetName() == itemName)
		{
			slot.quantity += selectedItem.ResultQuantity;
			return;
		}
	}

	// 없으면 새 슬롯에 추가
	for (InventorySlot& slot : m_inventorySlots)
	{
		if (slot.IsEmpty())
		{
			slot.item = newItem;
			slot.quantity = selectedItem.ResultQuantity;
			return;
		}
	}
}

void CGameFramework::InitializeItemIcons()
{
	auto& items = ItemManager::GetItems();

	for (auto& item : items)
	{
		std::string itemName = item->GetName(); // ex: "wood"
		std::wstring wItemName(itemName.begin(), itemName.end());
		std::wstring iconPath = L"ICON/" + wItemName + L".png";

		ImTextureID iconHandle = LoadIconTexture(iconPath.c_str());
		item->SetIconHandle(iconHandle);
	}
}

void CGameFramework::UpdateFurnace(float deltaTime)
{
	// 재료가 없거나 연료가 0이면 리턴
	if (!furnaceSlot.material || furnaceSlot.fuelAmount <= 0.0f)
		return;

	std::string mat = furnaceSlot.material->GetName();

	// 제련 가능 재료인지 확인
	if (mat != "pork" && mat != "iron_material") return;

	// 🔥 연료 소모
	furnaceSlot.fuelAmount -= deltaTime * 5.0f; // 연료 소모 속도 (초당 5 소비)
	if (furnaceSlot.fuelAmount < 0.0f)
		furnaceSlot.fuelAmount = 0.0f;

	// ⏱ 제련 시간 누적
	furnaceSlot.smeltTime += deltaTime;
	const float requiredTime = 2.0f;

	if (furnaceSlot.smeltTime >= requiredTime)
	{
		if (mat == "pork")
			furnaceSlot.result = ItemManager::GetItemByName("grill_pork").get();
		else if (mat == "iron_material")
			furnaceSlot.result = ItemManager::GetItemByName("iron").get();

		// ⏹ 완료 후 초기화
		furnaceSlot.material = nullptr;
		furnaceSlot.smeltTime = 0.0f;
	}
}

void CGameFramework::CreateCbvSrvDescriptorHeaps(int nConstantBufferViews, int nShaderResourceViews)
{
	// --- 1. 힙 생성 ---
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; // CBVs + SRVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&m_pd3dCbvSrvDescriptorHeap));

	// ---  시작 핸들 저장 및 카운트 저장 ---
	m_nTotalCbvDescriptors = nConstantBufferViews;
	m_nTotalSrvDescriptors = nShaderResourceViews;

	// 힙 전체의 시작 핸들을 얻어옵니다. 이것이 CBV 섹션의 시작 핸들이 됩니다.
	m_d3dCbvCpuHandleStart = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGpuHandleStart = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	// SRV 섹션의 시작 핸들을 계산합니다. (CBV 섹션 바로 다음)
	m_d3dSrvCpuHandleStart = m_d3dCbvCpuHandleStart; // 힙 시작에서
	// CBV 개수 * 디스크립터 크기 만큼 오프셋을 더합니다.
	m_d3dSrvCpuHandleStart.ptr += (UINT64)m_nTotalCbvDescriptors * m_nCbvSrvDescriptorIncrementSize;

	m_d3dSrvGpuHandleStart = m_d3dCbvGpuHandleStart; // 힙 시작에서
	// CBV 개수 * 디스크립터 크기 만큼 오프셋을 더합니다.
	m_d3dSrvGpuHandleStart.ptr += (UINT64)m_nTotalCbvDescriptors * m_nCbvSrvDescriptorIncrementSize;

	// --- 4. 오프셋 초기화 ---
	// 각 섹션에서 할당을 시작할 위치(오프셋)를 0으로 설정합니다.
	m_nNextCbvOffset = 0;
	m_nNextSrvOffset = 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE CGameFramework::CreateConstantBufferViews(int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	if (nConstantBufferViews <= 0 || pd3dConstantBuffers == nullptr)
	{
		return { 0 }; // 유효하지 않은 입력
	}


	D3D12_CPU_DESCRIPTOR_HANDLE cpuStartHandle = { 0 };
	D3D12_GPU_DESCRIPTOR_HANDLE gpuStartHandle = { 0 };
	// --- 1. 프레임워크의 할당 함수를 호출하여 디스크립터 슬롯 확보 ---
	if (!AllocateCbvDescriptors(nConstantBufferViews, cpuStartHandle, gpuStartHandle))
	{
		// 힙 공간 부족 등의 이유로 할당 실패
		// 오류 처리 (로그 남기기, 예외 발생 등)
		OutputDebugString(L"Failed to allocate CBV descriptors.\n");
		return { 0 }; // 실패 시 유효하지 않은 핸들 반환
	}

	// 상수 버퍼 정보 설정
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride; // CBV에서 볼 버퍼의 크기

	// 할당받은 시작 CPU 핸들부터 순차적으로 CBV 생성
	D3D12_CPU_DESCRIPTOR_HANDLE currentCpuHandle = cpuStartHandle;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		// 현재 CBV가 참조할 버퍼 위치 계산
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + ((UINT64)nStride * j); // UINT64 사용 권장

		// 현재 CPU 핸들 위치에 CBV 생성
		// m_pd3dDevice는 CGameFramework의 멤버라고 가정
		m_pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, currentCpuHandle);

		// 다음 디스크립터 슬롯으로 CPU 핸들 이동
		currentCpuHandle.ptr += m_nCbvSrvDescriptorIncrementSize; // 멤버 변수 사용
	}

	// --- 2. 할당된 블록의 시작 GPU 핸들 반환 ---
	// 이 핸들은 나중에 루트 시그니처에 바인딩할 때 사용될 수 있습니다. (예: 첫 번째 CBV 주소)
	return gpuStartHandle;
}

bool CGameFramework::AllocateCbvDescriptors(UINT nDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE& outCpuStartHandle, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuStartHandle) {
	if (m_nNextCbvOffset + nDescriptors > m_nTotalCbvDescriptors) {
		// 공간 부족!
		return false;
	}
	// 시작 핸들 계산 (SRV 영역 시작 + 현재 오프셋)
	outCpuStartHandle = m_d3dCbvCpuHandleStart;
	outCpuStartHandle.ptr += (UINT64)m_nNextCbvOffset * m_nCbvSrvDescriptorIncrementSize;

	outGpuStartHandle = m_d3dCbvGpuHandleStart;
	outGpuStartHandle.ptr += (UINT64)m_nNextCbvOffset * m_nCbvSrvDescriptorIncrementSize;

	// 다음 오프셋 업데이트
	m_nNextSrvOffset += nDescriptors;
	return true;
}

bool CGameFramework::AllocateSrvDescriptors(UINT nDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE& outCpuStartHandle, D3D12_GPU_DESCRIPTOR_HANDLE& outGpuStartHandle) {
	// --- 로그 추가 ---
	wchar_t buffer[128];
	swprintf_s(buffer, L"AllocateSrvDescriptors: Requesting %u slots. Current offset: %u / Total SRV slots: %u\n",
		nDescriptors, m_nNextSrvOffset, m_nTotalSrvDescriptors);
	OutputDebugStringW(buffer);
	// --- 로그 추가 ---

	if (m_nNextSrvOffset + nDescriptors > m_nTotalSrvDescriptors) {
		OutputDebugStringW(L"    --> Allocation FAILED: Not enough space in SRV heap!\n"); // 실패 로그
		return false; // 공간 부족!
	}

	outCpuStartHandle = m_d3dSrvCpuHandleStart;
	outCpuStartHandle.ptr += (UINT64)m_nNextSrvOffset * m_nCbvSrvDescriptorIncrementSize;

	outGpuStartHandle = m_d3dSrvGpuHandleStart;
	outGpuStartHandle.ptr += (UINT64)m_nNextSrvOffset * m_nCbvSrvDescriptorIncrementSize;

	// 다음 오프셋 업데이트
	m_nNextSrvOffset += nDescriptors;

	// --- 성공 로그 추가 ---
	swprintf_s(buffer, L"    --> Allocation SUCCEEDED. New offset: %u. Start GPU Handle: %p\n",
		m_nNextSrvOffset, (void*)outGpuStartHandle.ptr);
	OutputDebugStringW(buffer);
	// --- 성공 로그 추가 ---

	return true;
}


void CGameFramework::WaitForGpu()
{
	// 필수 객체들 유효성 검사
	if (!m_pd3dCommandQueue || !m_pd3dFence || m_hFenceEvent == INVALID_HANDLE_VALUE) {
		OutputDebugString(L"Warning: Cannot wait for GPU, essential objects are missing.\n");
		return;
	}

	// 1. 새로운 마스터 펜스 값을 사용하여 Signal 명령 예약
	m_nMasterFenceValue++; // 이전 값보다 1 증가된 새 값 사용
	UINT64 fenceValueToSignal = m_nMasterFenceValue;

	HRESULT hr = m_pd3dCommandQueue->Signal(m_pd3dFence, fenceValueToSignal);
	if (FAILED(hr)) {
		OutputDebugString(L"!!!!!!!! ERROR: CommandQueue->Signal failed! !!!!!!!!\n");
		return; // Signal 실패 시 대기 불가
	}

	// 2. GPU가 해당 펜스 값에 도달했는지 확인
	if (m_pd3dFence->GetCompletedValue() < fenceValueToSignal)
	{
		// 3. 아직 도달하지 못했다면, 이벤트 핸들을 사용하여 대기
		hr = m_pd3dFence->SetEventOnCompletion(fenceValueToSignal, m_hFenceEvent);
		if (FAILED(hr)) {
			OutputDebugString(L"!!!!!!!! ERROR: Fence->SetEventOnCompletion failed! !!!!!!!!\n");
			return; // 이벤트 설정 실패 시 대기 불가
		}

		// 4. 이벤트가 Signal 상태가 될 때까지 무한 대기
		OutputDebugString(L"Waiting for GPU to finish...\n");
		WaitForSingleObjectEx(m_hFenceEvent, INFINITE, FALSE);
		OutputDebugString(L"GPU finished.\n");
	}
	else {
		// 이미 GPU가 해당 값 이상으로 진행함 (바로 종료 가능)
		OutputDebugString(L"GPU already finished (Fence value check).\n");
	}
}
