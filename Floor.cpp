#include "Floor.hpp"

Floor::Floor(string floor_name): floor_label(floor_name), room_count(0){}
string Floor::getFloorName() const{return floor_label;}
int Floor::getRoomCount() const{return room_count;}
Room* Floor::getRoom(int room_index){
    if(room_index < 0 || room_index >= room_count) return nullptr;
    return &rooms[room_index];
}
const Room* Floor::getRoom(int room_index) const{
    if(room_index < 0 || room_index >= room_count) return nullptr;
    return &rooms[room_index];
}
bool Floor::addRoom(string room_name){
    if(room_count == MAX_ROOMS) return false;
    Room room;
    if(!room.setRoomName(room_name)) return false;
    rooms[room_count++] = room;
    return true;
}
bool Floor::removeRoom(int room_index){
    if(room_index < 0 || room_index >= room_count) return false;
    for(int i = room_index; i < room_count - 1; i++) rooms[i] = rooms[i + 1];
    rooms[--room_count] = Room();
    return true;
}
double Floor::getFloorLoad(int minute) const{
    double total = 0;
    for(int i = 0; i < room_count; i++) total += rooms[i].getRoomLoad(minute);
    return total;
}
