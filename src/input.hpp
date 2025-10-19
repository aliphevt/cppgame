#ifndef INPUT_HPP
#define INPUT_HPP
#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include "input.hpp"
#include <SFML/Window/Keyboard.hpp>

class Input {
public:
    sf::Keyboard::Key getKeyPress();
    void inputHandler();
private:
    std::string m_name; 
};

#endif // MY_CLASS_HPP