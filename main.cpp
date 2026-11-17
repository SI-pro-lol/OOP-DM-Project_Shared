#include"Gui.hpp"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show_command){
    Gui gui;
    return gui.run(instance, show_command);
}
