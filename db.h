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

class Db {
  map<int, int> outmap;
  vector<int> inmap;
  vector<string> names;
  vector<string> types;
  
  map<string, DwarfInfo> dinf;
  
public:
  
  vector<pair<string, vector<Change> > > scan(GameHandle *handle);
  void full_write(GameHandle *handle);

  Change click(int x, int y, GameHandle *handle); // returns the new value of that spot

  Db();
};

#endif
