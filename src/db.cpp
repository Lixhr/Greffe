#include "db.hpp"

void load_db(std::unique_ptr<GreffeCTX> &g_ctx) {
    greffe_msg("loading db\n");
    static const char name[] = NODE_NAME;

    netnode node(name, strlen(name), true);

    g_ctx->layout.load_from_db(node);
}

void save_db(std::unique_ptr<GreffeCTX> &g_ctx) {
    greffe_msg("saving db\n");
    static const char name[] = NODE_NAME;

    netnode node(name, strlen(name), true);

    g_ctx->layout.save_db(node);   
}
