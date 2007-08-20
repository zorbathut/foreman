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

#ifndef DNET_SMARTPTR
#define DNET_SMARTPTR

#include "debug.h"

template<typename T> class smart_ptr {
private:
  T *ptr;
  int *ct;

public:
  void reset() {
    if(ptr) {
      (*ct)--;
      if(*ct == 0) {
        delete ct;
        delete ptr;
      }
      ct = NULL;
      ptr = NULL;
    }
  }
  void reset(T *pt) {
    reset();
    if(pt) {
      ptr = pt;
      ct = new int(1);
    }
  }
  
  bool empty() const {
    return !ptr;
  }
  
  T *get() {
    CHECK(ptr);
    return ptr;
  }
  const T *get() const {
    CHECK(ptr);
    return ptr;
  }

  T *operator->() {
    return get();
  }
  const T *operator->() const {
    return get();
  }

  smart_ptr<T> &operator=(const smart_ptr<T> &x) {
    reset();
    if(x.ptr) {
      ptr = x.ptr;
      ct = x.ct;
      (*ct)++;
    }
    return *this;
  }

  smart_ptr() {
    ct = NULL;
    ptr = NULL;
  }
  smart_ptr(const smart_ptr<T> &x) {
    ct = NULL;
    ptr = NULL;
    *this = x;
  }
  explicit smart_ptr<T>(T *pt) {
    ct = NULL;
    ptr = NULL;
    reset(pt);
  }
  ~smart_ptr() {
    reset();
  }
};

#endif
