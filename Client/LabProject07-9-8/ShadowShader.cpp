#include "ShadowShader.h"

CShadowShader::CShadowShader(const std::string& name)
{
    m_strShaderName = name;
}

CShadowShader::~CShadowShader()
{
}

//void CShadowShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
//{
//    ::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
//    m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
//    m_d3dPipelineStateDesc.VS = CreateVertexShader();
//    m_d3dPipelineStateDesc.PS = CreatePixelShader();
//    m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
//    m_d3dPipelineStateDesc.BlendState = CreateBlendState();
//    m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
//    m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
//    m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
//    m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    // �׸��� �н��� �÷� ���� Ÿ�� ��� X
//    m_d3dPipelineStateDesc.NumRenderTargets = 0;
//    m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
//
//    m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
//    m_d3dPipelineStateDesc.SampleDesc.Count = 1;
//    m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
//
//    HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);
//
//    if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
//    if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();
//
//    if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
//}


// �Է� ���̾ƿ�: ���� ��ġ(POSITION)�� �ʿ��մϴ�.
D3D12_INPUT_LAYOUT_DESC CShadowShader::CreateInputLayout()
{
    UINT nInputElementDescs = 1;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return(d3dInputLayoutDesc);
}

// ����/���ٽ� ����: ���� �׽�Ʈ�� ���⸦ Ȱ��ȭ�մϴ�.
D3D12_DEPTH_STENCIL_DESC CShadowShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
    ::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
    d3dDepthStencilDesc.DepthEnable = TRUE;
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // �Ϲ����� ���� �׽�Ʈ
    d3dDepthStencilDesc.StencilEnable = FALSE;

    return(d3dDepthStencilDesc);
}


D3D12_SHADER_BYTECODE CShadowShader::CreateVertexShader()
{
    // "Shadow.hlsl" ������ �������մϴ�.
    //return(CShader::CompileShaderFromFile(L"StandardShaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));

    return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "VS", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CShadowShader::CreatePixelShader()
{
    // �ȼ� ���̴��� �ƹ� �۾��� ���� �����Ƿ� NULL�� ��ȯ�մϴ�.
    return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "PS", "ps_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_RASTERIZER_DESC CShadowShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc;
    ::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

    // DepthBias: ������ ����ŭ ���̸� �߰�
    d3dRasterizerDesc.DepthBias = 1000; 

    // SlopeScaledDepthBias: ǥ���� ���⿡ ���� ���̾�� �ٸ��� ����
    d3dRasterizerDesc.SlopeScaledDepthBias = 2.0f; // ���� ���ݾ� �÷�������. (��: 1.0f -> 1.5f �Ǵ� 2.0f)

    d3dRasterizerDesc.DepthBiasClamp = 0.0f;
    d3dRasterizerDesc.DepthClipEnable = TRUE;

    d3dRasterizerDesc.FrontCounterClockwise = FALSE;
    d3dRasterizerDesc.MultisampleEnable = FALSE;
    d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
    d3dRasterizerDesc.ForcedSampleCount = 0;
    d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


    return d3dRasterizerDesc;
}