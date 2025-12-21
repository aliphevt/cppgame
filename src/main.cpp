#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>

// ---------------- Tunables ----------------
constexpr float GRAVITY = 3000.f;
constexpr float MOVE_SPEED = 450.f;
constexpr float JUMP_VEL = -900.f;

constexpr float WALL_JUMP_X = 650.f;
constexpr float WALL_SLIDE_SPEED = 300.f;

constexpr float DASH_SPEED = 1200.f;
constexpr float DASH_TIME = 0.12f;

constexpr float SLAM_SPEED = 2200.f;

// Slam momentum boost
constexpr float SLAM_BOOST_SPEED = 400.f;
constexpr float SLAM_BOOST_DECAY = 900.f;

// Hitstop
constexpr float HITSTOP_TIME = 0.06f;

// View / world
constexpr float VIEW_W = 960.f;
constexpr float VIEW_H = 540.f;

constexpr float GROUND_Y = 500.f;
constexpr float PLAYER_W = 32.f;
constexpr float PLAYER_H = 48.f;

constexpr float FALL_DEATH_Y = 900.f;
const sf::Vector2f SPAWN_POINT{ 100.f, GROUND_Y - PLAYER_H };

// Camera follow (smooth + deadzone)
constexpr float CAMERA_FOLLOW_SPEED = 10.f;
constexpr float CAMERA_DEADZONE_X = 120.f;
constexpr float CAMERA_DEADZONE_Y = 80.f;

// Attack
constexpr float ATTACK_TIME = 0.12f;

// ---------------- Data ----------------
struct Player {
    sf::RectangleShape body;
    sf::Vector2f vel{ 0.f, 0.f };

    bool grounded = false;

    // wall state (for slide/jump)
    bool touchingWall = false;
    int wallDir = 0; // -1 = left wall, +1 = right wall

    // dash
    bool canDash = true;
    bool dashing = false;
    float dashTimer = 0.f;

    // slam
    bool slamming = false;

    // slam momentum boost
    float slamBoost = 0.f;

    // attack
    bool attacking = false;
    float attackTimer = 0.f;

    // direction
    int facing = 1; // -1 left, +1 right
};

struct Enemy {
    sf::RectangleShape body;
    bool alive = true;
};

static float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

static void respawn(Player& p) {
    p.body.setPosition(SPAWN_POINT);
    p.vel = { 0.f, 0.f };

    p.grounded = false;
    p.touchingWall = false;
    p.wallDir = 0;

    p.canDash = true;
    p.dashing = false;
    p.dashTimer = 0.f;

    p.slamming = false;
    p.slamBoost = 0.f;

    p.attacking = false;
    p.attackTimer = 0.f;

    p.facing = 1;
}

static sf::Vector2f centerOf(const sf::RectangleShape& r) {
    return r.getPosition() + r.getSize() * 0.5f;
}

int main() {
    // -------- Window --------
    sf::RenderWindow window(
        sf::VideoMode(sf::Vector2u(960, 540)),
        "Action Platformer Prototype",
        sf::Style::Default
    );
    window.setFramerateLimit(60);

    // -------- View --------
    sf::View view;
    view.setSize({ VIEW_W, VIEW_H });

    // -------- Font for win screen (put arial.ttf next to exe) --------
    sf::Font font;
    bool haveFont = font.openFromFile("arial.ttf");
    if (!haveFont) {
        std::cerr << "NOTE: arial.ttf not found next to the exe. Win overlay will show, but no text.\n";
    }

    // ? SFML 3: sf::Text has no default ctor, so construct it immediately
    sf::Text winText(font, "YOU WIN", 80);
    winText.setFillColor(sf::Color::White);

    sf::Text subText(font, "Press ESC to exit", 26);
    subText.setFillColor(sf::Color(210, 210, 210));

    // -------- Platforms --------
    std::vector<sf::RectangleShape> platforms;

    auto makePlatform = [&](float x, float y, float w, float h) {
        sf::RectangleShape p;
        p.setSize({ w, h });
        p.setPosition({ x, y });
        p.setFillColor(sf::Color(120, 120, 120));
        platforms.push_back(p);
        };

    // Ground + a wall + some platforms
    makePlatform(-500.f, GROUND_Y, 3000.f, 40.f);
    makePlatform(400.f, 350.f, 40.f, 200.f);     // wall
    makePlatform(650.f, 420.f, 140.f, 20.f);
    makePlatform(900.f, 360.f, 140.f, 20.f);

    // -------- Coin (win condition) --------
    sf::CircleShape coin(12.f);
    coin.setFillColor(sf::Color::Yellow);
    coin.setPosition({ 940.f, 320.f });
    bool coinCollected = false;

    // -------- Enemies --------
    std::vector<Enemy> enemies;
    for (int i = 0; i < 4; ++i) {
        Enemy e;
        e.body.setSize({ 32.f, 32.f });
        e.body.setFillColor(sf::Color::Red);
        e.body.setPosition({ 560.f + i * 70.f, GROUND_Y - 32.f });
        enemies.push_back(e);
    }

    // -------- Player --------
    Player p;
    p.body.setSize({ PLAYER_W, PLAYER_H });
    p.body.setFillColor(sf::Color::Green);
    respawn(p);

    view.setCenter(centerOf(p.body));
    window.setView(view);

    sf::Clock clock;
    float hitstopTimer = 0.f;
    bool won = false;

    // Attack debug shape (so you can SEE it)
    sf::RectangleShape attackDebug;
    attackDebug.setSize({ 26.f, 34.f });
    attackDebug.setFillColor(sf::Color(255, 255, 255, 200));

    while (window.isOpen()) {
        float dt = clampf(clock.restart().asSeconds(), 0.f, 0.05f);

        // -------- Events --------
        while (auto e = window.pollEvent()) {
            if (e->is<sf::Event::Closed>())
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            window.close();

        // -------- WIN STATE --------
        if (won) {
            // Draw the world frozen + overlay
            window.clear(sf::Color::Black);

            for (auto& pl : platforms) window.draw(pl);
            for (auto& en : enemies) if (en.alive) window.draw(en.body);
            window.draw(p.body);

            sf::RectangleShape overlay({ VIEW_W, VIEW_H });
            overlay.setFillColor(sf::Color(0, 0, 0, 200));
            overlay.setPosition(view.getCenter() - view.getSize() * 0.5f);
            window.draw(overlay);

            if (haveFont) {
                // Center text in view space
                auto vc = view.getCenter();

                auto wb = winText.getLocalBounds();
                winText.setPosition({
                    vc.x - wb.size.x * 0.5f,
                    vc.y - 110.f
                    });

                auto sb = subText.getLocalBounds();
                subText.setPosition({
                    vc.x - sb.size.x * 0.5f,
                    vc.y - 20.f
                    });

                window.draw(winText);
                window.draw(subText);
            }

            window.display();
            continue;
        }

        // -------- HITSTOP --------
        if (hitstopTimer > 0.f) {
            hitstopTimer -= dt;

            window.clear(sf::Color::Black);
            for (auto& pl : platforms) window.draw(pl);
            if (!coinCollected) window.draw(coin);
            for (auto& en : enemies) if (en.alive) window.draw(en.body);
            window.draw(p.body);
            window.display();
            continue;
        }

        // -------- Input --------
        float move = 0.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) move -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) move += 1.f;
        if (move != 0.f) p.facing = (move > 0.f) ? 1 : -1;

        bool jumpPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
        bool dashPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);
        bool slamPressed =
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);

        // Attack button = H
        bool attackPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H);

        // -------- Attack --------
        if (attackPressed && !p.attacking) {
            p.attacking = true;
            p.attackTimer = ATTACK_TIME;
        }
        if (p.attacking) {
            p.attackTimer -= dt;
            if (p.attackTimer <= 0.f)
                p.attacking = false;
        }

        // -------- Slam --------
        if (!p.grounded && slamPressed && !p.slamming) {
            p.slamming = true;
            p.dashing = false;
            p.vel.x = 0.f;
            p.vel.y = SLAM_SPEED;
        }

        // -------- Dash --------
        if (dashPressed && p.canDash && !p.dashing && !p.slamming) {
            p.dashing = true;
            p.canDash = false;
            p.dashTimer = DASH_TIME;
            p.vel = { DASH_SPEED * (float)p.facing, 0.f };
        }
        if (p.dashing) {
            p.dashTimer -= dt;
            if (p.dashTimer <= 0.f)
                p.dashing = false;
        }

        // -------- Horizontal speed (includes slam boost) --------
        if (!p.dashing && !p.slamming) {
            p.vel.x = move * MOVE_SPEED + (float)p.facing * p.slamBoost;
        }

        // -------- Gravity --------
        if (!p.dashing && !p.slamming)
            p.vel.y += GRAVITY * dt;

        // -------- Jump / Wall Jump --------
        if (jumpPressed && !p.slamming) {
            if (p.grounded) {
                p.vel.y = JUMP_VEL;
                p.grounded = false;
            }
            else if (p.touchingWall) {
                p.vel.y = JUMP_VEL;
                p.vel.x = -p.wallDir * WALL_JUMP_X;
                p.touchingWall = false;
            }
        }

        // ============================================================
        // COLLISION (AXIS-SEPARATED)
        // ============================================================

        // ---- Move Y ----
        p.body.move({ 0.f, p.vel.y * dt });
        p.grounded = false;

        sf::FloatRect pb = p.body.getGlobalBounds();
        for (auto& plat : platforms) {
            sf::FloatRect wb = plat.getGlobalBounds();
            if (pb.findIntersection(wb)) {
                if (p.vel.y > 0.f) { // falling onto surface
                    p.body.setPosition({ p.body.getPosition().x, wb.position.y - PLAYER_H });
                    p.vel.y = 0.f;
                    p.grounded = true;
                    p.canDash = true;

                    if (p.slamming) {
                        p.slamBoost = SLAM_BOOST_SPEED;
                        p.slamming = false;
                    }
                }
                else if (p.vel.y < 0.f) { // head hit
                    p.body.setPosition({ p.body.getPosition().x, wb.position.y + wb.size.y });
                    p.vel.y = 0.f;
                }
            }
        }

        // ---- Move X ----
        p.body.move({ p.vel.x * dt, 0.f });

        p.touchingWall = false;
        p.wallDir = 0;

        pb = p.body.getGlobalBounds();
        for (auto& plat : platforms) {
            sf::FloatRect wb = plat.getGlobalBounds();
            if (pb.findIntersection(wb)) {
                if (p.vel.x > 0.f) {
                    p.body.setPosition({ wb.position.x - PLAYER_W, p.body.getPosition().y });
                    p.vel.x = 0.f;
                    p.touchingWall = true;
                    p.wallDir = 1;
                }
                else if (p.vel.x < 0.f) {
                    p.body.setPosition({ wb.position.x + wb.size.x, p.body.getPosition().y });
                    p.vel.x = 0.f;
                    p.touchingWall = true;
                    p.wallDir = -1;
                }
            }
        }

        // ---- Wall slide ----
        if (p.touchingWall && !p.grounded && !p.slamming && p.vel.y > WALL_SLIDE_SPEED)
            p.vel.y = WALL_SLIDE_SPEED;

        // -------- Slam boost decay --------
        if (p.slamBoost > 0.f) {
            p.slamBoost -= SLAM_BOOST_DECAY * dt;
            if (p.slamBoost < 0.f) p.slamBoost = 0.f;
        }

        // -------- Attack hitbox + enemy hits --------
        if (p.attacking) {
            attackDebug.setPosition({
                p.body.getPosition().x + (float)p.facing * PLAYER_W,
                p.body.getPosition().y + 8.f
                });

            for (auto& en : enemies) {
                if (en.alive && attackDebug.getGlobalBounds().findIntersection(en.body.getGlobalBounds())) {
                    en.alive = false;

                    hitstopTimer = HITSTOP_TIME;

                    // Reward loop: regain movement option
                    p.canDash = true;               // dash reset
                    p.slamBoost = SLAM_BOOST_SPEED; // "re-juice" speed

                    p.slamming = false;
                }
            }
        }

        // -------- Respawn if fall too far --------
        if (p.body.getPosition().y > FALL_DEATH_Y) {
            respawn(p);
            view.setCenter(centerOf(p.body));
            window.setView(view);
        }

        // -------- Coin -> Win --------
        if (!coinCollected && p.body.getGlobalBounds().findIntersection(coin.getGlobalBounds())) {
            coinCollected = true;
            won = true;
        }

        // -------- Camera follow (smooth + deadzone) --------
        sf::Vector2f pc = centerOf(p.body);
        sf::Vector2f cc = view.getCenter();
        sf::Vector2f target = cc;

        float dx = pc.x - cc.x;
        if (dx > CAMERA_DEADZONE_X)  target.x = pc.x - CAMERA_DEADZONE_X;
        if (dx < -CAMERA_DEADZONE_X) target.x = pc.x + CAMERA_DEADZONE_X;

        float dy = pc.y - cc.y;
        if (dy > CAMERA_DEADZONE_Y)  target.y = pc.y - CAMERA_DEADZONE_Y;
        if (dy < -CAMERA_DEADZONE_Y) target.y = pc.y + CAMERA_DEADZONE_Y;

        float t = 1.f - std::exp(-CAMERA_FOLLOW_SPEED * dt);
        view.setCenter(cc + (target - cc) * t);
        window.setView(view);

        // -------- Render --------
        window.clear(sf::Color::Black);

        for (auto& pl : platforms) window.draw(pl);
        if (!coinCollected) window.draw(coin);

        for (auto& en : enemies) if (en.alive) window.draw(en.body);

        if (p.attacking) window.draw(attackDebug); // debug: visible attack box
        window.draw(p.body);

        window.display();
    }
}
