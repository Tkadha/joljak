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
        if (obj->position.isWithin(child->minBound, child->maxBound)) {
            child->insert(obj);
            return;  // �ùٸ� ���� ��忡 ���ԵǸ� ����
        }
    }
}

void Octree::remove(Object* obj)
{
    auto it = std::find(objects.begin(), objects.end(), obj);
    if (it != objects.end()) {
        objects.erase(it);
        return;
    }

    // �ڽ� ��忡�� ���� �õ�
    for (auto& child : children) {
        if (child != nullptr && obj->position.isWithin(child->minBound, child->maxBound)) {
            child->remove(obj);
            return;
        }
    }
}

void Octree::update(Object* obj, const Vec3& newpos)
{
    if (obj->position.isWithin(minBound, maxBound)) {
        obj->position = newpos;
        return;
    }

    // ���� ��忡�� ���� �� ���ο� ��ġ�� �ٽ� ����
    remove(obj);
    obj->position = newpos;
    insert(obj);
}

// Ư�� ���� �� ��ü �˻�
void Octree::query(const Object& object, const Vec3& distance, std::vector<Object*>& results) {
    // ���� ��尡 ������ �������� ������ ����
    if (!intersects(object.position - distance, object.position + distance)) return;

    // ���� ����� ��ü �߰�
    for (auto& obj : objects) {
        if (obj == &object) continue;
        if (obj->position.isWithin(object.position - distance, object.position + distance)) {
            results.push_back(obj);
        }
    }

    // ���� ��忡 ���� �˻�
    for (auto& child : children) {
        if (child != nullptr) {
            child->query(object, distance, results);
        }
    }
}

void Octree::subdivide() {
    Vec3 center = { (minBound.x + maxBound.x) / 2,
                (minBound.y + maxBound.y) / 2,
                (minBound.z + maxBound.z) / 2 };

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