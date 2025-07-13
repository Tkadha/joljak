// SkyBoxShader.cpp
#include "DebugShader.h"

CDebugShader::CDebugShader() {}

CDebugShader::~CDebugShader() {}

D3D12_INPUT_LAYOUT_DESC CDebugShader::CreateInputLayout()
{
    // 입력 요소가 2개 (POSITION, TEXCOORD) 이므로 크기를 2로 바꿉니다.
    UINT nInputElementDescs = 2;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    // 0번 요소: 위치
    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    // --- 1번 요소: 텍스처 좌표(UV)를 추가합니다. ---
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
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 깊이 쓰기는 안 함
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; 
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

D3D12_RASTERIZER_DESC CDebugShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc;
    ::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

    // 삼각형 내부를 채워서 그립니다.
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    // --- 가장 중요한 부분: 컬링을 완전히 끕니다. ---
    // 이제 삼각형의 앞면이든 뒷면이든 상관없이 무조건 그리게 됩니다.
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
    // 분리된 HLSL 파일 사용
    return CompileShaderFromFile(L"Debug.hlsl", "VS", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CDebugShader::CreatePixelShader()
{
    // 분리된 HLSL 파일 사용
    return CompileShaderFromFile(L"Debug.hlsl", "PS", "ps_5_1", &m_pd3dPixelShaderBlob);
}

ID3D12RootSignature* CDebugShader::CreateRootSignature(ID3D12Device* pd3dDevice)
{
    CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];
    pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    CD3DX12_ROOT_PARAMETER pd3dRootParameters[1];
    pd3dRootParameters[0].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서 샘플링

    auto staticSamplers = GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(pd3dRootParameters), pd3dRootParameters,
        1, &staticSamplers[0],
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    HRESULT hr;
    ID3D12RootSignature* pd3dRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
    hr = pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
    if (FAILED(hr)) return nullptr;
    return pd3dRootSignature;
}