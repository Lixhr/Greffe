#include "GreffeCTX.hpp"

std::unique_ptr<GreffeCTX> g_ctx;

GreffeCTX::GreffeCTX()
    : pinfo()
    , targets()
    , layout(pinfo, targets)
{}
