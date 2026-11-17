#ifndef Floor_HPP
#define Floor_HPP
#include "Room.hpp"

class Floor
{
    static const int MAX_ROOMS = 8;
    string floor_label;
    Room rooms[MAX_ROOMS];
    int room_count;
    public:
        Floor(string floor_name = "Ground floor");
        string getFloorName() const;
        int getRoomCount() const;
        Room* getRoom(int room_index);
        const Room* getRoom(int room_index) const;
        bool addRoom(string room_name);
        bool removeRoom(int room_index);
        double getFloorLoad(int minute) const;
};
#endif
