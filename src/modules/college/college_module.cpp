#include "modules/college/college_module.h"
#include "modules/college/open_stack_command.h"

CollegeModule::CollegeModule() {
    commands_.push_back(
        std::make_unique<OpenStackCommand>("D:\\PDE\\config\\college.json")
    );
}

std::string CollegeModule::GetName() const {
    return "college";
}

std::string CollegeModule::GetDescription() const {
    return "Launch college development stacks";
}

const std::vector<std::unique_ptr<ICommand>>& CollegeModule::GetCommands() const {
    return commands_;
}
