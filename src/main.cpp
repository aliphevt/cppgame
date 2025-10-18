#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>

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



int main()
{
    displayWindow();
    return 0;
}
