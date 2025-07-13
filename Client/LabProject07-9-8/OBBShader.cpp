// OBBShader.cpp
#include "stdafx.h" // 필요시
#include "OBBShader.h"

COBBShader::COBBShader(){}

COBBShader::~COBBShader(){}

D3D12_INPUT_LAYOUT_DESC COBBShader::CreateInputLayout()
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

D3D12_RASTERIZER_DESC COBBShader::CreateRasterizerState()
{
    // 기본 Rasterizer 상태를 가져와서 수정
    D3D12_RASTERIZER_DESC d3dRasterizerDesc = CShader::CreateRasterizerState();
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME; // 와이어프레임
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;     // 컬링 없음 (양면 보이도록)
    return d3dRasterizerDesc;
}

D3D12_SHADER_BYTECODE COBBShader::CreateVertexShader()
{
    // 분리된 HLSL 파일 사용
    return CompileShaderFromFile(L"OBBShader.hlsl", "VSOBB", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE COBBShader::CreatePixelShader()
{
    // 분리된 HLSL 파일 사용
    return CompileShaderFromFile(L"OBBShader.hlsl", "PSOBB", "ps_5_1", &m_pd3dPixelShaderBlob);
}

// CreateShader 함수 오버라이드 (Primitive Topology 설정 위해)
void COBBShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    // 1. 기본 PSO Desc 설정 (루트서명, VS, PS, 상태들 가져오기)
    ::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
    m_d3dPipelineStateDesc.VS = CreateVertexShader(); // VS 컴파일
    m_d3dPipelineStateDesc.PS = CreatePixelShader();  // PS 컴파일
    m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState(); // 오버라이드된 Rasterizer 사용
    m_d3dPipelineStateDesc.BlendState = CreateBlendState();        // 기본 Blend 사용
    m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState(); // 기본 Depth 사용
    m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();       // 오버라이드된 InputLayout 사용
    m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
    m_d3dPipelineStateDesc.NumRenderTargets = 1;
    m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    m_d3dPipelineStateDesc.SampleDesc.Count = 1;
    m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // --- 2. OBB 셰이더를 위한 수정 ---
    m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; // 라인 리스트로 변경!
    // --- 수정 끝 ---

    // 3. PSO 생성
    HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);

    // 4. 블롭 및 입력 레이아웃 메모리 해제 (베이스 클래스와 동일)
    if (m_pd3dVertexShaderBlob) { m_pd3dVertexShaderBlob->Release(); m_pd3dVertexShaderBlob = NULL; }
    if (m_pd3dPixelShaderBlob) { m_pd3dPixelShaderBlob->Release(); m_pd3dPixelShaderBlob = NULL; }
    if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) { delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs; }
}


ID3D12RootSignature* COBBShader::CreateRootSignature(ID3D12Device* pd3dDevice)
{
    CD3DX12_ROOT_PARAMETER pd3dDescriptorRanges[1]; // CBV(b0 Transform WVP)
    // OBB 셰이더 HLSL은 cbTransform:register(b0) 사용
    pd3dDescriptorRanges[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    // 샘플러나 텍스처 필요 없음
    CD3DX12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc(_countof(pd3dDescriptorRanges), pd3dDescriptorRanges,
        0, nullptr, // No static samplers
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

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