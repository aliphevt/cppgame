#include <SFML/Graphics.hpp>

// -------- Constants --------
constexpr float GRAVITY = 2500.f;
constexpr float MOVE_SPEED = 400.f;
constexpr float JUMP_VEL = -850.f;

// -------- Player --------
struct Player {
    sf::RectangleShape body;
    sf::Vector2f vel{ 0.f, 0.f };
    bool grounded = false;
};

int main() {
    sf::RenderWindow window(
        sf::VideoMode(sf::Vector2u(960, 540)),
        "SFML 3 Movement Test"
    );
    window.setFramerateLimit(60);

    Player p;
    p.body.setSize(sf::Vector2f(32.f, 48.f));
    p.body.setFillColor(sf::Color::White);
    p.body.setPosition(sf::Vector2f(200.f, 300.f));

    sf::RectangleShape ground;
    ground.setSize(sf::Vector2f(2000.f, 50.f));
    ground.setFillColor(sf::Color(100, 100, 100));
    ground.setPosition(sf::Vector2f(-500.f, 500.f));

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // ---- Events (SFML 3 ONLY WAY) ----
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // ---- Input ----
        float move = 0.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) move -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) move += 1.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && p.grounded) {
            p.vel.y = JUMP_VEL;
            p.grounded = false;
        }

        // ---- Physics ----
        p.vel.x = move * MOVE_SPEED;
        p.vel.y += GRAVITY * dt;

        p.body.move(p.vel * dt);

        // ---- Ground collision (SFML 3) ----
        if (auto hit = p.body.getGlobalBounds()
            .findIntersection(ground.getGlobalBounds())) {
            p.body.setPosition(sf::Vector2f(
                p.body.getPosition().x,
                ground.getPosition().y - p.body.getSize().y
            ));
            p.vel.y = 0.f;
            p.grounded = true;
        }

        window.clear(sf::Color::Black);
        window.draw(ground);
        window.draw(p.body);
        window.display();
    }
}
