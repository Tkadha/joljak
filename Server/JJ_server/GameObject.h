#pragma once
#include "stdafx.h"
#include "../Global.h"
#include "FSMManager.h"

class GameObject
{
	OBJECT_TYPE type;
	ANIMATION_TYPE animationType;
	XMFLOAT4X4	xmf4x4; // 11 12 13 right, 21 22 23 up, 31 32 33 look, 41 42 43 position
	int o_id{ -1 };


public:
	GameObject();
	virtual ~GameObject();

	void SetID(int id) { o_id = id; }
	int GetID() { return o_id; }
public:
	void SetPosition(float x, float y, float z) {
		xmf4x4._41 = x;
		xmf4x4._42 = y;
		xmf4x4._43 = z;
	}
	void SetPosition(XMFLOAT3 pos) {
		SetPosition(pos.x, pos.y, pos.z);
	}
	XMFLOAT3 GetPosition() { return(XMFLOAT3(xmf4x4._41, xmf4x4._42, xmf4x4._43)); }

	XMFLOAT3 GetNonNormalizeRight() { return(XMFLOAT3(xmf4x4._11, xmf4x4._12, xmf4x4._13)); }
	XMFLOAT3 GetNonNormalizeUp() { return(XMFLOAT3(xmf4x4._21, xmf4x4._22, xmf4x4._23)); }
	XMFLOAT3 GetNonNormalizeLook() { return(XMFLOAT3(xmf4x4._31, xmf4x4._32, xmf4x4._33)); }

	XMFLOAT3 GetRight() { return(Vector3::Normalize(XMFLOAT3(xmf4x4._11, xmf4x4._12, xmf4x4._13))); }
	XMFLOAT3 GetUp() { return(Vector3::Normalize(XMFLOAT3(xmf4x4._21, xmf4x4._22, xmf4x4._23))); }
	XMFLOAT3 GetLook() { return(Vector3::Normalize(XMFLOAT3(xmf4x4._31, xmf4x4._32, xmf4x4._33))); }
	
	void SetRight(XMFLOAT3 xmf3Right) {
		XMVECTOR vRight = XMLoadFloat3(&xmf3Right);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&xmf4x4._11), vRight);
	}
	void SetUp(XMFLOAT3 xmf3Up) {
		XMVECTOR vUp = XMLoadFloat3(&xmf3Up);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&xmf4x4._21), vUp);
	}
	void SetLook(XMFLOAT3 xmf3Look) {
		XMVECTOR vLook = XMLoadFloat3(&xmf3Look);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&xmf4x4._31), vLook);
	}

	void SetScale(XMFLOAT3 xmf3Scale) {
		XMVECTOR vScale = XMLoadFloat3(&xmf3Scale);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&xmf4x4._11), vScale);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&xmf4x4._21), vScale);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&xmf4x4._31), vScale);
	}
	void SetScale(float x, float y, float z)
	{
		XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
		xmf4x4 = Matrix4x4::Multiply(mtxScale, xmf4x4);
	}

	void SetType(OBJECT_TYPE type) { this->type = type; }
	OBJECT_TYPE GetType() { return type; }

	void SetAnimationType(ANIMATION_TYPE type) { this->animationType = type; }
	ANIMATION_TYPE GetAnimationType() { return animationType; }


	void MoveForward(float fDistance = 1.0f);
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

public:
	bool is_alive;
	std::shared_ptr<FSMManager<GameObject>> FSM_manager = NULL;
	void FSMUpdate()
	{
		if (FSM_manager) FSM_manager->Update();
	}
	void ChangeState(std::shared_ptr<FSMState<GameObject>> newstate)
	{
		FSM_manager->ChangeState(newstate);
	}

public:
	int _level = 0;
	int _hp = 20;
	int _atk = 3;
	void Sethp(int hp) { _hp = hp; }
	void Decreasehp(int num) { _hp -= num; }
	int Gethp() { return _hp; }
	int GetAtk() { return _atk; }
};

