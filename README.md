# odbc_explorer_wx
OBDC SQL database browser 
C++ wrapper for the ODBC C API (Windows, Linux)

## build 

```bash
git clone https://github.com/pedro-vicente/odbc_explorer_wx
cd odbc_explorer_wx
./build.cmake.sh
```

Dependencies
---
Installing the Microsoft ODBC Driver for SQL Server on Linux and macOS

https://docs.microsoft.com/en-us/sql/connect/odbc/linux-mac/installing-the-microsoft-odbc-driver-for-sql-server

## API

```cpp
 int connect(const std::string& conn);
 int disconnect();
 int exec_direct(const std::string& sql);
 int fetch(const std::string& sql, table_t& table);
```

## Table structures

A row is a vector of strings

```cpp
struct row_t
{
  std::vector<std::string> col;
};
```

A column has a name and a SQL type

```cpp
struct column_t
{
  std::string name;
  SQLSMALLINT sqltype;
};
```

A table is a vector with rows, with column information (name, SQL type)

```cpp
class table_t
{
public:
  table_t()
  {

  }
  std::vector<column_t> cols;
  std::vector<row_t> rows;
};
```


![Image](https://github.com/user-attachments/assets/28118ecc-44d0-4076-b330-5b1885ccdcdd)


