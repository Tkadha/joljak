// TerrainShader.cpp
#include "TerrainShader.h"

CTerrainShader::CTerrainShader(const std::string& name) { m_strShaderName = name; }

CTerrainShader::~CTerrainShader(){}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
    UINT nInputElementDescs = 4;
    // ���� �Ҵ� �� CShader::CreateShader���� ������
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    // CHeightMapGridMesh::OnPreRender ���� �����ϴ� ���� ������ ��ġ�ؾ� ��
    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; // Slot 0
    pd3dInputElementDescs[1] = { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; // Slot 1
    pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; // Slot 2 (UV0)
    pd3dInputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; // Slot 3 (UV1)

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return d3dInputLayoutDesc;
}

D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader()
{
    // �и��� HLSL ���� ���
    return CompileShaderFromFile(L"TerrainShaders.hlsl", "VSTerrain", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader()
{
    // �и��� HLSL ���� ���
    return CompileShaderFromFile(L"TerrainShaders.hlsl", "PSTerrain", "ps_5_1", &m_pd3dPixelShaderBlob);
}