#include "Menu.h"

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
    items_ = items;
    itemCount_ = count;
    selectedIndex_ = 0;
    scrollOffset_ = 0;
}

void Menu::setSelection(int selection)
{
    selectedIndex_ = clampSelection(selection);
    scrollOffset_ = 0;
}

int Menu::getSelection() const
{
    return selectedIndex_;
}

void Menu::moveNext()
{
    if (itemCount_ <= 1)
    {
        return;
    }

    selectedIndex_ = (selectedIndex_ + 1) % itemCount_;
    if (selectedIndex_ >= scrollOffset_ + 3)
    {
        scrollOffset_ = selectedIndex_ - 2;
    }
}

void Menu::movePrevious()
{
    if (itemCount_ <= 1)
    {
        return;
    }

    selectedIndex_ = (selectedIndex_ + itemCount_ - 1) % itemCount_;
    if (selectedIndex_ < scrollOffset_)
    {
        scrollOffset_ = selectedIndex_;
    }
}

void Menu::drawList(const char* title, int titleY, int topY, int lineHeight, int visibleItems,
                    uint16_t textColor, uint16_t highlightColor) const
{
    if (items_ == nullptr || itemCount_ <= 0)
    {
        return;
    }

    tft.fillScreen(0x0000);
    tft.setTextWrap(false);
    tft.setTextSize(2);
    tft.setTextColor(textColor);
    tft.setCursor(10, titleY);
    tft.print(title);

    tft.setTextSize(1);
    const int maxVisibleItems = clampValue(visibleItems, 1, itemCount_);
    int startIndex = 0;
    if (itemCount_ > maxVisibleItems)
    {
        startIndex = clampValue(selectedIndex_ - 1, 0, itemCount_ - maxVisibleItems);
    }

    int y = topY;
    for (int i = 0; i < maxVisibleItems; ++i)
    {
        const int index = startIndex + i;
        if (index >= itemCount_)
        {
            break;
        }

        tft.setCursor(20, y);
        if (index == selectedIndex_)
        {
            tft.setTextColor(highlightColor);
            tft.print("> ");
            tft.setTextColor(textColor);
        }
        else
        {
            tft.print("  ");
        }
        tft.print(items_[index]);
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
