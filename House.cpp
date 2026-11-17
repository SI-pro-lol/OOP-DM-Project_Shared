#include"House.hpp"

House::House(string new_home_name): home_n("My house"), grnd_flr("Ground floor"){
    setHouseName(new_home_name);
}
string House::getHouseName() const{return home_n;}
bool House::setHouseName(string new_home_name){
    if(new_home_name.empty() || new_home_name.size() > 60 || new_home_name.find_first_not_of(" \t\r\n") == string::npos)
        return false;
    home_n = new_home_name;
    return true;
}
Floor& House::getFloor(){return grnd_flr;}
const Floor& House::getFloor() const{return grnd_flr;}
double House::getTotalLoad(int minute) const{return grnd_flr.getFloorLoad(minute);}
