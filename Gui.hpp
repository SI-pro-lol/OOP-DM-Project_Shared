#ifndef Gui_HPP
#define Gui_HPP
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include<windows.h>
#include<commctrl.h>
#include "StatisticsReport.hpp"

class Gui{
    House home;
    GridSource grid;
    StatisticsReport day_rpt;
    double unit_rate;
    int rm_sel, app_sel, active_page;
    bool report_ready;

    HWND window, tab_control, status_text;
    HWND page_controls[3][70];
    int control_count[3];
    HFONT normal_font, title_font;

    HWND home_n_box, rm_n_box, room_list, appliance_list;
    HWND app_n_box, pwr_box, qty_box;
    HWND enabled_box;
    HWND grid_enabled_box, grid_limit_box, outage_start_box, outage_duration_box;
    HWND rate_box, report_list, summary_text;

    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT handleMessage(UINT message, WPARAM wparam, LPARAM lparam);
    HWND addControl(int page, const char* type, string txt, int id,
                    int x, int y, int width, int height, DWORD style = 0);
    void addLabel(int page, string txt, int x, int y, int width = 210);
    HWND addEdit(int page, string txt, int id, int x, int y, int width = 130);
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
