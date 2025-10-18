#include "input.hpp"


sf::Keyboard::Key getKeyPress() {
	sf::Event event;

	sf::Keyboard::Key Key;
	while (window.pollEvent(event)) {
		if (event.type == sf::Event::KeyPressed) {

			Key = event.key.code;

		}
	}
	Return Key;
}


void inputHandler() {
	getKeyPress();
	if (key == sf::Keyboard::W) {


	}
	else if (key == sf::Keyboard::S) {



	}
	else if (key == sf::Keyboard::A) {



	}
	else if (key == sf::Keyboard::D) {





	}



}