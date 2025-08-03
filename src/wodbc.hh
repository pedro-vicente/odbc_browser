#ifndef WX_WODBC_HH_
#define WX_WODBC_HH_
#include "wx/wxprec.h"
#include "wx/wx.h"
#include <string>
#include <sql.h>
#include <sqlext.h>

#include "table.hh"

//Unicode literal
#define WODBC_TEXT(s) u##s

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wodbc_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class wodbc_t
{
public:
  wodbc_t();

  int connect(const wxString& conn);
  int fetch(const wxString& sql, table_t& table);
  int exec_direct(const wxString& sql);
  int disconnect();

  SQLHENV henv; //environment handle
  SQLHDBC hdbc; //connection handle

  wxString make_wconn(const wxString& server, const wxString& database, wxString user = wxString(),
    wxString = wxString());

private:
  std::u16string tou16(const wxString& in);
  void wchar_to_str(std::wstring& dest, const SQLWCHAR* src);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //bind_column_data_t
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  struct bind_column_data_t
  {
    SQLSMALLINT target_type; //the C data type of the result data
    SQLPOINTER target_value_ptr; //pointer to storage for the data
    SQLINTEGER buf_len; //maximum length of the buffer being bound for data (including null-termination byte for char)
    SQLLEN strlen_or_ind; //number of bytes(excluding the null termination byte for character data) available
    //to return in the buffer prior to calling SQLFetch
  };

  void extract_error(SQLHANDLE handle, SQLSMALLINT type);
};

#endif
