// SkyBoxShader.cpp
#include "SkyBoxShader.h"

CSkyBoxShader::CSkyBoxShader(const std::string& name) { m_strShaderName = name; }

CSkyBoxShader::~CSkyBoxShader(){}

D3D12_INPUT_LAYOUT_DESC CSkyBoxShader::CreateInputLayout()
{
    UINT nInputElementDescs = 1;
    // 동적 할당 후 CShader::CreateShader에서 해제됨
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return d3dInputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC CSkyBoxShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
    ::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
    d3dDepthStencilDesc.DepthEnable = TRUE; // 깊이 테스트는 수행
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 깊이 쓰기는 안 함
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 먼저 그려진 객체 뒤에 그려지도록
    d3dDepthStencilDesc.StencilEnable = FALSE;
    d3dDepthStencilDesc.StencilReadMask = 0xFF;
    d3dDepthStencilDesc.StencilWriteMask = 0xFF;
    d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    d3dDepthStencilDesc.BackFace = d3dDepthStencilDesc.FrontFace; // 동일하게 설정

    return d3dDepthStencilDesc;
}

D3D12_RASTERIZER_DESC CSkyBoxShader::CreateRasterizerState()
{
    // 기본 Rasterizer 상태를 가져와서 수정
    D3D12_RASTERIZER_DESC d3dRasterizerDesc = CShader::CreateRasterizerState();
    // 스카이박스는 안쪽 면을 봐야 하므로 컬링을 끄거나 앞면 컬링
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // 또는 D3D12_CULL_MODE_FRONT
    return d3dRasterizerDesc;
}


D3D12_SHADER_BYTECODE CSkyBoxShader::CreateVertexShader()
{
    // 분리된 HLSL 파일 사용
    return CompileShaderFromFile(L"SkyboxShaders.hlsl", "VSSkyBox", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader()
{
    // 분리된 HLSL 파일 사용
    return CompileShaderFromFile(L"SkyboxShaders.hlsl", "PSSkyBox", "ps_5_1", &m_pd3dPixelShaderBlob);
}