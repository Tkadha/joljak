#include <iostream>
#include <vector>

// 3D 벡터 클래스
struct Vec3 {
    float x, y, z;

    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    float distanceTo(const Vec3& other) const {
        return std::sqrt((x - other.x) * (x - other.x) +
            (y - other.y) * (y - other.y) +
            (z - other.z) * (z - other.z));
    }

    // 점이 특정 범위 내에 있는지 확인
    bool isWithin(const Vec3& min, const Vec3& max) const {
        return x >= min.x && x <= max.x &&
            y >= min.y && y <= max.y &&
            z >= min.z && z <= max.z;
    }

    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    bool operator==(const Vec3& other) const {
        return (x == other.x && y == other.y && z == other.z);
    }
};

// 객체 클래스
struct Object {
    char u_id;
    Vec3 position; // 객체의 위치
    Object(char id, const Vec3& pos) : u_id(id), position(pos) {}

    bool operator==(const Object& other) {
        return (u_id == other.u_id && position == other.position);
    }
};

// 옥트리 노드 클래스
class Octree {
public:
    Vec3 minBound, maxBound; // 현재 노드의 경계
    std::vector<Object*> objects; // 현재 노드에 포함된 객체들
    Octree* children[8] = { nullptr }; // 8개의 하위 노드
    int maxObjects = 4; // 노드가 분할되기 전 허용되는 최대 객체 수
    int maxDepth = 5;   // 최대 분할 깊이

    Octree(const Vec3& min, const Vec3& max, int depth = 0)
        : minBound(min), maxBound(max), depth(depth) {
        maxObjects = 4 * (depth + 1);
        objects.reserve(maxObjects);
        std::fill(std::begin(children), std::end(children), nullptr);
    }
    ~Octree();

    void insert(Object* obj);
    void remove(Object* obj);
    void update(Object* obj, const Vec3& newpos);
    // 특정 범위 내 객체 검색
    void query(const Object& obj, const Vec3& distance, std::vector<Object*>& results);

private:
    int depth; // 현재 노드의 깊이

    // 현재 노드를 8개로 분할
    void subdivide();

    // 현재 노드가 범위와 교차하는지 확인
    bool intersects(const Vec3& queryMin, const Vec3& queryMax) const;
};