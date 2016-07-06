// ========================================================================
// Program PQTEST 
// ========================================================================
// Last updated on 9/28/06
// ========================================================================

#include <iostream>
#include <libpq-fe.h>
#include "general/sysfuncs.h"
#include "postgres/database.h"

const char* statusToString (ConnStatusType status)
{
   switch (status)
   {
      case CONNECTION_OK: 
         return "OK connection to database";

      case CONNECTION_BAD:
         return "Bad connectoin to database";

         /* Non-blocking mode only below here */
	    
         /*
          * The existence of these should never be relied upon - they
          * should only be used for user feedback or similar purposes.  */

         /* Waiting for connection to be made.  */
      case CONNECTION_STARTED:			
         return "Started";
	    
         /* Connection OK; waiting to send.	   */
      case CONNECTION_MADE:			
         return "Made";
	    
         /* Waiting for a response from the postmaster */
      case CONNECTION_AWAITING_RESPONSE:		
         return "Awaiting Response";
	    
         /* Received authentication; waiting for  backend startup. */
      case CONNECTION_AUTH_OK:	
         return "Auth OK";
	    
         /* Negotiating environment. */
      case CONNECTION_SETENV:			
         return "Setenv (Negotiating Environment";
	    
      case CONNECTION_SSL_STARTUP: 
         return "SSL Startup (Negotiating SSL)";

         /* Internal state: connect() needed */
      case CONNECTION_NEEDED:			
         return "Needed (connect() needed)";

      default:
         return "Unknown Status";
   }
}


// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);

   database db;
   db.connect();
   db.check_connection_status();

   db.set_SQL_command("select customer_id from customer");
   db.execute_SQL_command();

//   db.disconnect();

/*    
   const char* query = "select * from test_table";
   cout << "Sending query: " << query << endl; 

   PGresult* res = PQexec(db_connection, query);
    
   if (res == NULL)
   {
      cout << "Result is null, this is bad" << endl;
   }

   int nrows = PQntuples(res);
   cout << "Query resulted in: " << nrows << " results. " << endl;
    
   int nfields = PQnfields(res);
   cout << "Query results have " << nfields << " fields " << endl;

   char* value1 = PQgetvalue(res, 0, 0);
   cout << value1 << " ";
   char* value2 = PQgetvalue(res, 0, 1);
   cout << value2 << " " << endl;
*/

/*
   cout << "Disconnecting: " << endl;
   PQfinish(db_connection);
   db_connection = NULL;
    
   cout << "Disconnected: " << endl;
*/

   return 0;
}

