#ifndef Appliance_HPP
#define Appliance_HPP
#include<string>
using namespace std;

class Appliance{
    static int next_id;
    int app_id;
    string app_name;
    double rated_w;
    int app_qty;
    bool enabled;

    public:
        Appliance(string new_app_name = "Appliance", double power_w = 0, int qty = 1);
        virtual ~Appliance();
        int getApplianceId() const;
        string getApplianceName() const;
        double getRatedPower() const;
        double getTotalPower() const;
        int getQuantity() const;
        bool isEnabled() const;
        bool setData(string new_app_name, double power_w, int qty, bool is_on);
        void setEnabled(bool is_on);

        virtual bool isRunningAt(int minute) const = 0;
        virtual Appliance* clone() const = 0;
        virtual string getTypeName() const = 0;
};
#endif
