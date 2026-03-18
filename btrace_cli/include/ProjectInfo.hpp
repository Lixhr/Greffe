#pragma once
#include "IdaIPC.hpp"
#include <filesystem>
#include <iostream>

class Segment {
	private:
		const uint64_t start;
		const uint64_t end;
		const std::string name;

	public:
		Segment(const json &json_segments);
		~Segment();

		uint64_t			getStart() { return start; };
		uint64_t			getEnd() { return end; };
		const std::string   &getName() {return name; };

};

struct ProjectInfo {
	private:
		json fetchInfo(IdaIPC& client);
		ProjectInfo() = default;
		void populateFromJson(const json& body);
		void setupProjectDir();

		std::filesystem::path bin_path;
		std::filesystem::path project_dir;
		std::string arch;
		std::string endianness;
		int bits;
		std::vector<Segment>	segments;
	public:

		ProjectInfo(IdaIPC& client);
		~ProjectInfo();

		int getBits() const { return bits; }
	const std::filesystem::path& getProjectDir() const { return project_dir; }
};

	