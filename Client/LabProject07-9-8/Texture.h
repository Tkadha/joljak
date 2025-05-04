#pragma once

#include "Mesh.h"


#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05
#define RESOURCE_UNKNOWN            0x00


class CTexture
{
public:
	// ������ �ܼ�ȭ: ���ҽ� ���� ���� Load �Լ����� ����
	CTexture(UINT nResourceType = RESOURCE_TEXTURE2D);
	virtual ~CTexture();;

private:
	UINT							m_nTextureType; // �� �ؽ�ó Ÿ��

	int								m_nTextures = 0; 
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_vTextures; // GPU ���ҽ�
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_vTextureUploadBuffers; // ���ε� ����
	std::vector<UINT>				m_vResourceTypes; // �� ���ҽ��� ���� Ÿ�� ����

	std::vector<DXGI_FORMAT> m_vBufferFormats;
	std::vector<int> m_vBufferElements;

public:
	// �ε� �� ���� �Լ�
	bool LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pszFileName);
	bool LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex = 0); // ���۴� ���� �ε��� 0 ����
	ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);
	
	// SRV ���� ��ȯ
	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

	// ������ �Լ���
	int GetTextures() const { return m_nTextures; }
	ID3D12Resource* GetResource(int nIndex = 0) const;
	UINT GetTextureType(int nIndex = 0) const;
	DXGI_FORMAT GetBufferFormat(int nIndex = 0) const;
	int GetBufferElements(int nIndex = 0) const;

	void ReleaseUploadBuffers(); // ���ε� ���� ����

};
