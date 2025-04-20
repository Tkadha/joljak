#include "Texture.h"

#include "DDSTextureLoader12.h"
#include "WICTextureLoader12.h"

CTexture::CTexture(UINT nResourceType)
	: m_nTextureType(nResourceType), m_nTextures(0), m_nReferences(0)
{
}

CTexture::~CTexture()
{
	// ComPtr�� ���ҽ� �ڵ� ����
	ReleaseUploadBuffers(); // �Ҹ��ں��ٴ� ����� ȣ�� ����
	// ���÷� �ڵ� �迭 �� ����
}

void CTexture::LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex)
{
	// �ε����� ���� ũ�⺸�� ũ�� �������� �Ǵ� ���� ó�� �ʿ�
	if (nIndex >= m_vTextures.size()) {
		m_vTextures.resize(nIndex + 1);
		m_vTextureUploadBuffers.resize(nIndex + 1);
		m_vResourceTypes.resize(nIndex + 1);
		// ���� ���� �迭�� �������� �ʿ�
	}
	if (nIndex + 1 > m_nTextures) m_nTextures = nIndex + 1;


	// Ÿ�� ����
	m_vResourceTypes[nIndex] = RESOURCE_BUFFER;
	// ���� ���� ���� (�迭 �Ҵ� Ȯ�� �ʿ�)
	m_vBufferFormats[nIndex] = ndxgiFormat; 
	m_vBufferElements[nIndex] = nElements;

	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer; // �ӽ� ���ε� ���� ComPtr
	m_vTextures[nIndex] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pData, nElements * nStride, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, &uploadBuffer);
	// ���ε� ���� ����
	if (uploadBuffer) m_vTextureUploadBuffers[nIndex] = uploadBuffer;
}


ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue)
{
	// ���� �������� �� �ε��� ���� �ʿ�
	if (nIndex >= m_vTextures.size()) {
		m_vTextures.resize(nIndex + 1);
		m_vResourceTypes.resize(nIndex + 1);
	}
	if (nIndex + 1 > m_nTextures) m_nTextures = nIndex + 1;

	// Ÿ�� ����
	m_vResourceTypes[nIndex] = nResourceType;
	if (nIndex == 0) m_nTextureType = nResourceType; // �� Ÿ�Ե� ����

	m_vTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, pd3dCommandList, nWidth, nHeight, nElements, nMipLevels, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_vTextures[nIndex].Get());
}


bool CTexture::LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pszFileName)
{
	if (!pszFileName) return false;

	// ���� ���ҽ� ���� (�ʿ��)
	m_vTextures.clear();
	m_vTextureUploadBuffers.clear();
	m_vResourceTypes.clear();
	m_nTextures = 0;

	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> vSubresources;
	DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
	bool bIsCubeMap = false;

	Microsoft::WRL::ComPtr<ID3D12Resource> pTextureResource; // ComPtr ���

	HRESULT hr = DirectX::LoadDDSTextureFromFileEx(
		pd3dDevice, pszFileName, 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT,
		&pTextureResource, // ComPtr�� �ּҸ� ��� GetAddressOf() ���
		ddsData, vSubresources, &ddsAlphaMode, &bIsCubeMap);

	if (FAILED(hr)) {
		OutputDebugStringW((L"!!!!!!!! ERROR: LoadDDSTextureFromFileEx failed for: " + std::wstring(pszFileName) + L"\n").c_str());
		return false;
	}

	OutputDebugStringW((L"Successfully created texture resource (Default Heap): " + std::wstring(pszFileName) + L"\n").c_str());

	// ���ҽ� �� Ÿ�� ����
	m_vTextures.push_back(pTextureResource); // ComPtr ���Ϳ� �߰�
	m_nTextureType = bIsCubeMap ? RESOURCE_TEXTURE_CUBE : RESOURCE_TEXTURE2D; // �� Ÿ�� ����
	m_vResourceTypes.push_back(m_nTextureType); // Ÿ�� �迭���� �߰�
	m_nTextures = 1; // DDS ���� �ϳ��� ���ҽ� �ϳ��� ���� (�ؽ�ó �迭/ť��� ó���� LoadDDSTextureFromFileEx�� �˾Ƽ� ��)

	// --- ���ε� ó�� ---
	UINT nSubResources = (UINT)vSubresources.size();
	UINT64 nUploadBufferSize = GetRequiredIntermediateSize(pTextureResource.Get(), 0, nSubResources);

	D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(nUploadBufferSize);

	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer; // ComPtr ���
	HRESULT hr_upload = pd3dDevice->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pUploadBuffer));

	if (FAILED(hr_upload)) {
		OutputDebugStringW((L"!!!!!!!! ERROR: Failed to create upload buffer for: " + std::wstring(pszFileName) + L"\n").c_str());
		// m_vTextures.clear(); // �̹� ������ ���ҽ� ����
		return false;
	}

	// ������ ���� ��� ���
	::UpdateSubresources(pd3dCommandList, pTextureResource.Get(), pUploadBuffer.Get(),
		0, 0, nSubResources, &vSubresources[0]);

	// ���� ���� �踮�� ���
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		pTextureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	pd3dCommandList->ResourceBarrier(1, &barrier);

	// ���ε� ���۴� Ŀ�ǵ� ���� �Ϸ� �� �����ؾ� �ϹǷ� ����
	m_vTextureUploadBuffers.push_back(pUploadBuffer);

	return true;
}

void CTexture::ReleaseUploadBuffers()
{
	// ����� ��� ���ε� ���� ����
	m_vTextureUploadBuffers.clear();
}

D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc(int nIndex)
{
	// SRV ���� ����ü �ʱ�ȭ
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dSrvDesc = {};
	d3dSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	// �ش� �ε����� ���ҽ� ��������
	ID3D12Resource* pShaderResource = GetResource(nIndex);

	if (pShaderResource) {
		// ���ҽ��� ��ȿ�ϸ� Description ��������
		D3D12_RESOURCE_DESC Desc = pShaderResource->GetDesc();
		// ����� ���ҽ� Ÿ�� ��������
		UINT resourceType = GetTextureType(nIndex);

		// ���ҽ� Ÿ�Կ� ���� View Dimension �� ���� ���� ����
		switch (resourceType)
		{
		case RESOURCE_TEXTURE2D:
			// �ؽ�ó ���� Ȯ�� (Typeless�� ��� Ư�� ���� ���� �ʿ��� �� ����)
			d3dSrvDesc.Format = Desc.Format;
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dSrvDesc.Texture2D.MostDetailedMip = 0;
			d3dSrvDesc.Texture2D.MipLevels = Desc.MipLevels; // -1 �̸� ��� �� ����
			d3dSrvDesc.Texture2D.PlaneSlice = 0;
			d3dSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;

		case RESOURCE_TEXTURE2D_ARRAY: // �ؽ�ó �迭 ����
		case RESOURCE_TEXTURE2DARRAY:  // ���Ǿ� ó��
			d3dSrvDesc.Format = Desc.Format;
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			d3dSrvDesc.Texture2DArray.MostDetailedMip = 0;
			d3dSrvDesc.Texture2DArray.MipLevels = Desc.MipLevels;
			d3dSrvDesc.Texture2DArray.FirstArraySlice = 0; // ���� �迭 �ε���
			d3dSrvDesc.Texture2DArray.ArraySize = Desc.DepthOrArraySize; // �迭 ũ��
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
			d3dSrvDesc.Format = GetBufferFormat(nIndex); // ����� ���� ���� ���!
			d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			d3dSrvDesc.Buffer.FirstElement = 0;
			d3dSrvDesc.Buffer.NumElements = GetBufferElements(nIndex); // ����� ��� ���� ���!
			// Structured Buffer �� ��� Format=UNKNOWN, StructureByteStride ���� �ʿ�
			// Raw Buffer (ByteAddressBuffer) �� ��� Format=R32_TYPELESS, Flags=D3D12_BUFFER_SRV_FLAG_RAW ���� �ʿ�
			d3dSrvDesc.Buffer.StructureByteStride = 0; // Structured Buffer�� �ƴ϶�� ����
			d3dSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			// �ʿ信 ���� StructureByteStride, Flags ����
			break;

		default: // �𸣴� Ÿ���̸� �⺻ 2D�� ó�� �õ�
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
		// ���ҽ��� ���� ��� �⺻ Null SRV ���� (2D)
		d3dSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ���� ����
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