#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include <loader.hpp>
#include "ProjectInfo.hpp"

// IDA's pro.h and Qt both define these — undef IDA's versions first
#undef qstrlen
#undef qstrncmp

static plugmod_t *idaapi init() {
    msg("[greffe] plugin loaded\n");
    return PLUGIN_OK;
}

static bool idaapi run(size_t) {
    ProjectInfo pinfo;

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
    PLUGIN_UNL,
    init,
    nullptr,
    run,
    "Greffe instrumentation plugin",
    nullptr,
    "greffe",
    nullptr
};