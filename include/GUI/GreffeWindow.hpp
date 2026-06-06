#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

struct GreffeRow {
    uint64_t    ea;
    std::string handler_name;
};

class GreffeWindow {
public:
    using HandlerChangedCb = std::function<void(uint64_t ea, const std::string &name)>;
    using HandlerAddedCb   = std::function<void(const std::string &name)>;
    using HandlerDeletedCb = std::function<void(const std::string &name)>;
    using GreffeDeletedCb  = std::function<void(uint64_t ea)>;

    GreffeWindow();
    ~GreffeWindow();

    void show();
    void setGreffes(const std::vector<GreffeRow> &rows);
    void setHandlers(const std::vector<std::string> &names);

    void setHandlerChangedCb(HandlerChangedCb cb)  { _on_handler_changed = std::move(cb); }
    void setHandlerAddedCb(HandlerAddedCb cb)       { _on_handler_added   = std::move(cb); }
    void setHandlerDeletedCb(HandlerDeletedCb cb)   { _on_handler_deleted = std::move(cb); }
    void setGreffeDeletedCb(GreffeDeletedCb cb)     { _on_greffe_deleted  = std::move(cb); }

private:
    struct Impl;
    Impl *_impl;

    HandlerChangedCb _on_handler_changed;
    HandlerAddedCb   _on_handler_added;
    HandlerDeletedCb _on_handler_deleted;
    GreffeDeletedCb  _on_greffe_deleted;
};
