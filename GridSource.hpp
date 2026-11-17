#ifndef GridSource_HPP
#define GridSource_HPP
#include "ElectricitySource.hpp"

class GridSource: public ElectricitySource{
    double grid_limit;
    int outage_start, outage_duration;
    public:
        GridSource();
        bool setData(double limit_w, int outage_start_min, int outage_duration_min);
        double getGridLimit() const;
        int getOutageStart() const;
        int getOutageDuration() const;
        bool isGridAvailable(int minute) const;
        double getAvailablePower(int minute) const;
};
#endif
