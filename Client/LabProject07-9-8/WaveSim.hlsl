//=============================================================================
// WaveSim.hlsl by Frank Luna (C) 2011 All Rights Reserved.
//
// UpdateWavesCS(): Solves 2D wave equation using the compute shader.
//
// DisturbWavesCS(): Runs one thread to disturb a grid height and its
//     neighbors to generate a wave. 
//=============================================================================

// For updating the simulation.
cbuffer cbUpdateSettings
{
	float gWaveConstant0;
	float gWaveConstant1;
	float gWaveConstant2;
	
    float gDisturbMag;
    int2 gDisturbIndex; // ���� ��ġ �ε���
    float gTimeStep;
    float gSpatialStep;
};
 
RWTexture2D<float> gPrevSolInput : register(u0);
RWTexture2D<float> gCurrSolInput : register(u1);
RWTexture2D<float3> gOutput       : register(u2);
 
[numthreads(16, 16, 1)]
void UpdateWavesCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
	// We do not need to do bounds checking because:
	//	 *out-of-bounds reads return 0, which works for us--it just means the boundary of 
	//    our water simulation is clamped to 0 in local space.
	//   *out-of-bounds writes are a no-op.
	
	int x = dispatchThreadID.x;
	int y = dispatchThreadID.y;

	gOutput[int2(x,y)] = 
		gWaveConstant0 * gPrevSolInput[int2(x,y)].r +
		gWaveConstant1 * gCurrSolInput[int2(x,y)].r +
		gWaveConstant2 *(
			gCurrSolInput[int2(x,y+1)].r + 
			gCurrSolInput[int2(x,y-1)].r + 
			gCurrSolInput[int2(x+1,y)].r + 
			gCurrSolInput[int2(x-1,y)].r);
}

[numthreads(1, 1, 1)]
void DisturbWavesCS(int3 groupThreadID : SV_GroupThreadID,
                    int3 dispatchThreadID : SV_DispatchThreadID)
{
	// We do not need to do bounds checking because:
	//	 *out-of-bounds reads return 0, which works for us--it just means the boundary of 
	//    our water simulation is clamped to 0 in local space.
	//   *out-of-bounds writes are a no-op.
	
	int x = gDisturbIndex.x;
	int y = gDisturbIndex.y;

	float halfMag = 0.5f*gDisturbMag;

	// Buffer is RW so operator += is well defined.
	gOutput[int2(x,y)]   += gDisturbMag;
	gOutput[int2(x+1,y)] += halfMag;
	gOutput[int2(x-1,y)] += halfMag;
	gOutput[int2(x,y+1)] += halfMag;
	gOutput[int2(x,y-1)] += halfMag;
}



[numthreads(16, 16, 1)] // ������ �׷�� ������ ��
void CS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // �׸��� ��� üũ
    uint width, height;
    gCurrSolInput.GetDimensions(width, height);

    if (dispatchThreadID.x >= width || dispatchThreadID.y >= height)
        return;

    // �߾� ������ �̿��� �ĵ� ������ ���
    // (i,j)�� dispatchThreadID.xy�� ����
    int i = dispatchThreadID.x;
    int j = dispatchThreadID.y;

    float3 prevPos = float3(0, 0, 0);
    float3 currPos = float3(0, 0, 0);
    float3 nextPos = float3(0, 0, 0);

    // �����ڸ� ó�� (0���� ���� �Ǵ� �ٸ� ��� ����)
    // ���⼭�� d3d12book ����ó�� ��� �ٱ��� ���� ��ġ�� �״�� ����ϵ��� �ܼ�ȭ
    // (�����δ� �� ������ ��� ������ �ʿ��� �� ����)
    if (i == 0 || i == width - 1 || j == 0 || j == height - 1)
    {
        gOutput[dispatchThreadID.xy] = gCurrSolInput[dispatchThreadID.xy];
    }
    else
    {
        prevPos = gPrevSolInput[dispatchThreadID.xy];
        currPos = gCurrSolInput[dispatchThreadID.xy];

        // �̿� �ȼ� �� �б�
        float3 L = gCurrSolInput[int2(i - 1, j)];
        float3 R = gCurrSolInput[int2(i + 1, j)];
        float3 T = gCurrSolInput[int2(i, j - 1)];
        float3 B = gCurrSolInput[int2(i, j + 1)];

        // �ĵ� ������ ���� (���� y���� ����, xz�� ���� ����)
        nextPos.y = gWaveConstant0 * currPos.y +
                    gWaveConstant1 * prevPos.y +
                    gWaveConstant2 * (L.y + R.y + T.y + B.y);

        // x, z ��ǥ�� �Է°� �����ϰ� ���� (�ĵ� �ùķ��̼��� ���̸� �����Ѵٰ� ����)
        nextPos.x = currPos.x;
        nextPos.z = currPos.z;

        gOutput[dispatchThreadID.xy] = nextPos;
    }

    // ����ڰ� ������ ��ġ�� ���� �߰� (��: ���콺 Ŭ��)
    // Disturb ������ CPU���� �����ϰ�, ��ǻƮ ���̴������� �̹� ���� ����� �ݿ��ϰų�,
    // �Ǵ� CPU���� ���� ��� ����(gOutput)�� Ư�� ��ġ�� ������ �� �ֽ��ϴ�.
    // ���⼭�� ������ cb�� �ִ� gDisturbIndex ��ġ�� ���̸� gDisturbMag�� ����
    if (dispatchThreadID.x == gDisturbIndex.x && dispatchThreadID.y == gDisturbIndex.y && gDisturbMag > 0.0f)
    {
        gOutput[dispatchThreadID.xy].y += gDisturbMag; // �Ǵ� = gDisturbMag;
    }
}