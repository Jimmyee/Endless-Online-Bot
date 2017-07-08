#ifndef INVENTORY_HPP_INCLUDED
#define INVENTORY_HPP_INCLUDED

#include <vector>

class Inventory
{
public:
    std::vector<std::pair<short, int>> items;

    void AddItem(short id, int amount);
    void DelItem(short id, int amount);
    bool FindItem(short id, int amount);
    int GetItemAmount(short id);
    void Clear();
};

#endif // INVENTORY_HPP_INCLUDED
