// -------------------------------------------------------
// 
// 
// 
// -------------------------------------------------------

#include <iostream>
#include "Octree.h"

Octree oct(Vec3(0, 0, 0), Vec3(1500, 1500, 1500));

int main()
{
    Object obj1(1,Vec3(100, 200, 300));
    Object obj2(2,Vec3(400, 500, 600));
    Object obj3(3,Vec3(1400, 1200, 1300));
    Object obj4(4, Vec3(110, 210, 310));
    Object obj5(5, Vec3(0  , 20, 150));
    Object obj6(6, Vec3(111, 3123, 111));
    oct.insert(&obj1);
    oct.insert(&obj2);
    oct.insert(&obj3);
    oct.insert(&obj4);
    oct.insert(&obj5);
    oct.insert(&obj6);

    std::vector<Object*> results;

    oct.query(obj4, Vec3(200, 200, 200), results); // obj4과 +- 200,200,200 안에 있는 오브젝트를 results에 담기

    std::cout << "obj4 와 +-(200,200,200) 안에 있는 오브젝트들: " << std::endl;
    for (auto& obj : results) {
        std::cout << "id: " << (int)obj->u_id << " Object at (" << obj->position.x << ", "
            << obj->position.y << ", " << obj->position.z << ")"
            << std::endl;
    }
}