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

#include <string>

#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/tbarbase.h>
#include <wx/spinctrl.h>

#include "poker.h"
#include "os.h"
#include "util.h"

using namespace std;

const string foremanname =  "Dwarf Foreman";

/*************
 * ForemanWindow
 */

class ForemanWindow : public wxFrame {
public:
  
  ForemanWindow();
  
  DECLARE_EVENT_TABLE()
};

enum {
  // Menu items
    // Main menu
      ID_New = 1,
      ID_Open,
      ID_Save,
      ID_Saveas,
      ID_Quit,

    // Edit menu
      ID_Connect,
      ID_Scan,

    // Help menu
      ID_About,

};

BEGIN_EVENT_TABLE(ForemanWindow, wxFrame)
END_EVENT_TABLE()

ForemanWindow::ForemanWindow() : wxFrame((wxFrame *)NULL, -1, foremanname, wxDefaultPosition, wxSize(800, 600)) {
  wxMenuBar *menuBar = new wxMenuBar;
  
  {
    wxMenu *menuFile = new wxMenu;
    
    menuFile->Append(ID_New, "&New\tCtrl-N");
    menuFile->Append(ID_Open, "&Open...\tCtrl-O");
    menuFile->Append(ID_Save, "&Save\tCtrl+S");
    menuFile->Append(ID_Saveas, "Save &as...\tCtrl+Shift+S");
    
    menuFile->AppendSeparator();
    
    menuFile->Append(ID_Quit, "E&xit");
    
    menuBar->Append(menuFile, "&File");
  }
  
  {
    wxMenu *menuFile = new wxMenu;
    
    menuFile->Append(ID_Connect, "&Connect\tCtrl+K");
    menuFile->Append(ID_Scan, "&Scan\tCtrl+C");
    
    menuBar->Append(menuFile, "&Edit");
  }
  
  {
    wxMenu *menuFile = new wxMenu;
    
    menuFile->Append(ID_About, "&About...");
    
    menuBar->Append(menuFile, "&Help");
  }
  
  SetMenuBar(menuBar);
  
  // We make this first so it gets redrawn first, which reduces flicker a bit
  CreateStatusBar();
  SetStatusText("");
  
  wxToolBar *toolbar = new wxToolBar(this, wxID_ANY);
  
  toolbar->AddControl(new wxButton(toolbar, ID_Connect, "Connect"));
  
  toolbar->AddSeparator();
  
  toolbar->AddControl(new wxButton(toolbar, ID_Scan, "Scan"));
  
  toolbar->AddSeparator();
  
  toolbar->AddControl(new wxButton(toolbar, ID_Scan, "Full write"));
  
  toolbar->Realize();
  toolbar->SetMinSize(wxSize(0, 25));  // this shouldn't be needed >:(
}

/*************
 * VeceditMain
 */

class ForemanMain : public wxApp {
  virtual bool OnInit();
};

IMPLEMENT_APP(ForemanMain)

bool ForemanMain::OnInit() {
  set_exename("foreman.exe");
  
  wxFrame *frame = new ForemanWindow();
  frame->Show(true);
  SetTopWindow(frame);
  
  smart_ptr<GameHandle> gh = getGameHandle();
  CHECK(gh.get());
  dprintf("Got game handle\n");
  
  {
    smart_ptr<GameLock> gl = gh->lockGame();
    
    vector<pair<string, DwarfInfo> > gt = gl->get();
    for(int i = 0; i < gt.size(); i++) {
      string foo = StringPrintf("name: \"%s\"  joblist: ", gt[i].first.c_str());
      for(int j = 0; j < ARRAY_SIZE(gt[i].second.jobs); j++)
        foo += StringPrintf("%d", gt[i].second.jobs[j]);
      dprintf("%s\n", foo.c_str());
    }
    
    for(int i = 0; i < gt.size(); i++) {
      for(int j = 0; j < ARRAY_SIZE(gt[i].second.jobs); j++)
        gt[i].second.jobs[j] = C_MU;
      gt[i].second.jobs[0xC] = C_YES;
    }
    
    gl->set(gt);
    
    gt = gl->get();
    for(int i = 0; i < gt.size(); i++) {
      string foo = StringPrintf("name: \"%s\"  joblist: ", gt[i].first.c_str());
      for(int j = 0; j < ARRAY_SIZE(gt[i].second.jobs); j++) {
        foo += StringPrintf("%d", gt[i].second.jobs[j]);
        CHECK(gt[i].second.jobs[0xC] == C_YES);
      }
      dprintf("%s\n", foo.c_str());
    }
  }
  
  return true;
}
