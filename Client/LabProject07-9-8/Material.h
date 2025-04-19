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
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	CShader* m_pShader = NULL;

	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	void SetShader(CShader* pShader);
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void SetTexture(CTexture* pTexture, UINT nTexture = 0);

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);

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
	CTexture** m_ppTextures = NULL; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal

public:
	// �ؽ�ó �ε� �� �Ҵ���� SRV ����� ���� �ڵ� ����
	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dCpuSrvStartHandle = { 0 };
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dSrvGpuStartHandle = { 0 };

	// SRV ��ũ���� ũ�� (Framework ��� ���;� ��)
	UINT m_nCbvSrvDescriptorIncrementSize = 0;

	// �����ڿ��� �ؽ�ó ���� ���� �ܿ� Framework ������ �޵��� ����
	CMaterial(int nTextures, CGameFramework* pGameFramework);

	
	// �ؽ�ó ���̺� ���� GPU �ڵ� ��ȯ �Լ�
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureTableGpuHandle() const { return m_d3dSrvGpuStartHandle; }

	// �ؽ�ó ������ �迭 ������ (�ʿ��)
	CTexture* GetTexture(int index) const { return (index < m_nTextures) ? m_ppTextures[index] : nullptr; }
	int GetTextureCount() const { return m_nTextures; }

	void LoadTextureFromFile(
		ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		UINT nTextureIndex,         // �� �ؽ�ó�� ����� �ε��� (0:Albedo, 1:Specular...)
		UINT nTextureType,          // �� �ؽ�ó�� Ÿ�� ����ũ (MATERIAL_ALBEDO_MAP ��)
		CGameObject* pParent,       // �ߺ� �ؽ�ó ã��� �θ� ������
		FILE* pInFile,              // ���� ������
		ResourceManager* pResourceManager // ���ҽ� �Ŵ���
	);
};



