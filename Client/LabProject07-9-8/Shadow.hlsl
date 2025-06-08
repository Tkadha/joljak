// cbuffer�� ���ҽ� ���ε� ������ C++ �ڵ�� ��ġ�ؾ� �մϴ�.
// CScene::Render�� Shadow Pass���� ī�޶�(b0), ������Ʈ(b2)�� ���ε��߽��ϴ�.

cbuffer cbCamera : register(b0)
{
    float4x4 gView;
    float4x4 gProj;
};

cbuffer cbGameObject : register(b2)
{
    float4x4 gWorld;
    // �� cbuffer�� �ٸ� �����Ͱ� �־ ���⼭�� gWorld�� ����մϴ�.
};

struct VS_IN
{
    float3 PosL : POSITION;
};

struct VS_OUT
{
    float4 PosH : SV_POSITION;
};

// ���� ���̴�: ��ġ ��ȯ�� ����
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;
    float4x4 gViewProj = mul(gView, gProj);
    vout.PosH = mul(mul(float4(vin.PosL, 1.0f), gWorld), gViewProj);
    return vout;
}

// �ȼ� ���̴�: �ƹ� �۾��� ���� �ʽ��ϴ�.
// ���� ���� �ʿ��ϹǷ� ���� ����� �����ϴ�.
void PS()
{
}