#ifndef House_HPP
#define House_HPP
#include"Floor.hpp"

class House
{
    string home_n;
    Floor grnd_flr;
    public:
        House(string new_home_name = "My house");
        string getHouseName() const;
        bool setHouseName(string new_home_name);
        Floor& getFloor();
        const Floor& getFloor() const;
        double getTotalLoad(int minute) const;
};
#endif
