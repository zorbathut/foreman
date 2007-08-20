/*

Dwarf Foreman
Copyright (C) 2007 Ben Wilhelm

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, Version 3 only.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

I can be contacted at zorba-foreman@pavlovian.net

*/

#include "db.h"

#include "util.h"

#include <set>

#include <boost/static_assert.hpp>

using namespace std;

const string color_text[] = {
  "Miner",
  "Mason",
  "Carpenter",
  "Metalsmith",
  "Trapper",
  "Jeweler",
  "Craftsdwarf",
  "Farmer",
  "Fisherdwarf",
  "Mechanic",
  "Peasant",
};

struct LaborDescr {
  string descr;
  string type;
  int priority;
};

const LaborDescr labor_text[] = {
  { "Mining", "Miner", 0 },
  { "Stone Hauling", "Peasant", 0},
  { "Wood Hauling", "Peasant", 1 },
  { "Burial", "Peasant", 8 },
  { "Food Hauling", "Peasant", 3 },
  { "Refuse Hauling", "Peasant", 4 },
  { "Item Hauling", "Peasant", 2 },
  { "Furniture Hauling", "Peasant", 5 },
  { "Animal Hauling", "Peasant", 6 },
  { "Cleaning", "Peasant", 7 },
  { "Wood Cutting", "Carpenter", 0 },
  { "Carpentry", "Carpenter", 1 },
  { "Stone Detailing", "Miner", 1 },
  { "Masonry", "Mason", 0 },
  { "Architecture", "Mason", 1 },
  { "Animal Training", "Trapper", 2 },
  { "Animal Care", "Peasant", 9 },
  { "Health Care", "Peasant", 10 },
  { "Butchery", "Farmer", 3 },
  { "Trapping", "Trapper", 0 },
  { "Leatherworking", "Craftsdwarf", 2 },
  { "Tanning", "Craftsdwarf", 8 },
  { "Brewing", "Farmer", 2 },
  { "Alchemy", "Craftsdwarf", 9 },
  { "Weaving", "Craftsdwarf", 0 },
  { "Clothes Making", "Craftsdwarf", 1 },
  { "Milling", "Farmer", 4 },
  { "Farming Workshop", "Farmer", 6 },
  { "Cooking", "Farmer", 1 },
  { "Farming", "Farmer", 0 },
  { "Plant Gathering", "Farmer", 5 },
  { "Fishing", "Fisherdwarf", 0 },
  { "Fish Cleaning", "Fisherdwarf", 1 },
  { "Hunting", "Trapper", 1 },
  { "Furnace Operating", "Metalsmith", 1 },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "Metalsmithing", "Metalsmith", 0 },
  { "Jeweling", "Jeweler", 5 },
  { "Craftsworking", "Craftsdwarf", 3 },
  { "Glassmaking", "Craftsdwarf", 4 },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "Siege Engineering", "Peasant", 11 },
  { "Siege Operating", "Peasant", 12 },
  { "Crossbow Making", "Carpenter", 2 },
  { "Mechanics", "Mechanic", 0 },
  { "(unknown)", "" },
  { "(unknown)", "" },
  { "Dyeing", "Craftsdwarf", 7 },
  { "Wood Burning", "Carpenter", 3 },
  { "(unknown)", "" },
  { "Ashery Operating", "Carpenter", 4 },
};

vector<pair<string, vector<Change> > > Db::scan(GameHandle *handle) {
  {
    DwarfInfo dinf;
    CHECK(ARRAY_SIZE(labor_text) == ARRAY_SIZE(dinf.jobs));
    set<string> cname(color_text, color_text + ARRAY_SIZE(color_text));
    CHECK(cname.size() == ARRAY_SIZE(color_text));
    for(int i = 0; i < ARRAY_SIZE(labor_text); i++)
      CHECK(labor_text[i].descr == "(unknown)" && labor_text[i].type == "" || cname.count(labor_text[i].type));
  }
  
  vector<pair<string, DwarfInfo> > dat;
  {
    smart_ptr<GameLock> lock = handle->lockGame();
    dat = lock->get();
  }
  
  set<string> touched;
  for(int i = 0; i < dat.size(); i++) {
    if(dat[i].first == "")
      dat[i].first = "(unnamed)";
    
    if(touched.count(dat[i].first)) {
      for(int j = 0; j < ARRAY_SIZE(dat[i].second.jobs); j++) {
        if(dinf[dat[i].first].jobs[j] != dat[i].second.jobs[j]) {
          dinf[dat[i].first].jobs[j] = C_MU;
        }
      }
    } else {
      touched.insert(dat[i].first);
      dinf[dat[i].first] = dat[i].second;
    }
  }
  
  vector<pair<string, vector<Change> > > rv;
  for(map<string, DwarfInfo>::iterator itr = dinf.begin(); itr != dinf.end(); itr++) {
    for(int j = 0; j < ARRAY_SIZE(itr->second.jobs); j++) {
      if(labor_text[j].descr == "(unknown)")
        itr->second.jobs[j] = C_MU; // don't take any chances
    }
  }
  
  return rv;
}

Db::Db() {
  vector<pair<int, pair<int, int> > > sorty;
  for(int i = 0; i < ARRAY_SIZE(labor_text); i++)
    if(labor_text[i].descr != "(unknown)")
      sorty.push_back(make_pair(distance(color_text, find(color_text, color_text + ARRAY_SIZE(color_text), labor_text[i].type)), make_pair(labor_text[i].priority, i)));
  
  sort(sorty.begin(), sorty.end());
  
  for(int i = 0; i < sorty.size(); i++) {
    outmap[sorty[i].second.second] = i;
    inmap[i] = sorty[i].second.second;
    names.push_back(labor_text[sorty[i].second.second].descr);
    types.push_back(color_text[sorty[i].first]);
    dprintf("%s: %s\n", color_text[sorty[i].first].c_str(), labor_text[sorty[i].second.second].descr.c_str());
  }
}
