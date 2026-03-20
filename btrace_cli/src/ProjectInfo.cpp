#include "ProjectInfo.hpp"
#include "MakefileTemplates.hpp"
#include "utils.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>

Segment::Segment(const json &json_seg)
	: start (json_get<uint64_t>(json_seg, "start")),
	  end   (json_get<uint64_t>(json_seg, "end")),
	  name  (json_get<std::string>(json_seg, "name")) {

	if (end < start)
		throw std::runtime_error("Segment end < start");
}

Segment::~Segment() {}


void ProjectInfo::populateFromJson(const json& body) {
	bin_path = std::filesystem::path(json_get<std::string>(body, "bin_path"));
	arch = json_get<std::string>(body, "arch");
	endianness = json_get<std::string>(body, "endianness");
	bits = json_get<int>(body, "bits");

	for (const json& seg : json_get<json>(body, "segments"))
		segments.emplace_back(seg);
}

json ProjectInfo::fetchInfo(IdaIPC& client) {
	const json resp = client.send({ {"action", "info"} });

	if (!json_get<bool>(resp, "ok"))
		throw IdaIPCError(json_get<std::string>(resp, "body"));

	return (json_get<json>(resp, "body"));
}

void ProjectInfo::setupProjectDir() {
	project_dir = bin_path.parent_path() / "__btrace_workdir";

	if (!std::filesystem::exists(project_dir))
		std::filesystem::create_directory(project_dir);

	std::filesystem::path dest_mk = project_dir / "Makefile";
	if (!std::filesystem::exists(dest_mk)) {
		std::ofstream f(dest_mk);
		if (!f)
			throw std::runtime_error("cannot create " + dest_mk.string());
		f << MakefileTemplates::get(arch);
	}

	std::cout << "Working directory: " << project_dir.string() << "/" << std::endl;
}


ProjectInfo::ProjectInfo(IdaIPC& client) {
	json info = fetchInfo(client);
	populateFromJson(info);
	setupProjectDir();
}

ProjectInfo::~ProjectInfo() {}