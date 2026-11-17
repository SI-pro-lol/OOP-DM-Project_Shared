#include "GridSource.hpp"
#include<cmath>

GridSource::GridSource(): ElectricitySource("Grid"), grid_limit(5000),
    outage_start(1200), outage_duration(120){}

bool GridSource::setData(double limit_w, int outage_start_min, int outage_duration_min){
    if(!isfinite(limit_w) || limit_w < 0 || limit_w > 100000) return false;
    if(outage_start_min < 0 || outage_start_min > 1410 || outage_duration_min < 0 || outage_duration_min > 1440) return false;
    if(outage_start_min % 30 != 0 || outage_duration_min % 30 != 0) return false;
    grid_limit = limit_w;
    outage_start = outage_start_min;
    outage_duration = outage_duration_min;
    return true;
}
double GridSource::getGridLimit() const{return grid_limit;}
int GridSource::getOutageStart() const{return outage_start;}
int GridSource::getOutageDuration() const{return outage_duration;}
bool GridSource::isGridAvailable(int minute) const{
    if(!isEnabled() || minute < 0 || minute >= 1440) return false;
    int elapsed_min = (minute - outage_start + 1440) % 1440;
    return elapsed_min >= outage_duration;
}
double GridSource::getAvailablePower(int minute) const{
    if(isGridAvailable(minute)) return grid_limit;
    return 0;
}
