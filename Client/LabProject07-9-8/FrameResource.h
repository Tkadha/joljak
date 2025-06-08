#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

#ifndef MAX_LIGHTS
#define MAX_LIGHTS						16 
#endif

// HLSL�� cbuffer�� 1:1�� �����Ǵ� ������Ʈ ��� ����ü�Դϴ�.
// ���� CGameObject::Render�� �ִ� cbGameObjectInfo�� ��ü�մϴ�.
struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4(); // �ؽ�ó ��ȯ�� �ʿ��ϴٸ� ���

    //--- ���� MaterialInfoCpp�� ������ ���� ---
    DirectX::XMFLOAT4   AmbientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT4   DiffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT4   SpecularColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    DirectX::XMFLOAT4   EmissiveColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    float      Glossiness = 20.0f;
    float      Smoothness = 0.5f;
    float      SpecularHighlight = 1.0f;
    float      Metallic = 0.0f;
    float      GlossyReflection = 0.0f;
    UINT       gnTexturesMask = 0;
    // �е��� UploadBuffer�� �ڵ����� ó���� �ݴϴ�.
};

// HLSL�� cbuffer�� 1:1�� �����Ǵ� Pass ��� ����ü�Դϴ�.
struct PassConstants
{
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

    DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    // --- �׸��ڸ� ���� �߰��� ��� ---
    DirectX::XMFLOAT4X4 ShadowTransform = MathHelper::Identity4x4();

    // Lights ����ü�� ���� ������ StructuredBuffer�� �����ϴ� ���� ��õ������,
    // �ϴ��� d3d12book ����ó�� PassConstants�� ������ �� �ֽ��ϴ�.
    Light Lights[MAX_LIGHTS];
};


class FrameResource
{
public:
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();

    // �� �������� �ڽŸ��� Command Allocator�� �����ϴ�.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

    // �� �������� �ڽŸ��� ��� ���۸� �����ϴ�.
    std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
    std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
    // ���⿡ SkinnedCB, MaterialBuffer �� �ʿ��� ���۵��� �߰��� �� �ֽ��ϴ�.

    // GPU�� �� FrameResource�� Ŀ�ǵ���� ��� ó���� ������ ��ٸ��� ���� �潺 ���Դϴ�.
    UINT64 Fence = 0;
};