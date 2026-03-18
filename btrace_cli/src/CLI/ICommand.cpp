#include "ICommand.hpp"

std::vector<std::string> ICommand::complete(const Args& /*args*/) const {
    return {};
}
