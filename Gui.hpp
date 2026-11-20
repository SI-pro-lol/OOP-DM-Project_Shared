#ifndef Gui_HPP
#define Gui_HPP
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include<windows.h>
#include <SFML/Graphics.hpp>
#include <deque>
#include <vector>
#include "StatisticsReport.hpp"

class Gui{
    House home;
    GridSource grid;
    StatisticsReport day_rpt;
    double unit_rate;
    int rm_sel, app_sel, active_page;
    bool report_ready;

    struct Control {
        int page, id, kind, selected = -1, scroll = 0;
        sf::FloatRect bounds;
        string text;
        bool on = false;
        std::vector<std::vector<string>> rows;
        std::vector<string> columns;
        std::vector<int> widths;
    };
    using Widget = Control*;
    std::deque<Control> controls;
    sf::RenderWindow window;
    sf::Font font;
    Widget focused = nullptr;
    bool replace_text = false;
    int pending_remove = 0;
    string error;
    Widget status_text, home_n_box, rm_n_box, room_list, appliance_list;
    Widget app_n_box, pwr_box, qty_box, enabled_box;
    Widget grid_enabled_box, grid_limit_box, outage_start_box, outage_duration_box;
    Widget rate_box, report_list, summary_text;
    Widget addControl(int page, const char* type, string txt, int id,
                    int x, int y, int width, int height, unsigned long style = 0);
    void addLabel(int page, string txt, int x, int y, int width = 210);
    Widget addEdit(int page, string txt, int id, int x, int y, int width = 130);
    static string getText(Widget c);
    static void putText(Widget c, string text);
    static bool checked(Widget c);
    static void setChecked(Widget c, bool on);
    static bool readNumber(Widget c, double low, double high, double& value);
    static bool readTime(Widget c, int& minute, bool allow_end = false);
    static void addColumn(Widget c, int index, const char* title, int width);
    static void putCell(Widget c, int row, int column, string text);
    void draw();
    void handleEvent(const sf::Event& event);
    void text(string value, float x, float y, unsigned size, sf::Color color, float max_width = 2000);
    void box(sf::FloatRect rect, sf::Color color);
    void setup();
    void makeHouseholdPage();
    void makeSourcesPage();
    void makeReportPage();
    void showPage(int page);
    void addDemoData();
    void refreshRooms();
    void refreshAppliances();
    void loadAppliance();
    void clearApplianceForm();
    Appliance* readAppliance();
    bool applySources();
    void calculateReport();
    void showReport();
    void clearReport();
    void onCommand(int id);
    void setStatus(string txt);
    void showError(string txt);

    public:
        Gui();
        ~Gui();
        int run(HINSTANCE instance, int show_command);
};
#endif

