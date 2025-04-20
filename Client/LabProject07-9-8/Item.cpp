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
    m_Items.push_back(std::make_shared<Item>(id++, "막대기"));
    m_Items.push_back(std::make_shared<Item>(id++, "제작대"));
    m_Items.push_back(std::make_shared<Item>(id++, "화로"));
    m_Items.push_back(std::make_shared<Item>(id++, "그릇"));
    m_Items.push_back(std::make_shared<Item>(id++, "횃불"));

    m_Items.push_back(std::make_shared<Item>(id++, "나무 곡괭이"));
    m_Items.push_back(std::make_shared<Item>(id++, "나무 도끼"));
    m_Items.push_back(std::make_shared<Item>(id++, "나무 검"));
    m_Items.push_back(std::make_shared<Item>(id++, "나무 망치"));

    m_Items.push_back(std::make_shared<Item>(id++, "돌 곡괭이"));
    m_Items.push_back(std::make_shared<Item>(id++, "돌 도끼"));
    m_Items.push_back(std::make_shared<Item>(id++, "돌 검"));
    m_Items.push_back(std::make_shared<Item>(id++, "돌 망치"));

    m_Items.push_back(std::make_shared<Item>(id++, "철괴 곡괭이"));
    m_Items.push_back(std::make_shared<Item>(id++, "철괴 도끼"));
    m_Items.push_back(std::make_shared<Item>(id++, "철괴 검"));
    m_Items.push_back(std::make_shared<Item>(id++, "철괴 망치"));

    m_Items.push_back(std::make_shared<Item>(id++, "나무"));
    m_Items.push_back(std::make_shared<Item>(id++, "돌"));
    m_Items.push_back(std::make_shared<Item>(id++, "철괴"));
    m_Items.push_back(std::make_shared<Item>(id++, "석탄"));
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