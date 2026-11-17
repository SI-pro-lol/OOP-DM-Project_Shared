#ifndef StatisticsReport_HPP
#define StatisticsReport_HPP
#include "House.hpp"
#include "GridSource.hpp"

struct DailyStep{
    int minute;
    double demand_wh, grid_wh;
    double unmet_wh;
    bool grid_available;
};

class StatisticsReport{
    DailyStep steps[48];
    int step_count;
    double demand_kwh, grid_kwh, unmet_kwh;
    double estimated_cost, peak_load;
    public:
        StatisticsReport();
        bool calculateDay(const House& home, const GridSource& grid, double unit_rate);
        void addStep(const DailyStep& day_step, double unit_rate);
        int getStepCount() const;
        const DailyStep* getStep(int idx) const;
        double getDemandKWh() const;
        double getGridKWh() const;
        double getUnmetKWh() const;
        double getEstimatedCost() const;
        double getPeakLoad() const;
        double getEnergyBalanceError() const;
};
#endif
