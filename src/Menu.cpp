#include "Menu.h"
#include "Ui.h"

namespace
{
    int clampValue(int value, int minValue, int maxValue)
    {
        return value < minValue ? minValue : (value > maxValue ? maxValue : value);
    }
}

Menu::Menu()
{
}

void Menu::setItems(const char* const* items, int count)
{
    itemCount_ = clampValue(count, 0, kMaxItems);
    for (int i = 0; i < itemCount_; ++i)
    {
        strncpy(storage_[i], items[i] != nullptr ? items[i] : "", kMaxLabelLen - 1);
        storage_[i][kMaxLabelLen - 1] = '\0';
        pointers_[i] = storage_[i];
    }
    selectedIndex_ = 0;
}

void Menu::setItem(int index, const char* label)
{
    if (index < 0 || index >= kMaxItems)
    {
        return;
    }
    strncpy(storage_[index], label != nullptr ? label : "", kMaxLabelLen - 1);
    storage_[index][kMaxLabelLen - 1] = '\0';
    pointers_[index] = storage_[index];
}

void Menu::setCount(int count)
{
    itemCount_ = clampValue(count, 0, kMaxItems);
    selectedIndex_ = clampSelection(selectedIndex_);
}

void Menu::setSelection(int selection)
{
    selectedIndex_ = clampSelection(selection);
}

int Menu::getSelection() const
{
    return selectedIndex_;
}

int Menu::getCount() const
{
    return itemCount_;
}

void Menu::moveNext()
{
    if (itemCount_ <= 1)
    {
        return;
    }
    selectedIndex_ = (selectedIndex_ + 1) % itemCount_;
}

void Menu::movePrevious()
{
    if (itemCount_ <= 1)
    {
        return;
    }
    selectedIndex_ = (selectedIndex_ + itemCount_ - 1) % itemCount_;
}

void Menu::drawList(const char* title, int visibleItems) const
{
    Ui::fillBackground();
    Ui::drawTitle(title != nullptr ? title : "");

    if (itemCount_ <= 0)
    {
        Ui::drawCentered("No items", 110, DisplayColors::kDimText, 1);
        return;
    }

    const int maxVisible = clampValue(visibleItems, 1, itemCount_);
    int startIndex = 0;
    if (itemCount_ > maxVisible)
    {
        startIndex = clampValue(selectedIndex_ - maxVisible / 2, 0, itemCount_ - maxVisible);
    }

    const int lineHeight = 22;
    const int blockHeight = maxVisible * lineHeight;
    int y = (Config::kScreenSize - blockHeight) / 2 + 10;

    for (int i = 0; i < maxVisible; ++i)
    {
        const int index = startIndex + i;
        if (index >= itemCount_)
        {
            break;
        }

        const bool selected = index == selectedIndex_;
        const uint16_t color = selected ? Ui::primary() : DisplayColors::kText;
        Ui::drawCentered(storage_[index], y, color, 1);
        y += lineHeight;
    }
}

int Menu::clampSelection(int value) const
{
    if (itemCount_ <= 0)
    {
        return 0;
    }
    return clampValue(value, 0, itemCount_ - 1);
}
