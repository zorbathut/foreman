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

vector<pair<string, vector<Change> > > Db::scan(GameHandle *handle) {
  {
    DwarfInfo dinf;
    CHECK(ARRAY_SIZE(labor_text) == ARRAY_SIZE(dinf.jobs));
    set<string> cname(color_text, color_text + ARRAY_SIZE(color_text));
    CHECK(cname.size() == ARRAY_SIZE(color_text));
    for(int i = 0; i < ARRAY_SIZE(labor_text); i++)
      CHECK(labor_text[i].descr == "(unknown)" && labor_text[i].type == "" || cname.count(labor_text[i].type));
  }
  
  dinf.clear();
  
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
  
  for(map<string, DwarfInfo>::iterator itr = dinf.begin(); itr != dinf.end(); itr++) {
    for(int j = 0; j < ARRAY_SIZE(itr->second.jobs); j++) {
      if(labor_text[j].descr == "(unknown)")
        itr->second.jobs[j] = C_MU; // don't take any chances
    }
  }
  
  return dump();
}

void Db::full_write(GameHandle *handle) {
  smart_ptr<GameLock> lock = handle->lockGame();
  vector<pair<string, DwarfInfo> > dat;
  dat = lock->get();
  
  for(int i = 0; i < dat.size(); i++) {
    if(dinf.count(dat[i].first)) {
      for(int j = 0; j < ARRAY_SIZE(dat[i].second.jobs); j++) {
        dat[i].second.jobs[j] = dinf[dat[i].first].jobs[j];
      }
      
      if(dinf[dat[i].first].jobs[inmap[distance(names.begin(), find(names.begin(), names.end(), "Wood Cutting"))]] == C_YES) {
        // disable all weapons, enable axe
        dat[i].second.jobs[0x29] = C_YES;
        dat[i].second.jobs[0x2a] = C_NO;
        dat[i].second.jobs[0x2b] = C_NO;
        dat[i].second.jobs[0x2c] = C_NO;
        dat[i].second.jobs[0x2d] = C_NO;
        dat[i].second.jobs[0x2e] = C_NO;
        dat[i].second.jobs[0x2f] = C_NO;
      } else if(dinf[dat[i].first].jobs[inmap[distance(names.begin(), find(names.begin(), names.end(), "Mining"))]] == C_YES) {
        // disable all weapons
        dat[i].second.jobs[0x29] = C_NO;
        dat[i].second.jobs[0x2a] = C_NO;
        dat[i].second.jobs[0x2b] = C_NO;
        dat[i].second.jobs[0x2c] = C_NO;
        dat[i].second.jobs[0x2d] = C_NO;
        dat[i].second.jobs[0x2e] = C_NO;
        dat[i].second.jobs[0x2f] = C_NO;
      }
    }
  }
  
  lock->set(dat);
}

vector<pair<string, vector<Change> > > Db::click(int x, int y, GameHandle *handle) {
  dprintf("click at %d, %d\n", x, y);
  
  CHECK(y >= 0 && y < dinf.size());
  
  map<string, DwarfInfo>::iterator it = dinf.begin();
  advance(it, y);
  
  
  while(names[x] == "Wood Cutting" && !it->second.jobs[inmap[x]] && it->second.jobs[inmap[distance(names.begin(), find(names.begin(), names.end(), "Mining"))]] != C_NO)
    click(distance(names.begin(), find(names.begin(), names.end(), "Mining")), y, handle);  // turn mining off if we're turning woodcutting on
  
  while(names[x] == "Mining" && !it->second.jobs[inmap[x]] && it->second.jobs[inmap[distance(names.begin(), find(names.begin(), names.end(), "Wood Cutting"))]] != C_NO)
    click(distance(names.begin(), find(names.begin(), names.end(), "Wood Cutting")), y, handle);  // turn woodcutting off if we're turning mining on
  
  it->second.jobs[inmap[x]] = (Change)!it->second.jobs[inmap[x]]; // we do this *afterwards* so we can't end up in a cycle of infinite toggling
  
  {
    smart_ptr<GameLock> lock = handle->lockGame();
    vector<pair<string, DwarfInfo> > dat;
    dat = lock->get();
    
    for(int i = 0; i < dat.size(); i++) {
      for(int j = 0; j < ARRAY_SIZE(dat[i].second.jobs); j++)
        dat[i].second.jobs[j] = C_MU;
      
      if(dat[i].first == it->first) {
        dat[i].second.jobs[inmap[x]] = it->second.jobs[inmap[x]];
        
        if(names[x] == "Wood Cutting" && it->second.jobs[inmap[x]] == C_YES) {
          // disable all weapons, enable axe
          dat[i].second.jobs[0x29] = C_YES;
          dat[i].second.jobs[0x2a] = C_NO;
          dat[i].second.jobs[0x2b] = C_NO;
          dat[i].second.jobs[0x2c] = C_NO;
          dat[i].second.jobs[0x2d] = C_NO;
          dat[i].second.jobs[0x2e] = C_NO;
          dat[i].second.jobs[0x2f] = C_NO;
        } else if(names[x] == "Mining" && it->second.jobs[inmap[x]] == C_YES) {
          // disable all weapons
          dat[i].second.jobs[0x29] = C_NO;
          dat[i].second.jobs[0x2a] = C_NO;
          dat[i].second.jobs[0x2b] = C_NO;
          dat[i].second.jobs[0x2c] = C_NO;
          dat[i].second.jobs[0x2d] = C_NO;
          dat[i].second.jobs[0x2e] = C_NO;
          dat[i].second.jobs[0x2f] = C_NO;
        }
      }
    }
    
    lock->set(dat);
  }
  
  return dump();
}

vector<pair<string, vector<Change> > > Db::dump() const {
  vector<pair<string, vector<Change> > > rv;
  for(map<string, DwarfInfo>::const_iterator itr = dinf.begin(); itr != dinf.end(); itr++) {
    vector<Change> chang;
    for(int i = 0; i < inmap.size(); i++)
      chang.push_back(itr->second.jobs[inmap[i]]);
    
    rv.push_back(make_pair(itr->first, chang));
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
    inmap.push_back(sorty[i].second.second);
    names.push_back(labor_text[sorty[i].second.second].descr);
    types.push_back(color_text[sorty[i].first]);
    dprintf("%s: %s\n", color_text[sorty[i].first].c_str(), labor_text[sorty[i].second.second].descr.c_str());
  }
}
