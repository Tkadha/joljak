    // TerrainShader.cpp
#include "TerrainShader.h"

CTerrainShader::CTerrainShader(){}

CTerrainShader::~CTerrainShader(){}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
    UINT nInputElementDescs = 5;
    // 동적 할당 후 CShader::CreateShader에서 해제됨
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    // CHeightMapGridMesh::OnPreRender 에서 설정하는 버퍼 순서와 일치해야 함
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
    // 분리된 HLSL 파일 사용
    return CompileShaderFromFile(L"TerrainShaders.hlsl", "VSTerrain", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader()
{
    // 분리된 HLSL 파일 사용
    return CompileShaderFromFile(L"TerrainShaders.hlsl", "PSTerrain", "ps_5_1", &m_pd3dPixelShaderBlob);
}

ID3D12RootSignature* CTerrainShader::CreateRootSignature(ID3D12Device* pd3dDevice)
{
    CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];
    pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 1, 0); // 텍스쳐 4개 
    pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9, 0); // t9

    CD3DX12_ROOT_PARAMETER pd3dRootParameters[5]; // CBV(b1 Camera), Constants(b2 Object)
    pd3dRootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL); // 둘 다 사용
    pd3dRootParameters[1].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL);     
    pd3dRootParameters[2].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_PIXEL);  // b4: Lights
    pd3dRootParameters[3].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요
    pd3dRootParameters[4].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요


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

#include <comdef.h> // _com_error 사용을 위해 추가

void CTerrainShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    OutputDebugStringA("--- [DEBUG] Creating PSO for TerrainShader ---\n");

    // 1. 루트 서명 유효성 검사
    OutputDebugStringA("[CHECK] Root Signature...");
    if (!pd3dGraphicsRootSignature) {
        OutputDebugStringA(" FAILED: Root Signature pointer is NULL.\n");
        return;
    }
    OutputDebugStringA(" OK\n");
    m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;

    // 2. 입력 레이아웃 생성 및 검사
    OutputDebugStringA("[CHECK] Input Layout...");
    m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
    if (!m_d3dPipelineStateDesc.InputLayout.pInputElementDescs || m_d3dPipelineStateDesc.InputLayout.NumElements == 0) {
        OutputDebugStringA(" FAILED: Input Layout creation failed.\n");
        return;
    }
    OutputDebugStringA(" OK\n");

    // 3. 정점 셰이더 생성 및 검사
    OutputDebugStringA("[CHECK] Vertex Shader...");
    m_d3dPipelineStateDesc.VS = CreateVertexShader();
    if (!m_pd3dVertexShaderBlob) { // 멤버 변수 m_pd3dVertexShaderBlob 확인
        OutputDebugStringA(" FAILED: Vertex Shader blob is NULL.\n");
        return;
    }
    OutputDebugStringA(" OK\n");

    // 4. 픽셀 셰이더 생성 및 검사
    OutputDebugStringA("[CHECK] Pixel Shader...");
    m_d3dPipelineStateDesc.PS = CreatePixelShader();
    if (!m_pd3dPixelShaderBlob) { // 멤버 변수 m_pd3dPixelShaderBlob 확인
        OutputDebugStringA(" FAILED: Pixel Shader blob is NULL.\n");
        return;
    }
    OutputDebugStringA(" OK\n");

    // 5. 나머지 상태 설정 (이 부분은 보통 문제가 없습니다)
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

    // 6. 최종 PSO 생성 시도 및 예외 처리
    OutputDebugStringA("[ATTEMPT] Creating Graphics Pipeline State...\n");
    try
    {
        HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);
        if (FAILED(hResult))
        {
            // _com_error 예외를 통해 HRESULT 오류 코드를 얻습니다.
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

    // 7. 최종 결과 확인
    if (m_pd3dPipelineState) {
        OutputDebugStringA("--- [SUCCESS] PSO for TerrainShader created successfully! ---\n");
    }
    else {
        OutputDebugStringA("--- [FAILED] PSO for TerrainShader is NULL after creation attempt. ---\n");
    }

    // 사용이 끝난 임시 리소스들을 정리합니다.
    if (m_pd3dVertexShaderBlob) { m_pd3dVertexShaderBlob->Release(); m_pd3dVertexShaderBlob = nullptr; }
    if (m_pd3dPixelShaderBlob) { m_pd3dPixelShaderBlob->Release(); m_pd3dPixelShaderBlob = nullptr; }
    if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) { delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs; }
}