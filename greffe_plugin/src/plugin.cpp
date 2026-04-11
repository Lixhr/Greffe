#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include "ProjectInfo.hpp"
#include "utils.hpp"
#include "GreffeCTX.hpp"
#include "GUI/Actions.hpp"

static plugmod_t *idaapi init() {
    register_instr_action();
    register_region_action();
    greffe_msg("plugin loaded\n");
    return PLUGIN_OK;
}

static void idaapi term() {
    unregister_instr_action();
    unregister_region_action();
    greffe_msg("plugin unloaded\n");

}

static bool idaapi run(size_t) {
    if (!g_ctx)
        g_ctx = std::make_unique<GreffeCTX>();


    // TWidget *g_widget = find_widget("Greffe");

    // if ( g_widget == nullptr ) {
    //     g_widget = create_empty_widget("Greffe");

    //     QWidget *qw = reinterpret_cast<QWidget *>(g_widget);
    //     QVBoxLayout *layout = new QVBoxLayout(qw);
    //     QPushButton *btn = new QPushButton("Patch!", qw);
    //     layout->addWidget(btn);
    //     layout->addStretch();
    //     qw->setLayout(layout);

    //     QObject::connect(btn, &QPushButton::clicked, []() {
    //         msg("[greffe] Patch! clicked\n");
    //     });

    //     display_widget(g_widget, WOPN_DP_TAB | WOPN_RESTORE);
    // }
    // else {
    //     close_widget(g_widget, WCLS_SAVE);
    // }
    return true;
}

plugin_t PLUGIN = {
    IDP_INTERFACE_VERSION,
    PLUGIN_FIX,
    init,
    term,
    run,
    "Greffe instrumentation plugin",
    nullptr,
    "greffe",
    "Shift+R"
};