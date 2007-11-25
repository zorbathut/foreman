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
#include "conf.h"

#include <tlhelp32.h>
#include <psapi.h>

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
  if(len > cap)
    dprintf("Len is %d, cap is %d, I don't know what the fuck is going on here and so I'm going to crash\n", (int)len, (int)cap);
  CHECK(len <= cap);
  CHECK(len >= 0);
  CHECK(len < (1 << 20));   // no
  vector<char> dat(len);
  getMemory(handle, addr, &*dat.begin(), len);
  return string(dat.begin(), dat.end());
}

//vector<int> bitses(20 * 8);

int realdwarfid = -1;

bool isLivingDwarf(HANDLE handle, DWORD address) {
  
  if(realdwarfid == -1) {
    realdwarfid = getMemoryDW(handle, dwarfid);
    dprintf("Dwarf ID is %02x\n", realdwarfid);
  }

  /*dprintf("%s the '%s': %08x, %08x\n", getMemoryString(handle, address + 4).c_str(), getMemoryString(handle, address + 0x70).c_str(), (int)getMemoryDW(handle, address + 0xa0), (int)getMemoryDW(handle, address + 0xa4));
  
  for(int i = 0; i < 20; i++)  {
    char blorb = getMemoryChar(handle, address + 0xe0 - 8 + i);
    for(int j = 0; j < 8; j++) {
      if(blorb & (1 << j))
        bitses[i * 8 + j]++;
    }
  }*/
  
  //dprintf("%s the '%s': %d %d flags, %08x address, %08x %08x type\n", getMemoryString(handle, address + 4).c_str(), getMemoryString(handle, address + 0x70).c_str(), (int)getMemoryDW(handle, address + 0xe4) & (1 << 1), (int)getMemoryDW(handle, address + 0xe8) & (1 << 7), (unsigned long)address, getMemorySW(handle, address + 0x88), getMemorySW(handle, address + 0x8a));
  
  if(getMemoryDW(handle, address + 0xE8) & (1 << 7))
    return false;
  
  string prof = getMemoryString(handle, address + 0x70);
  
  CHECK(!prof.size() || getMemorySW(handle, address + 0x8c) == realdwarfid); // if a creature has a profession, something weird has happened
  
  if(getMemorySW(handle, address + 0x8c) != realdwarfid)
    return false;
  
  //dprintf("%s the '%s': %08x\n", getMemoryString(handle, address + 4).c_str(), getMemoryString(handle, address + 0x70).c_str(), (int)getMemoryDW(handle, address + 0xa0));
  
  return true;
}

string getProf(HANDLE handle, DWORD addr) {
  string lt = getMemoryString(handle, addr + 0x70);
  if(lt.size())
    return lt;
  
  int type = getMemorySW(handle, addr + 0x88);
  if(type == 0x00) return "(Miner)";
  if(type == 0x01) return "(Woodworker)";
  if(type == 0x02) return "(Carpenter)";
  if(type == 0x03) return "(Bowyer)";
  if(type == 0x04) return "(Woodcutter)";
  if(type == 0x05) return "(Stoneworker)";
  if(type == 0x06) return "(Engraver)";
  if(type == 0x07) return "(Mason)";
  if(type == 0x08) return "(Ranger)";
  if(type == 0x09) return "(Animal Caretaker)";
  if(type == 0x0a) return "(Animal Trainer)";
  if(type == 0x0b) return "(Hunter)";
  if(type == 0x0c) return "(Trapper)";
  if(type == 0x0d) return "(Animal Dissector)";
  if(type == 0x0e) return "(Metalsmith)";
  if(type == 0x0f) return "(Furnace Operator)";
  if(type == 0x10) return "(Weaponsmith)";
  if(type == 0x11) return "(Armorer)";
  if(type == 0x12) return "(Blacksmith)";
  if(type == 0x13) return "(Metalcrafter)";
  if(type == 0x14) return "(Jeweler)";
  if(type == 0x15) return "(Gem Cutter)";
  if(type == 0x16) return "(Gem Setter)";
  if(type == 0x17) return "(Craftsdwarf)";
  if(type == 0x18) return "(Woodcrafter)";
  if(type == 0x19) return "(Stonecrafter)";
  if(type == 0x1a) return "(Leatherworker)";
  if(type == 0x1b) return "(Bone Carver)";
  if(type == 0x1c) return "(Weaver)";
  if(type == 0x1d) return "(Clothier)";
  if(type == 0x1e) return "(Glassmaker)";
  if(type == 0x1f) return "(Strand Extractor)";
  if(type == 0x20) return "(Queen)";
  if(type == 0x21) return "(Queen Consort)";
  if(type == 0x22) return "(Fishery Worker)";
  if(type == 0x23) return "(Fisherdwarf)";
  if(type == 0x24) return "(Fish Dissector)";
  if(type == 0x25) return "(Fish Cleaner)";
  if(type == 0x26) return "(Farmer)";
  if(type == 0x27) return "(Cheese Maker)";
  if(type == 0x28) return "(Milker)";
  if(type == 0x29) return "(Cook)";
  if(type == 0x2a) return "(Thresher)";
  if(type == 0x2b) return "(Miller)";
  if(type == 0x2c) return "(Butcher)";
  if(type == 0x2d) return "(Tanner)";
  if(type == 0x2e) return "(Dyer)";
  if(type == 0x2f) return "(Planter)";
  if(type == 0x30) return "(Herbalist)";
  if(type == 0x31) return "(Brewer)";
  if(type == 0x32) return "(Soap Maker)";
  if(type == 0x33) return "(Potash Maker)";
  if(type == 0x34) return "(Lye Maker)";
  if(type == 0x35) return "(Wood Burner)";
  if(type == 0x36) return "(Engineer)";
  if(type == 0x37) return "(Mechanic)";
  if(type == 0x38) return "(Siege Engineer)";
  if(type == 0x39) return "(Siege Operator)";
  if(type == 0x3a) return "(Pump Operator)";
  if(type == 0x3b) return "(Clerk)";
  if(type == 0x3c) return "(Administrator)";
  if(type == 0x3d) return "(Trader)";
  if(type == 0x3e) return "(Architect)";
  if(type == 0x3f) return "(Alchemist)";
  if(type == 0x40) return "(Tax Collector)";
  if(type == 0x41) return "(Hammerer)";
  if(type == 0x42) return "(Baroness)";
  if(type == 0x43) return "(Countess)";
  if(type == 0x44) return "(Duchess)";
  if(type == 0x45) return "(Baroness Consort)";
  if(type == 0x46) return "(Countess Consort)";
  if(type == 0x47) return "(Duchess Consort)";
  if(type == 0x48) return "(Philosopher)";
  if(type == 0x49) return "(Advisor)";
  if(type == 0x4a) return "(Dungeon Master)";
  if(type == 0x4b) return "(Merchant)";
  if(type == 0x4c) return "(Diplomat)";
  if(type == 0x4d) return "(Guild Representative)";
  if(type == 0x4e) return "(Merchant Baroness)";
  if(type == 0x4f) return "(Merchant Princess)";
  if(type == 0x50) return "(Outpost Liaison)";
  if(type == 0x51) return "(Druid)";
  if(type == 0x52) return "(champion)";
  if(type == 0x53) return "(hammerdwarf)";
  if(type == 0x54) return "(hammer Lord)";
  if(type == 0x55) return "(speardwarf)";
  if(type == 0x56) return "(spearmaster)";
  if(type == 0x57) return "(crossbowdwarf)";
  if(type == 0x58) return "(elite Crossbowdwarf)";
  if(type == 0x59) return "(wrestler)";
  if(type == 0x5a) return "(elite Wrestler)";
  if(type == 0x5b) return "(axedwarf)";
  if(type == 0x5c) return "(axe Lord)";
  if(type == 0x5d) return "(swordsdwarf)";
  if(type == 0x5e) return "(swordmaster)";
  if(type == 0x5f) return "(macedwarf)";
  if(type == 0x60) return "(mace Lord)";
  if(type == 0x61) return "(pikedwarf)";
  if(type == 0x62) return "(pikemaster)";
  if(type == 0x63) return "(bowdwarf)";
  if(type == 0x64) return "(elite Bowdwarf)";
  if(type == 0x65) return "(blowgunner)";
  if(type == 0x66) return "(elite Bowgunner)";
  if(type == 0x67) return "(recruit)";
  if(type == 0x68) return "(--Hunting creature?--)";
  if(type == 0x69) return "(--War creature?--)";
  if(type == 0x6a) return "(--Master thief?--)";
  if(type == 0x6b) return "(--Thief?--)";
  if(type == 0x6c) return "(Peasant)";
  if(type == 0x6d) return "(Child)";
  if(type == 0x6e) return "(Baby)";
  if(type == 0x6f) return "(Drunk)";
  if(type == 0x70) return "(Lasher)";
  if(type == 0x71) return "(Master Lasher)";
  dprintf("Unknown profession! The dwarf with a first name of %s has profession %02x. Please look up whatever profession that is, then email it to zorba@pavlovian.net so he can get all this together without destroying his brain.", getMemoryString(handle, addr + 0x04).c_str(), (int)type);
  return "(Unknown)";
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
    if(bitses[i] == 7)
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
  const DWORD beg = 0x00400000;
  const DWORD end = 0x00401000;
  vector<char> kod(end - beg);
  bool failed;
  getMemoryFailable(handle, beg, &kod[0], end - beg, &failed);
  if(failed)
    return false;
  DWORD facu = 0;
  DWORD acu = 1;
  for(int i = 0; i < kod.size(); i++) {
    facu = facu + acu;
    acu = acu + kod[i];
  }
  if(facu != check)
    dprintf("is %08x vs %08x\n", (int)check, (int)facu);
  return facu == check;
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
      //dprintf("Found thread %d, opened with handle %d\n", (int)thread.th32ThreadID, (int)tt);
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
    MessageBox(NULL, ("I'm not sure this is the right version of Dwarf Fortress. This version requires Dwarf Fortress " + versionname).c_str(), "Error", MB_OK | MB_ICONERROR);
    return smart_ptr<GameLock>(NULL);
  }
  return pt;
}

// I AM TIRED OF THIS
// also: poker.cpp:373: error: invalid conversion from `BOOL (*)(HWND__*, LPARAM)' to `BOOL (*)(HWND__*, LPARAM)'
// I felt I had to immortalize that error somewhere.
HWND realwnd = 0;
BOOL CALLBACK enumcallback(HWND wnd, LPARAM lol) {
  DWORD pid;
  GetWindowThreadProcessId(wnd, &pid);
  HANDLE rv = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if(rv != NULL) {
    char filename[128];
    GetModuleFileNameEx(rv, NULL, filename, sizeof(filename));
    CloseHandle(rv);
    if(strstr(filename, "dwarfort.exe")) {
      realwnd = wnd;
      return FALSE;
    }
  }
  return TRUE;
}

smart_ptr<GameHandle> getGameHandle() {
  HANDLE rv;
  DWORD pid;
  
  EnumWindows(enumcallback, 0);
  
  if(realwnd == 0)
    return smart_ptr<GameHandle>();
  
  GetWindowThreadProcessId(realwnd, &pid);
  dprintf("%08x\n", (unsigned int)pid);
  rv = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  dprintf("%p\n", rv);
  char filename[128];
  GetModuleFileNameEx(rv, NULL, filename, sizeof(filename));
  dprintf("Found process is %s\n", filename);
  
  if(rv == NULL)
    return smart_ptr<GameHandle>();
  
  return smart_ptr<GameHandle>(new GameHandle(rv, pid));
}
