#include "Octree.h"

Octree::~Octree() {

    for (auto& child : children)
        delete child;
}

// ��ü ����
void Octree::insert(tree_obj* obj) {
    // ��ü�� ���� ��忡 ������ ������ ����
    if (!obj->isWithin(minBound, maxBound)) return;

    // ���� ���� ���ҵ��� �ʾҰ�, ��ü ���� ������ �ʰ����� ������ �߰�
    if (objects.size() < maxObjects || depth >= maxDepth) {
        objects.push_back(obj);
        return;
    }

    // ó������ ���� ��带 ����
    if (children[0] == nullptr) subdivide();

    // ���� ��忡 ��ü ���� �õ�
    for (auto& child : children) {
        if (obj->isWithin(child->minBound, child->maxBound)) {
            child->insert(obj);
            return;  // �ùٸ� ���� ��忡 ���ԵǸ� ����
        }
    }
}

void Octree::remove(tree_obj* obj)
{
    auto it = std::find(objects.begin(), objects.end(), obj);
    if (it != objects.end()) {
        objects.erase(it);
        return;
    }

    // �ڽ� ��忡�� ���� �õ�
    for (auto& child : children) {
        if (child != nullptr && obj->isWithin(child->minBound, child->maxBound)) {
            child->remove(obj);
            return;
        }
    }
}

void Octree::update(tree_obj* obj, const XMFLOAT3& newpos)
{
    if (obj->isWithin(minBound, maxBound)) {
        obj->position = newpos;
        return;
    }

    // ���� ��忡�� ���� �� ���ο� ��ġ�� �ٽ� ����
    remove(obj);
    obj->position = newpos;
    insert(obj);
}

// Ư�� ���� �� ��ü �˻�
void Octree::query(const tree_obj& object, const XMFLOAT3& distance, std::vector<tree_obj*>& results) {
    // ���� ��尡 ������ �������� ������ ����
    if (!intersects(object.Sub_Pos_return(distance), object.Add_Pos_return(distance))) return;

    // ���� ����� ��ü �߰�
    for (auto& obj : objects) {
        if (obj == &object) continue;
        if (obj->isWithin(object.Sub_Pos_return(distance), object.Add_Pos_return(distance))) {
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
    XMFLOAT3 center = { (minBound.x + maxBound.x) / 2,
                (minBound.y + maxBound.y) / 2,
                (minBound.z + maxBound.z) / 2 };

    children[0] = new Octree(minBound, center, depth + 1);
    children[1] = new Octree(XMFLOAT3(center.x, minBound.y, minBound.z), XMFLOAT3(maxBound.x, center.y, center.z), depth + 1);
    children[2] = new Octree(XMFLOAT3(minBound.x, center.y, minBound.z), XMFLOAT3(center.x, maxBound.y, center.z), depth + 1);
    children[3] = new Octree(XMFLOAT3(center.x, center.y, minBound.z), XMFLOAT3(maxBound.x, maxBound.y, center.z), depth + 1);
    children[4] = new Octree(XMFLOAT3(minBound.x, minBound.y, center.z), XMFLOAT3(center.x, center.y, maxBound.z), depth + 1);
    children[5] = new Octree(XMFLOAT3(center.x, minBound.y, center.z), XMFLOAT3(maxBound.x, center.y, maxBound.z), depth + 1);
    children[6] = new Octree(XMFLOAT3(minBound.x, center.y, center.z), XMFLOAT3(center.x, maxBound.y, maxBound.z), depth + 1);
    children[7] = new Octree(center, maxBound, depth + 1);
}

// ���� ��尡 ������ �����ϴ��� Ȯ��
bool Octree::intersects(const XMFLOAT3& queryMin, const XMFLOAT3& queryMax) const {
    return !(queryMax.x < minBound.x || queryMin.x > maxBound.x ||
        queryMax.y < minBound.y || queryMin.y > maxBound.y ||
        queryMax.z < minBound.z || queryMin.z > maxBound.z);
}