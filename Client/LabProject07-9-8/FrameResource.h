#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

#ifndef MAX_LIGHTS
#define MAX_LIGHTS						16 
#endif

// HLSL의 cbuffer와 1:1로 대응되는 오브젝트 상수 구조체입니다.
// 기존 CGameObject::Render에 있던 cbGameObjectInfo를 대체합니다.
struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4(); // 텍스처 변환이 필요하다면 사용

    //--- 기존 MaterialInfoCpp와 동일한 구조 ---
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
    // 패딩은 UploadBuffer가 자동으로 처리해 줍니다.
};

// HLSL의 cbuffer와 1:1로 대응되는 Pass 상수 구조체입니다.
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

    // --- 그림자를 위해 추가된 행렬 ---
    DirectX::XMFLOAT4X4 ShadowTransform = MathHelper::Identity4x4();

    // Lights 구조체는 이제 별도의 StructuredBuffer로 관리하는 것을 추천하지만,
    // 일단은 d3d12book 예제처럼 PassConstants에 포함할 수 있습니다.
    Light Lights[MAX_LIGHTS];
};


class FrameResource
{
public:
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();

    // 각 프레임은 자신만의 Command Allocator를 가집니다.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

    // 각 프레임은 자신만의 상수 버퍼를 가집니다.
    std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
    std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
    // 여기에 SkinnedCB, MaterialBuffer 등 필요한 버퍼들을 추가할 수 있습니다.

    // GPU가 이 FrameResource의 커맨드들을 모두 처리할 때까지 기다리기 위한 펜스 값입니다.
    UINT64 Fence = 0;
};