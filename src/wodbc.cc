#include "wodbc.hh"
#include <assert.h>

#ifdef _MSC_VER
wxString default_driver = "DRIVER={SQL Server};";
#else
wxString default_driver = "DRIVER=ODBC Driver 17 for SQL Server;TrustServerCertificate=yes;";
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
// odbc_log_info
/////////////////////////////////////////////////////////////////////////////////////////////////////

void odbc_log_info(const std::u16string& sql)
{
  std::wstring ws(sql.begin(), sql.end());
  const wchar_t* wc = ws.c_str();
  wxLogDebug("%s", wc);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//check
/////////////////////////////////////////////////////////////////////////////////////////////////////

void check(RETCODE rc)
{
  assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO || rc == SQL_NO_DATA);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wodbc_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

wodbc_t::wodbc_t() :
  henv(0),
  hdbc(0)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wodbc_t::tou16
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::u16string wodbc_t::tou16(const wxString& in)
{
  std::wstring ws = in.ToStdWstring();
  std::u16string u16str(ws.begin(), ws.end());
  return u16str;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wodbc_t::disconnect
/////////////////////////////////////////////////////////////////////////////////////////////////////

int wodbc_t::disconnect()
{
  RETCODE rc;
  rc = SQLDisconnect(hdbc);
  check(rc);
  rc = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
  check(rc);
  rc = SQLFreeHandle(SQL_HANDLE_ENV, henv);
  check(rc);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wodbc_t::connect
/////////////////////////////////////////////////////////////////////////////////////////////////////

int wodbc_t::connect(const wxString& conn_)
{
  std::u16string conn = tou16(conn_);

  SQLHSTMT hstmt;
  SQLWCHAR dsn[1024];
  SQLSMALLINT dsn_size = 0;
  RETCODE rc;
  rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
  check(rc);
  if (rc < 0) return -1;
  rc = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_UINTEGER);
  check(rc);
  if (rc < 0) return -1;
  rc = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
  check(rc);
  if (rc < 0) return -1;

  rc = SQLDriverConnect(
    hdbc
    , 0
    , (SQLWCHAR*)conn.c_str()
    , SQL_NTS
    , dsn
    , sizeof(dsn) / sizeof(SQLWCHAR)
    , &dsn_size
    , SQL_DRIVER_NOPROMPT
  );

  if (rc == SQL_ERROR)
  {
    extract_error(hdbc, SQL_HANDLE_DBC);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    hdbc = 0;
    return -1;
  }

  check(rc);
  if (rc < 0) return -1;
  std::wstring s;
  wchar_to_str(s, dsn);
  wxLogDebug("%s", s);

  //statement handle
  if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
  {
    rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    check(rc);
    if (rc < 0) return -1;

    //process data

    rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    check(rc);
    if (rc < 0) return -1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wodbc_t::wchar_to_str
/////////////////////////////////////////////////////////////////////////////////////////////////////

void wodbc_t::wchar_to_str(std::wstring& dest, const SQLWCHAR* src)
{
  while (*src)
  {
    dest += *src++;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wodbc_t::fetch
/////////////////////////////////////////////////////////////////////////////////////////////////////

int wodbc_t::fetch(const wxString& sql_, table_t& table)
{
  table.remove();
  std::u16string sql = tou16(sql_);
  odbc_log_info(sql);

  SQLSMALLINT nbr_cols = 0;
  size_t nbr_rows = 0;
  struct bind_column_data_t* bind_data = NULL;
  SQLHSTMT hstmt;
  RETCODE rc;

  rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  check(rc);
  if (rc < 0) return -1;
  rc = SQLExecDirect(hstmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

  if (rc == SQL_ERROR)
  {
    extract_error(hstmt, SQL_HANDLE_STMT);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    hdbc = 0;
  }

  check(rc);
  if (rc < 0) return -1;
  rc = SQLNumResultCols(hstmt, &nbr_cols);
  check(rc);
  if (rc < 0) return -1;

  bind_data = (bind_column_data_t*)malloc(nbr_cols * sizeof(bind_column_data_t));
  for (SQLUSMALLINT idx = 0; idx < nbr_cols; idx++)
  {
    bind_data[idx].target_value_ptr = NULL;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //get column names
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  for (SQLUSMALLINT idx = 0; idx < nbr_cols; idx++)
  {
    SQLWCHAR buf[1024];
    SQLSMALLINT sqltype = 0;
    SQLSMALLINT scale = 0;
    SQLSMALLINT nullable = 0;
    SQLSMALLINT len = 0;
    SQLULEN sqlsize = 0;

    rc = SQLDescribeCol(
      hstmt,
      idx + 1,
      (SQLWCHAR*)buf, //column name
      sizeof(buf) / sizeof(SQLWCHAR),
      &len,
      &sqltype,
      &sqlsize,
      &scale,
      &nullable);
    check(rc);
    if (rc < 0) return -1;

    //store
    std::wstring s;
    wchar_to_str(s, buf);
    wxString str(s);
    column_t col;
    col.name = str;
    col.sqltype = sqltype;
    table.cols.push_back(col);
  }

  for (SQLUSMALLINT idx = 0; idx < nbr_cols; idx++)
  {
    bind_data[idx].target_type = SQL_C_CHAR;
    bind_data[idx].buf_len = (4096 + 1);
    bind_data[idx].target_value_ptr = malloc(sizeof(unsigned char) * bind_data[idx].buf_len);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //SQLBindCol assigns the storage and data type for a column in a result set,
  //including:
  //a storage buffer that will receive the contents of a column of data
  //the length of the storage buffer
  //a storage location that will receive the actual length of the column of data
  //returned by the fetch operation data type conversion
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  for (SQLUSMALLINT idx = 0; idx < nbr_cols; idx++)
  {
    rc = SQLBindCol(
      hstmt,
      idx + 1,
      bind_data[idx].target_type,
      bind_data[idx].target_value_ptr,
      bind_data[idx].buf_len,
      &(bind_data[idx].strlen_or_ind));
    check(rc);
    if (rc < 0) return -1;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //get all rows
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  while (SQL_SUCCEEDED(SQLFetch(hstmt)))
  {
    nbr_rows++;
    row_t row;
    for (int idx_col = 0; idx_col < nbr_cols; idx_col++)
    {
      std::u16string str;
      if (bind_data[idx_col].strlen_or_ind != SQL_NULL_DATA)
      {
        char* buf = (char*)bind_data[idx_col].target_value_ptr;
        SQLLEN len = bind_data[idx_col].strlen_or_ind;
        std::u16string tmp(&buf[0], &buf[len]);
        str = tmp;
      }
      else
      {
        str = WODBC_TEXT("NULL");
      }
      std::wstring ws(str.begin(), str.end());
      wxString tmp(ws);
      row.col.push_back(tmp.ToStdString());
    }
    table.rows.push_back(row);
  }

  rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  check(rc);
  if (rc < 0) return -1;
  for (SQLUSMALLINT idx_col = 0; idx_col < nbr_cols; idx_col++)
  {
    if (bind_data[idx_col].target_value_ptr != NULL)
    {
      free(bind_data[idx_col].target_value_ptr);
    }
  }
  if (bind_data != NULL)
  {
    free(bind_data);
  }
  assert(nbr_rows == table.rows.size());
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wodbc_t::exec_direct
/////////////////////////////////////////////////////////////////////////////////////////////////////

int wodbc_t::exec_direct(const wxString& sql_)
{
  std::u16string sql = tou16(sql_);
  odbc_log_info(sql);

  RETCODE rc;
  SQLHSTMT hstmt;
  rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  check(rc);
  if (rc < 0) return -1;
  rc = SQLExecDirect(hstmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);

  if (rc == SQL_ERROR)
  {
    extract_error(hstmt, SQL_HANDLE_STMT);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    hdbc = 0;
  }

  check(rc);
  if (rc < 0) return -1;
  rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  check(rc);
  if (rc < 0) return -1;

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wodbc_t::extract_error
/////////////////////////////////////////////////////////////////////////////////////////////////////

void wodbc_t::extract_error(SQLHANDLE handle, SQLSMALLINT type)
{
  SQLSMALLINT idx = 0;
  SQLINTEGER native_error;
  SQLWCHAR sql_state[7];
  SQLSMALLINT str_len;
  SQLRETURN rc;

  do
  {
    SQLWCHAR str[1024];
    rc = SQLGetDiagRec(type,
      handle,
      ++idx,
      sql_state,
      &native_error,
      str,
      sizeof(str),
      &str_len);
    if (SQL_SUCCEEDED(rc))
    {
      std::wstring ws;
      wchar_to_str(ws, str);
      wxLogDebug("%s", ws);
    }
  } while (rc == SQL_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//wodbc_t::make_wconn
//make connection string without DNS (Windows, Linux)
//Driver=ODBC Driver 17 for SQL Server;Server=tcp:localhost,1433;UID=my_username;PWD=my_password
/////////////////////////////////////////////////////////////////////////////////////////////////////

wxString wodbc_t::make_wconn(const wxString& server, const wxString& database, wxString user, wxString password)
{
  wxString conn;
  conn += default_driver;
  conn += "SERVER=";
  conn += server;
  conn += ", 1433;";
  if (!user.empty())
  {
    conn += "UID=";
    conn += user;
    conn += ";";
  }
  if (!password.empty())
  {
    conn += "PWD=";
    conn += password;
    conn += ";";
  }
  if (!database.empty())
  {
    conn += "DATABASE=";
    conn += database;
    conn += ";";
  }
  return conn;
}

