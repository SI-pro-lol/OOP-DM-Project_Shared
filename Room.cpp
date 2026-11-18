#include"Room.hpp"

Room::Room(string new_room_name): room_name("Room"), app_count(0){
    for(int i = 0; i < MAX_APPLIANCES; i++) appliances[i] = nullptr;
    setRoomName(new_room_name);
}
Room::Room(const Room& source_room): room_name(source_room.room_name), app_count(source_room.app_count){
    for(int i = 0; i < MAX_APPLIANCES; i++) appliances[i] = nullptr;
    // Copy each appliance. Each room has its own set
    for(int i = 0; i < app_count; i++) appliances[i] = source_room.appliances[i]->clone();
}
Room& Room::operator=(const Room& source_room){
    if(this != &source_room){
        clearAppliances();
        room_name = source_room.room_name;
        for(int i = 0; i < source_room.app_count; i++)
            appliances[app_count++] = source_room.appliances[i]->clone();
    }
    return *this;
}
Room::~Room(){clearAppliances();}
void Room::clearAppliances(){
    for(int i = 0; i < app_count; i++){
        delete appliances[i];
        appliances[i] = nullptr;
    }
    app_count = 0;
}
string Room::getRoomName() const{return room_name;}
bool Room::setRoomName(string new_room_name){
    if(new_room_name.empty() || new_room_name.size() > 60 || new_room_name.find_first_not_of(" \t\r\n") == string::npos)
        return false;
    room_name = new_room_name;
    return true;
}
int Room::getApplianceCount() const{return app_count;}
Appliance* Room::getAppliance(int app_index){
    if(app_index < 0 || app_index >= app_count) return nullptr;
    return appliances[app_index];
}
const Appliance* Room::getAppliance(int app_index) const{
    if(app_index < 0 || app_index >= app_count) return nullptr;
    return appliances[app_index];
}
bool Room::addAppliance(Appliance* app){
    if(app == nullptr || app_count == MAX_APPLIANCES) return false;
    for(int i = 0; i < app_count; i++)
        if(appliances[i] == app) return false;
    appliances[app_count++] = app;
    return true;
}
bool Room::replaceAppliance(int app_index, Appliance* app){
    if(app_index < 0 || app_index >= app_count || app == nullptr) return false;
    if(appliances[app_index] == app) return true;
    for(int i = 0; i < app_count; i++)
        if(appliances[i] == app) return false;
    delete appliances[app_index];
    appliances[app_index] = app;
    return true;
}
bool Room::removeAppliance(int app_index){
    if(app_index < 0 || app_index >= app_count) return false;
    delete appliances[app_index];
    for(int i = app_index; i < app_count - 1; i++) appliances[i] = appliances[i + 1];
    appliances[--app_count] = nullptr;
    return true;
}
double Room::getRoomLoad(int minute) const{
    double total = 0;
    for(int i = 0; i < app_count; i++)
        if(appliances[i]->isRunningAt(minute)) total += appliances[i]->getTotalPower();
    return total;
}
