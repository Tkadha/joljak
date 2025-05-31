#include "stdafx.h"
#include <iostream>
#include "Terrain.h"
#include "GameObject.h"

GameObject::GameObject()
{
	animationType = ANIMATION_TYPE::UNKNOWN;
	type = OBJECT_TYPE::OB_UNKNOWN;
	xmf4x4 = Matrix4x4::Identity();
	is_alive = true;
	FSM_manager = std::make_shared<FSMManager<GameObject>>(this);
}

GameObject::~GameObject()
{
}

void GameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	XMFLOAT3 xmf3Scale = Terrain::terrain->GetScale();
	int z = (int)(xmf3Position.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = Terrain::terrain->GetHeight(xmf3Position.x, xmf3Position.z, bReverseQuad) + 0.0f;
	xmf3Position.y = fHeight;
	GameObject::SetPosition(xmf3Position);
}
void GameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	xmf4x4 = Matrix4x4::Multiply(mtxRotate, xmf4x4);
}