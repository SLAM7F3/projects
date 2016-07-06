// ==========================================================================
// Header file for database class
// ==========================================================================
// Last modified on 6/25/10; 6/26/10; 1/11/12
// ==========================================================================

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <libpq-fe.h>
#include "math/Genarray.h"

class database
{

  public:

   database();
   database(std::string host,std::string dbname,std::string user);
   database(const database& d);
   ~database();
   database& operator= (const database& d);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const database& d);

// Set and get member functions:

   bool get_connection_status_flag();
   std::string get_hostname() const;
   std::string get_databasename() const;
   std::string get_username() const;
   std::string get_password() const;
//   std::string get_connection_string() const;
   Genarray<std::string>* get_field_array_ptr();
   int get_nrows() const;
   PGconn* get_db_connection_ptr();
   void set_TableName(std::string& table_name);

// Postgres database connection member functions:

   bool connect();
   bool check_connection_status();
   void print_connection_status();
   void disconnect();

// PostgreSQL command handling member functions:

   void set_SQL_commands(std::vector<std::string> commands);
   bool execute_SQL_commands();
   Genarray<std::string>* select_data(std::string curr_select_command);

   bool send_query(std::string command);
   ExecStatusType receive_results();
   void post_execute_SQL_command();
//   ExecStatusType determine_result_status();
   int parse_rows();
   std::string parse_value(int row,int column);
   void fetch_rows_in_batches(int nrows_to_fetch,std::string cursor_name);

   int get_next_id(std::string sequence);
   int get_n_table_rows(std::string table_name);
   
  private: 

   bool connection_status_flag;
   bool nonblocking_flag,query_sent_flag;
   int n_rows,n_columns;
   Genarray<std::string>* field_array_ptr;
   std::string hostname,database_name,username,password;
   std::string TableName;
   std::vector<std::string> SQL_commands;
   PGconn* db_connection_ptr;
   ConnStatusType connection_status;
   PGresult* result_ptr;
   ExecStatusType ResultStatus;
   
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const database& d);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline std::string database::get_hostname() const
{
   return hostname;
}

inline std::string database::get_databasename() const
{
   return database_name;
}

inline std::string database::get_username() const
{
   return username;
}

inline std::string database::get_password() const
{
   return password;
}

inline bool database::get_connection_status_flag()
{
   return connection_status_flag;
}

inline int database::get_nrows() const
{
   return n_rows;
}

inline PGconn* database::get_db_connection_ptr()
{
   return db_connection_ptr;
}

inline void database::set_TableName(std::string& table_name)
{
   TableName=table_name;
}

inline Genarray<std::string>* database::get_field_array_ptr()
{
//   cout << "inside database::get_field_array_ptr()" << endl;
//   cout << "this = " << this << " field_array_ptr = " << field_array_ptr
//        << endl;
   return field_array_ptr;
}


#endif  // database.h
