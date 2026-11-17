#include"Appliance.hpp"
#include<cmath>
using namespace std;

int Appliance::next_id = 1;

Appliance::Appliance(string new_app_name, double power_w, int qty):
    app_id(next_id++), app_name("Appliance"), rated_w(0),
    app_qty(1), enabled(true){
    setData(new_app_name, power_w, qty, true);
}
Appliance::~Appliance(){}

int Appliance::getApplianceId() const{return app_id;}
string Appliance::getApplianceName() const{return app_name;}
double Appliance::getRatedPower() const{return rated_w;}
double Appliance::getTotalPower() const{return rated_w * app_qty;}
int Appliance::getQuantity() const{return app_qty;}
bool Appliance::isEnabled() const{return enabled;}

bool Appliance::setData(string new_app_name, double power_w, int qty, bool is_on){
    if(new_app_name.empty() || new_app_name.size() > 60 || new_app_name.find_first_not_of(" \t\r\n") == string::npos)
        return false;
    if(!isfinite(power_w) || power_w < 0 || power_w > 20000 || qty < 1 || qty > 100)
        return false;
    app_name = new_app_name;
    rated_w = power_w;
    app_qty = qty;
    enabled = is_on;
    return true;
}
void Appliance::setEnabled(bool is_on){enabled = is_on;}
