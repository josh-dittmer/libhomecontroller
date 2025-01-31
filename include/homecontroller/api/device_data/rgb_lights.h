#pragma once

#include <cinttypes>
#include <string>

namespace hc {
namespace api {
namespace rgb_lights {
struct State {
    bool m_powered;

    enum class Program {
        None,
        RainbowFade,
        PsychedelicFade,
        GuitarSync
    } m_program;

    struct Color {
        uint8_t m_r;
        uint8_t m_g;
        uint8_t m_b;
    } m_color;
};

enum class Command {
    PowerOn,
    PowerOff,
    StartProgram,
    InterruptProgram,
    StopProgram,
    SetColor
};

const std::string PRG_RAINBOW_FADE_NAME = "rainbowFade";
const std::string PRG_PSYCHEDELIC_FADE_NAME = "psychedelicFade";
const std::string PRG_GUITAR_SYNC_NAME = "guitarSync";
const std::string PRG_NONE_NAME = "none";

const std::string CMD_POWER_ON_NAME = "powerOn";
const std::string CMD_POWER_OFF_NAME = "powerOff";
const std::string CMD_START_PROGRAM_NAME = "startProgram";
const std::string CMD_INTERRUPT_PROGRAM_NAME = "interruptProgram";
const std::string CMD_STOP_PROGRAM_NAME = "stopProgram";
const std::string CMD_SET_COLOR_NAME = "setColor";

extern std::string program_to_string(State::Program program);
extern State::Program string_to_program(const std ::string& str);

extern Command string_to_command(const std::string& str);

} // namespace rgb_lights
} // namespace api
} // namespace hc