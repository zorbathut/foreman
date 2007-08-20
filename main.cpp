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
      ID_Undo,
      ID_Redo,
      ID_Cut,
      ID_Copy,
      ID_Paste,
      ID_Delete,

    // Help menu
      ID_About,
  
  // Toolbar items
    ID_NewPath,
    ID_NewPathMenu,
    ID_NewNode,
    ID_NewNodeMenu,
    ID_NewTank,
    ID_NewTankMenu,
    
    ID_GridToggle,
    ID_GridSpinner,
    
    ID_RotGridToggle,
    ID_RotGridSpinner,
    
    ID_ShowControlsToggle,

  // Property pane items
    ID_PathReflects,
    ID_PathRotation
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
    
    menuFile->Append(ID_Undo, "&Undo\tCtrl+Z");
    menuFile->Append(ID_Redo, "&Redo\tCtrl+Y");
    
    menuFile->AppendSeparator();
    
    menuFile->Append(ID_Cut, "Cu&t\tCtrl+X");
    menuFile->Append(ID_Copy, "&Copy\tCtrl+C");
    menuFile->Append(ID_Paste, "&Paste\tCtrl+V");
    menuFile->Append(ID_Delete, "&Delete\tDel");
    
    menuFile->AppendSeparator();
    
    menuFile->Append(ID_NewPathMenu, "Add path\tP");
    menuFile->Append(ID_NewNodeMenu, "Add node\tA");
    menuFile->Append(ID_NewTankMenu, "Add tank\tT");
    
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
  toolbar->AddTool(ID_NewPath, "add path", wxBitmap("data/vecedit/addpath.png", wxBITMAP_TYPE_PNG), "Add a new path", wxITEM_CHECK);
  toolbar->AddTool(ID_NewNode, "add node", wxBitmap("data/vecedit/addnode.png", wxBITMAP_TYPE_PNG), "Add a new node", wxITEM_CHECK);
  toolbar->AddTool(ID_NewTank, "add tank", wxBitmap("data/vecedit/addtank.png", wxBITMAP_TYPE_PNG), "Add a new tank", wxITEM_CHECK);
  
  toolbar->AddSeparator();
  toolbar->AddTool(ID_GridToggle, "toggle grid", wxBitmap("data/vecedit/grid.png", wxBITMAP_TYPE_PNG), "Activate grid lock", wxITEM_CHECK);
  toolbar->AddControl(new wxSpinCtrl(toolbar, ID_GridSpinner, "16", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 16384));  
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
  wxFrame *frame = new ForemanWindow();
  frame->Show(true);
  SetTopWindow(frame);
  return true;
}
