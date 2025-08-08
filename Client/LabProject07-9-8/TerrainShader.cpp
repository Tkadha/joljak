    // TerrainShader.cpp
#include "TerrainShader.h"

CTerrainShader::CTerrainShader(){}

CTerrainShader::~CTerrainShader(){}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
    UINT nInputElementDescs = 5;
    // ���� �Ҵ� �� CShader::CreateShader���� ������
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    // CHeightMapGridMesh::OnPreRender ���� �����ϴ� ���� ������ ��ġ�ؾ� ��
    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; 
    pd3dInputElementDescs[1] = { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    pd3dInputElementDescs[2] = { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    pd3dInputElementDescs[3] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; 
    pd3dInputElementDescs[4] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

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

ID3D12RootSignature* CTerrainShader::CreateRootSignature(ID3D12Device* pd3dDevice)
{
    CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];
    pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 1, 0); // �ؽ��� 4�� 
    pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9, 0); // t9

    CD3DX12_ROOT_PARAMETER pd3dRootParameters[5]; // CBV(b1 Camera), Constants(b2 Object)
    pd3dRootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL); // �� �� ���
    pd3dRootParameters[1].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL);     
    pd3dRootParameters[2].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_PIXEL);  // b4: Lights
    pd3dRootParameters[3].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS������ �ʿ�
    pd3dRootParameters[4].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL); // PS������ �ʿ�


    auto d3dStaticSamplers = CShader::GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
    d3dRootSignatureDesc.Init(_countof(pd3dRootParameters), pd3dRootParameters, (UINT)d3dStaticSamplers.size(), d3dStaticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
    return pd3dRootSignature;
}

#include <comdef.h> // _com_error ����� ���� �߰�

void CTerrainShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    OutputDebugStringA("--- [DEBUG] Creating PSO for TerrainShader ---\n");

    // 1. ��Ʈ ���� ��ȿ�� �˻�
    OutputDebugStringA("[CHECK] Root Signature...");
    if (!pd3dGraphicsRootSignature) {
        OutputDebugStringA(" FAILED: Root Signature pointer is NULL.\n");
        return;
    }
    OutputDebugStringA(" OK\n");
    m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;

    // 2. �Է� ���̾ƿ� ���� �� �˻�
    OutputDebugStringA("[CHECK] Input Layout...");
    m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
    if (!m_d3dPipelineStateDesc.InputLayout.pInputElementDescs || m_d3dPipelineStateDesc.InputLayout.NumElements == 0) {
        OutputDebugStringA(" FAILED: Input Layout creation failed.\n");
        return;
    }
    OutputDebugStringA(" OK\n");

    // 3. ���� ���̴� ���� �� �˻�
    OutputDebugStringA("[CHECK] Vertex Shader...");
    m_d3dPipelineStateDesc.VS = CreateVertexShader();
    if (!m_pd3dVertexShaderBlob) { // ��� ���� m_pd3dVertexShaderBlob Ȯ��
        OutputDebugStringA(" FAILED: Vertex Shader blob is NULL.\n");
        return;
    }
    OutputDebugStringA(" OK\n");

    // 4. �ȼ� ���̴� ���� �� �˻�
    OutputDebugStringA("[CHECK] Pixel Shader...");
    m_d3dPipelineStateDesc.PS = CreatePixelShader();
    if (!m_pd3dPixelShaderBlob) { // ��� ���� m_pd3dPixelShaderBlob Ȯ��
        OutputDebugStringA(" FAILED: Pixel Shader blob is NULL.\n");
        return;
    }
    OutputDebugStringA(" OK\n");

    // 5. ������ ���� ���� (�� �κ��� ���� ������ �����ϴ�)
    m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
    m_d3dPipelineStateDesc.BlendState = CreateBlendState();
    m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
    m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
    m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_d3dPipelineStateDesc.NumRenderTargets = 1;
    m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    m_d3dPipelineStateDesc.SampleDesc.Count = 1;
    m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // 6. ���� PSO ���� �õ� �� ���� ó��
    OutputDebugStringA("[ATTEMPT] Creating Graphics Pipeline State...\n");
    try
    {
        HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);
        if (FAILED(hResult))
        {
            // _com_error ���ܸ� ���� HRESULT ���� �ڵ带 ����ϴ�.
            _com_error err(hResult);
            OutputDebugString(err.ErrorMessage());
            OutputDebugStringA("\n");
        }
    }
    catch (_com_error& e)
    {
        wchar_t wszBuffer[256];
        swprintf_s(wszBuffer, L"!!! FATAL ERROR: _com_error exception caught. HRESULT: 0x%08X, Message: %s\n", e.Error(), e.ErrorMessage());
        OutputDebugString(wszBuffer);
    }

    // 7. ���� ��� Ȯ��
    if (m_pd3dPipelineState) {
        OutputDebugStringA("--- [SUCCESS] PSO for TerrainShader created successfully! ---\n");
    }
    else {
        OutputDebugStringA("--- [FAILED] PSO for TerrainShader is NULL after creation attempt. ---\n");
    }

    // ����� ���� �ӽ� ���ҽ����� �����մϴ�.
    if (m_pd3dVertexShaderBlob) { m_pd3dVertexShaderBlob->Release(); m_pd3dVertexShaderBlob = nullptr; }
    if (m_pd3dPixelShaderBlob) { m_pd3dPixelShaderBlob->Release(); m_pd3dPixelShaderBlob = nullptr; }
    if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) { delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs; }
}