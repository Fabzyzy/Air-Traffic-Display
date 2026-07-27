#pragma once

enum class Page
{
    RADAR,
    MENU,
    INFO
};

class App
{
public:
    void nextPage();
    void previousPage();
    void buttonPressed();

private:
    Page currentPage = Page::RADAR;
    const char* getPageName();
};

