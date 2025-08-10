#include "stdafx.h"
#include "SkinnedShadowShader.h"

CSkinnedShadowShader::CSkinnedShadowShader() : CSkinnedAnimationStandardShader()
{
}
CSkinnedShadowShader::~CSkinnedShadowShader() {}

// Vertex Shader는 새로 만든 VSSkinnedAnimationShadow 함수를 컴파일합니다.
D3D12_SHADER_BYTECODE CSkinnedShadowShader::CreateVertexShader()
{
    return(CShader::CompileShaderFromFile(L"SkinnedShaders.hlsl", "VSSkinnedAnimationShadow", "vs_5_1", &m_pd3dVertexShaderBlob));
}

// Pixel Shader는 필요 없으므로 NULL을 반환합니다.
D3D12_SHADER_BYTECODE CSkinnedShadowShader::CreatePixelShader()
{
    return(D3D12_SHADER_BYTECODE{ NULL, 0 });
}

// 뎁스 바이어스가 적용된 래스터라이저 상태를 사용합니다.
D3D12_RASTERIZER_DESC CSkinnedShadowShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc = CShader::CreateRasterizerState();
    d3dRasterizerDesc.DepthBias = 1000;
    d3dRasterizerDesc.SlopeScaledDepthBias = 1.5f;
    return d3dRasterizerDesc;
}

// PSO 생성 시, 컬러 렌더 타겟을 사용하지 않도록 설정합니다.
void CSkinnedShadowShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    // 1. PSO 설계도를 깨끗하게 초기화합니다.
    ::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;

    // 2. 필요한 재료들을 모두 가져옵니다.
    m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
    m_d3dPipelineStateDesc.VS = CreateVertexShader();
    m_d3dPipelineStateDesc.PS = CreatePixelShader(); // NULL 픽셀 셰이더
    m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState(); // 뎁스 바이어스 포함
    m_d3dPipelineStateDesc.BlendState = CreateBlendState();
    m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();

    // 3. 그림자 패스에 맞는 렌더 타겟 설정을 명시적으로 지정합니다.
    m_d3dPipelineStateDesc.NumRenderTargets = 0;
    m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 4. 나머지 공통 설정을 합니다.
    m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
    m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_d3dPipelineStateDesc.SampleDesc.Count = 1;
    m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // 5. PSO를 딱 한 번만 생성합니다.
    HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);

    // 사용이 끝난 임시 리소스들을 정리합니다.
    if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
    if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release(); // 실제로는 NULL
    if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}