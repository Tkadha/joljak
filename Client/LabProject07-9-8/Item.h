// Item.h
#pragma once
#include <string>
#include <vector>
#include <memory>

class Item
{
public:
    Item(int id, const std::string& name);

    int GetID() const { return m_ID; }
    const std::string& GetName() const { return m_Name; }

private:
    int m_ID;
    std::string m_Name;
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
