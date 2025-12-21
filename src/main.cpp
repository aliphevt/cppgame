#include <SFML/Graphics.hpp>
#include <vector>

// ---------------- Constants ----------------
constexpr float GRAVITY = 3000.f;
constexpr float MOVE_SPEED = 450.f;
constexpr float JUMP_VEL = -900.f;

constexpr float VIEW_W = 960.f;
constexpr float VIEW_H = 540.f;

constexpr float GROUND_Y = 500.f;
constexpr float PLAYER_W = 32.f;
constexpr float PLAYER_H = 48.f;

// Camera tuning (bigger = faster follow)
constexpr float CAMERA_FOLLOW_SPEED = 10.f;   // try 8..16
constexpr float CAMERA_DEADZONE_X = 120.f;  // pixels around center before camera moves
constexpr float CAMERA_DEADZONE_Y = 80.f;

// ---------------- Player ----------------
struct Player {
    sf::RectangleShape body;
    sf::Vector2f vel{ 0.f, 0.f };
    bool grounded = false;
};

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

int main() {
    // -------- WINDOW (SFML 3) --------
    sf::RenderWindow window(
        sf::VideoMode(sf::Vector2u(960, 540)),
        "SFML 3 Smooth Camera Test",
        sf::Style::Default
    );
    window.setFramerateLimit(60);

    // -------- VIEW --------
    sf::View view;
    view.setSize({ VIEW_W, VIEW_H });
    view.setCenter({ VIEW_W / 2.f, VIEW_H / 2.f });
    window.setView(view);

    // -------- PLATFORMS --------
    std::vector<sf::RectangleShape> platforms;

    auto makePlatform = [&](float x, float y, float w, float h) {
        sf::RectangleShape p;
        p.setSize({ w, h });
        p.setPosition({ x, y });
        p.setFillColor(sf::Color(120, 120, 120));
        platforms.push_back(p);
        };

    // Ground
    makePlatform(-500.f, GROUND_Y, 3000.f, 40.f);

    // Obstacle course (close enough to see immediately)
    makePlatform(220.f, 420.f, 140.f, 20.f);
    makePlatform(420.f, 380.f, 140.f, 20.f);
    makePlatform(640.f, 340.f, 140.f, 20.f);
    makePlatform(860.f, 420.f, 140.f, 20.f);

    // -------- PLAYER --------
    Player player;
    player.body.setSize({ PLAYER_W, PLAYER_H });
    player.body.setFillColor(sf::Color::Green);
    player.body.setPosition({ 100.f, 0.f }); // start in air so gravity is obvious

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        // Prevent huge dt spikes from making camera/physics go crazy
        dt = clampf(dt, 0.f, 0.05f);

        // -------- EVENTS --------
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // -------- INPUT --------
        float move = 0.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) move -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) move += 1.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && player.grounded) {
            player.vel.y = JUMP_VEL;
            player.grounded = false;
        }

        // -------- PHYSICS --------
        player.vel.x = move * MOVE_SPEED;
        player.vel.y += GRAVITY * dt;

        player.body.move(player.vel * dt);

        // -------- COLLISION (SFML 3) --------
        player.grounded = false;
        for (auto& plat : platforms) {
            if (auto hit = player.body.getGlobalBounds()
                .findIntersection(plat.getGlobalBounds())) {
                if (player.vel.y > 0.f) {
                    player.body.setPosition({
                        player.body.getPosition().x,
                        plat.getPosition().y - PLAYER_H
                        });
                    player.vel.y = 0.f;
                    player.grounded = true;
                }
            }
        }

        // -------- SMOOTH CAMERA FOLLOW (WITH DEADZONE) --------
        sf::Vector2f playerCenter = player.body.getPosition() + player.body.getSize() * 0.5f;
        sf::Vector2f camCenter = view.getCenter();

        // Deadzone: camera only moves if player goes outside a box around center
        sf::Vector2f target = camCenter;

        float dx = playerCenter.x - camCenter.x;
        if (dx > CAMERA_DEADZONE_X) target.x = playerCenter.x - CAMERA_DEADZONE_X;
        if (dx < -CAMERA_DEADZONE_X) target.x = playerCenter.x + CAMERA_DEADZONE_X;

        float dy = playerCenter.y - camCenter.y;
        if (dy > CAMERA_DEADZONE_Y) target.y = playerCenter.y - CAMERA_DEADZONE_Y;
        if (dy < -CAMERA_DEADZONE_Y) target.y = playerCenter.y + CAMERA_DEADZONE_Y;

        // Smooth follow (exponential-ish)
        float t = 1.f - std::exp(-CAMERA_FOLLOW_SPEED * dt);
        view.setCenter(camCenter + (target - camCenter) * t);
        window.setView(view);

        // -------- RENDER --------
        window.clear(sf::Color::Black);
        for (auto& plat : platforms)
            window.draw(plat);
        window.draw(player.body);
        window.display();
    }
}
