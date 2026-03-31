#include <iostream>
#include "IdaIPC.hpp"
#include "ProjectInfo.hpp"
#include "CLIContext.hpp"
#include "CLI.hpp"
#include "CompositeCommand.hpp"
#include "ICommand.hpp"
#include "colors.hpp"

extern "C" {
#include <gum/gum.h>
}

void print_banner() {
    using namespace Color;
    std::cout
        << BLUE << "  ╔══════════════════════════════════════╗\n"
        << BLUE << "  ║  " << BOLD << "greffe" << RST << BLUE << "                              ║\n"
        << BLUE << "  ║  " << DIM  << "Bare-Metal Binary Instrumentation" << RST << BLUE << "   ║\n"
        << BLUE << "  ╚══════════════════════════════════════╝\n"
        << RST
        << CYAN << "  Type " << BOLD << "help" << RST << CYAN << " to list available commands.\n"
        << RST << '\n';
}

int main() {
    print_banner();

    gum_init_embedded();
    try {
        IdaIPC      client;


        ProjectInfo pinfo(client);
        CLIContext  ctx{ client, pinfo };
        CLI         cli(ctx);

        client.start(ctx);
        cli.run();
    }
    catch (const std::exception& e) {
        std::cerr << "[fatal] " << e.what() << '\n';
        return 1;
    }

    return 0;
}
