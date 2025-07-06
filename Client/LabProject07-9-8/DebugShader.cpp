// SkyBoxShader.cpp
#include "DebugShader.h"

CDebugShader::CDebugShader(const std::string& name) { m_strShaderName = name; }

CDebugShader::~CDebugShader() {}

D3D12_INPUT_LAYOUT_DESC CDebugShader::CreateInputLayout()
{
    // �Է� ��Ұ� 2�� (POSITION, TEXCOORD) �̹Ƿ� ũ�⸦ 2�� �ٲߴϴ�.
    UINT nInputElementDescs = 2;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    // 0�� ���: ��ġ
    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    // --- 1�� ���: �ؽ�ó ��ǥ(UV)�� �߰��մϴ�. ---
    pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };


    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return d3dInputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC CDebugShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
    ::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
    d3dDepthStencilDesc.DepthEnable = FALSE; 
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // ���� ����� �� ��
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; 
    d3dDepthStencilDesc.StencilEnable = FALSE;
    d3dDepthStencilDesc.StencilReadMask = 0xFF;
    d3dDepthStencilDesc.StencilWriteMask = 0xFF;
    d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    d3dDepthStencilDesc.BackFace = d3dDepthStencilDesc.FrontFace; // �����ϰ� ����

    return d3dDepthStencilDesc;
}

D3D12_RASTERIZER_DESC CDebugShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc;
    ::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

    // �ﰢ�� ���θ� ä���� �׸��ϴ�.
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    // --- ���� �߿��� �κ�: �ø��� ������ ���ϴ�. ---
    // ���� �ﰢ���� �ո��̵� �޸��̵� ������� ������ �׸��� �˴ϴ�.
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

    d3dRasterizerDesc.FrontCounterClockwise = FALSE;
    d3dRasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    d3dRasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    d3dRasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    d3dRasterizerDesc.DepthClipEnable = TRUE;
    d3dRasterizerDesc.MultisampleEnable = FALSE;
    d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
    d3dRasterizerDesc.ForcedSampleCount = 0;
    d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    return d3dRasterizerDesc;
}


D3D12_SHADER_BYTECODE CDebugShader::CreateVertexShader()
{
    // �и��� HLSL ���� ���
    return CompileShaderFromFile(L"Debug.hlsl", "VS", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CDebugShader::CreatePixelShader()
{
    // �и��� HLSL ���� ���
    return CompileShaderFromFile(L"Debug.hlsl", "PS", "ps_5_1", &m_pd3dPixelShaderBlob);
}