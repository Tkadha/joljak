#include "Texture.h"

#include "DDSTextureLoader12.h"
#include "WICTextureLoader12.h"

CTexture::CTexture(UINT nResourceType)
	: m_nTextureType(nResourceType), m_nTextures(0), m_nReferences(0)
{
}

CTexture::~CTexture()
{
	// ComPtr이 리소스 자동 해제
	ReleaseUploadBuffers(); // 소멸자보다는 명시적 호출 권장
	// 샘플러 핸들 배열 등 해제
}

void CTexture::LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex)
{
	// 인덱스가 벡터 크기보다 크면 리사이즈 또는 오류 처리 필요
	if (nIndex >= m_vTextures.size()) {
		m_vTextures.resize(nIndex + 1);
		m_vTextureUploadBuffers.resize(nIndex + 1);
		m_vResourceTypes.resize(nIndex + 1);
		// 버퍼 관련 배열도 리사이즈 필요
	}
	if (nIndex + 1 > m_nTextures) m_nTextures = nIndex + 1;


	// 타입 설정
	m_vResourceTypes[nIndex] = RESOURCE_BUFFER;
	// 버퍼 정보 저장 (배열 할당 확인 필요)
	m_vBufferFormats[nIndex] = ndxgiFormat; 
	m_vBufferElements[nIndex] = nElements;

	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer; // 임시 업로드 버퍼 ComPtr
	m_vTextures[nIndex] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pData, nElements * nStride, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, &uploadBuffer);
	// 업로드 버퍼 저장
	if (uploadBuffer) m_vTextureUploadBuffers[nIndex] = uploadBuffer;
}


ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue)
{
	// 벡터 리사이즈 등 인덱스 관리 필요
	if (nIndex >= m_vTextures.size()) {
		m_vTextures.resize(nIndex + 1);
		m_vResourceTypes.resize(nIndex + 1);
	}
	if (nIndex + 1 > m_nTextures) m_nTextures = nIndex + 1;

	// 타입 설정
	m_vResourceTypes[nIndex] = nResourceType;
	if (nIndex == 0) m_nTextureType = nResourceType; // 주 타입도 설정

	m_vTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, pd3dCommandList, nWidth, nHeight, nElements, nMipLevels, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_vTextures[nIndex].Get());
}


bool CTexture::LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pszFileName)
{
	if (!pszFileName) return false;

	// 이전 리소스 해제 (필요시)
	m_vTextures.clear();
	m_vTextureUploadBuffers.clear();
	m_vResourceTypes.clear();
	m_nTextures = 0;

	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> vSubresources;
	DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
	bool bIsCubeMap = false;

	Microsoft::WRL::ComPtr<ID3D12Resource> pTextureResource; // ComPtr 사용

	HRESULT hr = DirectX::LoadDDSTextureFromFileEx(
		pd3dDevice, pszFileName, 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT,
		&pTextureResource, // ComPtr의 주소를 얻는 GetAddressOf() 사용
		ddsData, vSubresources, &ddsAlphaMode, &bIsCubeMap);

	if (FAILED(hr)) {
		OutputDebugStringW((L"!!!!!!!! ERROR: LoadDDSTextureFromFileEx failed for: " + std::wstring(pszFileName) + L"\n").c_str());
		return false;
	}

	OutputDebugStringW((L"Successfully created texture resource (Default Heap): " + std::wstring(pszFileName) + L"\n").c_str());

	// 리소스 및 타입 저장
	m_vTextures.push_back(pTextureResource); // ComPtr 벡터에 추가
	m_nTextureType = bIsCubeMap ? RESOURCE_TEXTURE_CUBE : RESOURCE_TEXTURE2D; // 주 타입 설정
	m_vResourceTypes.push_back(m_nTextureType); // 타입 배열에도 추가
	m_nTextures = 1; // DDS 파일 하나는 리소스 하나로 간주 (텍스처 배열/큐브맵 처리는 LoadDDSTextureFromFileEx가 알아서 함)

	// --- 업로드 처리 ---
	UINT nSubResources = (UINT)vSubresources.size();
	UINT64 nUploadBufferSize = GetRequiredIntermediateSize(pTextureResource.Get(), 0, nSubResources);

	D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(nUploadBufferSize);

	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer; // ComPtr 사용
	HRESULT hr_upload = pd3dDevice->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pUploadBuffer));

	if (FAILED(hr_upload)) {
		OutputDebugStringW((L"!!!!!!!! ERROR: Failed to create upload buffer for: " + std::wstring(pszFileName) + L"\n").c_str());
		// m_vTextures.clear(); // 이미 생성된 리소스 정리
		return false;
	}

	// 데이터 복사 명령 기록
	::UpdateSubresources(pd3dCommandList, pTextureResource.Get(), pUploadBuffer.Get(),
		0, 0, nSubResources, &vSubresources[0]);

	// 상태 전이 배리어 기록
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		pTextureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	pd3dCommandList->ResourceBarrier(1, &barrier);

	// 업로드 버퍼는 커맨드 실행 완료 후 해제해야 하므로 저장
	m_vTextureUploadBuffers.push_back(pUploadBuffer);

	return true;
}

void CTexture::ReleaseUploadBuffers()
{
	// 저장된 모든 업로드 버퍼 해제
	m_vTextureUploadBuffers.clear();
}

D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc(int nIndex)
{
	// SRV 설정 구조체 초기화
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dSrvDesc = {};
	d3dSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	// 해당 인덱스의 리소스 가져오기
	ID3D12Resource* pShaderResource = GetResource(nIndex);

	if (pShaderResource) {
		// 리소스가 유효하면 Description 가져오기
		D3D12_RESOURCE_DESC Desc = pShaderResource->GetDesc();
		// 저장된 리소스 타입 가져오기
		UINT resourceType = GetTextureType(nIndex);

		// 리소스 타입에 따라 View Dimension 및 관련 정보 설정
		switch (resourceType)
		{
		case RESOURCE_TEXTURE2D:
			// 텍스처 포맷 확인 (Typeless인 경우 특정 포맷 지정 필요할 수 있음)
			d3dSrvDesc.Format = Desc.Format;
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dSrvDesc.Texture2D.MostDetailedMip = 0;
			d3dSrvDesc.Texture2D.MipLevels = Desc.MipLevels; // -1 이면 모든 밉 레벨
			d3dSrvDesc.Texture2D.PlaneSlice = 0;
			d3dSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;

		case RESOURCE_TEXTURE2D_ARRAY: // 텍스처 배열 예시
		case RESOURCE_TEXTURE2DARRAY:  // 동의어 처리
			d3dSrvDesc.Format = Desc.Format;
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			d3dSrvDesc.Texture2DArray.MostDetailedMip = 0;
			d3dSrvDesc.Texture2DArray.MipLevels = Desc.MipLevels;
			d3dSrvDesc.Texture2DArray.FirstArraySlice = 0; // 시작 배열 인덱스
			d3dSrvDesc.Texture2DArray.ArraySize = Desc.DepthOrArraySize; // 배열 크기
			d3dSrvDesc.Texture2DArray.PlaneSlice = 0;
			d3dSrvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
			break;

		case RESOURCE_TEXTURE_CUBE:
			d3dSrvDesc.Format = Desc.Format;
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			d3dSrvDesc.TextureCube.MostDetailedMip = 0;
			d3dSrvDesc.TextureCube.MipLevels = Desc.MipLevels;
			d3dSrvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;

		case RESOURCE_BUFFER:
			d3dSrvDesc.Format = GetBufferFormat(nIndex); // 저장된 버퍼 포맷 사용!
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			d3dSrvDesc.Buffer.FirstElement = 0;
			d3dSrvDesc.Buffer.NumElements = GetBufferElements(nIndex); // 저장된 요소 개수 사용!
			// Structured Buffer 인 경우 Format=UNKNOWN, StructureByteStride 설정 필요
			// Raw Buffer (ByteAddressBuffer) 인 경우 Format=R32_TYPELESS, Flags=D3D12_BUFFER_SRV_FLAG_RAW 설정 필요
			d3dSrvDesc.Buffer.StructureByteStride = 0; // Structured Buffer가 아니라고 가정
			d3dSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			// 필요에 따라 StructureByteStride, Flags 수정
			break;

		default: // 모르는 타입이면 기본 2D로 처리 시도
			d3dSrvDesc.Format = Desc.Format;
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dSrvDesc.Texture2D.MostDetailedMip = 0;
			d3dSrvDesc.Texture2D.MipLevels = Desc.MipLevels;
			d3dSrvDesc.Texture2D.PlaneSlice = 0;
			d3dSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;
		}
	}
	else {
		// 리소스가 없을 경우 기본 Null SRV 설정 (2D)
		d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 예시 포맷
		d3dSrvDesc.Texture2D.MipLevels = 1;
		d3dSrvDesc.Texture2D.MostDetailedMip = 0;
		d3dSrvDesc.Texture2D.PlaneSlice = 0;
		d3dSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	}
	return d3dSrvDesc;
}

DXGI_FORMAT CTexture::GetBufferFormat(int nIndex) const {
	return (nIndex >= 0 && nIndex < m_vBufferFormats.size()) ? m_vBufferFormats[nIndex] : DXGI_FORMAT_UNKNOWN;
}
int CTexture::GetBufferElements(int nIndex) const {
	return (nIndex >= 0 && nIndex < m_vBufferElements.size()) ? m_vBufferElements[nIndex] : 0;
}