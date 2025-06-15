#include "stdafx.h"
#include <iostream>
#include "Terrain.h"
#include "GameObject.h"

int ReadIntegerFromFile(FILE* pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
	return(nValue);
}

float ReadFloatFromFile(FILE* pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
	return(fValue);
}

BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

GameObject::GameObject()
{
	animationType = ANIMATION_TYPE::UNKNOWN;
	type = OBJECT_TYPE::OB_UNKNOWN;
	xmf4x4 = Matrix4x4::Identity();
	is_alive = true;
	FSM_manager = std::make_shared<FSMManager<GameObject>>(this);
}

GameObject::GameObject(std::shared_ptr<CLoadedModelInfo> Model)
{
	if (Model) SetChild(Model->m_pModelRootObject);
	
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

void GameObject::SetOBB(float scalex, float scaley, float scalez, const XMFLOAT3& centeroffset)
{
	if (m_pMesh) {

		XMFLOAT3 minPos = m_pMesh->m_pxmf3Positions[0];
		XMFLOAT3 maxPos = m_pMesh->m_pxmf3Positions[0];
		for (int i = 1; i < m_pMesh->m_nPositions; ++i) {
			minPos.x = min(minPos.x, m_pMesh->m_pxmf3Positions[i].x);
			minPos.y = min(minPos.y, m_pMesh->m_pxmf3Positions[i].y);
			minPos.z = min(minPos.z, m_pMesh->m_pxmf3Positions[i].z);
			maxPos.x = max(maxPos.x, m_pMesh->m_pxmf3Positions[i].x);
			maxPos.y = max(maxPos.y, m_pMesh->m_pxmf3Positions[i].y);
			maxPos.z = max(maxPos.z, m_pMesh->m_pxmf3Positions[i].z);
		}
		localOBB.Center = XMFLOAT3(
			(minPos.x + maxPos.x) * 0.5f + centeroffset.x,
			(minPos.y + maxPos.y) * 0.5f + centeroffset.y,
			(minPos.z + maxPos.z) * 0.5f + centeroffset.z
		);
		localOBB.Extents = XMFLOAT3(
			(maxPos.x - minPos.x) * 0.5f * scalex,
			(maxPos.y - minPos.y) * 0.5f * scaley,
			(maxPos.z - minPos.z) * 0.5f * scalez
		);
		localOBB.Orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	if (m_pSibling) m_pSibling->SetOBB(scalex, scaley, scalez, centeroffset);
	if (m_pChild) m_pChild->SetOBB(scalex, scaley, scalez, centeroffset);
}

void GameObject::UpdateTransform(XMFLOAT4X4* p4x4parent)
{
	xmf4x4 = (p4x4parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *p4x4parent) : m_xmf4x4ToParent;


	XMMATRIX worldMatrix = XMLoadFloat4x4(&xmf4x4);


	XMVECTOR localCenter = XMLoadFloat3(&localOBB.Center);
	XMVECTOR worldCenter = XMVector3TransformCoord(localCenter, worldMatrix);
	XMStoreFloat3(&worldOBB.Center, worldCenter);


	XMMATRIX rotationMatrix = worldMatrix;
	rotationMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR orientation = XMQuaternionRotationMatrix(rotationMatrix);
	XMStoreFloat4(&worldOBB.Orientation, orientation);


	XMFLOAT3 scale;
	scale.x = XMVectorGetX(XMVector3Length(worldMatrix.r[0]));
	scale.y = XMVectorGetX(XMVector3Length(worldMatrix.r[1]));
	scale.z = XMVectorGetX(XMVector3Length(worldMatrix.r[2]));
	worldOBB.Extents.x = localOBB.Extents.x * scale.x;
	worldOBB.Extents.y = localOBB.Extents.y * scale.y;
	worldOBB.Extents.z = localOBB.Extents.z * scale.z;

	if (m_pSibling) m_pSibling->UpdateTransform(p4x4parent);
	if (m_pChild) m_pChild->UpdateTransform(&xmf4x4);
}


std::shared_ptr<GameObject> GameObject::LoadFrameHierarchyFromFile(std::shared_ptr<GameObject> pParent, FILE* pInFile, int* pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	std::shared_ptr<GameObject> pGameObject = make_shared<GameObject>();

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Frame>:"))
		{
			nFrame = ::ReadIntegerFromFile(pInFile);
			nTextures = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
		}
		else if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT3 xmf3Position, xmf3EulerRotation, xmf3Scale;
			XMFLOAT4 xmf4QuaternionRotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3EulerRotation, sizeof(float), 3, pInFile); 
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4QuaternionRotation, sizeof(float), 4, pInFile); 

			XMMATRIX mtxScale = XMMatrixScaling(xmf3Scale.x, xmf3Scale.y, xmf3Scale.z);
			XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(&xmf4QuaternionRotation));
			XMMATRIX mtxTranslate = XMMatrixTranslation(xmf3Position.x, xmf3Position.y, xmf3Position.z);

			XMStoreFloat4x4(&pGameObject->m_xmf4x4ToParent, XMMatrixMultiply(XMMatrixMultiply(mtxScale, mtxRotate), mtxTranslate));
		}
		else if (!strcmp(pstrToken, "<TransformMatrix>:"))
		{
			XMFLOAT4X4 xmf4x4TempWorldMatrix; // 임시 변수
			nReads = (UINT)::fread(&xmf4x4TempWorldMatrix, sizeof(float), 16, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			CStandardMesh* pMesh = new CStandardMesh();
			pMesh->LoadMeshFromFile(pInFile);
			pGameObject->SetMesh(pMesh);
		}
		else if (!strcmp(pstrToken, "<SkinningInfo>:"))
		{
			CSkinnedMesh* pSkinnedMesh = new CSkinnedMesh();
			pSkinnedMesh->LoadSkinInfoFromFile(pInFile);
			delete pSkinnedMesh;
			//::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
			//if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			int nMaterials = ::ReadIntegerFromFile(pInFile);
			if (nMaterials > 0)
			{
				for (int i = 0; i < nMaterials; i++)
				{
					char pstrMaterialName[64] = { '\0' };
					for (;;) {
						::ReadStringFromFile(pInFile, pstrMaterialName);
						XMFLOAT4 tempf4;
						float tempf;
						if (!strcmp(pstrMaterialName, "<Material>:")) {
							int a = ReadIntegerFromFile(pInFile);
						}
						else if (!strcmp(pstrMaterialName, "<AlbedoColor>:"))
						{
							nReads = (UINT)::fread(&tempf4, sizeof(float), 4, pInFile);
						}
						else if (!strcmp(pstrMaterialName, "<EmissiveColor>:"))
						{
							nReads = (UINT)::fread(&tempf4, sizeof(float), 4, pInFile);
						}
						else if (!strcmp(pstrMaterialName, "<SpecularColor>:"))
						{
							nReads = (UINT)::fread(&tempf4, sizeof(float), 4, pInFile);
						}
						else if (!strcmp(pstrMaterialName, "<Glossiness>:"))
						{
							nReads = (UINT)::fread(&tempf, sizeof(float), 1, pInFile);
						}
						else if (!strcmp(pstrMaterialName, "<Smoothness>:"))
						{
							nReads = (UINT)::fread(&tempf, sizeof(float), 1, pInFile);
						}
						else if (!strcmp(pstrMaterialName, "<Metallic>:"))
						{
							nReads = (UINT)::fread(&tempf, sizeof(float), 1, pInFile);
						}
						else if (!strcmp(pstrMaterialName, "<SpecularHighlight>:"))
						{
							nReads = (UINT)::fread(&tempf, sizeof(float), 1, pInFile);
						}
						else if (!strcmp(pstrMaterialName, "<GlossyReflection>:"))
						{
							nReads = (UINT)::fread(&tempf, sizeof(float), 1, pInFile);
						}
						else if (!strcmp(pstrMaterialName, "<AlbedoMap>:"))
						{
							::ReadStringFromFile(pInFile, pstrMaterialName);
						}
						else if (!strcmp(pstrMaterialName, "<SpecularMap>:"))
						{
							::ReadStringFromFile(pInFile, pstrMaterialName);
						}
						else if (!strcmp(pstrMaterialName, "<NormalMap>:"))
						{
							::ReadStringFromFile(pInFile, pstrMaterialName);
						}
						else if (!strcmp(pstrMaterialName, "<MetallicMap>:"))
						{
							::ReadStringFromFile(pInFile, pstrMaterialName);
						}
						else if (!strcmp(pstrMaterialName, "<EmissionMap>:"))
						{
							::ReadStringFromFile(pInFile, pstrMaterialName);
						}
						else if (!strcmp(pstrMaterialName, "<DetailAlbedoMap>:"))
						{
							::ReadStringFromFile(pInFile, pstrMaterialName);
						}
						else if (!strcmp(pstrMaterialName, "<DetailNormalMap>:"))
						{
							::ReadStringFromFile(pInFile, pstrMaterialName);
						}
						else if (!strcmp(pstrMaterialName, "</Materials>"))
						{
							break;
						}

					}
				}
			}
		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					std::shared_ptr<GameObject> pChild = GameObject::LoadFrameHierarchyFromFile(pGameObject, pInFile, pnSkinnedMeshes);
					if (pChild) pGameObject->SetChild(pChild);
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}
std::shared_ptr<CLoadedModelInfo> GameObject::LoadGeometryFromFile(char* pstrFileName)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	std::shared_ptr<CLoadedModelInfo> pLoadedModel = make_shared<CLoadedModelInfo>();
	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Hierarchy>:"))
		{
			pLoadedModel->m_pModelRootObject = GameObject::LoadFrameHierarchyFromFile( NULL, pInFile, &pLoadedModel->m_nSkinnedMeshes);
		}
		else if (!strcmp(pstrToken, "</Hierarchy>"))
		{
			break;
		}
	}
	if (pInFile) fclose(pInFile);
	return(pLoadedModel);
}
std::shared_ptr<GameObject> GameObject::FindFrame(char* pstrFrameName)
{
	std::shared_ptr<GameObject> pFrameObject = NULL;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(shared_from_this());

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}
void GameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void GameObject::SetChild(std::shared_ptr<GameObject> pChild)
{
	if (pChild)
	{
		pChild->m_pParent = shared_from_this();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}