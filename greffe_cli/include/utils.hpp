#pragma once
#include <nlohmann/json.hpp>
#include <iostream>
#include "IdaIPC.hpp"

template<typename T>
T json_get(const nlohmann::json& j, const std::string& key) {
	if (!j.contains(key))
		throw IdaIPCError("Missing key: " + key);

	return j.at(key).get<T>();
}
