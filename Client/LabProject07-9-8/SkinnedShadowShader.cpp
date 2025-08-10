#include "stdafx.h"
#include "SkinnedShadowShader.h"

CSkinnedShadowShader::CSkinnedShadowShader() : CSkinnedAnimationStandardShader()
{
}
CSkinnedShadowShader::~CSkinnedShadowShader() {}

// Vertex Shader�� ���� ���� VSSkinnedAnimationShadow �Լ��� �������մϴ�.
D3D12_SHADER_BYTECODE CSkinnedShadowShader::CreateVertexShader()
{
    return(CShader::CompileShaderFromFile(L"SkinnedShaders.hlsl", "VSSkinnedAnimationShadow", "vs_5_1", &m_pd3dVertexShaderBlob));
}

// Pixel Shader�� �ʿ� �����Ƿ� NULL�� ��ȯ�մϴ�.
D3D12_SHADER_BYTECODE CSkinnedShadowShader::CreatePixelShader()
{
    return(D3D12_SHADER_BYTECODE{ NULL, 0 });
}

// ���� ���̾�� ����� �����Ͷ����� ���¸� ����մϴ�.
D3D12_RASTERIZER_DESC CSkinnedShadowShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc = CShader::CreateRasterizerState();
    d3dRasterizerDesc.DepthBias = 1000;
    d3dRasterizerDesc.SlopeScaledDepthBias = 1.5f;
    return d3dRasterizerDesc;
}

// PSO ���� ��, �÷� ���� Ÿ���� ������� �ʵ��� �����մϴ�.
void CSkinnedShadowShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    // 1. PSO ���赵�� �����ϰ� �ʱ�ȭ�մϴ�.
    ::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;

    // 2. �ʿ��� ������ ��� �����ɴϴ�.
    m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
    m_d3dPipelineStateDesc.VS = CreateVertexShader();
    m_d3dPipelineStateDesc.PS = CreatePixelShader(); // NULL �ȼ� ���̴�
    m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState(); // ���� ���̾ ����
    m_d3dPipelineStateDesc.BlendState = CreateBlendState();
    m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();

    // 3. �׸��� �н��� �´� ���� Ÿ�� ������ ��������� �����մϴ�.
    m_d3dPipelineStateDesc.NumRenderTargets = 0;
    m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 4. ������ ���� ������ �մϴ�.
    m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
    m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_d3dPipelineStateDesc.SampleDesc.Count = 1;
    m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // 5. PSO�� �� �� ���� �����մϴ�.
    HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);

    // ����� ���� �ӽ� ���ҽ����� �����մϴ�.
    if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
    if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release(); // �����δ� NULL
    if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}