#include "input.hpp"

static bool key(sf::Keyboard::Key k) {
    return sf::Keyboard::isKeyPressed(k);
}

InputState pollInput() {
    InputState input{};
    input.move = 0.f;
    input.jump = false;
    input.dash = false;

    if (key(sf::Keyboard::Key::A)) input.move -= 1.f;
    if (key(sf::Keyboard::Key::D)) input.move += 1.f;

    input.jump = key(sf::Keyboard::Key::Space);
    input.dash = key(sf::Keyboard::Key::LShift);

    return input;
}
 