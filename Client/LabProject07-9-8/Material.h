#pragma once

#include "Texture.h"


#define MATERIAL_ALBEDO_MAP				0x01
#define MATERIAL_SPECULAR_MAP			0x02
#define MATERIAL_NORMAL_MAP				0x04
#define MATERIAL_METALLIC_MAP			0x08
#define MATERIAL_EMISSION_MAP			0x10
#define MATERIAL_DETAIL_ALBEDO_MAP		0x20
#define MATERIAL_DETAIL_NORMAL_MAP		0x40

class CShader;
class CStandardShader;
class CGameObject;
class ResourceManager;
class CGameFramework;

class CMaterial
{
public:
	CMaterial(int nTextures);
	CMaterial(int nTextures, CGameFramework* pGameFramework);

	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	CShader* m_pShader = NULL;

	XMFLOAT4 m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 
	XMFLOAT4 m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 m_xmf4SpecularColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f); // 약간의 회색 반사광
	XMFLOAT4 m_xmf4AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);  // 약간의 주변광 반응

	void SetShader(CShader* pShader);
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	
	virtual void ReleaseUploadBuffers();

public:
	UINT							m_nType = 0x00;

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

public:
	int 							m_nTextures = 0;
	_TCHAR(*m_ppstrTextureNames)[64] = NULL;
	std::vector<std::shared_ptr<CTexture>> m_vTextures; // shared_ptr 벡터 사용

	// 텍스처 로딩 시 할당받은 SRV 블록의 시작 핸들 저장
	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dCpuSrvStartHandle = { 0 };
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dSrvGpuStartHandle = { 0 };

	// SRV 디스크립터 크기 (Framework 등에서 얻어와야 함)
	UINT m_nCbvSrvDescriptorIncrementSize = 0;

	CGameFramework* m_pGameFramework = nullptr;

	
	// 텍스처 테이블 시작 GPU 핸들 반환 함수
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureTableGpuHandle() const { return m_d3dSrvGpuStartHandle; }

	// 텍스처 포인터 배열 접근자 (필요시)
	CTexture* GetTexture(int index) const;
	int GetTextureCount() const { return m_nTextures; }

	void LoadTextureFromFile(
		ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		UINT nTextureIndex,         // 이 텍스처가 저장될 인덱스 (0:Albedo, 1:Specular...)
		UINT nTextureType,          // 이 텍스처의 타입 마스크 (MATERIAL_ALBEDO_MAP 등)
		CGameObject* pParent,       // 중복 텍스처 찾기용 부모 포인터
		FILE* pInFile,              // 파일 포인터
		ResourceManager* pResourceManager // 리소스 매니저
	);

	// 텍스처 설정 및 SRV 생성 함수 추가
	bool AssignTexture(UINT nTextureIndex, std::shared_ptr<CTexture> pTexture, ID3D12Device* pd3dDevice);
};



