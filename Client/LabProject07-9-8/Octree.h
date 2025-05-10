#pragma once
#include <iostream>
#include "stdafx.h"


// 객체 클래스
struct tree_obj {
    int u_id;
    XMFLOAT3 position; // 객체의 위치
    tree_obj(int id, const XMFLOAT3& pos) : u_id(id), position(pos) {}

    float distanceTo(const tree_obj& other) const {
        return std::sqrt((position.x - other.position.x) * (position.x - other.position.x) +
            (position.y - other.position.y) * (position.y - other.position.y) +
            (position.z - other.position.z) * (position.z - other.position.z));
    }

    // 점이 특정 범위 내에 있는지 확인
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

// 옥트리 노드 클래스
class Octree {
public:
    XMFLOAT3 minBound, maxBound; // 현재 노드의 경계
    std::vector<std::unique_ptr<tree_obj>> objects; // 현재 노드에 포함된 객체들
    Octree* children[8] = { nullptr }; // 8개의 하위 노드
    int maxObjects = 6; // 노드가 분할되기 전 허용되는 최대 객체 수
    int maxDepth = 5;   // 최대 분할 깊이

    Octree(const XMFLOAT3& min, const XMFLOAT3& max, int depth = 0)
        : minBound(min), maxBound(max), depth(depth) {
        maxObjects = 100 * (depth + 1);
        objects.reserve(maxObjects);
    }
    ~Octree();

    void insert(std::unique_ptr<tree_obj> obj);
    bool remove(int id);
    void update(int id, const XMFLOAT3& newpos);
    // 특정 범위 내 객체 검색
    void query(const tree_obj& obj, const XMFLOAT3& distance, std::vector<tree_obj*>& results);

private:
    int depth; // 현재 노드의 깊이

    // 현재 노드를 8개로 분할
    void subdivide();

    // 현재 노드가 범위와 교차하는지 확인
    bool intersects(const XMFLOAT3& queryMin, const XMFLOAT3& queryMax) const;
};