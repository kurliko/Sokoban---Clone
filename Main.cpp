//Uwaga! Co najmniej C++17!!!
//Project-> ... Properties->Configuration Properties->General->C++ Language Standard = ISO C++ 17 Standard (/std:c++17)

#include "SFML/Graphics.hpp"
#include <fstream>
#include <optional>
#include <tuple>
#include <vector>
#include <cmath>

enum class Field { VOID, FLOOR, WALL, BOX, PARK, PLAYER };

class Sokoban : public sf::Drawable
{
public:
    void LoadMapFromFile(std::string fileName);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void SetDrawParameters(sf::Vector2u draw_area_size);
    void Move_Player_Left();
    void Move_Player_Right();
    void Move_Player_Up();
    void Move_Player_Down();
    bool Is_Victory() const;

private:
    std::vector<std::vector<Field>> map;
    sf::Vector2f shift, tile_size;
    sf::Vector2i player_position;
    std::vector<sf::Vector2i> park_positions;

    void move_player(int dx, int dy);
};

void Sokoban::LoadMapFromFile(std::string fileName)
{
    std::string str;
    std::vector<std::string> vos;

    std::ifstream in(fileName.c_str());
    if (!in)
    {

    }
    while (std::getline(in, str)) { vos.push_back(str); }
    in.close();

    map.clear();
    map.resize(vos.size(), std::vector<Field>(vos[0].size()));
    park_positions.clear();

    for (auto [row, row_end, y] = std::tuple{ vos.cbegin(), vos.cend(), 0 }; row != row_end; ++row, ++y)
        for (auto [element, end, x] = std::tuple{ row->begin(), row->end(), 0 }; element != end; ++element, ++x)
            switch (*element)
            {
            case 'X': map[y][x] = Field::WALL; break;
            case '*': map[y][x] = Field::VOID; break;
            case ' ': map[y][x] = Field::FLOOR; break;
            case 'B': map[y][x] = Field::BOX; break;
            case 'P': map[y][x] = Field::PARK; park_positions.push_back(sf::Vector2i(x, y)); break;
            case 'S': map[y][x] = Field::PLAYER; player_position = sf::Vector2i(x, y); break;
            default:  map[y][x] = Field::VOID; break;
            }
}

void Sokoban::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    sf::RectangleShape rect(tile_size);

    for (int y = 0; y < (int)map.size(); ++y)
        for (int x = 0; x < (int)map[y].size(); ++x)
        {
            sf::Vector2f pos = shift + sf::Vector2f(x * tile_size.x, y * tile_size.y);
            rect.setPosition(pos);

            switch (map[y][x])
            {
            case Field::VOID:   rect.setFillColor(sf::Color(30, 30, 30)); break;
            case Field::FLOOR:  rect.setFillColor(sf::Color(200, 200, 200)); break;
            case Field::WALL:   rect.setFillColor(sf::Color(100, 100, 100)); break;
            case Field::BOX:    rect.setFillColor(sf::Color(160, 82, 45)); break;
            case Field::PARK:   rect.setFillColor(sf::Color(0, 200, 0)); break;
            case Field::PLAYER: rect.setFillColor(sf::Color(0, 0, 255)); break;
            }
            rect.setOutlineThickness(1.f);
            rect.setOutlineColor(sf::Color::Black);
            target.draw(rect);
        }
}

void Sokoban::SetDrawParameters(sf::Vector2u draw_area_size)
{
    this->tile_size = sf::Vector2f(
        std::min(std::floor((float)draw_area_size.x / (float)map[0].size()),
            std::floor((float)draw_area_size.y / (float)map.size())),
        std::min(std::floor((float)draw_area_size.x / (float)map[0].size()),
            std::floor((float)draw_area_size.y / (float)map.size()))
    );
    this->shift = sf::Vector2f(
        ((float)draw_area_size.x - this->tile_size.x * map[0].size()) / 2.0f,
        ((float)draw_area_size.y - this->tile_size.y * map.size()) / 2.0f
    );
}

void Sokoban::Move_Player_Left() { move_player(-1, 0); }
void Sokoban::Move_Player_Right() { move_player(1, 0); }
void Sokoban::Move_Player_Up() { move_player(0, -1); }
void Sokoban::Move_Player_Down() { move_player(0, 1); }

void Sokoban::move_player(int dx, int dy)
{
    bool allow_move = false;
    sf::Vector2i new_pp(player_position.x + dx, player_position.y + dy);
    Field fts = map[new_pp.y][new_pp.x];
    Field ftsa = map[new_pp.y + dy][new_pp.x + dx];

    if (fts == Field::FLOOR || fts == Field::PARK) allow_move = true;

    if (fts == Field::BOX && (ftsa == Field::FLOOR || ftsa == Field::PARK))
    {
        allow_move = true;
        map[new_pp.y + dy][new_pp.x + dx] = Field::BOX;
        map[new_pp.y][new_pp.x] = Field::FLOOR;
    }

    if (allow_move)
    {
        map[player_position.y][player_position.x] = Field::FLOOR;
        player_position = new_pp;
        map[player_position.y][player_position.x] = Field::PLAYER;
    }

    for (auto park_position : park_positions)
        if (map[park_position.y][park_position.x] == Field::FLOOR)
            map[park_position.y][park_position.x] = Field::PARK;
}

bool Sokoban::Is_Victory() const
{
    for (auto park_position : park_positions)
        if (map[park_position.y][park_position.x] != Field::BOX)
            return false;
    return true;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Sokoban", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    Sokoban sokoban;
    std::optional<sf::Event> event;

    sokoban.LoadMapFromFile("plansza.txt");
    sokoban.SetDrawParameters(window.getSize());

    while (window.isOpen())
    {
        while (event = window.pollEvent())
        {
            if (const auto resized = event->getIf<sf::Event::Resized>())
            {
                window.setView(sf::View(sf::FloatRect({ 0,0 }, static_cast<sf::Vector2f>(resized->size))));
                sokoban.SetDrawParameters(window.getSize());

                window.clear();
                window.draw(sokoban);
                window.display();
            }
            if (const auto key = event->getIf<sf::Event::KeyPressed>())
            {
                switch (key->scancode)
                {
                case sf::Keyboard::Scan::Left:  sokoban.Move_Player_Left(); break;
                case sf::Keyboard::Scan::Right: sokoban.Move_Player_Right(); break;
                case sf::Keyboard::Scan::Up:    sokoban.Move_Player_Up(); break;
                case sf::Keyboard::Scan::Down:  sokoban.Move_Player_Down(); break;
                case sf::Keyboard::Scan::Escape: window.close(); break;
                default: break;
                }
            }
            if (const auto closed = event->getIf<sf::Event::Closed>())
                window.close();
        }

        window.clear();
        window.draw(sokoban);
        window.display();

        if (sokoban.Is_Victory())
            window.setTitle("Victory");
    }

    return 0;
}
