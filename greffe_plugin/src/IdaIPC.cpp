#include "IdaIPC.hpp"
#include "CLIContext.hpp"
#include "CLI/cli_fmt.hpp"
#include "colors.hpp"
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static void thread_print_error(const std::string& msg) {
    std::string line = std::string(Color::RED) + msg + Color::RST + '\n';
    write(STDERR_FILENO, line.c_str(), line.size());
}

static int connect_socket() {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        throw IdaIPCError(std::string("socket() failed: ") + std::strerror(errno));

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(fd);
        throw IdaIPCError(std::string("connect() failed: ") + std::strerror(errno));
    }

    return fd;
}

static void send_all(int fd, const std::string& data) {
    uint32_t len = htonl(static_cast<uint32_t>(data.size()));
    if (write(fd, &len, 4) != 4)
        throw IdaIPCError("IDAServer doesn't respond");

    size_t total = 0;
    while (total < data.size()) {
        ssize_t n = write(fd, data.c_str() + total, data.size() - total);
        if (n <= 0)
            throw IdaIPCError("IDAServer doesn't respond");
        total += n;
    }
}

static json recv_line(int fd, int timeout_ms) {
    timeval tv{};
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint32_t len    = 0;
    ssize_t  nread  = read(fd, &len, 4);
    if (nread <= 0)
        throw IdaIPCError("IDAServer doesn't respond");
    if (nread != 4)
        throw IdaIPCError("Invalid format");

    len = ntohl(len);
    if (len > 10 * 1024 * 1024)
        throw IdaIPCError("IPC message too large (" + std::to_string(len) + " bytes)");

    std::string buf(len, '\0');
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, buf.data() + total, len - total);
        if (n <= 0)
            throw IdaIPCError("IDAServer doesn't respond");
        total += n;
    }

    return json::parse(buf);
}


IdaIPC::IdaIPC() {
    signal(SIGPIPE, SIG_IGN);
    _fd = connect_socket();
}

IdaIPC::~IdaIPC() {
    _stop.store(true);
    shutdown(_fd, SHUT_RDWR);
    if (_thread.joinable())
        _thread.join();
    close(_fd);
}

json IdaIPC::send(const json& msg, int timeout_ms) {
    std::lock_guard<std::mutex> lk(_sock_mutex);
    return do_send(msg, timeout_ms);
}

json IdaIPC::do_send(const json& msg, int timeout_ms) {
    send_all(_fd, msg.dump());
    return recv_line(_fd, timeout_ms);
}

void IdaIPC::start(CLIContext& ctx) {
    if (_thread.joinable())
        throw std::logic_error("IdaIPC::start() already called");

    _thread = std::thread([this, &ctx] { run(ctx); });
}

void IdaIPC::run(CLIContext& ctx) {
    while (!_stop.load()) {
        try {
            json resp;
            {
                std::lock_guard<std::mutex> lk(_sock_mutex);
                resp = do_send({ {"action", "refresh"} }, 2000);
            }

            if (resp.value("ok", false)) {
                for (const auto& entry : resp["body"]["targets"]) {
                    try {
                        if (ctx.targets.add_direct(entry, ctx)) {
                            async_print(std::string(Color::GREEN) + "\nnew greffe: "
                                        + entry.value("name", "?") + Color::RST + '\n');
                        }
                    } catch (const std::exception& e) {
                        async_print(std::string(Color::RED) + "add: " + e.what() + Color::RST + '\n');
                    }
                }
            }
        }
        catch (const IdaIPCError& e) {
            thread_print_error(std::string("\nrefresh error: ") + e.what());
            rl_deprep_terminal();
            rl_clear_history();
            exit(1);
        }
        catch (const json::exception& e) {
            thread_print_error(std::string("json error: ") + e.what());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}
