// Qt headers must come before any IDA SDK headers in this TU.
// In the plugin build, QT_NAMESPACE=QT is set so all Qt types live in
// QT:: — matching IDA's runtime symbols.
#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QSizePolicy>

#include "GUI/GreffeWindow.hpp"
#include <algorithm>
#include <regex>
#include <sstream>

struct GreffeWindow::Impl {
    QWidget      *window;
    QTableWidget *greffes_table;
    QTableWidget *handlers_table;
    QPushButton  *add_btn;
    std::vector<std::string> handlers;
};

static QPushButton *make_del_btn() {
    auto *btn = new QPushButton("×");
    btn->setMaximumWidth(28);
    btn->setFlat(true);
    return btn;
}

GreffeWindow::GreffeWindow() : _impl(new Impl{}) {
    _impl->window = new QWidget(nullptr);
    _impl->window->setWindowTitle("Greffe Patch Manager");
    _impl->window->resize(640, 520);

    auto *root = new QVBoxLayout(_impl->window);
    root->setSpacing(4);
    root->setContentsMargins(8, 8, 8, 8);

    //  Greffes 
    root->addWidget(new QLabel("<b>Greffes</b>"));

    _impl->greffes_table = new QTableWidget(0, 3);
    _impl->greffes_table->setHorizontalHeaderLabels({"EA", "Handler", ""});
    auto *gh = _impl->greffes_table->horizontalHeader();
    gh->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    gh->setSectionResizeMode(1, QHeaderView::Stretch);
    gh->setSectionResizeMode(2, QHeaderView::Fixed);
    _impl->greffes_table->setColumnWidth(2, 28);
    _impl->greffes_table->verticalHeader()->hide();
    _impl->greffes_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _impl->greffes_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    root->addWidget(_impl->greffes_table);

    // Handlers
    root->addWidget(new QLabel("<b>Handlers</b>"));

    _impl->handlers_table = new QTableWidget(0, 2);
    _impl->handlers_table->setHorizontalHeaderLabels({"Name", ""});
    auto *hh = _impl->handlers_table->horizontalHeader();
    hh->setSectionResizeMode(0, QHeaderView::Stretch);
    hh->setSectionResizeMode(1, QHeaderView::Fixed);
    _impl->handlers_table->setColumnWidth(1, 28);
    _impl->handlers_table->verticalHeader()->hide();
    _impl->handlers_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _impl->handlers_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    root->addWidget(_impl->handlers_table);

    _impl->add_btn = new QPushButton("Add handler");
    _impl->add_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    root->addWidget(_impl->add_btn);

    QObject::connect(_impl->add_btn, &QPushButton::clicked, [this]() {
        static std::regex c_ident("^[A-Za-z_][A-Za-z0-9_]*$");
        bool ok = false;
        QString qname = QInputDialog::getText(
            _impl->window, "Add handler", "Handler name:",
            QLineEdit::Normal, {}, &ok);
        if (!ok || qname.isEmpty())
            return;
        std::string name = qname.toStdString();
        if (!std::regex_match(name, c_ident)) {
            QMessageBox::warning(_impl->window, "Greffe",
                "Handler name must be a valid C identifier.");
            return;
        }
        if (std::find(_impl->handlers.begin(), _impl->handlers.end(), name)
                != _impl->handlers.end()) {
            QMessageBox::information(_impl->window, "Greffe",
                "A handler with this name already exists.");
            return;
        }
        if (_on_handler_added)
            _on_handler_added(name);
    });
}

GreffeWindow::~GreffeWindow() {
    delete _impl->window; // destroys all child widgets
    delete _impl;
}


void GreffeWindow::show() {
    _impl->window->show();
    _impl->window->raise();
    _impl->window->activateWindow();
}

void GreffeWindow::setHandlers(const std::vector<std::string> &names) {
    _impl->handlers = names;

    _impl->handlers_table->setRowCount(0);
    for (int r = 0; r < (int)names.size(); ++r) {
        _impl->handlers_table->insertRow(r);

        auto *item = new QTableWidgetItem(names[r].c_str());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        _impl->handlers_table->setItem(r, 0, item);

        auto *del = make_del_btn();
        const std::string name = names[r];
        QObject::connect(del, &QPushButton::clicked, [this, name]() {
            if (_on_handler_deleted)
                _on_handler_deleted(name);
        });
        _impl->handlers_table->setCellWidget(r, 1, del);
    }

    for (int r = 0; r < _impl->greffes_table->rowCount(); ++r) {
        auto *combo = qobject_cast<QComboBox *>(
            _impl->greffes_table->cellWidget(r, 1));
        if (!combo) continue;
        combo->blockSignals(true);
        QString current = combo->currentText();
        combo->clear();
        combo->addItem("(none)");
        for (const auto &h : _impl->handlers)
            combo->addItem(h.c_str());
        int idx = combo->findText(current);
        combo->setCurrentIndex(idx > 0 ? idx : 0);
        combo->blockSignals(false);
    }
}

void GreffeWindow::setGreffes(const std::vector<GreffeRow> &rows) {
    _impl->greffes_table->setRowCount(0);
    for (int r = 0; r < (int)rows.size(); ++r) {
        _impl->greffes_table->insertRow(r);
        const uint64_t ea = rows[r].ea;

        std::ostringstream ss;
        ss << "0x" << std::hex << ea;
        auto *ea_item = new QTableWidgetItem(ss.str().c_str());
        ea_item->setFlags(ea_item->flags() & ~Qt::ItemIsEditable);
        _impl->greffes_table->setItem(r, 0, ea_item);

        auto *combo = new QComboBox;
        combo->addItem("(none)");
        for (const auto &h : _impl->handlers)
            combo->addItem(h.c_str());
        int idx = 0;
        for (int i = 0; i < (int)_impl->handlers.size(); ++i) {
            if (_impl->handlers[i] == rows[r].handler_name) { idx = i + 1; break; }
        }
        combo->setCurrentIndex(idx);
        QObject::connect(combo, &QComboBox::currentIndexChanged,
            [this, ea](int i) {
                std::string name = (i > 0 && i <= (int)_impl->handlers.size())
                                   ? _impl->handlers[i - 1] : "";
                if (_on_handler_changed)
                    _on_handler_changed(ea, name);
            });
        _impl->greffes_table->setCellWidget(r, 1, combo);

        auto *del = make_del_btn();
        QObject::connect(del, &QPushButton::clicked, [this, ea]() {
            if (_on_greffe_deleted)
                _on_greffe_deleted(ea);
        });
        _impl->greffes_table->setCellWidget(r, 2, del);
    }
}
