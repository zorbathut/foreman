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

#include "poker.h"

boost::optional<GameHandle> getGameHandle() {
  HANDLE rv;
  
  HWND wnd;
  DWORD pid;
  
  wnd = FindWindow(NULL, "Dwarf Fortress");
  dprintf("%08x\n", wnd);
  GetWindowThreadProcessId(wnd, &pid);
  dprintf("%08x\n", pid);
  rv = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  dprintf("%08x\n", rv);
  
  if(rv == NULL)
    return boost::optional<GameHandle>();
  
  return GameHandle(rv);
}

HANDLE GetProcess(char name[]){
	HANDLE handle;
	HWND h_window;
	DWORD pid;

	h_window = FindWindow(NULL,name);
	GetWindowThreadProcessId(h_window,&pid);
	handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	return(handle);
}
