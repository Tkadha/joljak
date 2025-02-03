// -------------------------------------------------------
// 
// 해당 프로젝트에서
// 
// -------------------------------------------------------

#include <iostream>
#include "Octree.h"

Octree oct(Vec3(0, 0, 0), Vec3(1500, 1500, 1500));

int main()
{
    Object obj1(Vec3(100, 200, 300));
    Object obj2(Vec3(400, 500, 600));
    Object obj3(Vec3(1400, 1200, 1300));
    Object obj4(Vec3(110, 210, 310));
    oct.insert(&obj1);
    oct.insert(&obj2);
    oct.insert(&obj3);
    oct.insert(&obj4);

    std::vector<Object*> results;
    oct.query(obj4.position, Vec3(400, 400, 400), results);

    
    for (auto obj : results) {
        std::cout << "Object at (" << obj->position.x << ", "
            << obj->position.y << ", " << obj->position.z << ")"
            << std::endl;
    }
}