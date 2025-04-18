#include "Item.h"

Item::Item(int id, const std::string& name)
    : m_id(id), m_name(name), m_amount(1), m_icon(nullptr) // 기본 수량 1
{
}

int Item::GetID() const {
    return m_id;
}

const std::string& Item::GetName() const {
    return m_name;
}

int Item::GetAmount() const {
    return m_amount;
}

void Item::SetAmount(int amount) {
    m_amount = amount;
}

void* Item::GetIcon() const {
    return m_icon;
}

void Item::SetIcon(void* icon) {
    m_icon = icon;
}

