#include "stdafx.h"
#include <iostream>
#include "Terrain.h"
#include "GameObject.h"

std::unordered_map<OBJECT_TYPE, std::shared_ptr<BoundingOrientedBox>> OBB_Manager::obb_list;
std::vector<shared_ptr<GameObject>> GameObject::gameObjects;


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

	// 충돌처리
	//XMFLOAT3 test_move_x = GetPosition();
	//test_move_x.x = xmf3Position.x;
	//BoundingOrientedBox testOBBX;
	//XMMATRIX matX = XMMatrixTranslation(test_move_x.x, test_move_x.y, test_move_x.z);
	//local_obb.Transform(testOBBX, matX);
	//testOBBX.Orientation.w = 1.f;
	//// 객체 순환해서 충돌 체크
	//bool bCollided = false;
	//{
	//
	//}
	//if (bCollided) xmf3Position.x = GetPosition().x;



	GameObject::SetPosition(xmf3Position);
}
void GameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	xmf4x4 = Matrix4x4::Multiply(mtxRotate, xmf4x4);
}

void GameObject::UpdateTransform()
{
	XMMATRIX worldMatrix = XMLoadFloat4x4(&xmf4x4);


	XMVECTOR localCenter = XMLoadFloat3(&local_obb.Center);
	XMVECTOR worldCenter = XMVector3TransformCoord(localCenter, worldMatrix);
	XMStoreFloat3(&world_obb.Center, worldCenter);


	XMMATRIX rotationMatrix = worldMatrix;
	rotationMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR orientation = XMQuaternionRotationMatrix(rotationMatrix);
	XMStoreFloat4(&world_obb.Orientation, orientation);


	XMFLOAT3 scale;
	scale.x = XMVectorGetX(XMVector3Length(worldMatrix.r[0]));
	scale.y = XMVectorGetX(XMVector3Length(worldMatrix.r[1]));
	scale.z = XMVectorGetX(XMVector3Length(worldMatrix.r[2]));
	world_obb.Extents.x = local_obb.Extents.x * scale.x;
	world_obb.Extents.y = local_obb.Extents.y * scale.y;
	world_obb.Extents.z = local_obb.Extents.z * scale.z;
}
