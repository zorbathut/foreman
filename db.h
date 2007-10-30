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

#ifndef FOREMAN_DB
#define FOREMAN_DB

#include "poker.h"

#include <map>

using namespace std;

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
  { "Stone Detailing", "Mason", 1 },
  { "Masonry", "Mason", 0 },
  { "Architecture", "Mason", 1 },
  { "Animal Training", "Trapper", 2 },
  { "Animal Care", "Trapper", 9 },
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
  { "Adam Extraction", "Adamantine", 20 },
  { "Adam Working", "Adamantine", 20 },
  { "Metalsmithing", "Metalsmith", 0 },
  { "Jeweling", "Jeweler", 5 },
  { "Craftsworking", "Craftsdwarf", 3 },
  { "Glassmaking", "Craftsdwarf", 4 },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "(unknown)", "Mason" },
  { "Siege Engineering", "Carpenter", 11 },
  { "Siege Operating", "Peasant", 12 },
  { "Crossbow Making", "Carpenter", 2 },
  { "Mechanics", "Mechanic", 0 },
  { "(unknown)", "Mason" },
  { "Ashery Operating", "Carpenter", 4 },
  { "Dyeing", "Craftsdwarf", 7 },
  { "Wood Burning", "Carpenter", 3 },
  { "(unknown)", "" },
  { "(unknown)", "" },
};

struct Color {
  unsigned char r, g, b;
  
  Color() { };
  Color(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) { };
};

inline bool operator!=(const Color &lhs, const Color &rhs) {
  return !(lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b);
}

class Db {
  map<int, int> outmap;
  vector<int> inmap;
  vector<pair<string, Color> > names;
  vector<string> types;
  
  map<string, DwarfInfo> dinf;
  
public:
  
  vector<pair<string, vector<Change> > > scan(GameHandle *handle);
  void full_write(GameHandle *handle);

  const vector<pair<string, Color> > &getNames() const { return names; }

  vector<pair<string, vector<Change> > > click(int x, int y, GameHandle *handle); // returns the new value of everything
  
  vector<pair<string, vector<Change> > > dump() const;

  Db();
};

#endif
