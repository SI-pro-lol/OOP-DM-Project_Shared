#ifndef GUI_HPP
#define GUI_HPP
#include <SFML/Graphics.hpp>
#include "StatisticsReport.hpp"
#include <functional>
#include <vector>
class Gui {
    struct Widget { sf::FloatRect box; std::string text; int field; std::function<void()> action; };
    House home;
    GridSource grid;
    StatisticsReport report;
    sf::RenderWindow window;
    sf::Font font;
    std::vector<Widget> widgets;
    std::vector<std::string> values;
    int page=0, room=0, appliance=-1, focus=-1, scroll=0;
    bool enabled=true, gridOn=true, ready=false;
    std::string status="Ready. Changes are kept for this session only.";
    void label(const std::string&,float,float,unsigned=20,sf::Color=sf::Color(220,230,240));
    void panel(float,float,float,float,sf::Color);
    void button(const std::string&,float,float,float,std::function<void()>);
    void field(int,const std::string&,float,float,float);
    void draw();
    void command(int);
    void selectRoom(int);
    void selectAppliance(int);
    void invalidate();
    bool applyGrid();
public:
    Gui();
    int run();
};
#endif
