#ifndef ElectricitySource_HPP
#define ElectricitySource_HPP
#include<string>
using namespace std;

class ElectricitySource{
    string source_label;
    bool enabled;
    public:
        ElectricitySource(string source_name);
        virtual ~ElectricitySource();
        string getSourceName() const;
        bool isEnabled() const;
        void setEnabled(bool is_on);
        virtual double getAvailablePower(int minute) const = 0;
};
#endif
