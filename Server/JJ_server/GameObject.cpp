#include "stdafx.h"
#include <iostream>
#include "Player.h"
#include "Terrain.h"
#include "GameObject.h"
#include "Octree.h"
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
	xmf3Position.y = fHeight + fly_height;
	// 충돌처리
	XMFLOAT3 test_move = GetPosition();
	test_move.x = xmf3Position.x;
	BoundingOrientedBox testOBBX;
	XMMATRIX matX = XMMatrixTranslation(test_move.x, test_move.y, test_move.z);
	local_obb.Transform(testOBBX, matX);
	testOBBX.Orientation.w = 1.f;
	// 객체 순환해서 충돌 체크
	std::vector<tree_obj*> presults;
	std::vector<tree_obj*> oresults;
	{
		tree_obj n_obj{ GetID() ,test_move };
		Octree::PlayerOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, presults);
		Octree::GameObjectOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, oresults);
		for (auto& p_obj : presults) {
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second->state != PC_INGAME)continue;
				if (cl.second->m_id != p_obj->u_id) continue;
				if (testOBBX.Intersects(cl.second->world_obb))
				{
					test_move.x = GetPosition().x;
					break;
				}
			}
		}
		for (auto& o_obj : oresults) {
			if (GameObject::gameObjects[o_obj->u_id]->GetID() < 0) continue;
			if (false == GameObject::gameObjects[o_obj->u_id]->is_alive) continue;
			if (testOBBX.Intersects(GameObject::gameObjects[o_obj->u_id]->world_obb))
			{
				test_move.x = GetPosition().x;
				break;
			}
		}
	}
	test_move.z = xmf3Position.z;
	BoundingOrientedBox testOBBZ;
	XMMATRIX matZ = XMMatrixTranslation(test_move.x, test_move.y, test_move.z);
	local_obb.Transform(testOBBZ, matZ);
	testOBBZ.Orientation.w = 1.f;

	presults.clear();
	oresults.clear();
	{
		tree_obj n_obj{ GetID() ,test_move };
		Octree::PlayerOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, presults);
		Octree::GameObjectOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, oresults);
		for (auto& p_obj : presults) {
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second->state != PC_INGAME)continue;
				if (cl.second->m_id != p_obj->u_id) continue;
				if (testOBBX.Intersects(cl.second->world_obb))
				{
					test_move.z = GetPosition().z;
					break;
				}
			}
		}
		for (auto& o_obj : oresults) {
			if (GameObject::gameObjects[o_obj->u_id]->GetID() < 0) continue;
			if (false == GameObject::gameObjects[o_obj->u_id]->is_alive) continue;
			if (testOBBX.Intersects(GameObject::gameObjects[o_obj->u_id]->world_obb))
			{
				test_move.z = GetPosition().z;
				break;
			}
		}
	}


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
