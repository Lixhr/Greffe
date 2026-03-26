#include "ICommand.hpp"

std::vector<std::string> ICommand::complete(const CLIContext */*ctx*/, const Args& /*args*/) const {
    return {};
}
