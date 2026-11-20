#include"Gui.hpp"
#include"BasicAppliance.hpp"
#include<sstream>
#include<iomanip>
#include<cmath>
#include<cstdlib>
#include<algorithm>
using namespace std;

enum ControlId{
    ID_TABS = 100, ID_HOUSE_NAME, ID_RENAME_HOUSE, ID_ROOM_LIST, ID_ROOM_NAME,
    ID_ADD_ROOM, ID_RENAME_ROOM, ID_REMOVE_ROOM, ID_APPLIANCE_LIST,
    ID_APPLIANCE_NAME, ID_POWER, ID_QUANTITY,
    ID_ENABLED, ID_ADD_APPLIANCE, ID_UPDATE_APPLIANCE, ID_REMOVE_APPLIANCE,
    ID_CLEAR_APPLIANCE, ID_GRID_ENABLED, ID_GRID_LIMIT, ID_OUTAGE_START, ID_OUTAGE_DURATION,
    ID_APPLY_SOURCES, ID_RATE, ID_RUN_DAY, ID_REPORT_LIST
};

string Gui::getText(Widget c){return c->text;}
void Gui::putText(Widget c, string txt){c->text = txt;}
bool Gui::checked(Widget c){return c->on;}
void Gui::setChecked(Widget c, bool on){c->on = on;}
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
bool Gui::readNumber(Widget control, double low, double high, double& number){
    string txt = getText(control);
    char* parse_end;
    number = strtod(txt.c_str(), &parse_end);
    return parse_end != txt.c_str() && *parse_end == '\0' && isfinite(number) && number >= low && number <= high;
}
bool Gui::readTime(Widget control, int& minute, bool allow_end){
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
void Gui::addColumn(Widget c, int, const char* title, int width){
    c->columns.push_back(title); c->widths.push_back(width);
}
void Gui::putCell(Widget c, int row, int column, string value){
    if(row >= static_cast<int>(c->rows.size())) c->rows.resize(row + 1);
    if(column >= static_cast<int>(c->rows[row].size())) c->rows[row].resize(column + 1);
    c->rows[row][column] = value;
}
Gui::Gui(): home("My home"), unit_rate(40), rm_sel(0), app_sel(-1),
    active_page(0), report_ready(false){addDemoData();}
Gui::~Gui() = default;
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
Gui::Widget Gui::addControl(int page, const char* type, string txt, int id,
    int x, int y, int width, int height, unsigned long style){
    Control c;
    c.page = page; c.id = id; c.text = txt;
    c.bounds = sf::FloatRect(float(x), float(y), float(width), float(height));
    string t(type);
    c.kind = t == "EDIT" ? 1 : t == "BUTTON" ? (style == 1 ? 3 : 2) :
             (t == "LISTBOX" || t == "TABLE") ? 4 : 0;
    controls.push_back(c);
    return &controls.back();
}
void Gui::addLabel(int page, string txt, int x, int y, int width){
    addControl(page, "STATIC", txt, 0, x, y, width, 23);
}
Gui::Widget Gui::addEdit(int page, string txt, int id, int x, int y, int width){
    return addControl(page, "EDIT", txt, id, x, y, width, 28);
}
void Gui::setup(){
    makeHouseholdPage(); makeSourcesPage(); makeReportPage();
    status_text = addControl(-1, "STATIC", "Ready. Changes are kept for this session only.",
                            0, 24, 657, 1030, 32);
    refreshRooms();
}
void Gui::makeHouseholdPage(){
    addLabel(0, "House name", 34, 125, 105);
    home_n_box = addEdit(0, home.getHouseName(), ID_HOUSE_NAME, 137, 121, 240);
    addControl(0, "BUTTON", "Rename house", ID_RENAME_HOUSE, 388, 120, 135, 28, 0);
    addLabel(0, "Ground floor", 553, 125, 200);
    addLabel(0, "Rooms (maximum 8)", 34, 169, 220);
    room_list = addControl(0, "LISTBOX", "", ID_ROOM_LIST, 34, 194, 235, 198,
        0);
    addLabel(0, "Room name", 34, 405, 220);
    rm_n_box = addEdit(0, "", ID_ROOM_NAME, 34, 430, 235);
    addControl(0, "BUTTON", "Add", ID_ADD_ROOM, 34, 470, 68, 29, 0);
    addControl(0, "BUTTON", "Rename", ID_RENAME_ROOM, 110, 470, 76, 29, 0);
    addControl(0, "BUTTON", "Remove", ID_REMOVE_ROOM, 194, 470, 75, 29, 0);
    addControl(0, "STATIC", "Select a room to view appliances.\r\n\r\nFloor-grid designer: pending.", 0,
        34, 519, 235, 95, 0);

    addLabel(0, "Appliances in the selected room (maximum 20)", 291, 169, 730);
    appliance_list = addControl(0, "TABLE", "", ID_APPLIANCE_LIST, 291, 194, 747, 198,
        0);

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
    enabled_box = addControl(0, "BUTTON", "Switched ON", ID_ENABLED, 620, 492, 160, 30, 1);
    setChecked(enabled_box, true);
    addControl(0, "BUTTON", "Add appliance", ID_ADD_APPLIANCE, 291, 541, 155, 32, 0);
    addControl(0, "BUTTON", "Update selected", ID_UPDATE_APPLIANCE, 459, 541, 165, 32, 0);
    addControl(0, "BUTTON", "Remove selected", ID_REMOVE_APPLIANCE, 637, 541, 168, 32, 0);
    addControl(0, "BUTTON", "Clear form", ID_CLEAR_APPLIANCE, 818, 541, 154, 32, 0);
}
void Gui::makeSourcesPage(){
    addLabel(1, "Grid connection", 40, 130, 260);
    grid_enabled_box = addControl(1, "BUTTON", "Grid enabled", ID_GRID_ENABLED, 40, 160, 240, 28, 1);
    setChecked(grid_enabled_box, grid.isEnabled());
    addLabel(1, "Connection limit (W)", 40, 205, 250);
    grid_limit_box = addEdit(1, numberText(grid.getGridLimit(), 0), ID_GRID_LIMIT, 40, 232, 260);
    addLabel(1, "Daily outage starts (HH:MM)", 40, 275, 270);
    outage_start_box = addEdit(1, timeText(grid.getOutageStart()), ID_OUTAGE_START, 40, 302, 260);
    addLabel(1, "Outage duration (minutes)", 40, 345, 265);
    outage_duration_box = addEdit(1, to_string(grid.getOutageDuration()), ID_OUTAGE_DURATION, 40, 372, 260);
    addControl(1, "STATIC", "One repeating daily outage.\r\nUse 0 minutes for no outage.\r\nDuration must be a multiple of 30.\r\nAn outage may cross midnight.", 0,
        40, 420, 270, 105, 0);

    addControl(1, "BUTTON", "Apply source settings", ID_APPLY_SOURCES, 40, 559, 260, 33, 0);
}
void Gui::makeReportPage(){
    addLabel(2, "Flat energy rate (PKR/kWh)", 38, 132, 245);
    rate_box = addEdit(2, numberText(unit_rate), ID_RATE, 280, 128, 125);
    addControl(2, "BUTTON", "Run day", ID_RUN_DAY, 430, 126, 145, 31, 0);
    addLabel(2, "24 hours | 30-minute steps | estimates only", 605, 133, 420);
    summary_text = addControl(2, "STATIC", "Click Run day to calculate this setup.\r\nOnly grid electricity is used.",
        0, 38, 178, 994, 118, 0);
    report_list = addControl(2, "TABLE", "", ID_REPORT_LIST, 38, 304, 994, 267,
        0);

    const char* columns[5] = {"Time", "Demand W", "Grid W", "Unmet W", "Grid status"};
    int widths[5] = {100, 220, 220, 220, 210};
    for(int i = 0; i < 5; i++) addColumn(report_list, i, columns[i], widths[i]);
    addLabel(2, "Taxes and surcharges are excluded.", 38, 591, 990);
}
void Gui::showPage(int page){active_page = page; focused = nullptr;}
void Gui::refreshRooms(){
    room_list->rows.clear();
    room_list->scroll = 0;
    room_list->selected = -1;
    int item_cnt = home.getFloor().getRoomCount();
    for(int i = 0; i < item_cnt; i++){
        string entered_name = home.getFloor().getRoom(i)->getRoomName();
        room_list->rows.push_back({entered_name});
    }
    if(item_cnt == 0) rm_sel = -1;
    else if(rm_sel < 0 || rm_sel >= item_cnt) rm_sel = 0;
    if(rm_sel >= 0){
        room_list->selected = rm_sel;
        putText(rm_n_box, home.getFloor().getRoom(rm_sel)->getRoomName());
    }else putText(rm_n_box, "");
    refreshAppliances();
}
void Gui::refreshAppliances(){
    app_sel = -1;
    appliance_list->rows.clear(); appliance_list->scroll = 0;
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
    appliance_list->selected = -1;
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
    report_list->rows.clear(); report_list->scroll = 0;
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
    report_list->rows.clear(); report_list->scroll = 0;
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
void Gui::showError(string txt){error = txt; focused = nullptr;}

void Gui::onCommand(int id){
    if(id == ID_RENAME_HOUSE){
        if(!home.setHouseName(getText(home_n_box))) showError("Enter a house name of 1-60 characters.");
        else setStatus("House name updated for this session.");
    }
    else if(id == ID_ROOM_LIST){
        rm_sel = room_list->selected;
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
        if(pending_remove != id){pending_remove = id; error = "Remove this room and its appliances?"; return;} pending_remove = 0;
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
        if(pending_remove != id){pending_remove = id; error = "Remove the selected appliance?"; return;} pending_remove = 0;
        room->removeAppliance(app_sel);
        refreshAppliances();
        clearReport();
        setStatus("Appliance removed.");
    }
    else if(id == ID_CLEAR_APPLIANCE) clearApplianceForm();
    else if(id == ID_APPLY_SOURCES) applySources();
    else if(id == ID_RUN_DAY) calculateReport();
}


void Gui::box(sf::FloatRect rect, sf::Color color){
    sf::RectangleShape shape({rect.width, rect.height});
    shape.setPosition(rect.left, rect.top); shape.setFillColor(color);
    shape.setOutlineColor(sf::Color::Black);
    shape.setOutlineThickness(-1.f);
    window.draw(shape);
}
void Gui::text(string value, float x, float y, unsigned size, sf::Color color, float max_width){
    value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
    sf::Text label(value, font, size);
    while(label.getLocalBounds().width > max_width && !value.empty()){
        value.pop_back(); label.setString(value);
    }
    label.setPosition(x, y); label.setFillColor(color); window.draw(label);
}
void Gui::draw(){
    const sf::Color ink = sf::Color::Black, muted = sf::Color::Black;
    window.clear(sf::Color::White);
    box({0, 0, 1080, 76}, sf::Color::White);
    text("SMART HOME", 24, 9, 26, sf::Color::Black);
    text("ENERGY SIMULATOR  /  Ground floor", 25, 43, 14, sf::Color::Black);
    text("ONE HOUSE   /   ONE DAY   /   YOUR ENERGY", 710, 28, 14, sf::Color::Black);
    const char* tabs[] = {"01  Rooms & appliances", "02  Power sources", "03  Daily report"};
    for(int i = 0; i < 3; ++i){
        sf::RectangleShape tab({340, 30});
        tab.setPosition(float(20 + i * 350), 83);
        tab.setFillColor(sf::Color::White);
        tab.setOutlineColor(sf::Color::Black);
        tab.setOutlineThickness(i == active_page ? 2.f : 0.f);
        window.draw(tab);
        text(tabs[i], float(34 + i * 350), 87, 16, ink);
    }
    box({20, 117, 1040, 520}, sf::Color::White);

    for(auto& c : controls){
        if(c.page >= 0 && c.page != active_page) continue;
        auto r = c.bounds;

        if(c.kind == 0){
            text(c.text, r.left, r.top, 15, muted, r.width); continue;
        }
        if(c.kind == 1){
            box({r.left-1, r.top-1, r.width+2, r.height+2}, sf::Color::White);
            box(r, sf::Color::White);
            if(focused == &c && replace_text) box(r, sf::Color::White);
            string value = c.text;
            sf::Text measure(value, font, 16);
            while(measure.getLocalBounds().width > r.width-18 && !value.empty()){
                value.erase(value.begin()); measure.setString(value);
            }
            text(value + (focused == &c ? "|" : ""), r.left+8, r.top+3, 16, ink, r.width-12);
        }else if(c.kind == 2){

            box(r, sf::Color::White);
            sf::Text measure(c.text, font, 14);
            text(c.text, r.left+(r.width-measure.getLocalBounds().width)/2, r.top+5, 14, sf::Color::Black, r.width-6);
        }else if(c.kind == 3){
            box({r.left, r.top+4, 20, 20}, sf::Color::White);
            if(c.on) text("X", r.left+4, r.top+3, 17, sf::Color::Black);
            text(c.text, r.left+29, r.top+4, 15, ink, r.width-29);
        }else if(c.kind == 4){
            box(r, sf::Color::White);
            float head = c.columns.empty() ? 0.f : 28.f;
            if(head){
                box({r.left, r.top, r.width, head}, sf::Color::White);
                float x = r.left;
                for(size_t i=0; i<c.columns.size(); ++i){
                    text(c.columns[i], x+9, r.top+4, 14, sf::Color::Black, float(c.widths[i]-12));
                    x += c.widths[i];
                }
            }
            int visible = int((r.height-head)/28);
            for(int n=0; n<visible && n+c.scroll<int(c.rows.size()); ++n){
                int row = n+c.scroll; float y=r.top+head+n*28;
                bool selected = row == c.selected;
                box({r.left, y, r.width, 27}, selected ? sf::Color::White :
                    n%2 ? sf::Color::White : sf::Color::White);
                float x = r.left;
                for(size_t k=0; k<c.rows[row].size(); ++k){
                    float width = c.widths.empty() ? r.width : float(c.widths[k]);
                    text(c.rows[row][k], x+9, y+4, 14, ink, width-16); x+=width;
                }
            }
            if(c.rows.empty()) text("No entries yet", r.left+12, r.top+head+12, 14, muted);
            if(int(c.rows.size()) > visible)
                text("Scroll for more", r.left+r.width-110, r.top+r.height-15, 11, muted);
        }
    }
    if(!error.empty()){
        box({0,0,1080,695}, sf::Color::White);
        box({200,245,680,185}, sf::Color::White);
        text(pending_remove ? "Confirm removal" : "Check input", 225,263,23,ink);
        // Wrap long validation messages into readable lines.
        string line; float y=306;
        std::istringstream words(error); string word;
        while(words >> word){
            sf::Text measure(line+word, font, 16);
            if(measure.getLocalBounds().width > 620){text(line,225,y,16,ink); line.clear(); y+=22;}
            line+=word+" ";
        }
        text(line,225,y,16,ink);
        box({655,383,195,32},sf::Color::White);
        text(pending_remove ? "Remove (Enter)" : "OK (Enter)",675,388,15,sf::Color::Black);
        if(pending_remove) text("Cancel (Esc)",230,388,15,muted);
    }
    window.display();
}
void Gui::handleEvent(const sf::Event& e){
    if(e.type == sf::Event::Closed){window.close(); return;}
    if(!error.empty()){
        bool accept = e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Enter;
        bool cancel = e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape;
        if(e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left){
            auto p=window.mapPixelToCoords({e.mouseButton.x,e.mouseButton.y});
            accept=sf::FloatRect(655,383,195,32).contains(p);
            cancel=pending_remove && sf::FloatRect(220,380,200,40).contains(p);
        }
        if(accept){int id=pending_remove; error.clear(); if(id) onCommand(id);}
        else if(cancel){error.clear(); pending_remove=0;}
        return;
    }
    if(e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left){
        auto p=window.mapPixelToCoords({e.mouseButton.x,e.mouseButton.y}); focused=nullptr; replace_text=false;
        for(int i=0;i<3;++i)
            if(sf::FloatRect(float(20+i*350),83,340,30).contains(p)){showPage(i); return;}
        for(auto& c:controls){
            if(c.page != active_page || !c.bounds.contains(p)) continue;
            if(c.kind==1){focused=&c; return;}
            if(c.kind==2){onCommand(c.id); return;}
            if(c.kind==3){c.on=!c.on; return;}
            if(c.kind==4){
                float head=c.columns.empty()?0.f:28.f;
                if(p.y<c.bounds.top+head) return;
                int row=int((p.y-c.bounds.top-head)/28)+c.scroll;
                if(row>=int(c.rows.size())) return;
                c.selected=row;
                if(c.id==ID_ROOM_LIST) onCommand(c.id);
                else if(c.id==ID_APPLIANCE_LIST){app_sel=row; loadAppliance();}
                return;
            }
        }
    }
    if(e.type == sf::Event::MouseWheelScrolled){
        auto p=window.mapPixelToCoords({e.mouseWheelScroll.x,e.mouseWheelScroll.y});
        for(auto& c:controls) if(c.page==active_page && c.kind==4 && c.bounds.contains(p)){
            int visible=int((c.bounds.height-(c.columns.empty()?0:28))/28);
            int maximum=std::max(0,int(c.rows.size())-visible);
            c.scroll=std::max(0,std::min(maximum,c.scroll+(e.mouseWheelScroll.delta<0?3:-3)));
        }
    }
    if(e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Tab){
        std::vector<Widget> fields;
        for(auto& c:controls) if(c.page==active_page && c.kind>=1 && c.kind<=3) fields.push_back(&c);
        if(!fields.empty()){
            auto it=std::find(fields.begin(),fields.end(),focused);
            int index=it==fields.end() ? (e.key.shift ? 0 : -1) : int(it-fields.begin());
            focused=fields[(index+(e.key.shift ? int(fields.size())-1 : 1))%fields.size()];
            replace_text=false;
        }
    }
    if(e.type==sf::Event::KeyPressed && focused){
        if(e.key.code==sf::Keyboard::Enter && focused->kind==2){onCommand(focused->id); return;}
        if(e.key.code==sf::Keyboard::Space && focused->kind==3){focused->on=!focused->on; return;}
        if(e.key.control && e.key.code==sf::Keyboard::A && focused->kind==1){
            replace_text=true; return;
        }
    }
    if(e.type==sf::Event::TextEntered && focused && focused->kind==1){
        auto ch=e.text.unicode;
        if(ch==8){
            if(replace_text) focused->text.clear();
            else if(!focused->text.empty()) focused->text.pop_back();
            replace_text=false;
        }else if(ch>=32 && ch<127){
            if(replace_text) focused->text.clear();
            replace_text=false;
            if(focused->text.size()<60) focused->text+=char(ch);
        }
    }
}
int Gui::run(HINSTANCE, int){
    // Preserve the existing WinMain entry point; all GUI rendering and input use SFML.
    char path[MAX_PATH]={};
    GetModuleFileNameA(nullptr,path,MAX_PATH);
    string base(path); base=base.substr(0,base.find_last_of("\\/")+1);
    if(!font.loadFromFile(base+"assets/AtkinsonHyperlegible-Regular.ttf") &&
       !font.loadFromFile("assets/AtkinsonHyperlegible-Regular.ttf")) return 1;
    window.create(sf::VideoMode(1080,695),"Smart Home Energy Simulator",
                  sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    setup();
    while(window.isOpen()){
        sf::Event e;
        while(window.pollEvent(e)) handleEvent(e);
        if(window.isOpen()) draw();
    }
    return 0;
}


