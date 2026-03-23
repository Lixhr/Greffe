#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>
#include <string>
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

		json send(const json& msg, int timeout_ms = 2000);
		void		start(CLIContext &ctx);
	private:
		json		do_send(const json& msg, int timeout_ms);
		void		run(CLIContext &ctx);

		int			   fd_{ -1 };
		std::thread	   thread_;
		std::atomic<bool> stop_{ false };
		std::mutex		sock_mutex_;
};