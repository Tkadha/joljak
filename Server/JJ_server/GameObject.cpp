#include "stdafx.h"
#include <iostream>
#include "Player.h"
#include "Terrain.h"
#include "GameObject.h"
#include "Octree.h"
std::unordered_map<OBJECT_TYPE, std::shared_ptr<BoundingOrientedBox>> OBB_Manager::obb_list;
std::vector<shared_ptr<GameObject>> GameObject::gameObjects;
std::vector<shared_ptr<GameObject>> GameObject::ConstructObjects;


GameObject::GameObject()
{
	animationType = ANIMATION_TYPE::UNKNOWN;
	type = OBJECT_TYPE::OB_UNKNOWN;
	xmf4x4 = Matrix4x4::Identity();
	is_alive = true;
	FSM_manager = std::make_shared<FSMManager<GameObject>>(this);
}

GameObject::GameObject(bool makefsm)
{
	animationType = ANIMATION_TYPE::UNKNOWN;
	type = OBJECT_TYPE::OB_UNKNOWN;
	xmf4x4 = Matrix4x4::Identity();
	is_alive = true;
	FSM_manager = nullptr;
}

GameObject::~GameObject()
{
}

void GameObject::MoveForward(float fDistance)
{
	BoundingOrientedBox myCurrentOBB;
	XMMATRIX myCurrentWorldMat = XMMatrixScalingFromVector(XMLoadFloat3(&GetScale()));
	myCurrentWorldMat *= XMMatrixRotationQuaternion(XMLoadFloat4(&GetOrientation()));
	myCurrentWorldMat *= XMMatrixTranslationFromVector(XMLoadFloat3(&GetPosition()));
	local_obb.Transform(myCurrentOBB, myCurrentWorldMat);
	std::vector<tree_obj*> nearby_objects;
	tree_obj n_obj{ GetID(), GetPosition() };
	Octree::GameObjectOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, nearby_objects);

	XMVECTOR totalPushOutVector = XMVectorSet(0, 0, 0, 0);
	int collisionCount = 0;
	for (auto& other_info : nearby_objects)
	{
		// 자기 자신은 제외
		if (other_info->u_id == GetID()) continue;

		auto& other_obj = GameObject::gameObjects[other_info->u_id];
		if (!other_obj || !other_obj->is_alive) continue;

		if (this->GetType() == OBJECT_TYPE::OB_BAT)
		{
			if (other_obj->GetType() != OBJECT_TYPE::OB_TREE) continue;
		}

		// 충돌했다면 밀어낼 방향 계산
		if (myCurrentOBB.Intersects(other_obj->world_obb))
		{
			XMVECTOR myCenter = XMLoadFloat3(&GetPosition());
			XMVECTOR otherCenter = XMLoadFloat3(&other_obj->GetPosition());

			// 상대방 중심 -> 내 중심 방향으로 밀어낼 벡터 계산
			XMVECTOR pushDir = XMVector3Normalize(XMVectorSubtract(myCenter, otherCenter));

			// 여러 객체와 겹쳤을 경우를 대비해 방향을 더해줌
			totalPushOutVector = XMVectorAdd(totalPushOutVector, pushDir);
			collisionCount++;
		}
	}
	if (collisionCount > 0)
	{
		// 평균적인 밀어내기 방향 계산
		totalPushOutVector = XMVector3Normalize(totalPushOutVector);

		// 밀어내는 힘 (이 값은 실험을 통해 조절해야 합니다)
		float pushMagnitude = 0.5f;

		XMVECTOR currentPosVec = XMLoadFloat3(&GetPosition());
		XMVECTOR newPosVec = XMVectorAdd(currentPosVec, XMVectorScale(totalPushOutVector, pushMagnitude));

		XMFLOAT3 newPos;
		XMStoreFloat3(&newPos, newPosVec);

		// 위치 보정 (이때는 충돌 검사 없이 바로 적용)
		SetPosition(newPos);
	}

	XMFLOAT3 xmf3CurrentPosition = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	XMFLOAT3 xmf3TargetPosition = Vector3::Add(xmf3CurrentPosition, xmf3Look, fDistance);

	xmf3TargetPosition.y = Terrain::terrain->GetHeight(xmf3TargetPosition.x, xmf3TargetPosition.z) + fly_height;

	// 충돌처리
	XMFLOAT3 test_move = xmf3CurrentPosition;
	test_move.x = xmf3TargetPosition.x;
	BoundingOrientedBox testOBBX;

	XMMATRIX matX = XMMatrixScalingFromVector(XMLoadFloat3(&GetScale()));
	matX *= XMMatrixRotationQuaternion(XMLoadFloat4(&GetOrientation()));
	matX *= XMMatrixTranslation(test_move.x, Terrain::terrain->GetHeight(test_move.x, test_move.z), test_move.z);
	local_obb.Transform(testOBBX, matX);
	// 객체 순환해서 충돌 체크
	std::vector<tree_obj*> presults;
	std::vector<tree_obj*> oresults;
	{
		tree_obj n_obj{ GetID() ,test_move };
		Octree::PlayerOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, presults);
		Octree::GameObjectOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, oresults);
		for (auto& p_obj : presults) {
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second->state != PC_INGAME) continue;
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
			if (GameObject::gameObjects[o_obj->u_id]->Gethp() <= 0) continue;
			if (false == GameObject::gameObjects[o_obj->u_id]->is_alive) continue;
			auto t = GameObject::gameObjects[o_obj->u_id]->GetType();
			if (this->GetType() == OBJECT_TYPE::OB_BAT)
			{
				if (t != OBJECT_TYPE::OB_TREE) continue;
			}
			if (testOBBX.Intersects(GameObject::gameObjects[o_obj->u_id]->world_obb))
			{
				test_move.x = GetPosition().x;
				break;
			}
		}
	}
	test_move.z = xmf3TargetPosition.z;
	BoundingOrientedBox testOBBZ;
	XMMATRIX matZ = XMMatrixScalingFromVector(XMLoadFloat3(&GetScale()));
	matZ *= XMMatrixRotationQuaternion(XMLoadFloat4(&GetOrientation()));
	matZ *= XMMatrixTranslation(test_move.x, Terrain::terrain->GetHeight(test_move.x, test_move.z), test_move.z);

	local_obb.Transform(testOBBZ, matZ);

	{
		for (auto& p_obj : presults) {
			for (auto& cl : PlayerClient::PlayerClients) {
				if (cl.second->state != PC_INGAME)continue;
				if (cl.second->m_id != p_obj->u_id) continue;
				if (testOBBZ.Intersects(cl.second->world_obb))
				{
					test_move.z = GetPosition().z;
					break;
				}
			}
		}
		for (auto& o_obj : oresults) {
			if (GameObject::gameObjects[o_obj->u_id]->GetID() < 0) continue;
			if (GameObject::gameObjects[o_obj->u_id]->Gethp() <= 0) continue;
			if (false == GameObject::gameObjects[o_obj->u_id]->is_alive) continue;
			auto t = GameObject::gameObjects[o_obj->u_id]->GetType();
			if (testOBBZ.Intersects(GameObject::gameObjects[o_obj->u_id]->world_obb))
			{
				test_move.z = GetPosition().z;
				break;
			}
		}
	}

	test_move.y = Terrain::terrain->GetHeight(test_move.x, test_move.z) + fly_height;

	GameObject::SetPosition(test_move);
}
void GameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	auto before4x4 = xmf4x4;
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	xmf4x4 = Matrix4x4::Multiply(mtxRotate, xmf4x4);

	BoundingOrientedBox testOBB;
	local_obb.Transform(testOBB, XMLoadFloat4x4(&xmf4x4));

	std::vector<tree_obj*> presults;
	std::vector<tree_obj*> oresults;

	XMVECTOR totalPushOutVector = XMVectorSet(0, 0, 0, 0);
	int collisionCount = 0;

	tree_obj n_obj{ GetID(), GetPosition() };
	Octree::PlayerOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, presults);
	Octree::GameObjectOctree.query(n_obj, XMFLOAT3{ 500,300,500 }, oresults);

	for (auto& p_obj : presults) {
		for (auto& cl : PlayerClient::PlayerClients) {
			if (cl.second->state != PC_INGAME) continue;
			if (cl.second->m_id != p_obj->u_id) continue;

			if (testOBB.Intersects(cl.second->world_obb))
			{
				XMVECTOR myCenter = XMLoadFloat3(&GetPosition());
				XMVECTOR otherCenter = XMLoadFloat3(&cl.second->world_obb.Center);
				XMVECTOR pushDir = XMVector3Normalize(XMVectorSubtract(myCenter, otherCenter));
				totalPushOutVector = XMVectorAdd(totalPushOutVector, pushDir);
				collisionCount++;
				break;
			}
		}
	}

	for (auto& o_obj : oresults) {
		auto& other_obj = GameObject::gameObjects[o_obj->u_id];
		if (!other_obj || !other_obj->is_alive || other_obj->Gethp() <= 0) continue;
		if (this->GetType() == OBJECT_TYPE::OB_BAT)
		{
			if (other_obj->GetType() != OBJECT_TYPE::OB_TREE) continue;
		}
		if (testOBB.Intersects(other_obj->world_obb))
		{
			XMVECTOR myCenter = XMLoadFloat3(&GetPosition());
			XMVECTOR otherCenter = XMLoadFloat3(&other_obj->world_obb.Center);
			XMVECTOR pushDir = XMVector3Normalize(XMVectorSubtract(myCenter, otherCenter));
			totalPushOutVector = XMVectorAdd(totalPushOutVector, pushDir);
			collisionCount++;
		}
	}

	if (collisionCount > 0)
	{
		totalPushOutVector = XMVector3Normalize(totalPushOutVector);
		totalPushOutVector = XMVectorSetY(totalPushOutVector, 0.0f);
		totalPushOutVector = XMVector3Normalize(totalPushOutVector);
		float pushMagnitude = 0.1f;

		// 최종 위치 보정
		xmf4x4._41 += XMVectorGetX(totalPushOutVector) * pushMagnitude;
		//xmf4x4._42 += XMVectorGetY(totalPushOutVector) * pushMagnitude;
		xmf4x4._43 += XMVectorGetZ(totalPushOutVector) * pushMagnitude;
	}
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
