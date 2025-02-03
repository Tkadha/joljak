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
};

// ��ü Ŭ����
struct Object {
    Vec3 position; // ��ü�� ��ġ
    Object(const Vec3& pos) : position(pos) {}
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
    }
    ~Octree();

    // ��ü ����
    void insert(Object* obj);

    // Ư�� ���� �� ��ü �˻�
    void query(const Vec3& obj_pos, const Vec3& distance, std::vector<Object*>& results);
private:
    int depth; // ���� ����� ����

    // ���� ��带 8���� ����
    void subdivide();

    // ���� ��尡 ������ �����ϴ��� Ȯ��
    bool intersects(const Vec3& queryMin, const Vec3& queryMax) const;
};