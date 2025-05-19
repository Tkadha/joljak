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
    int2 gDisturbIndex; // 교란 위치 인덱스
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



[numthreads(16, 16, 1)] // 스레드 그룹당 스레드 수
void CS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // 그리드 경계 체크
    uint width, height;
    gCurrSolInput.GetDimensions(width, height);

    if (dispatchThreadID.x >= width || dispatchThreadID.y >= height)
        return;

    // 중앙 차분을 이용한 파동 방정식 계산
    // (i,j)는 dispatchThreadID.xy와 동일
    int i = dispatchThreadID.x;
    int j = dispatchThreadID.y;

    float3 prevPos = float3(0, 0, 0);
    float3 currPos = float3(0, 0, 0);
    float3 nextPos = float3(0, 0, 0);

    // 가장자리 처리 (0으로 고정 또는 다른 경계 조건)
    // 여기서는 d3d12book 예제처럼 경계 바깥은 현재 위치를 그대로 사용하도록 단순화
    // (실제로는 더 정교한 경계 조건이 필요할 수 있음)
    if (i == 0 || i == width - 1 || j == 0 || j == height - 1)
    {
        gOutput[dispatchThreadID.xy] = gCurrSolInput[dispatchThreadID.xy];
    }
    else
    {
        prevPos = gPrevSolInput[dispatchThreadID.xy];
        currPos = gCurrSolInput[dispatchThreadID.xy];

        // 이웃 픽셀 값 읽기
        float3 L = gCurrSolInput[int2(i - 1, j)];
        float3 R = gCurrSolInput[int2(i + 1, j)];
        float3 T = gCurrSolInput[int2(i, j - 1)];
        float3 B = gCurrSolInput[int2(i, j + 1)];

        // 파동 방정식 적용 (높이 y값만 변경, xz는 고정 가정)
        nextPos.y = gWaveConstant0 * currPos.y +
                    gWaveConstant1 * prevPos.y +
                    gWaveConstant2 * (L.y + R.y + T.y + B.y);

        // x, z 좌표는 입력과 동일하게 유지 (파도 시뮬레이션이 높이만 변경한다고 가정)
        nextPos.x = currPos.x;
        nextPos.z = currPos.z;

        gOutput[dispatchThreadID.xy] = nextPos;
    }

    // 사용자가 지정한 위치에 교란 추가 (예: 마우스 클릭)
    // Disturb 로직은 CPU에서 제어하고, 컴퓨트 셰이더에서는 이미 계산된 결과에 반영하거나,
    // 또는 CPU에서 직접 출력 버퍼(gOutput)의 특정 위치를 수정할 수 있습니다.
    // 여기서는 간단히 cb에 있는 gDisturbIndex 위치의 높이를 gDisturbMag로 설정
    if (dispatchThreadID.x == gDisturbIndex.x && dispatchThreadID.y == gDisturbIndex.y && gDisturbMag > 0.0f)
    {
        gOutput[dispatchThreadID.xy].y += gDisturbMag; // 또는 = gDisturbMag;
    }
}