#include "Octree.h"

Octree::~Octree() {

    for (auto& child : children)
        delete child;
}

// 객체 삽입
void Octree::insert(std::unique_ptr<tree_obj> obj) {
    // 객체가 현재 노드에 속하지 않으면 무시
    if (!obj->isWithin(minBound, maxBound)) return;

    // 하위 노드로 분할되지 않았고, 객체 수가 제한을 초과하지 않으면 추가
    if (objects.size() < maxObjects || depth >= maxDepth) {
        objects.push_back(std::move(obj));
        return;
    }

    // 처음으로 하위 노드를 생성
    if (children[0] == nullptr) subdivide();

    // 하위 노드에 객체 삽입 시도
    for (auto& child : children) {
        if (obj->isWithin(child->minBound, child->maxBound)) {
            child->insert(std::move(obj));
            return;  // 올바른 하위 노드에 삽입되면 종료
        }
    }
}

bool Octree::remove(char id)
{
    auto it = std::find_if(objects.begin(), objects.end(), [id](const std::unique_ptr<tree_obj>& obj){
        return obj->u_id == id;
        });
    if (it != objects.end()) {
        objects.erase(it);
        return true;
    }
    // 자식 노드에서 제거 시도
    for (auto& child : children) {
        if (child != nullptr && child->remove(id)) {
            return true;
        }
    }
    return false;
}

void Octree::update(char id, const XMFLOAT3& newpos)
{
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if ((*it)->u_id == id) {
            if ((*it)->isWithin(minBound, maxBound)) {
                (*it)->position = newpos;
                return;
            }

            // 객체를 이동 후 제거 및 재삽입
            std::unique_ptr<tree_obj> temp = std::move(*it);
            objects.erase(it);

            temp->position = newpos;
            insert(std::move(temp));
            return;
        }
    }

    for (auto& child : children) {
        if (child) {
            child->update(id, newpos);
        }
    }
}

// 특정 범위 내 객체 검색
void Octree::query(const tree_obj& object, const XMFLOAT3& distance, std::vector<tree_obj*>& results) {
    // 현재 노드가 범위와 교차하지 않으면 종료
    if (!intersects(object.Sub_Pos_return(distance), object.Add_Pos_return(distance))) return;

    // 현재 노드의 객체 추가
    for (auto& obj : objects) {
        if (obj->u_id == object.u_id) continue;
        if (obj->isWithin(object.Sub_Pos_return(distance), object.Add_Pos_return(distance))) {
            results.push_back(obj.get());
        }
    }

    // 하위 노드에 대해 검색
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

// 현재 노드가 범위와 교차하는지 확인
bool Octree::intersects(const XMFLOAT3& queryMin, const XMFLOAT3& queryMax) const {
    return !(queryMax.x < minBound.x || queryMin.x > maxBound.x ||
        queryMax.y < minBound.y || queryMin.y > maxBound.y ||
        queryMax.z < minBound.z || queryMin.z > maxBound.z);
}