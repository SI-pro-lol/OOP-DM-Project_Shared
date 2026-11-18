#include"ElectricitySource.hpp"

// Each power source starts turned on
ElectricitySource::ElectricitySource(string source_name): source_label(source_name), enabled(true){}
ElectricitySource::~ElectricitySource(){}
string ElectricitySource::getSourceName() const{return source_label;}
bool ElectricitySource::isEnabled() const{return enabled;}
void ElectricitySource::setEnabled(bool is_on){enabled = is_on;}
