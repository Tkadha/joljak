#include "Octree.h"

Octree::~Octree() {
    for (auto& child : children)
        delete child;
}

// ��ü ����
void Octree::insert(Object* obj) {
    // ��ü�� ���� ��忡 ������ ������ ����
    if (!obj->position.isWithin(minBound, maxBound)) return;

    // ���� ���� ���ҵ��� �ʾҰ�, ��ü ���� ������ �ʰ����� ������ �߰�
    if (objects.size() < maxObjects || depth >= maxDepth) {
        objects.push_back(obj);
        return;
    }

    // ó������ ���� ��带 ����
    if (children[0] == nullptr) subdivide();

    // ���� ��忡 ��ü ���� �õ�
    for (auto& child : children) {
        child->insert(obj);
    }
}

// Ư�� ���� �� ��ü �˻�
void Octree::query(const Vec3& obj_pos, const Vec3& distance, std::vector<Object*>& results) {
    // ���� ��尡 ������ �������� ������ ����
    if (!intersects(obj_pos - distance, obj_pos + distance)) return;

    // ���� ����� ��ü �߰�
    for (auto obj : objects) {
        if (obj->position.isWithin(obj_pos - distance, obj_pos + distance)) {
            results.push_back(obj);
        }
    }

    // ���� ��忡 ���� �˻�
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

// ���� ��尡 ������ �����ϴ��� Ȯ��
bool Octree::intersects(const Vec3& queryMin, const Vec3& queryMax) const {
    return !(queryMax.x < minBound.x || queryMin.x > maxBound.x ||
        queryMax.y < minBound.y || queryMin.y > maxBound.y ||
        queryMax.z < minBound.z || queryMin.z > maxBound.z);
}