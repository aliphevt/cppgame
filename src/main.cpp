#include <SFML/Graphics.hpp>


void makeRect() {
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

       
        window.display();
	sf::RectangleShape rectangle;
	rectangle.setSize(sf::Vector2f(500, 1000));
	rectangle.setOutlineColor(sf::Color::Red);
	rectangle.setOutlineThickness(1000);
	rectangle.setPosition({ 100,20 });

	window.draw(rectangle);
    }


}




int main()
{
    makeRect();
    
}
