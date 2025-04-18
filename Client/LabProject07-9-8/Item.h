// Item.h
#pragma once
#include <string>

class Item {
public:

    Item(int id, const std::string& name, int amount);
    Item(int id, const std::string& name);

    int GetID() const;
    const std::string& GetName() const;
    int GetAmount() const;
    void SetAmount(int amount);

    void* GetIcon() const;         // 아이콘 (ImTextureID 대신 void*로 선언)
    void SetIcon(void* icon);       // 아이콘 설정 함수

    

private:
    int m_id;
    std::string m_name;
    int m_amount;                  
    void* m_icon;
};
