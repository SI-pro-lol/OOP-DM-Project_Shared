#ifndef Room_HPP
#define Room_HPP
#include "Appliance.hpp"

class Room{
    static const int MAX_APPLIANCES = 20;
    string room_name;
    Appliance* appliances[MAX_APPLIANCES];
    int app_count;
    void clearAppliances();

    public:
        Room(string new_room_name = "Room");
        Room(const Room& source_room);
        Room& operator=(const Room& source_room);
        ~Room();
        string getRoomName() const;
        bool setRoomName(string new_room_name);
        int getApplianceCount() const;
        Appliance* getAppliance(int app_index);
        const Appliance* getAppliance(int app_index) const;
        bool addAppliance(Appliance* app);
        bool replaceAppliance(int app_index, Appliance* app);
        bool removeAppliance(int app_index);
        double getRoomLoad(int minute) const;
};
#endif
