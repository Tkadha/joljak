#include "stdafx.h"
#include "Octree.h"

Octree Octree::PlayerOctree = { XMFLOAT3 {0,0,0}, XMFLOAT3{10200,6000,10200} };
Octree Octree::GameObjectOctree{ XMFLOAT3 {0,0,0}, XMFLOAT3{10200,6000,10200} };

Octree::~Octree() {

    for (auto& child : children)
        delete child;
}

void Octree::insert(std::unique_ptr<tree_obj> obj) {
    std::unique_lock<std::mutex> oct_lock(oct_mu);
    if (!obj->isWithin(minBound, maxBound)) return;

    if (objects.size() < maxObjects || depth >= maxDepth) {
        objects.push_back(std::move(obj));
        return;
    }

    if (children[0] == nullptr) subdivide();
    oct_lock.unlock();
    for (auto& child : children) {
        if (obj->isWithin(child->minBound, child->maxBound)) {
            child->insert(std::move(obj));
            return;  
        }
    }
}

bool Octree::remove(long long id)
{
    std::unique_lock<std::mutex> oct_lock(oct_mu);
    auto it = std::find_if(objects.begin(), objects.end(), [id](const std::unique_ptr<tree_obj>& obj){
        return obj->u_id == id;
        });
    if (it != objects.end()) {
        objects.erase(it);
        return true;
    }
    oct_lock.unlock();
    for (auto& child : children) {
        if (child != nullptr && child->remove(id)) {
            return true;
        }
    }
    return false;
}

void Octree::update(long long id, const XMFLOAT3& newpos)
{
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if (!*it) continue;
        if ((*it)->u_id == id) {
            (*it)->position = newpos;
            if ((*it)->isWithin(minBound, maxBound)) return;

            std::unique_ptr<tree_obj> temp = std::move(*it);
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
void Octree::query(const tree_obj& object, const XMFLOAT3& distance, std::vector<tree_obj*>& results) {
    std::unique_lock<std::mutex> oct_lock(oct_mu);
    if (!intersects(object.Sub_Pos_return(distance), object.Add_Pos_return(distance))) return;

    for (auto& obj : objects) {
        if (!obj) continue;
        if (obj->u_id == object.u_id) continue;
        if (obj->isWithin(object.Sub_Pos_return(distance), object.Add_Pos_return(distance))) {
            results.push_back(obj.get());
        }
    }
    oct_lock.unlock();
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

bool Octree::intersects(const XMFLOAT3& queryMin, const XMFLOAT3& queryMax) const {
    return !(queryMax.x < minBound.x || queryMin.x > maxBound.x ||
        queryMax.y < minBound.y || queryMin.y > maxBound.y ||
        queryMax.z < minBound.z || queryMin.z > maxBound.z);
}