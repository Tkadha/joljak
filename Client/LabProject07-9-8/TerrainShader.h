// TerrainShader.h
#pragma once
#include "Shader.h"

class CTerrainShader : public CShader
{
public:
    CTerrainShader();
    virtual ~CTerrainShader();

    // Input Layout �������̵� (���� �޽��� ����)
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;

    // ���̴� ����Ʈ�ڵ� ���� �������̵�
    virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

    // �ٸ� ����(Rasterizer, Blend, DepthStencil)�� CShader �⺻�� ���
};