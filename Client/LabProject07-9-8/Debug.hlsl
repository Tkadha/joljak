Texture2D gDebugTexture : register(t0); // ������� �ؽ�ó�� t0 ���Կ��� ����
SamplerState gssPoint : register(s0); // ���� �⺻���� �� ���÷� ���

struct VS_IN
{
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
};

struct VS_OUT
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};

// ���� ���̴�: 2D �簢���� ���� ��ġ�� UV�� �״�� ����
VS_OUT VS(VS_IN vin)
{
    VS_OUT vout;
    vout.PosH = float4(vin.PosL, 1.0f);
    vout.TexC = vin.TexC;
    return vout;
}

// �ȼ� ���̴�: �ؽ�ó�� ���ø��ؼ� ȭ�鿡 ���
float4 PS(VS_OUT pin) : SV_TARGET
{
    // �׸��� ���� ���� ��(R ä��)�� ������ �����Ƿ�, R ä�� ���� R,G,B�� ��� �־� ������� ǥ��
    float depth = gDebugTexture.Sample(gssPoint, pin.TexC).r;
    return float4(depth, depth, depth, 1.0f);
}