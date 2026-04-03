#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <nlohmann/json.hpp>
#include <arpa/inet.h>

class CLIContext;

using json = nlohmann::json;

static constexpr const char* SOCKET_PATH = "/tmp/greffe.sock";


class IdaIPCError : public std::runtime_error {
    public:
        explicit IdaIPCError(const std::string& msg)
            : std::runtime_error(msg) {}
};


class IdaIPC {
    public:
        IdaIPC();
        ~IdaIPC();

        json    send(const json& msg, int timeout_ms = 2000);
        void    start(CLIContext& ctx);

    private:
        json    do_send(const json& msg, int timeout_ms);
        void    run(CLIContext& ctx);

        int               _fd{ -1 };
        std::thread       _thread;
        std::atomic<bool> _stop{ false };
        std::mutex        _sock_mutex;
};
