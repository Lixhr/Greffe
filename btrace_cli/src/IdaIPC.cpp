#include "IdaIPC.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <cerrno>

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
	if (write(fd, &len, 4) != 4) throw IdaIPCError("IDAServer doesn't responds");

	size_t total = 0;
	while (total < data.size()) {
		ssize_t n = write(fd, data.c_str() + total, data.size() - total);
		if (n <= 0) throw IdaIPCError("IDAServer doesn't responds");
		total += n;
	}
}

static json recv_line(int fd, int timeout_ms) {
	timeval tv{};
	tv.tv_sec  = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	uint32_t len = 0;
	ssize_t bytes_read = read(fd, &len, 4);
	if (bytes_read <= 0)
		throw IdaIPCError("IDAServer doesn't responds");
	else if (bytes_read != 4)
		throw IdaIPCError("Invalid format");

	len = ntohl(len);
	std::string buf(len, '\0');
	size_t total = 0;
	while (total < len) {
		ssize_t n = read(fd, buf.data() + total, len - total);
		if (n <= 0) throw IdaIPCError("IDAServer doesn't responds");
		total += n;
	}

	return json::parse(buf);
}

IdaIPC::IdaIPC() {
	fd_ = connect_socket();
}

IdaIPC::~IdaIPC() {
	stop_.store(true);
	shutdown(fd_, SHUT_RDWR);
	if (thread_.joinable())
		thread_.join();
	close(fd_);
}

json IdaIPC::send(const json& msg, int timeout_ms) {
	std::lock_guard<std::mutex> lk(sock_mutex_);
	return do_send(msg, timeout_ms);
}

json IdaIPC::do_send(const json& msg, int timeout_ms) {
	send_all(fd_, msg.dump());
	return recv_line(fd_, timeout_ms);
}

void IdaIPC::start(CLIContext& ctx) {
    if (thread_.joinable())
        throw std::logic_error("IdaIPC::start() already called");

    thread_ = std::thread([this, &ctx]{ run(ctx); });
}

void IdaIPC::run(CLIContext &ctx) {
    (void)ctx;
	while (!stop_.load()) {
		try {
			json resp;
			{
				std::lock_guard<std::mutex> lk(sock_mutex_);
				resp = do_send({ {"action", "refresh"} }, 2000);
			}
		} 
		catch (const IdaIPCError& e) {
			std::cerr << "[IdaIPC] refresh error: " << e.what() << "\n";
		} 
		catch (const json::exception& e) {
			std::cerr << "[IdaIPC] json error: " << e.what() << "\n";
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}