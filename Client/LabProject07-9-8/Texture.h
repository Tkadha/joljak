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
	// 생성자 단순화: 리소스 개수 등은 Load 함수에서 결정
	CTexture(UINT nResourceType = RESOURCE_TEXTURE2D);
	virtual ~CTexture();;

private:
	UINT							m_nTextureType; // 주 텍스처 타입

	int								m_nTextures = 0; 
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_vTextures; // GPU 리소스
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_vTextureUploadBuffers; // 업로드 버퍼
	std::vector<UINT>				m_vResourceTypes; // 각 리소스의 실제 타입 저장

	std::vector<DXGI_FORMAT> m_vBufferFormats;
	std::vector<int> m_vBufferElements;

public:
	// 로딩 및 생성 함수
	bool LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const wchar_t* pszFileName);
	bool LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex = 0); // 버퍼는 보통 인덱스 0 가정
	ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);
	
	// SRV 설정 반환
	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

	// 접근자 함수들
	int GetTextures() const { return m_nTextures; }
	ID3D12Resource* GetResource(int nIndex = 0) const;
	UINT GetTextureType(int nIndex = 0) const;
	DXGI_FORMAT GetBufferFormat(int nIndex = 0) const;
	int GetBufferElements(int nIndex = 0) const;

	void ReleaseUploadBuffers(); // 업로드 버퍼 해제

};
