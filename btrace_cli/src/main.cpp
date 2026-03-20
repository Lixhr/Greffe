#include <iostream>
#include "IdaIPC.hpp"
#include "ProjectInfo.hpp"
#include "CLIContext.hpp"
#include "CLI.hpp"
#include "CompositeCommand.hpp"
#include "ICommand.hpp"

extern "C" {
#include <gum/gum.h>
}

int main() {
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
