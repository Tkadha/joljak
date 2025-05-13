#include "Texture.h"

#include "DDSTextureLoader12.h"
#include "WICTextureLoader12.h"
#include <algorithm>

CTexture::CTexture(UINT nResourceType)
	: m_nTextureType(nResourceType), m_nTextures(0)
{
}

CTexture::~CTexture()
{
	ReleaseUploadBuffers();
}

bool CTexture::LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex)
{
	// 인덱스에 맞춰 벡터 크기 동적 조절
	size_t requiredSize = nIndex + 1;
	if (requiredSize > m_vTextures.size()) m_vTextures.resize(requiredSize);
	if (requiredSize > m_vTextureUploadBuffers.size()) m_vTextureUploadBuffers.resize(requiredSize);
	if (requiredSize > m_vResourceTypes.size()) m_vResourceTypes.resize(requiredSize, RESOURCE_UNKNOWN);
	if (requiredSize > m_vBufferFormats.size()) m_vBufferFormats.resize(requiredSize, DXGI_FORMAT_UNKNOWN);
	if (requiredSize > m_vBufferElements.size()) m_vBufferElements.resize(requiredSize, 0);

    // 실제 텍스처 개수 업데이트
    m_nTextures = std::max(m_nTextures, (int)(nIndex + 1));

	// 타입 설정
	m_vResourceTypes[nIndex] = RESOURCE_BUFFER;
	// 버퍼 정보 저장
	m_vBufferFormats[nIndex] = ndxgiFormat; 
	m_vBufferElements[nIndex] = nElements;

	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer; // 임시 업로드 버퍼 ComPtr
	m_vTextures[nIndex] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pData, nElements * nStride, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, &uploadBuffer);
	// 업로드 버퍼 저장
	if (uploadBuffer) m_vTextureUploadBuffers[nIndex] = uploadBuffer;
	return true;
}


ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue)
{
	size_t requiredSize = nIndex + 1;
	if (requiredSize > m_vTextures.size()) m_vTextures.resize(requiredSize);
	if (requiredSize > m_vResourceTypes.size()) m_vResourceTypes.resize(requiredSize, RESOURCE_UNKNOWN);
	m_nTextures = std::max(m_nTextures, (int)(nIndex + 1));

	// 타입 설정
	m_vResourceTypes[nIndex] = nResourceType;
	if (nIndex == 0) m_nTextureType = nResourceType; // 주 타입도 설정

	m_vTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, pd3dCommandList, nWidth, nHeight, nElements, nMipLevels, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_vTextures[nIndex].Get());
}


bool CTexture::LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pszFileName)
{
	if (!pszFileName) return false;

	m_vTextures.clear();
    m_vTextureUploadBuffers.clear();
    m_vResourceTypes.clear();
    m_vBufferFormats.clear(); // 관련 벡터들도 클리어
    m_vBufferElements.clear();
    m_nTextures = 0;

	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> vSubresources;
	DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
	bool bIsCubeMap = false;

	Microsoft::WRL::ComPtr<ID3D12Resource> pTextureResource; // ComPtr 사용

	HRESULT hr = DirectX::LoadDDSTextureFromFileEx(
		pd3dDevice, pszFileName, 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT,
		pTextureResource.GetAddressOf(), ddsData, vSubresources, &ddsAlphaMode, &bIsCubeMap);

	if (FAILED(hr)) {
		OutputDebugStringW((L"!!!!!!!! ERROR: LoadDDSTextureFromFileEx failed for: " + std::wstring(pszFileName) + L"\n").c_str());
		return false;
	}

	//OutputDebugStringW((L"Successfully created texture resource (Default Heap): " + std::wstring(pszFileName) + L"\n").c_str());

	// 리소스 및 타입 저장
	m_vTextures.push_back(pTextureResource); // ComPtr 벡터에 추가
	m_nTextureType = bIsCubeMap ? RESOURCE_TEXTURE_CUBE : RESOURCE_TEXTURE2D; // 주 타입 설정
	m_vResourceTypes.push_back(m_nTextureType); // 타입 배열에도 추가
	m_nTextures = 1; // DDS 파일 하나는 리소스 하나로 간주 (텍스처 배열/큐브맵 처리는 LoadDDSTextureFromFileEx가 알아서 함)
	// 버퍼 정보 벡터도 크기 맞춰주기
	m_vBufferFormats.resize(m_nTextures);
	m_vBufferElements.resize(m_nTextures);


	// --- 업로드 처리 ---
	UINT nSubResources = (UINT)vSubresources.size();
	UINT64 nUploadBufferSize = GetRequiredIntermediateSize(pTextureResource.Get(), 0, nSubResources);

	D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(nUploadBufferSize);

	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer;
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
		d3dSrvDesc.Format = Desc.Format;
		// 저장된 리소스 타입 가져오기
		UINT resourceType = GetTextureType(nIndex);

		// 리소스 타입에 따라 View Dimension 및 관련 정보 설정
		switch (resourceType) {
		case RESOURCE_TEXTURE_CUBE:
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			d3dSrvDesc.TextureCube.MostDetailedMip = 0;
			d3dSrvDesc.TextureCube.MipLevels = -1;
			d3dSrvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;
		case RESOURCE_TEXTURE2D_ARRAY:
		case RESOURCE_TEXTURE2DARRAY:
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			d3dSrvDesc.Texture2DArray.MipLevels = -1;
			d3dSrvDesc.Texture2DArray.MostDetailedMip = 0;
			d3dSrvDesc.Texture2DArray.FirstArraySlice = 0;
			d3dSrvDesc.Texture2DArray.ArraySize = Desc.DepthOrArraySize;
			d3dSrvDesc.Texture2DArray.PlaneSlice = 0;
			d3dSrvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
			break;
		case RESOURCE_BUFFER:
			d3dSrvDesc.Format = GetBufferFormat(nIndex);
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			d3dSrvDesc.Buffer.FirstElement = 0;
			d3dSrvDesc.Buffer.NumElements = GetBufferElements(nIndex);
			d3dSrvDesc.Buffer.StructureByteStride = 0;
			d3dSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			break;
		case RESOURCE_TEXTURE2D:
		default:
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dSrvDesc.Texture2D.MostDetailedMip = 0;
			d3dSrvDesc.Texture2D.MipLevels = -1;
			d3dSrvDesc.Texture2D.PlaneSlice = 0;
			d3dSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;
		}
	}
	else {
		// 리소스가 없을 경우 기본 Null SRV 설정 (2D)
		d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dSrvDesc.Texture2D.MipLevels = 1;
		d3dSrvDesc.Texture2D.MostDetailedMip = 0;
		d3dSrvDesc.Texture2D.PlaneSlice = 0;
		d3dSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	}
	return d3dSrvDesc;
}


ID3D12Resource* CTexture::GetResource(int nIndex) const {
	return (nIndex >= 0 && nIndex < m_vTextures.size()) ? m_vTextures[nIndex].Get() : nullptr;
}
UINT CTexture::GetTextureType(int nIndex) const {
	return (nIndex >= 0 && nIndex < m_vResourceTypes.size()) ? m_vResourceTypes[nIndex] : m_nTextureType;
}
DXGI_FORMAT CTexture::GetBufferFormat(int nIndex) const {
	return (nIndex >= 0 && nIndex < m_vBufferFormats.size()) ? m_vBufferFormats[nIndex] : DXGI_FORMAT_UNKNOWN;
}
int CTexture::GetBufferElements(int nIndex) const {
	return (nIndex >= 0 && nIndex < m_vBufferElements.size()) ? m_vBufferElements[nIndex] : 0;
}


void CTexture::ReleaseUploadBuffers()
{
	// 저장된 모든 업로드 버퍼 해제
	m_vTextureUploadBuffers.clear();
}