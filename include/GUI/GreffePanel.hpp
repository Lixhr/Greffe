#pragma once

class GreffePanel {
public:
    static GreffePanel &instance();
    void show();
    void refresh();

private:
    GreffePanel();
};
