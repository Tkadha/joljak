#include "Item.h"

std::vector<std::shared_ptr<Item>> ItemManager::m_Items;

// Item ?�성??
Item::Item(int id, const std::string& name)
    : m_ID(id), m_Name(name)
{
}

// ?�이??리스??초기??
void ItemManager::Initialize()
{
    m_Items.clear();

    int id = 0;
    m_Items.push_back(std::make_shared<Item>(id++, std::string("wood")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("stone")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("iron")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("coal")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("iron_material")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("wooden_pickaxe")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("wooden_axe")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("wooden_sword")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("wooden_hammer")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("stone_pickaxe")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("stone_axe")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("stone_sword")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("stone_hammer")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("iron_pickaxe")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("iron_axe")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("iron_sword")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("iron_hammer")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("crafting_table")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("furnace")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("bowl")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("torch")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("stick")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("fire")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("direction")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("pork")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("grill_pork")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("ruby")));




    /*m_Items.push_back(std::make_shared<Item>(id++, std::string("?�작?�")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("?�로")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("그릇")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("?�불")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("?�무곡괭??)));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("?�무?�끼")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("?�무검")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("?�무망치")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("??곡괭??)));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("???�끼")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("??검")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("??망치")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("철괴 곡괭??)));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("철괴 ?�끼")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("철괴 검")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("철괴 망치")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("?�무")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("??)));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("철괴")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("?�탄")));*/
}

// ?�이??리스??반환
const std::vector<std::shared_ptr<Item>>& ItemManager::GetItems()
{
    return m_Items;
}

// ID�??�이??찾기
std::shared_ptr<Item> ItemManager::GetItemByID(int id)
{
    for (auto& item : m_Items)
    {
        if (item->GetID() == id)
            return item;
    }
    return nullptr;
}

std::shared_ptr<Item> ItemManager::GetItemByName(const std::string& name)
{
    for (auto& item : m_Items)
    {
        if (item->GetName() == name)
            return item;
    }
    return nullptr;
}