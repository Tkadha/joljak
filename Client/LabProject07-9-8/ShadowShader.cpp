#include "ShadowShader.h"

CShadowShader::CShadowShader()
{
}

CShadowShader::~CShadowShader()
{
}

// 입력 레이아웃: 정점 위치(POSITION)만 필요합니다.
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

// 깊이/스텐실 상태: 깊이 테스트와 쓰기를 활성화합니다.
D3D12_DEPTH_STENCIL_DESC CShadowShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
    ::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
    d3dDepthStencilDesc.DepthEnable = TRUE;
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // 일반적인 깊이 테스트
    d3dDepthStencilDesc.StencilEnable = FALSE;

    return(d3dDepthStencilDesc);
}


D3D12_SHADER_BYTECODE CShadowShader::CreateVertexShader()
{
    // "Shadow.hlsl" 파일을 컴파일합니다.
    //return(CShader::CompileShaderFromFile(L"StandardShaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));

    return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "VS", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CShadowShader::CreatePixelShader()
{
    // 픽셀 셰이더는 아무 작업도 하지 않으므로 NULL을 반환합니다.
    return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "PS", "ps_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_RASTERIZER_DESC CShadowShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc;
    ::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

    // DepthBias: 고정된 값만큼 깊이를 추가
    d3dRasterizerDesc.DepthBias = 1000; 

    // SlopeScaledDepthBias: 표면의 기울기에 따라 바이어스를 다르게 적용
    d3dRasterizerDesc.SlopeScaledDepthBias = 2.0f; // 값을 조금씩 늘려보세요. (예: 1.0f -> 1.5f 또는 2.0f)

    d3dRasterizerDesc.DepthBiasClamp = 0.0f;
    d3dRasterizerDesc.DepthClipEnable = TRUE;

    d3dRasterizerDesc.FrontCounterClockwise = FALSE;
    d3dRasterizerDesc.MultisampleEnable = FALSE;
    d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
    d3dRasterizerDesc.ForcedSampleCount = 0;
    d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


    return d3dRasterizerDesc;
}

ID3D12RootSignature* CShadowShader::CreateRootSignature(ID3D12Device* pd3dDevice)
{
    // Shadow 셰이더는 카메라 정보(b0)와 월드 행렬 정보(b2)만 필요합니다.
    CD3DX12_ROOT_PARAMETER pd3dRootParameters[2];
    pd3dRootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b0: Camera
    pd3dRootParameters[1].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX);    // b2: GameObject 

    CD3DX12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
    d3dRootSignatureDesc.Init(_countof(pd3dRootParameters), pd3dRootParameters, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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