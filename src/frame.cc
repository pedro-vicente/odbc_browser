#include "frame.hh"
#include "database.hh"

#include <sstream>
#include <wx/font.h>
#include <wx/settings.h>

wxIMPLEMENT_APP(wxAppTables);
wxColour clr_sql_bar = wxColour(240, 240, 240);
wxColour clr_browser = wxColour(233, 231, 234);

//"SELECT name FROM master.sys.databases;"
//"SELECT * FROM information_schema.tables;"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wxAppTables
/////////////////////////////////////////////////////////////////////////////////////////////////////

bool wxAppTables::OnInit()
{
  wxImage::AddHandler(new wxPNGHandler);

  wxString db_connection;
  wxString db_server;
  wxString db_name;
  db_server = "localhost";
  db_connection = query.make_wconn(db_server, db_name);
  wxLogDebug("%s", db_connection.ToStdString().c_str());
  if (query.connect(db_connection) < 0)
  {
    assert(0);
    return false;
  }

  if (!wxApp::OnInit())
  {
    return false;
  }

  wxFrameTables* frame = new wxFrameTables("Table");
  frame->Show(true);
  return true;
}

int wxAppTables::OnExit()
{
  if (query.disconnect() < 0)
  {
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wxFrameTables::wxFrameTables
/////////////////////////////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(wxFrameTables, wxFrame)
EVT_MENU(wxID_EXIT, wxFrameTables::OnQuit)
EVT_MENU(wxID_ABOUT, wxFrameTables::OnAbout)
wxEND_EVENT_TABLE()

wxFrameTables::wxFrameTables(const wxString& title)
  : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE)
{
  wxDisplay display(wxDisplay::GetFromWindow(this));
  wxRect rect = display.GetClientArea();
  SetSize(rect);
  wxMenu* menu_file = new wxMenu;
  menu_file->Append(wxID_EXIT, "E&xit\tAlt-X", "Quit this program");
  wxMenu* menu_help = new wxMenu;
  menu_help->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");
  wxMenuBar* menu_bar = new wxMenuBar();
  menu_bar->Append(menu_file, "&File");
  menu_bar->Append(menu_help, "&Help");
  SetMenuBar(menu_bar);
  CreateStatusBar(2);
  SetStatusText("Ready");

  if (get_odbc_databases(wxGetApp().query, list_databases_) < 0)
  {
    assert(0);
  }
  if (get_odbc_tables(wxGetApp().query, "master", list_tables_) < 0)
  {
    assert(0);
  }

  splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_THIN_SASH);
  splitter->SetSashInvisible(false);
  PanelBrowser* panel_browser = new PanelBrowser(splitter);
  PanelCurrent* panel_current = new PanelCurrent(splitter);
  splitter->SplitVertically(panel_browser, panel_current, 400);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wxFrameTables::OnQuit
/////////////////////////////////////////////////////////////////////////////////////////////////////

void wxFrameTables::OnQuit(wxCommandEvent& WXUNUSED(event))
{
  Close(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wxFrameTables::OnAbout
/////////////////////////////////////////////////////////////////////////////////////////////////////

void wxFrameTables::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  wxMessageBox("ODBC Browser - Pedro Vicente, 2025\n\n",
    "About", wxOK | wxICON_INFORMATION);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//DialogConnect
/////////////////////////////////////////////////////////////////////////////////////////////////////

DialogConnect::DialogConnect(const wxString& title)
  : wxDialog(NULL, -1, title, wxDefaultPosition, wxSize(400, 230))
{
  wxPanel* panel = new wxPanel(this, wxID_ANY);
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* sizer_btn = new wxBoxSizer(wxHORIZONTAL);
  wxButton* btn_ok = new wxButton(this, wxID_OK, wxT("Ok"), wxDefaultPosition, wxSize(70, 30));
  wxButton* btn_cancel = new wxButton(this, wxID_CANCEL, wxT("Close"), wxDefaultPosition, wxSize(70, 30));
  sizer_btn->Add(btn_ok, 1);
  sizer_btn->Add(btn_cancel, 1, wxLEFT, 5);
  sizer->Add(panel, 1);
  sizer->Add(sizer_btn, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);
  SetSizer(sizer);
  Centre();
  ShowModal();
  Destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//PanelBrowser::PanelBrowser
/////////////////////////////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(PanelBrowser, wxPanel)
EVT_TREE_SEL_CHANGED(ID_TREE_TABLES, PanelBrowser::OnTreeSelectionChanged)
wxEND_EVENT_TABLE()

PanelBrowser::PanelBrowser(wxWindow* parent) :
  wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
  wxFrameTables* main = (wxFrameTables*)wxGetApp().GetTopWindow();

  wxPanel* panel = new wxPanel(this, wxID_ANY);
  panel->SetBackgroundColour(clr_browser);
  wxColour clr = panel->GetBackgroundColour();

  tree = new wxTreeCtrl(panel, ID_TREE_TABLES, wxDefaultPosition, wxSize(400, 200));

  wxSize size = FromDIP(wxSize(16, 16));
  wxImageList* list = new wxImageList(size.x, size.y, true, 2);
  list->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, size));
  list->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, size));

  tree->AssignImageList(list);
  tree->SetBackgroundColour(clr);
  wxTreeItemId root = tree->AddRoot("Tables", 0);

  std::vector<std::string>& databases = main->list_databases_;

  wodbc_t& query = wxGetApp().query;

  for (size_t idx_dba = 0; idx_dba < databases.size(); idx_dba++)
  {
    std::string str_dba = databases.at(idx_dba);
    wxLogDebug("%s", str_dba);
    wxTreeItemId item_dba = tree->AppendItem(root, str_dba, 1);

    std::vector<std::string> tables;

    if (get_odbc_tables(query, str_dba, tables) < 0)
    {
      wxLogDebug("%s", str_dba);
    }

    for (size_t idx_tbl = 0; idx_tbl < tables.size(); idx_tbl++)
    {
      wxString str_tbl = tables.at(idx_tbl);
      wxLogDebug("%s", str_tbl);
      wxTreeItemId item_table = tree->AppendItem(item_dba, str_tbl, 1);
    }

  }
  tree->ExpandAll();

  wxBoxSizer* sizer_panel = new wxBoxSizer(wxHORIZONTAL);
  sizer_panel->Add(tree, 1, wxEXPAND);
  panel->SetSizer(sizer_panel);

  wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(panel, 1, wxEXPAND);
  SetSizerAndFit(sizer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//PanelBrowser::OnTreeSelectionChanged
/////////////////////////////////////////////////////////////////////////////////////////////////////

void PanelBrowser::OnTreeSelectionChanged(wxTreeEvent& evt)
{
  if (tree)
  {
    wxTreeItemId item = evt.GetItem();
    wxString str_tbl = tree->GetItemText(item);
    wxTreeItemId parent_item = tree->GetItemParent(item);
    if (parent_item.IsOk())
    {
      wxString str_dba = tree->GetItemText(parent_item);
      std::stringstream sql;
      //sql << "USE [" << str_dba << "]; ";

      wxFrameTables* main = (wxFrameTables*)wxGetApp().GetTopWindow();
      PanelCurrent* panel = (PanelCurrent*)main->splitter->GetWindow2();
      sql << "SELECT TOP 10 * FROM [" << str_dba << "].[dbo].[" << str_tbl << "];";
      wxLogDebug("%s", sql.str());
      panel->make_odbc_query(sql.str());
    }

  }
  evt.Skip();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//PanelCurrent::PanelCurrent
/////////////////////////////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(PanelCurrent, wxPanel)
wxEND_EVENT_TABLE()

PanelCurrent::PanelCurrent(wxWindow* parent) :
  wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
  SetBackgroundColour(wxColour(0, 255, 0));

  splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_THIN_SASH);
  splitter->SetSashInvisible(false);
  PanelSQL* win_sql = new PanelSQL(splitter);
  wxWindow* win_grid = new wxWindow(splitter, wxID_ANY);
  win_grid->SetBackgroundColour(wxColour(255, 255, 255));
  splitter->SplitHorizontally(win_sql, win_grid, 150);
  wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(splitter, 1, wxGROW | wxALL, 0);
  SetSizerAndFit(sizer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//PanelCurrent::make_odbc_query
/////////////////////////////////////////////////////////////////////////////////////////////////////

void PanelCurrent::make_odbc_query(const wxString& sql)
{
  PanelSQL* win_sql = new PanelSQL(splitter);
  win_sql->make_query(sql);

  wxWindow* win_query = splitter->GetWindow1();
  splitter->ReplaceWindow(win_query, win_sql);
  win_query->Destroy();

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //query
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  wodbc_t& query = wxGetApp().query;

  table_t table;
  wxLogDebug("%s", sql);
  if (query.fetch(sql, table) < 0)
  {
    assert(0);
    return;
  }

  size_t nbr_rows = table.rows.size();
  size_t nbr_cols = table.cols.size();

  /////////////////////////////////////////////////////////////////////////////////////////////////////
   //grid
   /////////////////////////////////////////////////////////////////////////////////////////////////////

  wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

  wxGrid* grid = new wxGrid(splitter, wxID_ANY);
  grid->SetLabelFont(font);
  grid->CreateGrid(nbr_rows, nbr_cols);
  grid->EnableEditing(false);
  grid->SetRowLabelSize(30);

  for (int idx_col = 0; idx_col < nbr_cols; idx_col++)
  {
    grid->SetColLabelValue(idx_col, table.cols.at(idx_col).name);
    grid->AutoSizeColumn(idx_col);
  }

  if (!nbr_rows)
  {
    //always replace the grid, even if there are no rows
    wxWindow* win_grid = splitter->GetWindow2();
    splitter->ReplaceWindow(win_grid, grid);
    win_grid->Destroy();

    return;
  }

  for (size_t idx_row = 0; idx_row < nbr_rows; idx_row++)
  {
    row_t row = table.rows.at(idx_row);
    for (int idx_col = 0; idx_col < nbr_cols; idx_col++)
    {
      grid->SetCellValue(idx_row, idx_col, row.col.at(idx_col));
    }
  }

  wxWindow* win_grid = splitter->GetWindow2();
  splitter->ReplaceWindow(win_grid, grid);
  win_grid->Destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//PanelSQL::PanelSQL
/////////////////////////////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(PanelSQL, wxPanel)
wxEND_EVENT_TABLE()

PanelSQL::PanelSQL(wxWindow* parent) :
  wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
  SetBackgroundColour(wxColour(255, 0, 0));

  splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_THIN_SASH);
  splitter->SetSashInvisible(false);
  wxWindow* win_bar = new wxWindow(splitter, wxID_ANY);
  win_bar->SetBackgroundColour(clr_sql_bar);
  wxWindow* win_sql = new wxWindow(splitter, wxID_ANY);
  win_sql->SetBackgroundColour(wxColour(255, 255, 255));
  splitter->SplitHorizontally(win_bar, win_sql, 30);
  wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(splitter, 1, wxGROW | wxALL, 0);
  SetSizerAndFit(sizer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//PanelSQL::make_query
/////////////////////////////////////////////////////////////////////////////////////////////////////

void PanelSQL::make_query(const wxString& sql)
{
  PanelSQLBar* win_sql_bar = new PanelSQLBar(splitter);
  wxWindow* win_bar = splitter->GetWindow1();
  splitter->ReplaceWindow(win_bar, win_sql_bar);
  win_bar->Destroy();

  wxStyledTextCtrl* text;
  text = new wxStyledTextCtrl(splitter, wxID_ANY);
  text->StyleClearAll();
  text->SetUseHorizontalScrollBar(false);
  text->SetMarginWidth(1, 30);
  text->SetMarginType(1, wxSTC_MARGIN_NUMBER);
  text->StyleSetSpec(wxSTC_STYLE_LINENUMBER, "fore:#000000,back:#FFFFFF");
  text->SetText(sql);
  text->SetLexer(wxSTC_LEX_MSSQL);
  text->StyleSetForeground(wxSTC_MSSQL_COMMENT, wxColour(255, 0, 0));
  text->StyleSetForeground(wxSTC_MSSQL_LINE_COMMENT, wxColour(255, 0, 0));
  text->StyleSetForeground(wxSTC_MSSQL_STRING, wxColour(255, 0, 0));
  text->StyleSetForeground(wxSTC_MSSQL_NUMBER, wxColour(255, 0, 0));
  text->StyleSetForeground(wxSTC_MSSQL_OPERATOR, wxColour(255, 0, 0));
  text->StyleSetForeground(wxSTC_MSSQL_IDENTIFIER, wxColour(0, 150, 0));
  text->StyleSetForeground(wxSTC_MSSQL_VARIABLE, wxColour(0, 150, 0));
  text->StyleSetForeground(wxSTC_MSSQL_COLUMN_NAME, wxColour(0, 0, 255));
  text->StyleSetForeground(wxSTC_MSSQL_STATEMENT, wxColour(0, 0, 150));
  text->StyleSetForeground(wxSTC_MSSQL_DATATYPE, wxColour(150, 150, 150));
  text->StyleSetForeground(wxSTC_MSSQL_SYSTABLE, wxColour(150, 150, 150));
  text->StyleSetForeground(wxSTC_MSSQL_GLOBAL_VARIABLE, wxColour(150, 150, 150));
  text->StyleSetForeground(wxSTC_MSSQL_FUNCTION, wxColour(150, 150, 150));
  text->StyleSetForeground(wxSTC_MSSQL_STORED_PROCEDURE, wxColour(150, 150, 150));
  text->StyleSetForeground(wxSTC_MSSQL_DEFAULT_PREF_DATATYPE, wxColour(150, 150, 150));
  text->StyleSetForeground(wxSTC_MSSQL_COLUMN_NAME_2, wxColour(0, 0, 255));

  wxWindow* win_query = splitter->GetWindow2();
  splitter->ReplaceWindow(win_query, text);
  win_query->Destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//PanelSQLBar::PanelSQLBar
/////////////////////////////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(PanelSQLBar, wxPanel)
EVT_BUTTON(ID_BUTTON_RUN_SQL, PanelSQLBar::OnRun)
wxEND_EVENT_TABLE()

PanelSQLBar::PanelSQLBar(wxWindow* parent) :
  wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
  SetBackgroundColour(clr_sql_bar);
  wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton* btn = new wxButton(this, ID_BUTTON_RUN_SQL, _T("Run"), wxDefaultPosition, wxDefaultSize, 0);
  sizer->Add(btn);
  SetSizerAndFit(sizer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//PanelCurrent::OnRun
/////////////////////////////////////////////////////////////////////////////////////////////////////

void PanelSQLBar::OnRun(wxCommandEvent&)
{
  wxFrameTables* main = (wxFrameTables*)wxGetApp().GetTopWindow();
  PanelCurrent* panel_current = (PanelCurrent*)main->splitter->GetWindow2();
  wxSplitterWindow* splitter = panel_current->splitter;
  PanelSQL* win_sql = (PanelSQL*)splitter->GetWindow1();
  wxStyledTextCtrl* win_text = (wxStyledTextCtrl*)win_sql->splitter->GetWindow2();
  wxString sql = win_text->GetText();
  panel_current->make_odbc_query(sql);
}
