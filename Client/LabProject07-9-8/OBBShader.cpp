// OBBShader.cpp
#include "stdafx.h" // �ʿ��
#include "OBBShader.h"

COBBShader::COBBShader(const std::string& name) { m_strShaderName = name; }

COBBShader::~COBBShader(){}

D3D12_INPUT_LAYOUT_DESC COBBShader::CreateInputLayout()
{
    UINT nInputElementDescs = 1;
    // ���� �Ҵ� �� CShader::CreateShader���� ������
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return d3dInputLayoutDesc;
}

D3D12_RASTERIZER_DESC COBBShader::CreateRasterizerState()
{
    // �⺻ Rasterizer ���¸� �����ͼ� ����
    D3D12_RASTERIZER_DESC d3dRasterizerDesc = CShader::CreateRasterizerState();
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME; // ���̾�������
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;     // �ø� ���� (��� ���̵���)
    return d3dRasterizerDesc;
}

D3D12_SHADER_BYTECODE COBBShader::CreateVertexShader()
{
    // �и��� HLSL ���� ���
    return CompileShaderFromFile(L"OBBShader.hlsl", "VSOBB", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE COBBShader::CreatePixelShader()
{
    // �и��� HLSL ���� ���
    return CompileShaderFromFile(L"OBBShader.hlsl", "PSOBB", "ps_5_1", &m_pd3dPixelShaderBlob);
}

// CreateShader �Լ� �������̵� (Primitive Topology ���� ����)
void COBBShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    // 1. �⺻ PSO Desc ���� (��Ʈ����, VS, PS, ���µ� ��������)
    ::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
    m_d3dPipelineStateDesc.VS = CreateVertexShader(); // VS ������
    m_d3dPipelineStateDesc.PS = CreatePixelShader();  // PS ������
    m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState(); // �������̵�� Rasterizer ���
    m_d3dPipelineStateDesc.BlendState = CreateBlendState();        // �⺻ Blend ���
    m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState(); // �⺻ Depth ���
    m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();       // �������̵�� InputLayout ���
    m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
    m_d3dPipelineStateDesc.NumRenderTargets = 1;
    m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    m_d3dPipelineStateDesc.SampleDesc.Count = 1;
    m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // --- 2. OBB ���̴��� ���� ���� ---
    m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; // ���� ����Ʈ�� ����!
    // --- ���� �� ---

    // 3. PSO ����
    HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);

    // 4. ��� �� �Է� ���̾ƿ� �޸� ���� (���̽� Ŭ������ ����)
    if (m_pd3dVertexShaderBlob) { m_pd3dVertexShaderBlob->Release(); m_pd3dVertexShaderBlob = NULL; }
    if (m_pd3dPixelShaderBlob) { m_pd3dPixelShaderBlob->Release(); m_pd3dPixelShaderBlob = NULL; }
    if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) { delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs; }
}