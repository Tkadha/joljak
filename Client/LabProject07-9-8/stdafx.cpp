// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// LabProject03-1.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"

#include "DDSTextureLoader12.h"
#include "WICTextureLoader12.h"

UINT gnCbvSrvDescriptorIncrementSize = 0;
UINT gnRtvDescriptorIncrementSize = 0;
UINT gnDsvDescriptorIncrementSize = 0;

void WaitForGpuComplete(ID3D12CommandQueue* pd3dCommandQueue, ID3D12Fence* pd3dFence, UINT64 nFenceValue, HANDLE hFenceEvent)
{
	HRESULT hResult = pd3dCommandQueue->Signal(pd3dFence, nFenceValue);

	if (pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = pd3dFence->SetEventOnCompletion(nFenceValue, hFenceEvent);
		::WaitForSingleObject(hFenceEvent, INFINITE);
	}
}

void ExecuteCommandList(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Fence* pd3dFence, UINT64 nFenceValue, HANDLE hFenceEvent)
{
	pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { pd3dCommandList };
	pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	::WaitForGpuComplete(pd3dCommandQueue, pd3dFence, nFenceValue, hFenceEvent);
}

void SynchronizeResourceTransition(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* pd3dResource, D3D12_RESOURCE_STATES d3dStateBefore, D3D12_RESOURCE_STATES d3dStateAfter)
{
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = pd3dResource;
	d3dResourceBarrier.Transition.StateBefore = d3dStateBefore;
	d3dResourceBarrier.Transition.StateAfter = d3dStateAfter;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
}

ID3D12Resource* CreateTextureResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nBytes, D3D12_RESOURCE_DIMENSION d3dResourceDimension, UINT nWidth, UINT nHeight, UINT nDepthOrArraySize, UINT nMipLevels, D3D12_RESOURCE_FLAGS d3dResourceFlags, DXGI_FORMAT dxgiFormat, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, ID3D12Resource** ppd3dUploadBuffer)
{
	ID3D12Resource* pd3dBuffer = NULL;

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = d3dHeapType;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = d3dResourceDimension; //D3D12_RESOURCE_DIMENSION_BUFFER, D3D12_RESOURCE_DIMENSION_TEXTURE1D, D3D12_RESOURCE_DIMENSION_TEXTURE2D
	d3dResourceDesc.Alignment = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT : 0;
	d3dResourceDesc.Width = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? nBytes : nWidth;
	d3dResourceDesc.Height = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? 1 : nHeight;
	d3dResourceDesc.DepthOrArraySize = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? 1 : nDepthOrArraySize;
	d3dResourceDesc.MipLevels = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? 1 : nMipLevels;
	d3dResourceDesc.Format = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? DXGI_FORMAT_UNKNOWN : dxgiFormat;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = d3dResourceFlags; //D3D12_RESOURCE_FLAG_NONE

	switch (d3dHeapType)
	{
	case D3D12_HEAP_TYPE_DEFAULT:
	{		
		D3D12_RESOURCE_STATES d3dResourceInitialStates = (ppd3dUploadBuffer && pData) ? D3D12_RESOURCE_STATE_COMMON : d3dResourceStates;
//		D3D12_RESOURCE_STATES d3dResourceInitialStates = (ppd3dUploadBuffer && pData) ? D3D12_RESOURCE_STATE_COPY_DEST : d3dResourceStates;
		HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, d3dResourceInitialStates, NULL, __uuidof(ID3D12Resource), (void**)&pd3dBuffer);
		if (ppd3dUploadBuffer && pData)
		{
			d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;

			d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			d3dResourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			d3dResourceDesc.Width = nBytes;
			d3dResourceDesc.Height = 1;
			d3dResourceDesc.DepthOrArraySize = 1;
			d3dResourceDesc.MipLevels = 1;
			d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
			d3dResourceDesc.SampleDesc.Count = 1;
			d3dResourceDesc.SampleDesc.Quality = 0;
			d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);
#ifdef _WITH_MAPPING
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			(*ppd3dUploadBuffer)->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pData, nBytes);
			(*ppd3dUploadBuffer)->Unmap(0, NULL);

			pd3dCommandList->CopyResource(pd3dBuffer, *ppd3dUploadBuffer);
#else
			D3D12_SUBRESOURCE_DATA d3dSubResourceData;
			::ZeroMemory(&d3dSubResourceData, sizeof(D3D12_SUBRESOURCE_DATA));
			d3dSubResourceData.pData = pData;
			d3dSubResourceData.SlicePitch = d3dSubResourceData.RowPitch = nBytes;
			::UpdateSubresources<1>(pd3dCommandList, pd3dBuffer, *ppd3dUploadBuffer, 0, 0, 1, &d3dSubResourceData);
#endif
			D3D12_RESOURCE_BARRIER d3dResourceBarrier;
			::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
			d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			d3dResourceBarrier.Transition.pResource = pd3dBuffer;
			d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;
			d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
		}
		break;
	}
	case D3D12_HEAP_TYPE_UPLOAD:
	{
		d3dResourceStates |= D3D12_RESOURCE_STATE_GENERIC_READ;
		HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, d3dResourceStates, NULL, __uuidof(ID3D12Resource), (void**)&pd3dBuffer);
		if (pData)
		{
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			pd3dBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pData, nBytes);
			pd3dBuffer->Unmap(0, NULL);
		}
		break;
	}
	case D3D12_HEAP_TYPE_READBACK:
	{
		d3dResourceStates |= D3D12_RESOURCE_STATE_COPY_DEST;
		HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, d3dResourceStates, NULL, __uuidof(ID3D12Resource), (void**)&pd3dBuffer);
		if (pData)
		{
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			pd3dBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pData, nBytes);
			pd3dBuffer->Unmap(0, NULL);
		}
		break;
	}
	}
	return(pd3dBuffer);
}

ID3D12Resource* CreateBufferResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nBytes, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, ID3D12Resource** ppd3dUploadBuffer)
{
	return(CreateTextureResource(pd3dDevice, pd3dCommandList, pData, nBytes, D3D12_RESOURCE_DIMENSION_BUFFER, nBytes, 1, 1, 1, D3D12_RESOURCE_FLAG_NONE, DXGI_FORMAT_UNKNOWN, d3dHeapType, d3dResourceStates, ppd3dUploadBuffer));
}

ID3D12Resource* CreateTextureResourceFromDDSFile(ID3D12Device* pd3dDevice, 
	ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, 
	ID3D12Resource** ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates)
{
	ID3D12Resource* pd3dTexture = NULL;
	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> vSubresources;
	DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
	bool bIsCubeMap = false;

	HRESULT hResult = DirectX::LoadDDSTextureFromFileEx(pd3dDevice, pszFileName, 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &pd3dTexture, ddsData, vSubresources, &ddsAlphaMode, &bIsCubeMap);

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	//D3D12_RESOURCE_DESC d3dTextureResourceDesc = pd3dTexture->GetDesc();
	//UINT nSubResources = d3dTextureResourceDesc.DepthOrArraySize * d3dTextureResourceDesc.MipLevels;
	UINT nSubResources = (UINT)vSubresources.size();
	//UINT64 nBytes = 0;
	//pd3dDevice->GetCopyableFootprints(&d3dResourceDesc, 0, nSubResources, 0, NULL, NULL, NULL, &nBytes);
	UINT64 nBytes = GetRequiredIntermediateSize(pd3dTexture, 0, nSubResources);

	D3D12_RESOURCE_DESC d3dUploadResourceDesc;
	::ZeroMemory(&d3dUploadResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dUploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //Upload Heap에는 텍스쳐를 생성할 수 없음
	d3dUploadResourceDesc.Alignment = 0;
	d3dUploadResourceDesc.Width = nBytes;
	d3dUploadResourceDesc.Height = 1;
	d3dUploadResourceDesc.DepthOrArraySize = 1;
	d3dUploadResourceDesc.MipLevels = 1;
	d3dUploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	d3dUploadResourceDesc.SampleDesc.Count = 1;
	d3dUploadResourceDesc.SampleDesc.Quality = 0;
	d3dUploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	d3dUploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dUploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);

	//UINT nSubResources = (UINT)vSubresources.size();
	//D3D12_SUBRESOURCE_DATA *pd3dSubResourceData = new D3D12_SUBRESOURCE_DATA[nSubResources];
	//for (UINT i = 0; i < nSubResources; i++) pd3dSubResourceData[i] = vSubresources.at(i);

	//	std::vector<D3D12_SUBRESOURCE_DATA>::pointer ptr = &vSubresources[0];
	::UpdateSubresources(pd3dCommandList, pd3dTexture, *ppd3dUploadBuffer, 0, 0, nSubResources, &vSubresources[0]);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = pd3dTexture;
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	//	delete[] pd3dSubResourceData;

	return(pd3dTexture);
}

ID3D12Resource* CreateTextureResourceFromDDSFile(
	ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	const wchar_t* pszFileName,         // 텍스처 파일 경로
	ID3D12Resource** ppd3dUploadBuffer, // 중간 업로드 버퍼 (출력)
	D3D12_RESOURCE_STATES d3dResourceStates  // 최종 상태 지정
)
{
	ID3D12Resource* pd3dTexture = nullptr; // 최종 텍스처 리소스 (Default Heap)
	std::unique_ptr<uint8_t[]> ddsData;    // DDS 파일 데이터
	std::vector<D3D12_SUBRESOURCE_DATA> vSubresources; // 서브리소스 정보
	DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
	bool bIsCubeMap = false;

	// 1. DDS 파일 로드 및 Default Heap 텍스처 리소스 생성 시도
	HRESULT hResult = DirectX::LoadDDSTextureFromFileEx(
		pd3dDevice,           // D3D12 디바이스
		pszFileName,          // 파일 경로
		0,                    // 최대 크기 제한 없음
		D3D12_RESOURCE_FLAG_NONE, // 리소스 플래그
		DDS_LOADER_DEFAULT,   // 로더 플래그 (기본값)
		&pd3dTexture,         // 생성된 텍스처 리소스 출력 포인터 주소
		ddsData,              // DDS 데이터 unique_ptr
		vSubresources,        // 서브리소스 데이터 벡터
		&ddsAlphaMode,        // 알파 모드 출력
		&bIsCubeMap           // 큐브맵 여부 출력
	);

	// --- 중요: LoadDDSTextureFromFileEx 결과 확인 ---
	if (FAILED(hResult))
	{
		OutputDebugStringA("!!!!!!!! ERROR: Failed to load DDS texture file or create texture resource: ");
		OutputDebugStringW(pszFileName);
		OutputDebugStringA(" !!!!!!!!\n");
		if (pd3dTexture) {
			// 실패했지만 리소스가 생성되었을 수도 있는 예외 케이스 처리
			pd3dTexture->Release();
			pd3dTexture = nullptr;
		}
		*ppd3dUploadBuffer = nullptr; // 업로드 버퍼도 생성 안 됨
		return nullptr; // 실패 시 nullptr 반환
	}
	// --- 확인 끝 ---

	// 2. 텍스처 리소스 생성이 성공한 경우에만 업로드 버퍼 생성 및 데이터 복사 수행
	OutputDebugStringA("Successfully created texture resource (Default Heap): ");
	OutputDebugStringW(pszFileName);
	OutputDebugStringA("\n");

	// 2.1. 업로드 버퍼 생성 (Upload Heap)
	//      중간 데이터 복사를 위한 업로드 힙 버퍼 크기 계산
	UINT nSubResources = (UINT)vSubresources.size();
	UINT64 nUploadBufferSize = GetRequiredIntermediateSize(pd3dTexture, 0, nSubResources);

	//      업로드 힙 속성
	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	//      업로드 버퍼 리소스 डिस्क्रिप्टर
	D3D12_RESOURCE_DESC d3dUploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(nUploadBufferSize);

	//      업로드 버퍼 생성
	HRESULT hr_upload = pd3dDevice->CreateCommittedResource(
		&d3dHeapPropertiesDesc,
		D3D12_HEAP_FLAG_NONE,
		&d3dUploadResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // 업로드 힙은 보통 Generic Read 상태
		nullptr, // 최적화된 Clear 값 없음
		IID_PPV_ARGS(ppd3dUploadBuffer)    // 출력 포인터 주소
	);

	//      업로드 버퍼 생성 HRESULT 확인 (추가)
	if (FAILED(hr_upload)) {
		OutputDebugStringA("!!!!!!!! ERROR: Failed to create upload buffer for texture: ");
		OutputDebugStringW(pszFileName);
		OutputDebugStringA(" !!!!!!!!\n");
		pd3dTexture->Release(); // 이미 생성된 최종 텍스처 리소스도 해제
		*ppd3dUploadBuffer = nullptr;
		return nullptr;
	}

	// 2.2. 데이터 복사 (CPU -> Upload Heap -> Default Heap Texture)
	//      UpdateSubresources 함수가 내부적으로 CommandList에 복사 명령 기록
	::UpdateSubresources(
		pd3dCommandList,      // 복사 명령을 기록할 커맨드 리스트
		pd3dTexture,          // 대상 텍스처 리소스 (Default Heap)
		*ppd3dUploadBuffer,   // 중간 업로드 버퍼 (Upload Heap)
		0,                    // 업로드 버퍼 시작 오프셋
		0,                    // 첫 번째 서브리소스 인덱스
		nSubResources,        // 복사할 서브리소스 개수
		&vSubresources[0]     // CPU 메모리의 서브리소스 데이터 배열
	);

	// 2.3. 리소스 상태 전이 배리어 설정
	//      업로드 완료 후 텍스처를 셰이더에서 읽을 수 있는 상태로 변경
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = pd3dTexture; // 상태를 변경할 리소스
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST; // 현재 상태 (복사 대상)
	d3dResourceBarrier.Transition.StateAfter = d3dResourceStates; // 최종 목표 상태 (예: 픽셀 셰이더 리소스)
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES; // 모든 서브리소스
	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier); // 커맨드 리스트에 배리어 기록

	// 3. 성공적으로 생성된 텍스처 리소스 포인터 반환
	return pd3dTexture;
}
	
ID3D12Resource* CreateTexture2DResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue)
{
	ID3D12Resource* pd3dTexture = NULL;

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dTextureResourceDesc;
	::ZeroMemory(&d3dTextureResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dTextureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dTextureResourceDesc.Alignment = 0;
	d3dTextureResourceDesc.Width = nWidth;
	d3dTextureResourceDesc.Height = nHeight;
	d3dTextureResourceDesc.DepthOrArraySize = nElements;
	d3dTextureResourceDesc.MipLevels = nMipLevels;
	d3dTextureResourceDesc.Format = dxgiFormat;
	d3dTextureResourceDesc.SampleDesc.Count = 1;
	d3dTextureResourceDesc.SampleDesc.Quality = 0;
	d3dTextureResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dTextureResourceDesc.Flags = d3dResourceFlags;

	HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dTextureResourceDesc, d3dResourceStates, pd3dClearValue, __uuidof(ID3D12Resource), (void**)&pd3dTexture);

	return(pd3dTexture);
}

ID3D12Resource* CreateTextureResourceFromWICFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, ID3D12Resource** ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates)
{
	ID3D12Resource* pd3dTexture = NULL;
	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA d3dSubresource;

	HRESULT hResult = DirectX::LoadWICTextureFromFileEx(pd3dDevice, pszFileName, 0, D3D12_RESOURCE_FLAG_NONE, WIC_LOADER_DEFAULT, &pd3dTexture, decodedData, d3dSubresource);

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	UINT64 nBytes = GetRequiredIntermediateSize(pd3dTexture, 0, 1);

	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //Upload Heap에는 텍스쳐를 생성할 수 없음
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = nBytes;
	d3dResourceDesc.Height = 1;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);

	::UpdateSubresources(pd3dCommandList, pd3dTexture, *ppd3dUploadBuffer, 0, 0, 1, &d3dSubresource);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = pd3dTexture;
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	return(pd3dTexture);
}
