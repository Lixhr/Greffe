#include "utils.hpp"
#include <filesystem>
#include <spawn.h>
#include "kernwin.hpp"

void workdir_popup(const std::filesystem::path absolute, const std::string relative) {
    std::string label = "HIDECANCEL\nWorkdir created at " + relative;

    int resp = ask_buttons(
      "Open with VSCode",
      "Close",
      nullptr,
      ASKBTN_YES,
      label.c_str()
    );

    if (resp == ASKBTN_BTN1) {
        pid_t pid;
        char *args[] = {(char *)"code", (char *)absolute.c_str(), nullptr};
        int err = posix_spawnp(&pid, "code", nullptr, nullptr, args, environ);
        if (err)
            warning("Failed to launch VSCode: %s", strerror(err));
    }
}

