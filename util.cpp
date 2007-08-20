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

#include "util.h"

#include "debug.h"

#include <vector>
#include <cmath>
#include <sstream>
#include <stdarg.h>

using namespace std;

bool ffwd = false;

static bool inthread = false;

string StringPrintf(const char *bort, ...) {
  CHECK(!inthread);
  inthread = true;
  
  static vector< char > buf(2);
  va_list args;

  int done = 0;
  bool noresize = false;
  do {
    if(done && !noresize)
      buf.resize(buf.size() * 2);
    va_start(args, bort);
    done = vsnprintf(&(buf[0]), buf.size() - 1,  bort, args);
    if(done >= (int)buf.size()) {
      CHECK(noresize == false);
      CHECK(buf[buf.size() - 2] == 0);
      buf.resize(done + 2);
      done = -1;
      noresize = true;
    } else {
      CHECK(done < (int)buf.size());
    }
    va_end(args);
  } while(done == buf.size() - 1 || done == -1);

  CHECK(done < (int)buf.size());

  string rv = string(buf.begin(), buf.begin() + done);
  
  CHECK(inthread);
  inthread = false;
  
  return rv;
};

string stringFromLongdouble(long double x) {
  stringstream str;
  str << x;
  return str.str();
}

string prettyFloatFormat(float v) {
  string borf = StringPrintf("%.2f", v);
  while(borf.size() > 4 && borf[borf.size() - 1] != '.')
    borf.erase(borf.end() - 1);
  if(borf[borf.size() - 1] == '.')
    borf.erase(borf.end() - 1);
  return borf;
}

/*************
 * Cashola
 */

string Money::textual() const {
  string text;
  long long moneytmp = money;
  CHECK(moneytmp >= 0);
  while(moneytmp) {
    text.push_back(moneytmp % 10 + '0');
    moneytmp /= 10;
  }
  if(!text.size())
    text = "0";
  
  int ks = 0;
  while(text.size() >= 5 && count(text.begin(), text.begin() + 3, '0') == 3) {
    text.erase(text.begin(), text.begin() + 3);
    ks++;
  }
  
  reverse(text.begin(), text.end());
  
  if(ks == 0)
    text += " _";
  else if(ks == 1)
    text += " K";
  else if(ks == 2)
    text += " M";
  else if(ks == 3)
    text += " B";
  else if(ks == 4)
    text += " T";
  else
    CHECK(0);
  
  return text;
}

Money::Money() { };
Money::Money(long long in) { money = in; };

long long Money::value() const { return money; };
float Money::toFloat() const { return (float)money; };

Money operator+(const Money &lhs, const Money &rhs) {
  return Money(lhs.value() + rhs.value()); }
 Money operator-(const Money &lhs, const Money &rhs) {
  return Money(lhs.value() - rhs.value()); }

Money operator*(const Money &lhs, double rhs) {
  return Money((long long)(lhs.value() * rhs)); }
Money operator*(const Money &lhs, int rhs) {
  return Money(lhs.value() * rhs); }
int operator/(const Money &lhs, const Money &rhs) {
  return int(min(lhs.value() / rhs.value(), 2000000000LL)); }
Money operator/(const Money &lhs, int rhs) {
  return Money(lhs.value() / rhs); }
Money operator/(const Money &lhs, double rhs) {
  return Money((long long)(lhs.value() / rhs)); }

const Money &operator+=(Money &lhs, const Money &rhs) {
  lhs = lhs + rhs; return lhs; }
const Money &operator-=(Money &lhs, const Money &rhs) {
  lhs = lhs - rhs; return lhs; }

bool operator==(const Money &lhs, const Money &rhs) {
  return lhs.value() == rhs.value(); }
bool operator!=(const Money &lhs, const Money &rhs) {
  return lhs.value() != rhs.value(); }
bool operator<(const Money &lhs, const Money &rhs) {
  return lhs.value() < rhs.value(); }
bool operator<=(const Money &lhs, const Money &rhs) {
  return lhs < rhs || lhs == rhs; }
bool operator>(const Money &lhs, const Money &rhs) {
  return rhs < lhs; }
bool operator>=(const Money &lhs, const Money &rhs) {
  return rhs <= lhs; }

Money moneyFromString(const string &rhs) {
  CHECK(rhs.size());
  for(int i = 0; i < rhs.size() - 1; i++) {
    if(!isdigit(rhs[i])) {
      dprintf("%s isn't money!\n", rhs.c_str());
      CHECK(0);
    }
  }
  CHECK(isdigit(rhs[rhs.size() - 1]) || tolower(rhs[rhs.size() - 1]) == 'k' || tolower(rhs[rhs.size() - 1]) == 'm' || tolower(rhs[rhs.size() - 1]) == 'g');
  
  Money accum = Money(0);
  
  for(int i = 0; i < rhs.size(); i++) {
    if(isdigit(rhs[i])) {
      accum = accum * 10;
      accum = accum + Money(rhs[i] - '0');
    }
  }
  
  if(tolower(rhs[rhs.size() - 1]) == 'k') {
    accum = accum * 1000;
  } else if(tolower(rhs[rhs.size() - 1]) == 'm') {
    accum = accum * 1000000;
  } else if(tolower(rhs[rhs.size() - 1]) == 'g') {
    accum = accum * 1000000000;
  }
  
  return accum;
}

void adler(Adler32 *adl, const Money &money) {
  adler(adl, money.value());
}

// It's made of money!

/*************
 * Misc
 */

int modurot(int val, int mod) {
  if(val < 0)
    val += abs(val) / mod * mod + mod;
  return val % mod;
}

void checkEndian() {
  float j = 12;
  CHECK(reinterpret_cast<unsigned char*>(&j)[3] == 65);
}

string rawstrFromFloat(float x) {
  // This is awful.
  CHECK(sizeof(x) == 4);
  CHECK(numeric_limits<float>::is_iec559);  // yayz
  checkEndian();
  unsigned char *dat = reinterpret_cast<unsigned char*>(&x);
  string beef;
  for(int i = 0; i < 4; i++)
    beef += StringPrintf("%02x", dat[i]);
  CHECK(beef.size() == 8);
  return beef;
}

float floatFromString(const string &x) {
  // This is also awful.
  CHECK(sizeof(float) == 4);
  CHECK(numeric_limits<float>::is_iec559); // wootz
  checkEndian();
  CHECK(x.size() == 8);
  float rv;
  unsigned char *dat = reinterpret_cast<unsigned char*>(&rv);
  for(int i = 0; i < 4; i++) {
    int v;
    CHECK(sscanf(x.c_str() + i * 2, "%2x", &v));
    CHECK(v >= 0 && v < 256);
    dat[i] = v;
  }
  CHECK(rawstrFromFloat(rv) == x);
  return rv;
}

string roman_number(int rid) {
  CHECK(rid >= 0);
  rid++; // okay this is kind of grim
  CHECK(rid < 40); // lazy
  string rv;
  while(rid >= 10) {
    rid -= 10;
    rv += "X";
  }
  
  if(rid == 9) {
    rid -= 9;
    rv += "IX";
  }
  
  if(rid >= 5) {
    rid -= 5;
    rv += "V";
  }
  
  if(rid == 4) {
    rid -= 4;
    rv += "IV";
  }
  
  rv += string(rid, 'I');
  
  return rv;
}

int roman_max() {
  return 38;
}

bool withinEpsilon(float a, float b, float e) {
  CHECK(e >= 0);
  if(a == b)
    return true; // I mean even if they're both 0
  float diff = a / b;
  if(diff < 0)
    return false; // it's not even the same *sign*
  if(abs(diff) < 1.0)
    diff = 1 / diff;
  return 1 - diff <= e;
}

float lerp(float lhs, float rhs, float dist) {
  return lhs * (1.0 - dist) + rhs * dist;
}

static vector<pair<int, string> > errs;

void addErrorMessage(const string &str) {
  errs.push_back(make_pair(frameNumber, str));
}
vector<string> returnErrorMessages() {
  while(errs.size() && errs[0].first < frameNumber + 600)
    errs.erase(errs.begin());
  vector<string> rv;
  for(int i = 0; i < errs.size(); i++)
    rv.push_back(errs[i].second);
  return rv;
}
