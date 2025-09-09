#pragma once
#include "stdafx.h"
#include <vector>
#include <mutex>

static XMFLOAT3 oct_distance{ 2000,5000,2000 };

struct tree_obj {
    long long u_id;
    XMFLOAT3 position; 
    tree_obj(long long id, const XMFLOAT3& pos) : u_id(id), position(pos) {}

    float distanceTo(const tree_obj& other) const {
        return std::sqrt((position.x - other.position.x) * (position.x - other.position.x) +
            (position.y - other.position.y) * (position.y - other.position.y) +
            (position.z - other.position.z) * (position.z - other.position.z));
    }

    
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

class Octree {
public:
    static Octree PlayerOctree;
    static Octree GameObjectOctree;

public:
    Octree(const XMFLOAT3& min, const XMFLOAT3& max, int depth = 0)
        : minBound(min), maxBound(max), depth(depth) {
        maxObjects = 300 * (depth + 1);
        objects.reserve(maxObjects);
    }
    ~Octree();

    void insert(std::unique_ptr<tree_obj> obj);
    bool remove(long long id);
    void update(long long id, const XMFLOAT3& newpos);
    void query(const tree_obj& obj, const XMFLOAT3& distance, std::vector<tree_obj*>& results);
    void clear();

private:
    XMFLOAT3 minBound, maxBound; 
    std::vector<std::unique_ptr<tree_obj>> objects; 
    int maxObjects = 6; 
    int maxDepth = 5;   
    Octree* children[8] = { nullptr }; 
    std::mutex oct_mu;
    int depth;   
    void subdivide();
    bool intersects(const XMFLOAT3& queryMin, const XMFLOAT3& queryMax) const;
};