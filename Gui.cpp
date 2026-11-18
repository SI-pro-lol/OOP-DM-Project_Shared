#include"Gui.hpp"
#include"BasicAppliance.hpp"
#include<sstream>
#include<iomanip>
#include<cmath>
#include<cstdlib>
using namespace std;

enum ControlId{
    ID_TABS = 100, ID_HOUSE_NAME, ID_RENAME_HOUSE, ID_ROOM_LIST, ID_ROOM_NAME,
    ID_ADD_ROOM, ID_RENAME_ROOM, ID_REMOVE_ROOM, ID_APPLIANCE_LIST,
    ID_APPLIANCE_NAME, ID_POWER, ID_QUANTITY,
    ID_ENABLED, ID_ADD_APPLIANCE, ID_UPDATE_APPLIANCE, ID_REMOVE_APPLIANCE,
    ID_CLEAR_APPLIANCE, ID_GRID_ENABLED, ID_GRID_LIMIT, ID_OUTAGE_START, ID_OUTAGE_DURATION,
    ID_APPLY_SOURCES, ID_RATE, ID_RUN_DAY, ID_REPORT_LIST
};

static string getText(HWND control){
    char txt[256] = {};
    GetWindowTextA(control, txt, 256);
    return txt;
}
static void putText(HWND control, string txt){SetWindowTextA(control, txt.c_str());}
static bool checked(HWND control){return SendMessage(control, BM_GETCHECK, 0, 0) == BST_CHECKED;}
static void setChecked(HWND control, bool is_on){SendMessage(control, BM_SETCHECK, is_on ? BST_CHECKED : BST_UNCHECKED, 0);}
static string numberText(double number, int places = 2){
    ostringstream txt;
    txt << fixed << setprecision(places) << number;
    return txt.str();
}
static string timeText(int minute){
    ostringstream txt;
    txt << setfill('0') << setw(2) << minute / 60 << ':' << setw(2) << minute % 60;
    return txt.str();
}
static bool readNumber(HWND control, double low, double high, double& number){
    string txt = getText(control);
    char* parse_end;
    number = strtod(txt.c_str(), &parse_end);
    return parse_end != txt.c_str() && *parse_end == '\0' && isfinite(number) && number >= low && number <= high;
}
static bool readTime(HWND control, int& minute, bool allow_end = false){
    string txt = getText(control);
    if(txt.size() != 5 || txt[2] != ':') return false;
    if(txt[0] < '0' || txt[0] > '9' || txt[1] < '0' || txt[1] > '9' ||
       txt[3] < '0' || txt[3] > '9' || txt[4] < '0' || txt[4] > '9') return false;
    int hour = (txt[0] - '0') * 10 + txt[1] - '0';
    int minute_part = (txt[3] - '0') * 10 + txt[4] - '0';
    if(allow_end && hour == 24 && minute_part == 0){minute = 1440; return true;}
    if(hour > 23 || (minute_part != 0 && minute_part != 30)) return false;
    minute = hour * 60 + minute_part;
    return true;
}
static void addColumn(HWND list, int idx, const char* title, int width){
    LVCOLUMNA column = {};
    column.mask = LVCF_TEXT | LVCF_WIDTH;
    column.pszText = const_cast<char*>(title);
    column.cx = width;
    SendMessageA(list, LVM_INSERTCOLUMNA, idx, reinterpret_cast<LPARAM>(&column));
}
static void putCell(HWND list, int row, int column, string txt){
    LVITEMA item = {};
    item.iItem = row;
    item.iSubItem = column;
    item.mask = LVIF_TEXT;
    item.pszText = const_cast<char*>(txt.c_str());
    if(column == 0) SendMessageA(list, LVM_INSERTITEMA, 0, reinterpret_cast<LPARAM>(&item));
    else SendMessageA(list, LVM_SETITEMTEXTA, row, reinterpret_cast<LPARAM>(&item));
}

Gui::Gui(): home("My home"), unit_rate(40), rm_sel(0), app_sel(-1),
    active_page(0), report_ready(false), window(nullptr), tab_control(nullptr), status_text(nullptr),
    normal_font(nullptr), title_font(nullptr){
    for(int i = 0; i < 3; i++) control_count[i] = 0;
    home_n_box = rm_n_box = room_list = appliance_list = nullptr;
    app_n_box = pwr_box = qty_box = nullptr;
    enabled_box = nullptr;
    grid_enabled_box = grid_limit_box = outage_start_box = outage_duration_box = nullptr;
    rate_box = report_list = summary_text = nullptr;
    addDemoData();
}
Gui::~Gui(){
    if(normal_font) DeleteObject(normal_font);
    if(title_font) DeleteObject(title_font);
}
void Gui::addDemoData(){
    home.getFloor().addRoom("Living room");
    home.getFloor().addRoom("Kitchen");
    BasicAppliance* fan = new BasicAppliance("Ceiling fan", 75, 2);
    fan->setData("Ceiling fan", 75, 2, true);
    home.getFloor().getRoom(0)->addAppliance(fan);
    BasicAppliance* light = new BasicAppliance("LED light", 10, 3);
    light->setData("LED light", 10, 3, true);
    home.getFloor().getRoom(0)->addAppliance(light);
    BasicAppliance* fridge = new BasicAppliance("Refrigerator", 150, 1);
    fridge->setData("Refrigerator", 150, 1, true);
    home.getFloor().getRoom(1)->addAppliance(fridge);
}
HWND Gui::addControl(int page, const char* type, string txt, int id,
                     int x, int y, int width, int height, DWORD style){
    DWORD extended = 0;
    if(string(type) == "EDIT" || string(type) == WC_LISTVIEWA || string(type) == "LISTBOX")
        extended = WS_EX_CLIENTEDGE;
    DWORD window_style = WS_CHILD | style;
    if(string(type) != "STATIC") window_style |= WS_TABSTOP;
    HWND control = CreateWindowExA(extended, type, txt.c_str(), window_style,
        x, y, width, height, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        GetModuleHandle(nullptr), nullptr);
    if(!control) return nullptr;
    SendMessage(control, WM_SETFONT, reinterpret_cast<WPARAM>(normal_font), TRUE);
    if(page >= 0 && control_count[page] < 70) page_controls[page][control_count[page]++] = control;
    else ShowWindow(control, SW_SHOW);
    return control;
}
void Gui::addLabel(int page, string txt, int x, int y, int width){
    addControl(page, "STATIC", txt, 0, x, y, width, 23, SS_LEFT);
}
HWND Gui::addEdit(int page, string txt, int id, int x, int y, int width){
    HWND control = addControl(page, "EDIT", txt, id, x, y, width, 26, ES_AUTOHSCROLL);
    SendMessage(control, EM_SETLIMITTEXT, 60, 0);
    return control;
}
void Gui::setup(){
    normal_font = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
    title_font = CreateFontA(-23, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
    HWND title = addControl(-1, "STATIC", "Smart Home Energy Simulator", 0, 20, 14, 660, 31, SS_LEFT);
    SendMessage(title, WM_SETFONT, reinterpret_cast<WPARAM>(title_font), TRUE);
    addLabel(-1, "C++ progress demo | one house, one floor, one simulated day", 20, 48, 950);
    tab_control = addControl(-1, WC_TABCONTROLA, "", ID_TABS, 16, 82, 1048, 555, WS_CLIPSIBLINGS);
    const char* tabs[3] = {"Rooms and appliances", "Power sources", "Daily report"};
    for(int i = 0; i < 3; i++){
        TCITEMA item = {};
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<char*>(tabs[i]);
        SendMessageA(tab_control, TCM_INSERTITEMA, i, reinterpret_cast<LPARAM>(&item));
    }
    makeHouseholdPage();
    makeSourcesPage();
    makeReportPage();
    status_text = addControl(-1, "STATIC", "Ready. This demo does not save changes after closing.", 0,
        20, 651, 1040, 25, SS_LEFT);
    refreshRooms();
    showPage(0);
}
void Gui::makeHouseholdPage(){
    addLabel(0, "House name", 34, 125, 105);
    home_n_box = addEdit(0, home.getHouseName(), ID_HOUSE_NAME, 137, 121, 240);
    addControl(0, "BUTTON", "Rename house", ID_RENAME_HOUSE, 388, 120, 135, 28, BS_PUSHBUTTON);
    addLabel(0, "Ground floor", 553, 125, 200);
    addLabel(0, "Rooms (maximum 8)", 34, 169, 220);
    room_list = addControl(0, "LISTBOX", "", ID_ROOM_LIST, 34, 194, 235, 198,
        LBS_NOTIFY | WS_VSCROLL | LBS_NOINTEGRALHEIGHT);
    addLabel(0, "Room name", 34, 405, 220);
    rm_n_box = addEdit(0, "", ID_ROOM_NAME, 34, 430, 235);
    addControl(0, "BUTTON", "Add", ID_ADD_ROOM, 34, 470, 68, 29, BS_PUSHBUTTON);
    addControl(0, "BUTTON", "Rename", ID_RENAME_ROOM, 110, 470, 76, 29, BS_PUSHBUTTON);
    addControl(0, "BUTTON", "Remove", ID_REMOVE_ROOM, 194, 470, 75, 29, BS_PUSHBUTTON);
    addControl(0, "STATIC", "Select a room to view appliances.\r\n\r\nFloor-grid designer: pending.", 0,
        34, 519, 235, 95, SS_LEFT);

    addLabel(0, "Appliances in the selected room (maximum 20)", 291, 169, 730);
    appliance_list = addControl(0, WC_LISTVIEWA, "", ID_APPLIANCE_LIST, 291, 194, 747, 198,
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
    ListView_SetExtendedListViewStyle(appliance_list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    addColumn(appliance_list, 0, "Name", 180);
    addColumn(appliance_list, 1, "W/unit", 72);
    addColumn(appliance_list, 2, "Qty", 45);
    addColumn(appliance_list, 3, "ON", 45);

    addLabel(0, "Appliance name", 291, 405, 230);
    addLabel(0, "Runs all day when switched ON", 565, 435, 210);
    addLabel(0, "Power per unit (W)", 787, 405, 150);
    addLabel(0, "Quantity", 948, 405, 85);
    app_n_box = addEdit(0, "", ID_APPLIANCE_NAME, 291, 430, 254);
    pwr_box = addEdit(0, "75", ID_POWER, 787, 430, 140);
    qty_box = addEdit(0, "1", ID_QUANTITY, 948, 430, 85);
    enabled_box = addControl(0, "BUTTON", "Switched ON", ID_ENABLED, 620, 492, 160, 30, BS_AUTOCHECKBOX);
    setChecked(enabled_box, true);
    addControl(0, "BUTTON", "Add appliance", ID_ADD_APPLIANCE, 291, 541, 155, 32, BS_PUSHBUTTON);
    addControl(0, "BUTTON", "Update selected", ID_UPDATE_APPLIANCE, 459, 541, 165, 32, BS_PUSHBUTTON);
    addControl(0, "BUTTON", "Remove selected", ID_REMOVE_APPLIANCE, 637, 541, 168, 32, BS_PUSHBUTTON);
    addControl(0, "BUTTON", "Clear form", ID_CLEAR_APPLIANCE, 818, 541, 154, 32, BS_PUSHBUTTON);
}
void Gui::makeSourcesPage(){
    addLabel(1, "Grid connection", 40, 130, 260);
    grid_enabled_box = addControl(1, "BUTTON", "Grid enabled", ID_GRID_ENABLED, 40, 160, 240, 28, BS_AUTOCHECKBOX);
    setChecked(grid_enabled_box, grid.isEnabled());
    addLabel(1, "Connection limit (W)", 40, 205, 250);
    grid_limit_box = addEdit(1, numberText(grid.getGridLimit(), 0), ID_GRID_LIMIT, 40, 232, 260);
    addLabel(1, "Daily outage starts (HH:MM)", 40, 275, 270);
    outage_start_box = addEdit(1, timeText(grid.getOutageStart()), ID_OUTAGE_START, 40, 302, 260);
    addLabel(1, "Outage duration (minutes)", 40, 345, 265);
    outage_duration_box = addEdit(1, to_string(grid.getOutageDuration()), ID_OUTAGE_DURATION, 40, 372, 260);
    addControl(1, "STATIC", "One repeating daily outage.\r\nUse 0 minutes for no outage.\r\nDuration must be a multiple of 30.\r\nAn outage may cross midnight.", 0,
        40, 420, 270, 105, SS_LEFT);

    addControl(1, "BUTTON", "Apply source settings", ID_APPLY_SOURCES, 40, 559, 260, 33, BS_PUSHBUTTON);
}
void Gui::makeReportPage(){
    addLabel(2, "Flat energy rate (PKR/kWh)", 38, 132, 245);
    rate_box = addEdit(2, numberText(unit_rate), ID_RATE, 280, 128, 125);
    addControl(2, "BUTTON", "Run day", ID_RUN_DAY, 430, 126, 145, 31, BS_PUSHBUTTON);
    addLabel(2, "24 hours | 30-minute steps | estimates only", 605, 133, 420);
    summary_text = addControl(2, "STATIC", "Click Run day to calculate this setup.\r\nOnly grid electricity is used.",
        0, 38, 178, 994, 118, SS_LEFT);
    report_list = addControl(2, WC_LISTVIEWA, "", ID_REPORT_LIST, 38, 304, 994, 267,
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
    ListView_SetExtendedListViewStyle(report_list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    const char* columns[5] = {"Time", "Demand W", "Grid W", "Unmet W", "Grid status"};
    int widths[5] = {100, 220, 220, 220, 210};
    for(int i = 0; i < 5; i++) addColumn(report_list, i, columns[i], widths[i]);
    addLabel(2, "Taxes and surcharges are excluded.", 38, 591, 990);
}
void Gui::showPage(int page){
    active_page = page;
    for(int p = 0; p < 3; p++)
        for(int i = 0; i < control_count[p]; i++)
            ShowWindow(page_controls[p][i], p == page ? SW_SHOW : SW_HIDE);
    InvalidateRect(window, nullptr, TRUE);
}
void Gui::refreshRooms(){
    SendMessage(room_list, LB_RESETCONTENT, 0, 0);
    int item_cnt = home.getFloor().getRoomCount();
    for(int i = 0; i < item_cnt; i++){
        string entered_name = home.getFloor().getRoom(i)->getRoomName();
        SendMessageA(room_list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(entered_name.c_str()));
    }
    if(item_cnt == 0) rm_sel = -1;
    else if(rm_sel < 0 || rm_sel >= item_cnt) rm_sel = 0;
    if(rm_sel >= 0){
        SendMessage(room_list, LB_SETCURSEL, rm_sel, 0);
        putText(rm_n_box, home.getFloor().getRoom(rm_sel)->getRoomName());
    }else putText(rm_n_box, "");
    refreshAppliances();
}
void Gui::refreshAppliances(){
    app_sel = -1;
    ListView_DeleteAllItems(appliance_list);
    Room* room = home.getFloor().getRoom(rm_sel);
    if(room){
        for(int i = 0; i < room->getApplianceCount(); i++){
            const Appliance* app = room->getAppliance(i);
            putCell(appliance_list, i, 0, app->getApplianceName());
            putCell(appliance_list, i, 1, numberText(app->getRatedPower(), 1));
            putCell(appliance_list, i, 2, to_string(app->getQuantity()));
            putCell(appliance_list, i, 3, app->isEnabled() ? "Yes" : "No");
        }
    }
    clearApplianceForm();
}
void Gui::clearApplianceForm(){
    app_sel = -1;
    ListView_SetItemState(appliance_list, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
    putText(app_n_box, "");
    putText(pwr_box, "75");
    putText(qty_box, "1");
    setChecked(enabled_box, true);
}
void Gui::loadAppliance(){
    Room* room = home.getFloor().getRoom(rm_sel);
    if(!room) return;
    const Appliance* app = room->getAppliance(app_sel);
    if(!app) return;
    putText(app_n_box, app->getApplianceName());
    putText(pwr_box, numberText(app->getRatedPower(), 1));
    putText(qty_box, to_string(app->getQuantity()));
    setChecked(enabled_box, app->isEnabled());
}
Appliance* Gui::readAppliance(){
    string entered_name = getText(app_n_box);
    double power_w, qty;
    if(!readNumber(pwr_box, 0, 20000, power_w) || !readNumber(qty_box, 1, 100, qty) || floor(qty) != qty){
        showError("Enter a valid power (0-20000 W) and whole-number quantity (1-100).");
        return nullptr;
    }
    Appliance* app = new BasicAppliance();
    if(!app->setData(entered_name, power_w, static_cast<int>(qty), checked(enabled_box))){
        delete app;
        showError("Enter an appliance name of 1-60 characters.");
        return nullptr;
    }
    return app;
}
bool Gui::applySources(){
    double limit, duration;
    int start;
    if(!readNumber(grid_limit_box, 0, 100000, limit) || !readTime(outage_start_box, start) ||
       !readNumber(outage_duration_box, 0, 1440, duration) || fmod(duration, 30) != 0){
        showError("Check the grid limit and outage. Use HH:MM and a duration in multiples of 30 minutes.");
        return false;
    }
    GridSource new_grid = grid;
    if(!new_grid.setData(limit, start, static_cast<int>(duration))){
        showError("One or more source settings are outside the allowed range.");
        return false;
    }
    new_grid.setEnabled(checked(grid_enabled_box));
    grid = new_grid;
    clearReport();
    setStatus("Source settings applied. Run the day again to update the results.");
    return true;
}
void Gui::clearReport(){
    report_ready = false;
    ListView_DeleteAllItems(report_list);
    putText(summary_text, "Settings have changed. Click Run day to calculate this setup.\r\nAll switched-on appliances are treated as running all day.");
}
void Gui::calculateReport(){
    double rate;
    if(!readNumber(rate_box, 0, 10000, rate)){
        showError("Enter a flat energy rate between 0 and 10000 PKR per kWh.");
        return;
    }
    if(!applySources()) return;
    unit_rate = rate;
    if(day_rpt.calculateDay(home, grid, unit_rate)){
        report_ready = true;
        showReport();
        setStatus("Daily calculation complete: 48 half-hour intervals.");
    }
}
void Gui::showReport(){
    if(!report_ready) return;
    string summary = "Demand: " + numberText(day_rpt.getDemandKWh()) +
        " kWh     Grid import: " + numberText(day_rpt.getGridKWh()) +
        " kWh\r\n\r\nUnmet demand: " + numberText(day_rpt.getUnmetKWh()) +
        " kWh     Estimated grid cost: PKR " + numberText(day_rpt.getEstimatedCost());
    putText(summary_text, summary);
    ListView_DeleteAllItems(report_list);
    for(int i = 0; i < day_rpt.getStepCount(); i++){
        const DailyStep* day_step = day_rpt.getStep(i);
        putCell(report_list, i, 0, timeText(day_step->minute));
        putCell(report_list, i, 1, numberText(day_step->demand_wh * 2, 1));
        putCell(report_list, i, 2, numberText(day_step->grid_wh * 2, 1));
        putCell(report_list, i, 3, numberText(day_step->unmet_wh * 2, 1));
        putCell(report_list, i, 4, day_step->grid_available ? "Available" : "Outage / off");
    }
}
void Gui::setStatus(string txt){putText(status_text, txt);}
void Gui::showError(string txt){MessageBoxA(window, txt.c_str(), "Check input", MB_OK | MB_ICONINFORMATION);}

void Gui::onCommand(int id){
    if(id == ID_RENAME_HOUSE){
        if(!home.setHouseName(getText(home_n_box))) showError("Enter a house name of 1-60 characters.");
        else setStatus("House name updated for this session.");
    }
    else if(id == ID_ROOM_LIST){
        rm_sel = static_cast<int>(SendMessage(room_list, LB_GETCURSEL, 0, 0));
        Room* room = home.getFloor().getRoom(rm_sel);
        if(room) putText(rm_n_box, room->getRoomName());
        refreshAppliances();
    }
    else if(id == ID_ADD_ROOM){
        if(!home.getFloor().addRoom(getText(rm_n_box))) showError("Enter a room name. Maximum 8 rooms.");
        else{
            rm_sel = home.getFloor().getRoomCount() - 1;
            refreshRooms();
            clearReport();
            setStatus("Room added.");
        }
    }
    else if(id == ID_RENAME_ROOM){
        Room* room = home.getFloor().getRoom(rm_sel);
        if(!room || !room->setRoomName(getText(rm_n_box))) showError("Select a room and enter a valid name.");
        else{refreshRooms(); setStatus("Room renamed.");}
    }
    else if(id == ID_REMOVE_ROOM){
        if(rm_sel < 0){showError("Select a room first."); return;}
        if(MessageBoxA(window, "Remove this room and its appliances?", "Remove room", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
        home.getFloor().removeRoom(rm_sel);
        refreshRooms();
        clearReport();
        setStatus("Room removed.");
    }
    else if(id == ID_ADD_APPLIANCE || id == ID_UPDATE_APPLIANCE){
        Room* room = home.getFloor().getRoom(rm_sel);
        if(!room){showError("Add or select a room first."); return;}
        if(id == ID_UPDATE_APPLIANCE && app_sel < 0){showError("Select an appliance row first."); return;}
        Appliance* app = readAppliance();
        if(!app) return;
        bool accepted;
        if(id == ID_ADD_APPLIANCE) accepted = room->addAppliance(app);
        else accepted = room->replaceAppliance(app_sel, app);
        if(!accepted){delete app; showError("Could not add or update the appliance. Maximum 20 per room."); return;}
        refreshAppliances();
        clearReport();
        setStatus("Appliance saved for this session. Run the day to see its effect.");
    }
    else if(id == ID_REMOVE_APPLIANCE){
        Room* room = home.getFloor().getRoom(rm_sel);
        if(!room || app_sel < 0){showError("Select an appliance row first."); return;}
        if(MessageBoxA(window, "Remove the selected appliance?", "Remove appliance", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
        room->removeAppliance(app_sel);
        refreshAppliances();
        clearReport();
        setStatus("Appliance removed.");
    }
    else if(id == ID_CLEAR_APPLIANCE) clearApplianceForm();
    else if(id == ID_APPLY_SOURCES) applySources();
    else if(id == ID_RUN_DAY) calculateReport();
}
LRESULT CALLBACK Gui::windowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam){
    Gui* gui = reinterpret_cast<Gui*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if(message == WM_NCCREATE){
        CREATESTRUCT* data = reinterpret_cast<CREATESTRUCT*>(lparam);
        gui = static_cast<Gui*>(data->lpCreateParams);
        gui->window = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(gui));
    }
    if(gui) return gui->handleMessage(message, wparam, lparam);
    return DefWindowProc(hwnd, message, wparam, lparam);
}
LRESULT Gui::handleMessage(UINT message, WPARAM wparam, LPARAM lparam){
    if(message == WM_CREATE){setup(); return 0;}
    if(message == WM_COMMAND){
        if(HIWORD(wparam) == BN_CLICKED || (LOWORD(wparam) == ID_ROOM_LIST && HIWORD(wparam) == LBN_SELCHANGE))
            onCommand(LOWORD(wparam));
        return 0;
    }
    if(message == WM_NOTIFY){
        NMHDR* notification = reinterpret_cast<NMHDR*>(lparam);
        if(notification->hwndFrom == tab_control && notification->code == TCN_SELCHANGE)
            showPage(TabCtrl_GetCurSel(tab_control));
        if(notification->hwndFrom == appliance_list && notification->code == LVN_ITEMCHANGED){
            NMLISTVIEW* item = reinterpret_cast<NMLISTVIEW*>(lparam);
            if((item->uNewState & LVIS_SELECTED) && item->iItem >= 0){
                app_sel = item->iItem;
                loadAppliance();
            }
        }
        return 0;
    }
    if(message == WM_DESTROY){PostQuitMessage(0); return 0;}
    return DefWindowProc(window, message, wparam, lparam);
}
int Gui::run(HINSTANCE instance, int show_command){
    INITCOMMONCONTROLSEX controls = {};
    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&controls);
    WNDCLASSA window_class = {};
    window_class.lpfnWndProc = windowProc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    window_class.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    window_class.lpszClassName = "SmartHomeMilestone";
    if(!RegisterClassA(&window_class)) return 1;
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT size = {0, 0, 1080, 695};
    AdjustWindowRect(&size, style, FALSE);
    window = CreateWindowA("SmartHomeMilestone", "Smart Home Energy Simulator",
        style, CW_USEDEFAULT, CW_USEDEFAULT, size.right - size.left, size.bottom - size.top,
        nullptr, nullptr, instance, this);
    if(!window){MessageBoxA(nullptr, "Could not create the application window.", "Startup error", MB_OK); return 1;}
    ShowWindow(window, show_command);
    UpdateWindow(window);
    MSG message;
    int result;
    while((result = GetMessage(&message, nullptr, 0, 0)) > 0){
        if(!IsDialogMessage(window, &message)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
    return result == -1 ? 1 : 0;
}
