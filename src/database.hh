#ifndef DATABASE_HH
#define DATABASE_HH

#include <vector>
#include "wodbc.hh"


int get_odbc_databases(wodbc_t& query, std::vector<std::string>& list);
int get_odbc_tables(wodbc_t& query, const std::string& db_name, std::vector<std::string>& list);


#endif 