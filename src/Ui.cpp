#include "Ui.h"

namespace
{
    uint16_t gPrimary = DisplayColors::kGreen;
}

void Ui::setPrimary(uint16_t color)
{
    gPrimary = color;
}

uint16_t Ui::primary()
{
    return gPrimary;
}

uint16_t Ui::dim(uint16_t color, uint8_t brightness)
{
    const uint8_t r = ((color >> 11) & 0x1F) * brightness / 255;
    const uint8_t g = ((color >> 5) & 0x3F) * brightness / 255;
    const uint8_t b = (color & 0x1F) * brightness / 255;
    return static_cast<uint16_t>((r << 11) | (g << 5) | b);
}

const char* Ui::colorName(uint16_t color)
{
    for (int i = 0; i < DisplayColors::kCount; ++i)
    {
        if (DisplayColors::kPalette[i] == color)
        {
            return DisplayColors::kNames[i];
        }
    }
    return "Custom";
}

int Ui::colorIndex(uint16_t color)
{
    for (int i = 0; i < DisplayColors::kCount; ++i)
    {
        if (DisplayColors::kPalette[i] == color)
        {
            return i;
        }
    }
    return 0;
}

uint16_t Ui::colorAt(int index)
{
    if (index < 0)
    {
        index = DisplayColors::kCount - 1;
    }
    index %= DisplayColors::kCount;
    return DisplayColors::kPalette[index];
}

int Ui::textWidth(const char* text, uint8_t size)
{
    if (text == nullptr)
    {
        return 0;
    }

    tft.setTextSize(size);
    int16_t x1 = 0;
    int16_t y1 = 0;
    uint16_t w = 0;
    uint16_t h = 0;
    tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    return static_cast<int>(w);
}

void Ui::fillBackground()
{
    tft.fillScreen(DisplayColors::kBackground);
}

void Ui::drawCentered(const char* text, int y, uint16_t color, uint8_t size)
{
    if (text == nullptr)
    {
        return;
    }

    tft.setTextWrap(false);
    tft.setTextSize(size);
    tft.setTextColor(color);
    const int width = textWidth(text, size);
    int x = (Config::kScreenSize - width) / 2;
    if (x < Config::kSafeInset)
    {
        x = Config::kSafeInset;
    }
    tft.setCursor(x, y);
    tft.print(text);
}

void Ui::drawTitle(const char* title)
{
    drawCentered(title, 36, DisplayColors::kText, 2);
}

void Ui::drawHint(const char* text)
{
    drawCentered(text, 196, DisplayColors::kDimText, 1);
}
