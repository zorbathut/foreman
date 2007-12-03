#ifndef FOREMAN_CONF
#define FOREMAN_CONF

#include <windows.h>

#include <string>

using namespace std;

const string foremanname =  "Dwarf Foreman 0.3.8b";

void loadConfig();

extern DWORD check;
extern DWORD critter_start;
extern DWORD dwarfidpos;
extern DWORD profession_start;

#endif
