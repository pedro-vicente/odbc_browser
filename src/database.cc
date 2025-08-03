#include "database.hh"
#include "table.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_odbc_databases
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_odbc_databases(wodbc_t& query, std::vector<std::string>& list)
{
  std::string sql("SELECT name FROM master.sys.databases;");

  table_t table;
  if (query.fetch(sql, table) < 0)
  {
    assert(0);
    return -1;
  }

  list.clear();
  size_t nbr_rows = table.rows.size();
  for (int idx_row = 0; idx_row < nbr_rows; idx_row++)
  {
    row_t row = table.rows.at(idx_row);
    std::string str = row.col.at(0);
    list.push_back(str);
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_odbc_tables
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_odbc_tables(wodbc_t& query, const std::string& db_name, std::vector<std::string>& list)
{
  std::string sql;
  sql = "ALTER LOGIN [SA] WITH DEFAULT_DATABASE = [";
  sql += db_name;
  sql += "];";
  sql += "SELECT TABLE_NAME FROM ";
  sql += "[";
  sql += db_name;
  sql += "]";
  sql += " .INFORMATION_SCHEMA.TABLES;";

  table_t table;
  if (query.fetch(sql, table) < 0)
  {
    assert(0);
    return -1;
  }

  list.clear();
  size_t nbr_rows = table.rows.size();
  for (int idx_row = 0; idx_row < nbr_rows; idx_row++)
  {
    row_t row = table.rows.at(idx_row);
    std::string str = row.col.at(0);
    list.push_back(str);
  }

  return 0;
}

