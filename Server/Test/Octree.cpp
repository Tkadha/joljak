#include "Octree.h"

Octree::~Octree() {
    for (auto& child : children)
        delete child;
}

// 객체 삽입
void Octree::insert(Object* obj) {
    // 객체가 현재 노드에 속하지 않으면 무시
    if (!obj->position.isWithin(minBound, maxBound)) return;

    // 하위 노드로 분할되지 않았고, 객체 수가 제한을 초과하지 않으면 추가
    if (objects.size() < maxObjects || depth >= maxDepth) {
        objects.push_back(obj);
        return;
    }

    // 처음으로 하위 노드를 생성
    if (children[0] == nullptr) subdivide();

    // 하위 노드에 객체 삽입 시도
    for (auto& child : children) {
        child->insert(obj);
    }
}

// 특정 범위 내 객체 검색
void Octree::query(const Vec3& obj_pos, const Vec3& distance, std::vector<Object*>& results) {
    // 현재 노드가 범위와 교차하지 않으면 종료
    if (!intersects(obj_pos - distance, obj_pos + distance)) return;

    // 현재 노드의 객체 추가
    for (auto obj : objects) {
        if (obj->position.isWithin(obj_pos - distance, obj_pos + distance)) {
            results.push_back(obj);
        }
    }

    // 하위 노드에 대해 검색
    for (auto& child : children) {
        if (child != nullptr) {
            child->query(obj_pos - distance, obj_pos + distance, results);
        }
    }
}

void Octree::subdivide() {
    Vec3 halfSize = { (maxBound.x - minBound.x) / 2,
                     (maxBound.y - minBound.y) / 2,
                     (maxBound.z - minBound.z) / 2 };
    Vec3 center = { minBound.x + halfSize.x, minBound.y + halfSize.y, minBound.z + halfSize.z };

    children[0] = new Octree(minBound, center, depth + 1);
    children[1] = new Octree(Vec3(center.x, minBound.y, minBound.z), Vec3(maxBound.x, center.y, center.z), depth + 1);
    children[2] = new Octree(Vec3(minBound.x, center.y, minBound.z), Vec3(center.x, maxBound.y, center.z), depth + 1);
    children[3] = new Octree(Vec3(center.x, center.y, minBound.z), Vec3(maxBound.x, maxBound.y, center.z), depth + 1);
    children[4] = new Octree(Vec3(minBound.x, minBound.y, center.z), Vec3(center.x, center.y, maxBound.z), depth + 1);
    children[5] = new Octree(Vec3(center.x, minBound.y, center.z), Vec3(maxBound.x, center.y, maxBound.z), depth + 1);
    children[6] = new Octree(Vec3(minBound.x, center.y, center.z), Vec3(center.x, maxBound.y, maxBound.z), depth + 1);
    children[7] = new Octree(center, maxBound, depth + 1);
}

// 현재 노드가 범위와 교차하는지 확인
bool Octree::intersects(const Vec3& queryMin, const Vec3& queryMax) const {
    return !(queryMax.x < minBound.x || queryMin.x > maxBound.x ||
        queryMax.y < minBound.y || queryMin.y > maxBound.y ||
        queryMax.z < minBound.z || queryMin.z > maxBound.z);
}