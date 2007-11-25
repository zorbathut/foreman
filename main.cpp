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

#include "poker.h"
#include "os.h"
#include "util.h"
#include "db.h"
#include "functor.h"
#include "conf.h"

#include <boost/noncopyable.hpp>

using namespace std;

/*************
 * ForemanGrid
 */

class ForemanGrid : public wxScrolledWindow, boost::noncopyable {
  vector<pair<string, vector<Change> > > dat;
  vector<pair<string, Color> > names;
  
  smart_ptr<Callback<vector<pair<string, vector<Change> > >, pair<int, int> > > clicky;
  
  void ScrolledRR(const wxRect &rec);
  
public:

  void setGrid(const vector<pair<string, vector<Change> > > &foo);
  void setNames(const vector<pair<string, Color> > &in_names) { names = in_names; };
  
  void OnPaint(wxPaintEvent& event);
  void OnMouse(wxMouseEvent &event);

  ForemanGrid(wxWindow *parent, smart_ptr<Callback<vector<pair<string, vector<Change> > >, pair<int, int> > > clicky);
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ForemanGrid, wxPanel)
  EVT_PAINT(ForemanGrid::OnPaint)
  EVT_LEFT_DOWN(ForemanGrid::OnMouse)
END_EVENT_TABLE()

int xsz = 16;
int ysz = 16;

int xborder = 50;
int yborder = 88;

void ForemanGrid::setGrid(const vector<pair<string, vector<Change> > > &foo) {
  dat = foo;
  
  {
    int segments = 0;
    for(int i = 0; i < names.size(); i++)
      if(i && names[i].second != names[i-1].second)
        segments++;
    if(dat.size()) {
      SetVirtualSize(xborder + xsz * (dat[0].second.size() + segments) + xsz / 2, yborder + ysz * dat.size() + ysz / 2);
      SetScrollRate(20, 20);
    }
  }
  
  
  Refresh();
}

void ForemanGrid::OnPaint(wxPaintEvent& event) {
  wxPaintDC dc(this);
  DoPrepareDC(dc);
  
  dc.SetFont(*wxSWISS_FONT);
  
  xborder = 0;
  for(int i = 0; i < dat.size(); i++) {
    wxCoord w;
    wxCoord h;
    dc.GetTextExtent(dat[i].first, &w, &h);
    xborder = max(xborder, w + 15);
  }
  
  dc.SetPen(*wxTRANSPARENT_PEN);
  
  {
    int segments = 0;
    int start = 0;
    for(int i = 0; i < names.size(); i++) {
      if(i && names[i].second != names[i-1].second) {
        dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(wxColour(names[i - 1].second.r, names[i - 1].second.g, names[i - 1].second.b)));
        dc.DrawRectangle(xborder + xsz * (start + segments), yborder - 8, xsz * (i - start + 1), ysz * (dat.size() + 1));
        start = i;
        segments++;
      }
    }
    dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(wxColour(names.back().second.r, names.back().second.g, names.back().second.b)));
    dc.DrawRectangle(xborder + xsz * (start + segments), yborder - 8, xsz * (names.size() - start + 1), ysz * (dat.size() + 1));
  }
  
  if(dat.size()) {
    for(int m = 0; m < 3; m++) {
      if(m == C_NO)
        dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(wxColour(60, 60, 60)));
      else if(m == C_YES)
        dc.SetBrush(*wxGREEN_BRUSH);
      else if(m == C_MU)
        dc.SetBrush(*wxGREY_BRUSH);
      else
        CHECK(0);
      
      int segments = 0;
      for(int j = 0; j < dat[0].second.size(); j++) {
        if(j && names[j].second != names[j-1].second)
          segments++;
        
        for(int i = 0; i < dat.size(); i++) {
          if(dat[i].second[j] == m) {
            dc.DrawRectangle(int(xborder + xsz * (j + segments + 0.5)), yborder + ysz * i, xsz, ysz);
          }
        }
        
        wxCoord w;
        wxCoord h;
        dc.GetTextExtent(names[j].first, &w, &h);
        dc.DrawRotatedText(names[j].first, (int)(xborder + xsz * (j + segments + 0.5) - w * 0.68 + xsz), (int)(yborder - w * 0.68 - 15 - 8), -45);
      }
    }
  }
  
  /*dc.SetPen(*wxThePenList->FindOrCreatePen(wxColour(60, 60, 60), 1, wxDOT));
  
  for(int i = 1; i < names.size(); i++)
    dc.DrawLine(xborder + xsz * i, yborder - 5, xborder + xsz * i, 10000);
  
  for(int i = 1; i < dat.size(); i++)
    dc.DrawLine(0, yborder + ysz * i, 10000, yborder + ysz * i);*/
  
  dc.SetPen(*wxBLACK_PEN);
  
  {
    int left = 0;
    int segments = 0;
    for(int i = 0; i < names.size(); i++) {
      if(i && names[i].second != names[i-1].second) {
        segments++;
        left = 0;
      }
      if(left == 3) {
        dc.DrawLine(int(xborder + xsz * (i + segments + 0.5)), yborder - 5, int(xborder + xsz * (i + segments + 0.5)), 10000);
        left = 0;
      }
      left++;
    }
  }
  
  for(int i = 3; i < dat.size(); i += 3)
    dc.DrawLine(0, yborder + ysz * i, 10000, yborder + ysz * i);
  
  for(int i = 0; i < dat.size(); i++)
    dc.DrawText(dat[i].first, 0, yborder + ysz * i);
}

inline int findname(const vector<pair<string, Color> > &names, const string &name) {
  for(int i = 0; i < names.size(); i++)
    if(names[i].first == name)
      return i;
  CHECK(0);
}

void ForemanGrid::ScrolledRR(const wxRect &rec) {
  wxRect rec2 = rec;
  CalcScrolledPosition(rec.x, rec.y, &rec2.x, &rec2.y); 
  RefreshRect(rec2);
}

void ForemanGrid::OnMouse(wxMouseEvent &event) {
  event.Skip();
  
  int x;
  int y;
  CalcUnscrolledPosition(event.GetX(), event.GetY(), &x, &y);
  
  x -= xborder;
  y -= yborder;
  
  x -= xsz / 2;
  
  x /= xsz;
  y /= ysz;
  
  int ox = x;
  int oy = y;
  
  for(int i = 0; i <= x; i++) {
    if(i >= names.size())
      return;
    if(i && names[i].second != names[i-1].second) {
      x--;
      if(x < i)
        return;
    }
  }
  
  if(y >= 0 && y < dat.size() && x >= 0 && x < dat[0].second.size()) {
    vector<pair<string, vector<Change> > > dt = clicky->Run(make_pair(x, y));
    if(dt.size())
      dat = dt;
    if(names[x].first == "Wood Cutting" || names[x].first == "Mining") {
      int a = findname(names, "Wood Cutting");
      int b = findname(names, "Mining");
      if(a > b)
        swap(a, b);
      ScrolledRR(wxRect(xborder + xsz * a, yborder + ysz * y, (b - a + 4) * xsz, ysz));
      // whee hack
    } else {
      ScrolledRR(wxRect(xborder + xsz * ox + xsz / 2, yborder + ysz * oy, xsz, ysz));
    }
  }
}

ForemanGrid::ForemanGrid(wxWindow *parent, smart_ptr<Callback<vector<pair<string, vector<Change> > >, pair<int, int> > > clicky) : wxScrolledWindow(parent, wxID_ANY), clicky(clicky) {
};

/*************
 * ForemanWindow
 */

class ForemanWindow : public wxFrame, boost::noncopyable {
  Db db;
  
  ForemanGrid *grid;
  
public:
  
  void OnScan(wxCommandEvent &event);
  void OnFullWrite(wxCommandEvent &event);
  void OnAbout(wxCommandEvent &event);

  void scan();
  vector<pair<string, vector<Change> > > gridclicky(pair<int, int> val);

  smart_ptr<GameHandle> stdConnect();
  
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
      ID_Scan,
      ID_FullWrite,

    // Help menu
      ID_About,
};

BEGIN_EVENT_TABLE(ForemanWindow, wxFrame)
  EVT_TOOL(ID_Scan, ForemanWindow::OnScan)
  EVT_TOOL(ID_FullWrite, ForemanWindow::OnFullWrite)

  EVT_MENU(ID_About, ForemanWindow::OnAbout)

  EVT_BUTTON(ID_Scan, ForemanWindow::OnScan)
  EVT_BUTTON(ID_FullWrite, ForemanWindow::OnFullWrite)
END_EVENT_TABLE()

void ForemanWindow::OnScan(wxCommandEvent &event) {
  scan();
}
void ForemanWindow::OnFullWrite(wxCommandEvent &event) {
  smart_ptr<GameHandle> hnd = stdConnect();
  if(!hnd.get())
    return;
  
  db.full_write(hnd.get());
}
void ForemanWindow::OnAbout(wxCommandEvent &event) {
  wxMessageBox(StringPrintf("%s, written by Ben Wilhelm.\n\nReleased under the GPL.\n\nLatest version and source available at http://www.pavlovian.net/foreman", foremanname.c_str()));
}

void ForemanWindow::scan() {
  smart_ptr<GameHandle> hnd = stdConnect();
  if(hnd.empty())
    return;
  
  vector<pair<string, vector<Change> > > matrix = db.scan(hnd.get());
  grid->setGrid(matrix);
}

vector<pair<string, vector<Change> > > ForemanWindow::gridclicky(pair<int, int> val) {
  smart_ptr<GameHandle> hnd = stdConnect();
  if(hnd.empty())
    return vector<pair<string, vector<Change> > >();
  
  return db.click(val.first, val.second, hnd.get());
}

smart_ptr<GameHandle> ForemanWindow::stdConnect() {
  smart_ptr<GameHandle> hand = getGameHandle();
  if(!hand.empty()) {
    SetStatusText("");
  } else {
    SetStatusText("Couldn't connect to Dwarf Fortress, is it running?");
  }
  
  return hand;
}

ForemanWindow::ForemanWindow() : wxFrame((wxFrame *)NULL, -1, foremanname + " (" + versionname + " compatible)", wxDefaultPosition, wxSize(1000, 600)) {
  wxMenuBar *menuBar = new wxMenuBar;
  
  {
    wxMenu *menuFile = new wxMenu;
    
    /*
    menuFile->Append(ID_New, "&New\tCtrl-N");
    menuFile->Append(ID_Open, "&Open...\tCtrl-O");
    menuFile->Append(ID_Save, "&Save\tCtrl+S");
    menuFile->Append(ID_Saveas, "Save &as...\tCtrl+Shift+S");
    
    menuFile->AppendSeparator();*/
    
    menuFile->Append(ID_Quit, "E&xit");
    
    menuBar->Append(menuFile, "&File");
  }
  
  {
    wxMenu *menuFile = new wxMenu;
    
    menuFile->Append(ID_Scan, "&Load from DF\tCtrl+C");
    menuFile->Append(ID_FullWrite, "&Write to DF\tCtrl+F");
    
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
  
  toolbar->AddControl(new wxButton(toolbar, ID_Scan, "Load from DF"));
  
  toolbar->AddSeparator();
  
  toolbar->AddControl(new wxButton(toolbar, ID_FullWrite, "Write to DF"));
  
  toolbar->Realize();
  toolbar->SetMinSize(wxSize(0, 25));  // this shouldn't be needed >:(
  
  grid = new ForemanGrid(this, NewFunctor(this, &ForemanWindow::gridclicky));
  
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(toolbar, 0, wxEXPAND);
  sizer->Add(grid, 1, wxEXPAND);
  SetSizer(sizer);
  
  grid->setNames(db.getNames());
  
  scan();
}

/*************
 * VeceditMain
 */

class ForemanMain : public wxApp, boost::noncopyable {
  virtual bool OnInit();
};

IMPLEMENT_APP(ForemanMain)

bool ForemanMain::OnInit() {
  set_exename("foreman.exe");
  
  wxFrame *frame = new ForemanWindow();
  frame->Show(true);
  SetTopWindow(frame);
  
  return true;
}
