#ifndef BasicAppliance_HPP
#define BasicAppliance_HPP
#include "Appliance.hpp"

class BasicAppliance: public Appliance
{
    public:
        BasicAppliance(string new_app_name = "Appliance", double power_w = 0, int qty = 1);
        bool isRunningAt(int minute) const;
        Appliance* clone() const;
        string getTypeName() const;
};
#endif
