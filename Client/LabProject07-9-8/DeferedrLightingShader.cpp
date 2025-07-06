#include "DeferedrLightingShader.h"

CLightingShader::CLightingShader(const std::string& name)
{
    m_strShaderName = name;
}

CLightingShader::~CLightingShader()
{
}

// 입력 레이아웃: 정점 위치(POSITION)만 필요합니다.
D3D12_INPUT_LAYOUT_DESC CLightingShader::CreateInputLayout()
{
    UINT nInputElementDescs = 1;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return(d3dInputLayoutDesc);
}


D3D12_SHADER_BYTECODE CLightingShader::CreateVertexShader()
{
    // "Shadow.hlsl" 파일을 컴파일합니다.
    //return(CShader::CompileShaderFromFile(L"StandardShaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));

    return(CShader::CompileShaderFromFile(L"DefetedLighting.hlsl", "VS", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CLightingShader::CreatePixelShader()
{
    // 픽셀 셰이더는 아무 작업도 하지 않으므로 NULL을 반환합니다.
    return(CShader::CompileShaderFromFile(L"DefetedLighting.hlsl", "PS", "ps_5_1", &m_pd3dVertexShaderBlob));
}
