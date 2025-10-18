#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include "input.hpp"

sf::RectangleShape spawnPlayer() {
    sf::RectangleShape player;
    player.setSize(sf::Vector2f(100, 50));
    player.setOutlineColor(sf::Color::Red);
    player.setFillColor(sf::Color::White);
    player.setOutlineThickness(5);
    player.setOutlineColor(sf::Color::Red);
    player.setPosition({ 100, 100 });

    return player;

}

void displayWindow() {
    auto window = sf::RenderWindow(sf::VideoMode({ 1920u, 1080u }), "Screen");
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        sf::RectangleShape player = spawnPlayer();
        window.draw(player);
        window.display();

    }


}

//Next get keyboard inputs and updating position to player

int main()
{
    displayWindow();
   


    //Initial based concept. I may need a inturrpt or something like that to stop the while loop for a second to update my postion

    // using cin for game inputs on second guess may not be the best idea.
    char input;
    std::cin >> input;

    std::cout << input;
 return 0;


}
