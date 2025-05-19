#include "stdafx.h" // ������Ʈ�� �°� ����
#include "ComputeShader.h"
#include <d3dcompiler.h> // D3DCompileFromFile

CComputeShaderWrapper::CComputeShaderWrapper() {}
CComputeShaderWrapper::~CComputeShaderWrapper() {}

// �� �Լ��� �׷��Ƚ� ���̴����̹Ƿ�, ��ǻƮ ���ۿ����� ������ �ʽ��ϴ�.
void CComputeShaderWrapper::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature)
{
    // OutputDebugStringA("CComputeShaderWrapper::CreateShader called - this class is for compute shaders. Use BuildComputePipelineState.\n");
    // �ƹ� �۾��� ���� �ʰų�, m_pd3dPipelineState�� nullptr�� ������ �� �ֽ��ϴ�.
    // �Ǵ�, CShader�� m_pd3dPipelineState�� ������� �����Ƿ� �θ� Ŭ������ �Լ��� ȣ������ �ʵ��� �մϴ�.
}

void CComputeShaderWrapper::BuildComputePipelineState(ID3D12Device* pd3dDevice, const std::wstring& shaderFileName, const std::string& entryPoint, ID3D12RootSignature* pd3dRootSignature)
{
    if (!pd3dRootSignature) {
        OutputDebugStringA("Error: Root signature is null for compute PSO creation.\n");
        return;
    }
    SetRootSignature(pd3dRootSignature); // �θ� Ŭ������ ��Ʈ �ñ״�ó ����

    UINT nCompileFlags = 0;
#if defined(_DEBUG)
    nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> pd3dComputeShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pd3dErrorBlob;

    HRESULT hr = D3DCompileFromFile(shaderFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.c_str(), "cs_5_1", nCompileFlags, 0, &pd3dComputeShaderBlob, &pd3dErrorBlob);

    if (FAILED(hr)) {
        if (pd3dErrorBlob) OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
        OutputDebugStringA(("Failed to compile compute shader: " + std::string(shaderFileName.begin(), shaderFileName.end()) + "\n").c_str());
        return;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC computePSODesc = {};
    computePSODesc.pRootSignature = pd3dRootSignature; // ���⼭ SetRootSignature�� ����� ���� ����ϰų�, �Ķ���� ���� ���
    computePSODesc.CS = CD3DX12_SHADER_BYTECODE(pd3dComputeShaderBlob.Get());
    computePSODesc.NodeMask = 0;
    computePSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    hr = pd3dDevice->CreateComputePipelineState(&computePSODesc, IID_PPV_ARGS(&m_pd3dComputePipelineState));
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create Compute PSO in CComputeShaderWrapper.\n");
    }
}