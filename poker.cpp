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
#include "util.h"
#include "db.h"

#include <tlhelp32.h>

void getMemory(HANDLE handle, DWORD address, void *target, int bytes) {
  DWORD obytes = 0xffffffff;
  int rv = ReadProcessMemory(handle, (void*)address, target, bytes, &obytes);
  CHECK(obytes == bytes);
  CHECK(rv);
}

void setMemory(HANDLE handle, DWORD address, void *target, int bytes) {
  DWORD obytes = 0xffffffff;
  int rv = WriteProcessMemory(handle, (void*)address, target, bytes, &obytes);
  CHECK(obytes == bytes);
  CHECK(rv);
}

DWORD getMemoryDW(HANDLE handle, DWORD address) {
  DWORD storage;
  getMemory(handle, address, &storage, 4);
  return storage;
}

char getMemoryChar(HANDLE handle, DWORD address) {
  char storage;
  getMemory(handle, address, &storage, 1);
  return storage;
}

void setMemoryChar(HANDLE handle, DWORD address, char data) {
  setMemory(handle, address, &data, 1);
}

string getMemoryLT(HANDLE handle, DWORD address) {
  DWORD sptr = getMemoryDW(handle, address + 4);
  DWORD len = getMemoryDW(handle, address + 8);
  vector<char> dat(len);
  getMemory(handle, sptr, &*dat.begin(), len);
  return string(dat.begin(), dat.end());
}

bool isLivingDwarf(HANDLE handle, DWORD address) {
  if(getMemoryDW(handle, address + 0xDC) & (1 << 1))
    return false;
  
  string prof = getMemoryLT(handle, address + 0x60);
  
  CHECK(!prof.size() || getMemoryDW(handle, address + 0x78) == 72); // if a creature has a profession, something weird has happened
  
  if(getMemoryDW(handle, address + 0x78) != 72)
    return false;
  
  int type = getMemoryDW(handle, address + 0x70);
  
  if(0
    || type == 0x00   // miner
    || type == 0x01   // carpenter
    || type == 0x02   // mason
    || type == 0x03   // trapper
    || type == 0x04   // metalsmith
    || type == 0x05   // jeweler
    || type == 0x06   // craftsman
    || type == 0x09   // fisherman
    || type == 0x0A   // farmer
    || type == 0x0B   // mechanic
    || type == 0x52   // peasant
  )
    return true;
  
  return false;
}

const DWORD critter_start = 0x00ACB3EC;

string getProf(HANDLE handle, DWORD addr) {
  string lt = getMemoryLT(handle, addr + 0x60);
  if(lt.size())
    return lt;
  
  int type = getMemoryDW(handle, addr + 0x70);
  if(type == 0x00) return "(Miner)";
  if(type == 0x01) return "(Carpenter)";
  if(type == 0x02) return "(Mason)";
  if(type == 0x03) return "(Trapper)";
  if(type == 0x04) return "(Metalsmith)";
  if(type == 0x05) return "(Jeweler)";
  if(type == 0x06) return "(Craftsdwarf)";
  if(type == 0x09) return "(Fisherdwarf)";
  if(type == 0x0A) return "(Farmer)";
  if(type == 0x0B) return "(Mechanic)";
  if(type == 0x52) return "(Peasant)";
  return "(unnamed)";
}

vector<pair<string, DwarfInfo> > GameLock::get() const {
  DWORD start = getMemoryDW(handle, critter_start);
  DWORD end = getMemoryDW(handle, critter_start + 4);
  dprintf("SE is %08x %08x\n", (unsigned int)start, (unsigned int)end);
  CHECK(end >= start);
  CHECK((end - start) % 4 == 0);
  CHECK(start % 4 == 0);
  CHECK(end % 4 == 0);
  DWORD count = (end - start) / 4;
  
  vector<pair<string, DwarfInfo> > rv;
  
  for(int i = 0; i < count; i++) {
    DWORD addr = getMemoryDW(handle, start);
    start += 4;
    
    if(!isLivingDwarf(handle, addr))
      continue;
    
    string prof = getProf(handle, addr);
    
    //dprintf("Dwarfish: %08x %s is %d\n", addr, prof.c_str(), getMemoryDW(handle, addr + 0x78));
    
    DwarfInfo info;
    for(int i = 0; i < ARRAY_SIZE(info.jobs); i++) {
      if(labor_text[i].descr == "(unknown)") {
        info.jobs[i] = C_MU;
      } else {
        char kar = getMemoryChar(handle, addr + 0x460 + i);
        CHECK(!!kar == kar);
        info.jobs[i] = (Change)kar;
      }
    }
    
    rv.push_back(make_pair(prof, info));
  }

  return rv;
};

void GameLock::set(const vector<pair<string, DwarfInfo> > &sinf) {
  DWORD start = getMemoryDW(handle, critter_start);
  DWORD end = getMemoryDW(handle, critter_start + 4);
  dprintf("SE is %08x %08x\n", (unsigned int)start, (unsigned int)end);
  CHECK(end >= start);
  CHECK((end - start) % 4 == 0);
  CHECK(start % 4 == 0);
  CHECK(end % 4 == 0);
  DWORD count = (end - start) / 4;
  
  vector<pair<string, DwarfInfo> > rv;
  
  int cn = 0;
  for(int i = 0; i < count; i++) {
    DWORD addr = getMemoryDW(handle, start);
    start += 4;
    
    if(!isLivingDwarf(handle, addr))
      continue;
    
    //string firstname = getMemoryLT(handle, addr);
    //string lastname = getMemoryLT(handle, addr + 16);
    
    string prof = getProf(handle, addr);
    
    CHECK(sinf[cn].first == prof);
    
    for(int i = 0; i < ARRAY_SIZE(sinf[cn].second.jobs); i++) {
      if(sinf[cn].second.jobs[i] == C_MU)
        continue;
      setMemoryChar(handle, addr + 0x460 + i, sinf[cn].second.jobs[i]);
      CHECK(getMemoryChar(handle, addr + 0x460 + i) == sinf[cn].second.jobs[i]);
    }
    
    cn++;
  }
  
  CHECK(cn == sinf.size());
};

void SuspendProcess(DWORD pid, bool suspend) { 
  HANDLE snapshot;
  
  dprintf("Switching %d %d\n", (int)pid, suspend);

  snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
  CHECK(snapshot != INVALID_HANDLE_VALUE);
  
  THREADENTRY32 thread;
  thread.dwSize = sizeof(THREADENTRY32); 
 
  CHECK(Thread32First(snapshot, &thread));
 
  do { 
    if(thread.th32OwnerProcessID == pid) {
      HANDLE tt = OpenThread(THREAD_SUSPEND_RESUME, FALSE, thread.th32ThreadID);
      dprintf("Found thread %d, opened with handle %d\n", (int)thread.th32ThreadID, (int)tt);
      if(tt == 0) {
        DWORD gle = GetLastError();
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, gle, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
        dprintf("Failed to open thread - error %d, message %s\n", (int)gle, (char*)lpMsgBuf);
        LocalFree(lpMsgBuf);
        CHECK(0);
      }
      if(suspend) {
        SuspendThread(tt);
      } else {
        ResumeThread(tt);
      }
      CloseHandle(tt);
    }
  } while(Thread32Next(snapshot, &thread));
  
  CloseHandle(snapshot); 
  
  dprintf("Done switching\n");
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

smart_ptr<GameHandle> getGameHandle() {
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
    return smart_ptr<GameHandle>();
  
  return smart_ptr<GameHandle>(new GameHandle(rv, pid));
}
