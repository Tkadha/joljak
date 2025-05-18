#pragma once
#pragma once
#include "Shader.h" // CShader �⺻ Ŭ���� ���

class CComputeShaderWrapper : public CShader
{
public:
    CComputeShaderWrapper();
    virtual ~CComputeShaderWrapper();

    // CShader�� ���� ���� �Լ� ����
    virtual std::string GetShaderType() const override { return "ComputeShaderWrapper"; }

    // �׷��Ƚ� PSO ���� �Լ� (CShader���� ���) - ��ǻƮ ���̴��� ��� �� ��
    // ����ΰų�, ȣ�� �� ����/��� ����ϵ��� �������̵� �� �� �ֽ��ϴ�.
    virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature) override;

    // ��ǻƮ PSO ���� �Լ�
    void BuildComputePipelineState(ID3D12Device* pd3dDevice, const std::wstring& shaderFileName, const std::string& entryPoint, ID3D12RootSignature* pd3dRootSignature);
    ID3D12PipelineState* GetComputePSO() const { return m_pd3dComputePipelineState.Get(); }

protected:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pd3dComputePipelineState;
};