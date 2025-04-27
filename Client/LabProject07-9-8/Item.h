// Item.h
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "imgui.h"



class Item
{
public:
    Item(int id, const std::string& name);

    int GetID() const { return m_ID; }
    const std::string& GetName() const { return m_Name; }

    void SetIconHandle(ImTextureID icon) { m_iconHandle = icon; }
    ImTextureID GetIconHandle() const { return m_iconHandle; }

private:
    int m_ID;
    std::string m_Name;
    ImTextureID m_iconHandle = (ImTextureID)nullptr; // 아이콘 핸들 추가
};

class ItemManager
{
public:
    static void Initialize();
    static const std::vector<std::shared_ptr<Item>>& GetItems();
    static std::shared_ptr<Item> GetItemByID(int id);
    static std::shared_ptr<Item> GetItemByName(const std::string& name);

private:
    static std::vector<std::shared_ptr<Item>> m_Items;
};
