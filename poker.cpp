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

#include <tlhelp32.h>

void SuspendProcess(DWORD pid, bool suspend) { 
  HANDLE snapshot;

  snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
  CHECK(snapshot != INVALID_HANDLE_VALUE);
  
  THREADENTRY32 thread;
  thread.dwSize = sizeof(THREADENTRY32); 
 
  CHECK(Thread32First(snapshot, &thread));
 
  do { 
    if(thread.th32OwnerProcessID == pid) {
      HANDLE tt = OpenThread(THREAD_SUSPEND_RESUME, FALSE, thread.th32ThreadID);
      dprintf("Found thread %p\n", tt);
      if(suspend) {
        SuspendThread(tt);
      } else {
        ResumeThread(tt);
      }
      CloseHandle(tt);
    }
  } while(Thread32Next(snapshot, &thread));
  
  CloseHandle(snapshot); 
}

DWORD crash_pid = 0;
void emergency_unsuspend() {
  SuspendProcess(crash_pid, false);
}

GameLock::GameLock(HANDLE handle, DWORD pid) : handle(handle), pid(pid) {
  CHECK(crash_pid == 0);
  SuspendProcess(pid, true);
  crash_pid = pid;
  registerCrashFunction(emergency_unsuspend);
};
GameLock::~GameLock() {
  CHECK(crash_pid == pid);
  unregisterCrashFunction(emergency_unsuspend);
  crash_pid = 0;
  SuspendProcess(pid, false);
};

smart_ptr<GameLock> GameHandle::lockGame() {
  return smart_ptr<GameLock>(new GameLock(handle, pid));
}

boost::optional<GameHandle> getGameHandle() {
  HANDLE rv;
  
  HWND wnd;
  DWORD pid;
  
  wnd = FindWindow(NULL, "Dwarf Fortress");
  dprintf("%p\n", wnd);
  GetWindowThreadProcessId(wnd, &pid);
  dprintf("%08x\n", (unsigned int)pid);
  rv = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  dprintf("%p\n", rv);
  
  if(rv == NULL)
    return boost::optional<GameHandle>();
  
  return GameHandle(rv, pid);
}
