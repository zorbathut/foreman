
#include "conf.h"
#include "parse.h"

#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>

using namespace std;

DWORD check;
DWORD critter_start;
DWORD dwarfidpos;
DWORD profession_start;

DWORD getv(const string &str) {
  CHECK(str.size() <= 8);
  unsigned int dw;
  sscanf(str.c_str(), "%x", &dw);
  return dw;
}

void loadConfig() {
  ifstream ifs("config");
  
  string lin;
  set<string> got;
  while(getline(ifs, lin)) {
    if(!lin.size())
      continue;
    dprintf("Got %s\n", lin.c_str());
    if(lin[lin.size() - 1] == '\r')
      lin.resize(lin.size() - 1);
    vector<string> tok = tokenize(lin, "=");
    CHECK(tok.size() == 2);
    
    DWORD v = getv(tok[1]);
    dprintf("Parsed %s as %d\n", tok[0].c_str(), (unsigned int)v);
    if(tok[0] == "check") {
      check = v;
    } else if(tok[0] == "critter_start") {
      critter_start = v;
    } else if(tok[0] == "dwarfidpos") {
      dwarfidpos = v;
    } else if(tok[0] == "profession_start") {
      profession_start = v;
    } else {
      CHECK(0);
    }
    
    got.insert(tok[0]);
  }
  
  CHECK(got.size() == 4);
}
