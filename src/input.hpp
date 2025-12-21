#pragma once
#include <SFML/Window/Keyboard.hpp>

struct InputState {
    float move;   // -1 left, 0 idle, 1 right
    bool jump;
    bool dash;
};

InputState pollInput();
