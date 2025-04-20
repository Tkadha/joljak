#include "Item.h"

std::vector<std::shared_ptr<Item>> ItemManager::m_Items;

// Item 생성자
Item::Item(int id, const std::string& name)
    : m_ID(id), m_Name(name)
{
}

// 아이템 리스트 초기화
void ItemManager::Initialize()
{
    m_Items.clear();

    int id = 0;
    m_Items.push_back(std::make_shared<Item>(id++, std::string("wood")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("stone")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("iron")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("coal")));

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

    /*m_Items.push_back(std::make_shared<Item>(id++, std::string("제작대")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("화로")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("그릇")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("횃불")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("나무곡괭이")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("나무도끼")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("나무검")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("나무망치")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("돌 곡괭이")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("돌 도끼")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("돌 검")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("돌 망치")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("철괴 곡괭이")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("철괴 도끼")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("철괴 검")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("철괴 망치")));

    m_Items.push_back(std::make_shared<Item>(id++, std::string("나무")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("돌")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("철괴")));
    m_Items.push_back(std::make_shared<Item>(id++, std::string("석탄")));*/
}

// 아이템 리스트 반환
const std::vector<std::shared_ptr<Item>>& ItemManager::GetItems()
{
    return m_Items;
}

// ID로 아이템 찾기
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