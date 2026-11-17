#include "BasicAppliance.hpp"

BasicAppliance::BasicAppliance(string new_app_name, double power_w, int qty): Appliance(new_app_name, power_w, qty){}

bool BasicAppliance::isRunningAt(int minute) const{
    return isEnabled() && minute >= 0 && minute < 1440;
}
Appliance* BasicAppliance::clone() const{return new BasicAppliance(*this);}
string BasicAppliance::getTypeName() const{return "Basic";}
