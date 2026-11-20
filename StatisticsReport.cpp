#include"StatisticsReport.hpp"
#include<algorithm>
#include<cmath>

StatisticsReport::StatisticsReport(): steps(), step_count(0), demand_kwh(0),
    grid_kwh(0), unmet_kwh(0), estimated_cost(0), peak_load(0){}

void StatisticsReport::addStep(const DailyStep& day_step, double unit_rate){
    if(step_count == 48) return;
    steps[step_count++] = day_step;
    demand_kwh += day_step.demand_wh / 1000;
    grid_kwh += day_step.grid_wh / 1000;
    unmet_kwh += day_step.unmet_wh / 1000;
    estimated_cost += day_step.grid_wh / 1000 * unit_rate;
    if(day_step.demand_wh * 2 > peak_load) peak_load = day_step.demand_wh * 2;
}
int StatisticsReport::getStepCount() const{return step_count;}
const DailyStep* StatisticsReport::getStep(int idx) const{
    if(idx < 0 || idx >= step_count) return nullptr;
    return &steps[idx];
}
double StatisticsReport::getDemandKWh() const{return demand_kwh;}
double StatisticsReport::getGridKWh() const{return grid_kwh;}
double StatisticsReport::getUnmetKWh() const{return unmet_kwh;}
double StatisticsReport::getEstimatedCost() const{return estimated_cost;}
double StatisticsReport::getPeakLoad() const{return peak_load;}
double StatisticsReport::getEnergyBalanceError() const{
    double input_kwh = grid_kwh;
    double output_kwh = demand_kwh - unmet_kwh;
    return input_kwh - output_kwh;
}

bool StatisticsReport::calculateDay(const House& home, const GridSource& grid, double unit_rate){
    if(!std::isfinite(unit_rate) || unit_rate < 0 || unit_rate > 10000) return false;
    StatisticsReport day_rpt;
    for(int min_idx = 0; min_idx < 1440; min_idx += 30){
        DailyStep day_step = {};
        day_step.minute = min_idx;
        day_step.grid_available = grid.isGridAvailable(min_idx);
        // Multiply watts by half an hour to get watt-hours.
        day_step.demand_wh = home.getTotalLoad(min_idx) * 0.5;
        day_step.grid_wh = std::min(day_step.demand_wh, grid.getAvailablePower(min_idx) * 0.5);
        day_step.unmet_wh = day_step.demand_wh - day_step.grid_wh;
        day_rpt.addStep(day_step, unit_rate);
    }
    *this = day_rpt;
    return true;
}
