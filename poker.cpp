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

void getMemoryFailable(HANDLE handle, DWORD address, void *target, int bytes, bool *failed) {
  DWORD obytes = 0xffffffff;
  int rv = ReadProcessMemory(handle, (void*)address, target, bytes, &obytes);
  if(obytes != bytes || !rv)
    *failed = true;
  else
    *failed = false;
}

void setMemory(HANDLE handle, DWORD address, void *target, int bytes) {
  DWORD obytes = 0xffffffff;
  int rv = WriteProcessMemory(handle, (void*)address, target, bytes, &obytes);
  CHECK(obytes == bytes);
  CHECK(rv);
}

short getMemorySW(HANDLE handle, DWORD address) {
  short storage;
  getMemory(handle, address, &storage, 2);
  return storage;
}

DWORD getMemoryDW(HANDLE handle, DWORD address) {
  DWORD storage;
  getMemory(handle, address, &storage, 4);
  return storage;
}

DWORD getMemoryDWFailable(HANDLE handle, DWORD address, bool *failed) {
  DWORD storage;
  getMemoryFailable(handle, address, &storage, 4, failed);
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

string getMemoryString(HANDLE handle, DWORD address) {
  DWORD cap = getMemoryDW(handle, address + 20);
  DWORD addr;
  if(cap < 16) {
    addr = address;
  } else {
    addr = getMemoryDW(handle, address);
  }
  DWORD len = getMemoryDW(handle, address + 16);
  CHECK(len <= cap);
  vector<char> dat(len);
  getMemory(handle, addr, &*dat.begin(), len);
  return string(dat.begin(), dat.end());
}

//vector<int> bitses(20 * 8);

bool isLivingDwarf(HANDLE handle, DWORD address) {

  /*
  for(int i = 0; i < 20; i++)  {
    char blorb = getMemoryChar(handle, address + 0xe0 - 8 + i);
    for(int j = 0; j < 8; j++) {
      if(blorb & (1 << j))
        bitses[i * 8 + j]++;
    }
  }*/
  
  //dprintf("%s: %d %d flags, %08x address\n", getMemoryString(handle, address + 4).c_str(), (int)getMemoryDW(handle, address + 0xe4) & (1 << 1), (int)getMemoryDW(handle, address + 0xe8) & (1 << 7), (unsigned long)address);
  
  CHECK((getMemoryDW(handle, address + 0xE4) & (1 << 1)) == (getMemoryDW(handle, address + 0xE8) & (1 << 7)));
  
  if(getMemoryDW(handle, address + 0xE4) & (1 << 1))
    return false;
  
  string prof = getMemoryString(handle, address + 0x70);
  
  CHECK(!prof.size() || getMemorySW(handle, address + 0x88) != 0x6c); // if a creature has a profession, something weird has happened
  
  if(getMemorySW(handle, address + 0x88) == 0x6c)
    return false;
  
  return true;
}

const DWORD critter_start = 0x01416A88;
const DWORD prof_start = 0x458;

string getProf(HANDLE handle, DWORD addr) {
  string lt = getMemoryString(handle, addr + 0x70);
  if(lt.size())
    return lt;
  
  return "(unnamed)";
  
  /*
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
  return "(unnamed)";*/
}

vector<pair<string, DwarfInfo> > GameLock::get() const {
  DWORD start = getMemoryDW(handle, critter_start + 4);
  DWORD end = getMemoryDW(handle, critter_start + 8);
  dprintf("SE is %08x %08x, %d items\n", (unsigned int)start, (unsigned int)end, (int)((end - start) / 4));
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
      if(labor_text[i].descr[0] == '(') {
        info.jobs[i] = C_MU;
      } else {
        char kar = getMemoryChar(handle, addr + prof_start + i);
        CHECK(!!kar == kar);
        info.jobs[i] = (Change)kar;
      }
    }
    
    rv.push_back(make_pair(prof, info));
  }
  
  /*
  for(int i = 0; i < bitses.size(); i++)
    if(bitses[i] == 2)
      dprintf("Possible: %d\n", i);
    CHECK(0);*/

  return rv;
};

void GameLock::set(const vector<pair<string, DwarfInfo> > &sinf) {
  DWORD start = getMemoryDW(handle, critter_start + 4);
  DWORD end = getMemoryDW(handle, critter_start + 8);
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
      setMemoryChar(handle, addr + prof_start + i, sinf[cn].second.jobs[i]);
      CHECK(getMemoryChar(handle, addr + prof_start + i) == sinf[cn].second.jobs[i]);
    }
    
    cn++;
  }
  
  CHECK(cn == sinf.size());
};

bool GameLock::confirm() {
  char bf[4];
  bool failed;
  getMemoryFailable(handle, 0x031289A9, bf, sizeof(bf), &failed);
  if(failed)
    return false;
  if(memcmp(bf, ".33a", sizeof(bf)) == 0)
    return true;
  return false;
}

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
  smart_ptr<GameLock> pt = smart_ptr<GameLock>(new GameLock(handle, pid));
  if(!pt->confirm()) {
    MessageBox(NULL, "I'm not sure this is the right version of Dwarf Fortress. Check the Dwarf Foreman titlebar - you need to have that version, otherwise this won't work!\r\n\r\nAlso, if you have any other windows named \"Dwarf Fortress\", you should close them. Don't ask.", "Error", MB_OK | MB_ICONERROR);
    return smart_ptr<GameLock>(NULL);
  }
  return pt;
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
