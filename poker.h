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

#ifndef FOREMAN_POKER
#define FOREMAN_POKER

#include "smartptr.h"

#include <vector>
#include <utility>

#include <windows.h>

const int jobcount = 62;

enum Change { C_NO, C_YES, C_MU };

struct DwarfInfo {
  Change jobs[64];
};

class GameLock {
  HANDLE handle;
  DWORD pid;
  
public:
  
  vector<pair<string, DwarfInfo> > get() const;
  
  void set(const vector<DwarfInfo> &info);

  GameLock(HANDLE handle, DWORD pid);
  ~GameLock();
};

class GameHandle {
  
  HANDLE handle;
  DWORD pid;
  
public:
  
  smart_ptr<GameLock> lockGame();
  bool live() const;

  GameHandle(HANDLE handle, DWORD pid) : handle(handle), pid(pid) { }
  ~GameHandle() { CloseHandle(handle); }

};

smart_ptr<GameHandle> getGameHandle();

#endif
