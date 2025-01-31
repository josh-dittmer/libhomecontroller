#include "homecontroller/api/device_data/rgb_lights.h"

#include <map>

namespace hc {
namespace api {
namespace rgb_lights {

std::string program_to_string(State::Program program) {
    static std::map<State::Program, std::string> program_to_str_map = {
        {State::Program::RainbowFade, PRG_RAINBOW_FADE_NAME},
        {State::Program::PsychedelicFade, PRG_PSYCHEDELIC_FADE_NAME},
        {State::Program::GuitarSync, PRG_GUITAR_SYNC_NAME}};

    auto mit = program_to_str_map.find(program);
    if (mit == program_to_str_map.end()) {
        return PRG_NONE_NAME;
    }

    return mit->second;
}

State::Program string_to_program(const std::string& str) {
    static std::map<std::string, State::Program> str_to_program_map = {
        {PRG_RAINBOW_FADE_NAME, State::Program::RainbowFade},
        {PRG_PSYCHEDELIC_FADE_NAME, State::Program::PsychedelicFade},
        {PRG_GUITAR_SYNC_NAME, State::Program::GuitarSync}};

    auto mit = str_to_program_map.find(str);
    if (mit == str_to_program_map.end()) {
        return State::Program::None;
    }

    return mit->second;
}

Command string_to_command(const std::string& str) {
    static std::map<std::string, Command> str_to_command_map = {
        {CMD_POWER_ON_NAME, Command::PowerOn},
        {CMD_POWER_OFF_NAME, Command::PowerOff},
        {CMD_START_PROGRAM_NAME, Command::StartProgram},
        {CMD_INTERRUPT_PROGRAM_NAME, Command::InterruptProgram},
        {CMD_STOP_PROGRAM_NAME, Command::StopProgram},
        {CMD_SET_COLOR_NAME, Command::SetColor}};

    auto mit = str_to_command_map.find(str);
    if (mit == str_to_command_map.end()) {
        throw new std::exception();
    }

    return mit->second;
}
} // namespace rgb_lights
} // namespace api
} // namespace hc