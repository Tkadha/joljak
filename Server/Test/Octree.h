#include <iostream>
#include <vector>

// 3D ���� Ŭ����
struct Vec3 {
    float x, y, z;

    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    float distanceTo(const Vec3& other) const {
        return std::sqrt((x - other.x) * (x - other.x) +
            (y - other.y) * (y - other.y) +
            (z - other.z) * (z - other.z));
    }

    // ���� Ư�� ���� ���� �ִ��� Ȯ��
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

// ��ü Ŭ����
struct Object {
    char u_id;
    Vec3 position; // ��ü�� ��ġ
    Object(char id, const Vec3& pos) : u_id(id), position(pos) {}

    bool operator==(const Object& other) {
        return (u_id == other.u_id && position == other.position);
    }
};

// ��Ʈ�� ��� Ŭ����
class Octree {
public:
    Vec3 minBound, maxBound; // ���� ����� ���
    std::vector<Object*> objects; // ���� ��忡 ���Ե� ��ü��
    Octree* children[8] = { nullptr }; // 8���� ���� ���
    int maxObjects = 4; // ��尡 ���ҵǱ� �� ���Ǵ� �ִ� ��ü ��
    int maxDepth = 5;   // �ִ� ���� ����

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
    // Ư�� ���� �� ��ü �˻�
    void query(const Object& obj, const Vec3& distance, std::vector<Object*>& results);

private:
    int depth; // ���� ����� ����

    // ���� ��带 8���� ����
    void subdivide();

    // ���� ��尡 ������ �����ϴ��� Ȯ��
    bool intersects(const Vec3& queryMin, const Vec3& queryMax) const;
};