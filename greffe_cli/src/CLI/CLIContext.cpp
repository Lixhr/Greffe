#include "CLIContext.hpp"

CLIContext::CLIContext(IdaIPC& client, ProjectInfo& pinfo)
        : client(client)
        , pinfo(pinfo)
        , targets(client)
        , layout(pinfo, targets)
        , bin_base(pinfo.getBinBase()) {

    auto greffe = pinfo.getProjectDir() / ".greffe";

    if (std::filesystem::exists(greffe)) {
        auto cfg = targets.load(greffe, pinfo);
        if (cfg.bin_base)   bin_base = *cfg.bin_base;
        if (cfg.patch_base) pinfo.setPatchBase(*cfg.patch_base);
    }

    if (pinfo.getPatchBase() == -1ULL)
        pinfo.initPatchBase(bin_base);

}
