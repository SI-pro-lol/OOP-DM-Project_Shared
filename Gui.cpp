#include "Gui.hpp"
#include "BasicAppliance.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <algorithm>
namespace {
std::string num(double n,int precision=2){std::ostringstream s;s<<std::fixed<<std::setprecision(precision)<<n;return s.str();}
std::string timeOf(int m){std::ostringstream s;s<<std::setfill('0')<<std::setw(2)<<m/60<<":"<<std::setw(2)<<m%60;return s.str();}
bool number(const std::string& s,double low,double high,double& n){try{size_t p;n=std::stod(s,&p);return p==s.size()&&std::isfinite(n)&&n>=low&&n<=high;}catch(...){return false;}}
}
Gui::Gui():home("My home"),values{"My home","Living room","","75","1","5000","18:00","0","40"}{
 home.getFloor().addRoom("Living room");home.getFloor().addRoom("Kitchen");
 auto add=[&](int r,std::string n,double w,int q){auto a=new BasicAppliance();a->setData(n,w,q,true);home.getFloor().getRoom(r)->addAppliance(a);};
 add(0,"Ceiling fan",75,2);add(0,"LED light",10,3);add(1,"Refrigerator",150,1);
 values[5]=num(grid.getGridLimit(),0);values[6]=timeOf(grid.getOutageStart());values[7]=std::to_string(grid.getOutageDuration());gridOn=grid.isEnabled();
}
void Gui::label(const std::string& s,float x,float y,unsigned size,sf::Color c){sf::Text t(s,font,size);t.setPosition(x,y);t.setFillColor(c);window.draw(t);}
void Gui::panel(float x,float y,float w,float h,sf::Color c){sf::RectangleShape r({w,h});r.setPosition(x,y);r.setFillColor(c);window.draw(r);}
void Gui::button(const std::string& s,float x,float y,float w,std::function<void()> action){
 sf::FloatRect r(x,y,w,36);auto m=window.mapPixelToCoords(sf::Mouse::getPosition(window));panel(x,y,w,36,r.contains(m)?sf::Color(50,104,125):sf::Color(32,65,84));label(s,x+12,y+5,18);widgets.push_back({r,s,-1,action});
}
void Gui::field(int id,const std::string& title,float x,float y,float w){
 label(title,x,y,16,sf::Color(144,167,187));panel(x,y+25,w,38,focus==id?sf::Color(38,79,98):sf::Color(13,29,44));
 std::string shown=values[id];while(!shown.empty()){sf::Text t(shown,font,19);if(t.getLocalBounds().width<w-24)break;shown.erase(0,1);}label(shown+(focus==id?"|":""),x+10,y+30,19);widgets.push_back({{x,y+25,w,38},"",id,{}});
}
void Gui::invalidate(){ready=false;scroll=0;}
void Gui::selectRoom(int i){room=i;values[1]=home.getFloor().getRoom(i)->getRoomName();appliance=-1;values[2]="";values[3]="75";values[4]="1";enabled=true;}
void Gui::selectAppliance(int i){auto a=home.getFloor().getRoom(room)->getAppliance(i);appliance=i;values[2]=a->getApplianceName();values[3]=num(a->getRatedPower(),1);values[4]=std::to_string(a->getQuantity());enabled=a->isEnabled();}
bool Gui::applyGrid(){
 double limit,duration;int start=-1;auto s=values[6];
 if(s.size()==5&&s[2]==':'&&s[0]>='0'&&s[0]<='9'&&s[1]>='0'&&s[1]<='9'&&s[3]>='0'&&s[3]<='9'&&s[4]>='0'&&s[4]<='9'){
 int h=(s[0]-'0')*10+s[1]-'0',m=(s[3]-'0')*10+s[4]-'0';if(h<24&&(m==0||m==30))start=h*60+m;}
 if(!number(values[5],0,100000,limit)||!number(values[7],0,1440,duration)||std::fmod(duration,30)!=0||start<0){status="Check grid input: HH:00 or HH:30; duration 0-1440 in steps of 30.";return false;}
 GridSource next=grid;if(!next.setData(limit,start,static_cast<int>(duration))){status="Invalid grid settings.";return false;}next.setEnabled(gridOn);grid=next;invalidate();status="Grid settings applied.";return true;
}
void Gui::command(int id){
 auto r=home.getFloor().getRoom(room);
 if(id==0){status=home.setHouseName(values[0])?"House renamed.":"House name must have 1-60 characters.";return;}
 if(id==1){if(!home.getFloor().addRoom(values[1])){status="Enter a room name (maximum 8 rooms).";return;}selectRoom(home.getFloor().getRoomCount()-1);}
 if(id==2){if(!r||!r->setRoomName(values[1])){status="Select a room and enter a valid name.";return;}}
 if(id==3){if(!r){status="Select a room first.";return;}home.getFloor().removeRoom(room);room=home.getFloor().getRoomCount()?0:-1;appliance=-1;if(room>=0)selectRoom(room);else values[1]="";}
 if(id==4||id==5){
 if(!r||(id==5&&appliance<0)){status="Select a room / appliance first.";return;}double w,q;
 if(!number(values[3],0,20000,w)||!number(values[4],1,100,q)||std::floor(q)!=q){status="Power: 0-20000 W. Quantity: whole number 1-100.";return;}
 auto a=new BasicAppliance();if(!a->setData(values[2],w,static_cast<int>(q),enabled)){delete a;status="Appliance name must have 1-60 characters.";return;}
 bool ok=id==4?r->addAppliance(a):r->replaceAppliance(appliance,a);if(!ok){delete a;status="Cannot save appliance (maximum 20 per room).";return;}appliance=-1;values[2]="";
 }
 if(id==6){if(!r||appliance<0){status="Select an appliance first.";return;}r->removeAppliance(appliance);appliance=-1;values[2]="";}
 if(id==7){applyGrid();return;}
 if(id==8){double rate;if(!number(values[8],0,10000,rate)){status="Rate must be 0-10000 PKR/kWh.";return;}if(!applyGrid())return;ready=report.calculateDay(home,grid,rate);status=ready?"Daily report calculated: 48 half-hour intervals.":"Calculation failed.";return;}
 invalidate();status="Changes applied. Run day to update the report.";
}
void Gui::draw(){
 widgets.clear();window.clear(sf::Color(9,20,33));panel(0,0,226,800,sf::Color(16,35,51));label("SMART HOME",24,30,27);label("ENERGY SIMULATOR",24,68,15,sf::Color(87,206,181));
 const char* tabs[]={"01   Household","02   Power sources","03   Daily report"};for(int i=0;i<3;i++){if(page==i)panel(15,143+i*58,196,44,sf::Color(28,78,86));button(tabs[i],22,147+i*58,182,[this,i]{page=i;focus=-1;scroll=0;});}
 label("ONE HOUSE / ONE FLOOR",24,646,14);label("24 hours / 30-minute steps",24,674,15);label("Font: Abel / SIL OFL",24,740,14);
 label(home.getHouseName(),260,24,32);label(page==0?"Manage your rooms and connected appliances":page==1?"Configure the electricity supply":"Understand your daily energy use",260,66,19,sf::Color(144,167,187));
 panel(250,116,1004,610,sf::Color(18,37,54));
 if(page==0){
 field(0,"House name",274,132,280);button("Rename house",570,157,158,[this]{command(0);});
 label("ROOMS",274,222,17,sf::Color(87,206,181));label("APPLIANCES   /   W per unit   /   Qty   /   State",574,222,19);
 for(int i=0;i<home.getFloor().getRoomCount();i++){if(i==room)panel(268,256+i*38,276,36,sf::Color(28,78,86));button(home.getFloor().getRoom(i)->getRoomName(),274,256+i*38,260,[this,i]{selectRoom(i);});}
 auto r=home.getFloor().getRoom(room);if(r){int count=r->getApplianceCount();scroll=std::max(0,std::min(scroll,std::max(0,count-7)));for(int i=scroll;i<std::min(count,scroll+7);i++){
 auto a=r->getAppliance(i);std::string n=a->getApplianceName();if(n.size()>30)n=n.substr(0,27)+"...";if(i==appliance)panel(566,256+(i-scroll)*38,656,36,sf::Color(28,78,86));button(n+"   /   "+num(a->getRatedPower(),1)+" W   /   "+std::to_string(a->getQuantity())+"   /   "+(a->isEnabled()?"ON":"OFF"),574,256+(i-scroll)*38,638,[this,i]{selectAppliance(i);});}}
 field(1,"Room name",274,570,260);button("Add",274,648,78,[this]{command(1);});button("Rename",358,648,82,[this]{command(2);});button("Remove",446,648,88,[this]{command(3);});
 field(2,"Appliance name",574,546,290);field(3,"Power (W)",880,546,140);field(4,"Quantity",1036,546,176);
 button(enabled?"[x] Switched ON":"[ ] Switched OFF",574,623,205,[this]{enabled=!enabled;});button("Add",574,676,105,[this]{command(4);});button("Update",691,676,105,[this]{command(5);});button("Remove",808,676,105,[this]{command(6);});button("Clear form",925,676,135,[this]{appliance=-1;values[2]="";values[3]="75";values[4]="1";enabled=true;});
 label("Scroll over the appliance list to see more.",820,629,16,sf::Color(144,167,187));
 }else if(page==1){
 label("GRID CONNECTION",286,145,23,sf::Color(87,206,181));button(gridOn?"[x] Grid enabled":"[ ] Grid disabled",286,199,290,[this]{gridOn=!gridOn;});field(5,"Connection limit (W)",286,267,290);field(6,"Outage start (HH:MM)",286,361,290);field(7,"Outage duration (minutes)",286,455,290);button("Apply source settings",286,574,290,[this]{command(7);});
 label("Supply rules",664,201,29);label("One repeating daily outage",664,265,23);label("Start time: HH:00 or HH:30",664,311,21);label("Duration: multiples of 30 minutes",664,353,21);label("Use 0 minutes for no outage",664,395,21);label("Outages can cross midnight",664,437,21);label("All enabled appliances run all day.",664,527,21,sf::Color(87,206,181));
 }else{
 field(8,"Flat rate (PKR/kWh)",278,134,235);button("Run day",534,159,150,[this]{command(8);});label("Taxes and surcharges excluded",718,167,19);
 if(!ready)label("Run day to calculate the current setup.",286,294,26);
 else{const std::string titles[]={"Demand / kWh","Grid import / kWh","Unmet / kWh","Estimated / PKR"};double v[]={report.getDemandKWh(),report.getGridKWh(),report.getUnmetKWh(),report.getEstimatedCost()};for(int i=0;i<4;i++){panel(278+i*239,239,225,99,sf::Color(25,53,70));label(titles[i],292+i*239,250,17);label(num(v[i]),292+i*239,279,30,sf::Color(87,206,181));}
 label("Time                 Demand W             Grid W                Unmet W             Grid status",286,360,20);scroll=std::max(0,std::min(scroll,report.getStepCount()-7));for(int i=scroll;i<std::min(report.getStepCount(),scroll+7);i++){auto s=report.getStep(i);float y=401+(i-scroll)*38;if(i%2==0)panel(278,y,950,36,sf::Color(22,45,61));label(timeOf(s->minute),290,y+4,19);label(num(s->demand_wh*2,1),465,y+4,19);label(num(s->grid_wh*2,1),665,y+4,19);label(num(s->unmet_wh*2,1),849,y+4,19);label(s->grid_available?"Available":"Outage / off",1045,y+4,19);}label("Mouse wheel: scroll through all 48 intervals",286,687,17);}
 }
 label(status,260,748,18,sf::Color(244,201,115));window.display();
}
int Gui::run(){
 if(!font.loadFromFile("assets/Abel-Regular.ttf")){std::cerr<<"Missing assets/Abel-Regular.ttf. Run from the project folder.\n";return 1;}
 window.create(sf::VideoMode(1280,800),"Smart Home | SFML",sf::Style::Default);window.setFramerateLimit(60);window.setKeyRepeatEnabled(true);
 while(window.isOpen()){
 draw();sf::Event e;while(window.pollEvent(e)){
 if(e.type==sf::Event::Closed)window.close();
 if(e.type==sf::Event::Resized){sf::View v(sf::FloatRect(0,0,1280,800));float ratio=static_cast<float>(e.size.width)/std::max(1u,e.size.height),base=1.6f;if(ratio>base)v.setViewport({(1-base/ratio)/2,0,base/ratio,1});else v.setViewport({0,(1-ratio/base)/2,1,ratio/base});window.setView(v);}
 if(e.type==sf::Event::MouseButtonPressed&&e.mouseButton.button==sf::Mouse::Left){auto p=window.mapPixelToCoords({e.mouseButton.x,e.mouseButton.y});focus=-1;for(const auto& w:widgets)if(w.box.contains(p)){focus=w.field;auto action=w.action;if(action)action();break;}}
 if(e.type==sf::Event::MouseWheelScrolled){scroll+=e.mouseWheelScroll.delta<0?1:-1;}
 if(e.type==sf::Event::TextEntered&&focus>=0){auto& s=values[focus];if(e.text.unicode==8){if(!s.empty())s.pop_back();}else if(e.text.unicode>=32&&e.text.unicode<127&&s.size()<60)s+=static_cast<char>(e.text.unicode);if(focus==8)invalidate();}
 if(e.type==sf::Event::KeyPressed&&e.key.code==sf::Keyboard::Escape)focus=-1;
 if(e.type==sf::Event::KeyPressed&&e.key.code==sf::Keyboard::Tab){std::vector<int> ids;for(auto& w:widgets)if(w.field>=0)ids.push_back(w.field);auto it=std::find(ids.begin(),ids.end(),focus);if(!ids.empty())focus=it==ids.end()||++it==ids.end()?ids.front():*it;}
 }
 }return 0;
}
