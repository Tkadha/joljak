#pragma once
#include <iostream>
#include "stdafx.h"


// ��ü Ŭ����
struct tree_obj {
    int u_id;
    XMFLOAT3 position; // ��ü�� ��ġ
    tree_obj(int id, const XMFLOAT3& pos) : u_id(id), position(pos) {}

    float distanceTo(const tree_obj& other) const {
        return std::sqrt((position.x - other.position.x) * (position.x - other.position.x) +
            (position.y - other.position.y) * (position.y - other.position.y) +
            (position.z - other.position.z) * (position.z - other.position.z));
    }

    // ���� Ư�� ���� ���� �ִ��� Ȯ��
    bool isWithin(const XMFLOAT3& min, const XMFLOAT3& max) const {
        return position.x >= min.x && position.x <= max.x &&
            position.y >= min.y && position.y <= max.y &&
            position.z >= min.z && position.z <= max.z;
    }
    XMFLOAT3 Add_Pos_return(const XMFLOAT3& p) const {
        return XMFLOAT3(position.x + p.x, position.y + p.y, position.z + p.z);
    }
    XMFLOAT3 Sub_Pos_return(const XMFLOAT3& p) const {
        return XMFLOAT3(position.x - p.x, position.y - p.y, position.z - p.z);
    }
    bool operator==(const tree_obj& other) const {
        return (u_id == other.u_id) &&
            (position.x == other.position.x &&
             position.y == other.position.y &&
             position.z == other.position.z);
    }
};

// ��Ʈ�� ��� Ŭ����
class Octree {
public:
    XMFLOAT3 minBound, maxBound; // ���� ����� ���
    std::vector<std::unique_ptr<tree_obj>> objects; // ���� ��忡 ���Ե� ��ü��
    Octree* children[8] = { nullptr }; // 8���� ���� ���
    int maxObjects = 6; // ��尡 ���ҵǱ� �� ���Ǵ� �ִ� ��ü ��
    int maxDepth = 5;   // �ִ� ���� ����

    Octree(const XMFLOAT3& min, const XMFLOAT3& max, int depth = 0)
        : minBound(min), maxBound(max), depth(depth) {
        maxObjects = 100 * (depth + 1);
        objects.reserve(maxObjects);
    }
    ~Octree();

    void insert(std::unique_ptr<tree_obj> obj);
    bool remove(int id);
    void update(int id, const XMFLOAT3& newpos);
    // Ư�� ���� �� ��ü �˻�
    void query(const tree_obj& obj, const XMFLOAT3& distance, std::vector<tree_obj*>& results);

private:
    int depth; // ���� ����� ����

    // ���� ��带 8���� ����
    void subdivide();

    // ���� ��尡 ������ �����ϴ��� Ȯ��
    bool intersects(const XMFLOAT3& queryMin, const XMFLOAT3& queryMax) const;
};