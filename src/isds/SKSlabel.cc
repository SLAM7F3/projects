// ==========================================================================
// SKSlabel class member function definitions
// ==========================================================================
// Last updated on 10/19/06
// ==========================================================================

#include <math.h>
#include <vector>
#include "astro_geo/latlong2utmfuncs.h"
#include "isds/SKSlabel.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void SKSlabel::allocate_member_objects()
{
}		       

void SKSlabel::initialize_member_objects()
{
   kEntityIdBufSize = 32;
}		       

SKSlabel::SKSlabel()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

SKSlabel::~SKSlabel()
{
}

// -------------------------------------------------------------------
void SKSlabel::initialize_string_constants()
{
   insert_comment_preamble = "INSERT INTO comment (identifier, dt_created, source_url, data_url, metadata_url, mimetype, comment_typ, name, text, labeltextx, labeltexty, labeltype, linecolor, movieclip, id, poi_id, textbackgroundcolor, textcolor, url, username, x, y, xscale, yscale, zoom) VALUES (";

   insert_relation_preamble = "INSERT INTO e_relation (src_id, src_type, tgt_id, tgt_type, typ) VALUES (";
   
   sql_quote = "'";
   sql_comma = ",";
}

// -------------------------------------------------------------------
static void noticeMessageHandler (const char* fmt, ...)
{
   printf("GEOS Error - %s\n", fmt);
}

static void errorMessageHandler (const char* fmt, ...)
{
   printf("GEOS Error - %s\n", fmt);
}
 
// -------------------------------------------------------------------
bool SKSlabel::Setup (void)
{
   // Setup GEOS

   initGEOS(noticeMessageHandler, errorMessageHandler);


   // Now query the entity sequence id table so we can 
   // use the nextval function. 

   char lastEntityId[32];
   memset(lastEntityId, 0, sizeof(lastEntityId));
   initEntityIdSequence(g_connection, lastEntityId, sizeof(lastEntityId));
    
   return (status == CONNECTION_OK);
}

// -------------------------------------------------------------------
//  initEntityIdSequence 
  
//  Query the sequence table so we can then call the sequence nextval
//  function.

void SKSlabel::initEntityIdSequence (
   PGconn* db_connection, char* result, int bufsize)
{
   assert(db_connection != NULL);
   assert(result != NULL);
   const char* query = "select last_value from entity_id_seq;";    

   PGresult* res = PQexec(db_connection, query);
    
   int nrows = PQntuples(res);
   int nfields = PQnfields(res);
    
   assert(nrows == 1);
   assert(nfields == 1);
    
   // We should have at least3 one 
   char* value1 = PQgetvalue(res, 0, 0);
    
   strncpy(result, value1, bufsize);
}

// -------------------------------------------------------------------
// Query the database and place the next value into the

void SKSlabel::getNextEntityId (
   PGconn* db_connection, char* nextEntityId, int bufsize)
{
   assert(db_connection != NULL);
   assert(nextEntityId != NULL);

   const char* query = "select nextval('entity_id_seq')";
    
   PGresult* res = PQexec(db_connection, query);
    
   int nrows = PQntuples(res);
   int nfields = PQnfields(res);

   // We are expecting at least one row and at least one column.
  
   if (nrows < 1 || nfields < 1)
   {
      // Indicate an error and return. 
      assert(false && "Expected at least one value from nextval query");
   }

   char* query_value = PQgetvalue(res, 0, 0);
    
   strncat(nextEntityId, query_value, bufsize);
   strncat(nextEntityId, "@LL", bufsize);
}

// -------------------------------------------------------------------
void SKSlabel::addRelation (PGconn* db_connection, 
                            const char* srcEntityId, 
                            const char* srcEntityType, 
                            const char* tgtEntityId,
                            const char* tgtEntityType)
{
   string sql_buffer = insert_relation_preamble;
   sql_buffer += sql_quote + string(srcEntityId) + sql_quote + sql_comma;
   sql_buffer += sql_quote + string(srcEntityType) + sql_quote + sql_comma;
   sql_buffer += sql_quote + string(tgtEntityId) + sql_quote + sql_comma;
   sql_buffer += sql_quote + string(tgtEntityType) + sql_quote + sql_comma;
   sql_buffer += "'contains');";

   // Now execute this statement.
   PGresult* res = PQexec(db_connection, sql_buffer.c_str());

   // I'm not sure what res will be for an insert
}

// -------------------------------------------------------------------
int SKSlabel::getNextCommentId (PGconn* db_connection, 
                                const char* imageID, 
                                int poi_id,
                                int comment_type)
{
   // Figure out what the largest id (not identifer) is for the
   // poi under consideration. 
   char parameterBuffer[64];
    
   // Find the comments for a given poi_id 
   string sql_buffer = 
      "select c.id from comment c, e_relation e where e.src_id = ";
   sql_buffer += sql_quote + imageID + sql_quote;
   
   sql_buffer += " and c.identifier = e.tgt_id and e.tgt_type = ";

   if (comment_type == comment_type_label)
      sql_buffer += "'com_label'";
   else
      sql_buffer += "'com_note'";

   sql_buffer += " and c.poi_id = ";
    
   sprintf(parameterBuffer, "%d", poi_id);
   sql_buffer += parameterBuffer;
   sql_buffer += ";";

   PGresult* res = PQexec(db_connection, sql_buffer.c_str());

   int nrows = PQntuples(res);
   int nfields = PQnfields(res);

   if (nrows <= 0)
      return 0;  // there are no records matching this criteria. 

   int label_id = atoi(PQgetvalue(res, 0, 0));
    
   for (int i = 1; i < nrows; i++)
   {
      int temp_id = atoi(PQgetvalue(res, i, 0));

      if (temp_id > label_id)
         label_id = temp_id; 
   }

   return label_id + 1;
}

// -------------------------------------------------------------------
bool SKSlabel::imageHasWholeImagePOI (
   PGconn* db_connection, const char* imageID, int& id)
{
   // Look in e_relation to see if there is a com_poi corresponding
   // to this image, and if so, what is it's identifier and id (these
   // are different things.
    
   // - Is it the whole image POI (?)
   // - What is it's identifier (tgt_id). 
   string sql_buffer = 
      "select c.id, c.identifier from comment c, e_relation e where e.src_id = ";
   sql_buffer += sql_quote + string(imageID) + sql_quote;
   sql_buffer += 
      " and c.identifier = e.tgt_id and c.name = 'Whole Image' and e.tgt_type = 'com_poi';";
    
   PGresult* res = PQexec(db_connection, sql_buffer.c_str());

   int nrows = PQntuples(res);
   int nfields = PQnfields(res);

   // Get the smallest id to use as our Whole Image POI id.
    
   if (nrows <= 0)
      return false; // no poi id
    
   int poi_id = atoi(PQgetvalue(res, 0, 0));
    
   for (int i = 1; i < nrows; i++)
   {
      int temp_id = atoi(PQgetvalue(res, i, 0));

      if (temp_id < poi_id)
         poi_id = temp_id; 
   }

   id = poi_id; 

   return (nrows > 0);
}

// -------------------------------------------------------------------
void SKSlabel::addWholeImagePoiComment (
   PGconn* db_connection, const char* imageID)
{
   char nextEntityId[kEntityIdBufSize];  
   memset(nextEntityId, 0, kEntityIdBufSize);
  
   // Get the next id. 
   getNextEntityId(db_connection, nextEntityId, kEntityIdBufSize);

   // Add the Whole Image comment
   // set up the relation in the e_relation table

   // FIXME - Change this to use a string for more readable code
   string sql_buffer = "INSERT INTO ";
   sql_buffer += "comment (identifier, dt_created, source_url, data_url, metadata_url, mimetype,";
   sql_buffer += "comment_typ, name, text, labeltextx, labeltexty, labeltype, linecolor, movieclip, id, poi_id,";
   sql_buffer += "textbackgroundcolor, textcolor, url, username, x, y, xscale, yscale, zoom) ";
   sql_buffer += "VALUES ('";
   sql_buffer += nextEntityId;
   sql_buffer += "',";
   sql_buffer += "now(), '',  '',  '',  '',  'com_poi', 'Whole Image', '',  '',  '', '', '', '', '0', '', '', '', '', '', 0, 0, 0, 0, '-1');";


   // Now execute this statement.
   PGresult* res = PQexec(db_connection, sql_buffer.c_str());

   addRelation(db_connection, imageID, "image", nextEntityId, "com_poi");
  
}

// -------------------------------------------------------------------
void SKSlabel::addPoiComment (PGconn* db_connection, 
                              const char* imageID, 
                              const char* poiName)
{
   // Get the next id
   // Add the POI comment
   // Set up the relation in the e_relation table. 
   char nextEntityId[kEntityIdBufSize];  
   memset(nextEntityId, 0, kEntityIdBufSize);
  
   // Get the next id. 
   getNextEntityId(db_connection, nextEntityId, kEntityIdBufSize);

   // Add the Whole Image comment
   // set up the relation in the e_relation table
  
   // FIXME - Change this to use a string for more readable code
   string sql_buffer = "INSERT INTO ";
   sql_buffer += 
      "comment (identifier, dt_created, source_url, data_url, metadata_url, mimetype,";
   sql_buffer += 
      "comment_typ, name, text, labeltextx, labeltexty, labeltype, linecolor, movieclip, id, poi_id,";
   sql_buffer += 
      "textbackgroundcolor, textcolor, url, username, x, y, xscale, yscale, zoom) ";
   sql_buffer += "VALUES ('";
   sql_buffer += nextEntityId;
   sql_buffer += "',";
   sql_buffer += "now(), '',  '',  '',  '',  'com_poi', ";
   sql_buffer += "'"; 
   sql_buffer += poiName;
   sql_buffer += "',";
   sql_buffer += "'',  '',  '', '', '', '', '1', '', '', '', '', '', 0, 0, 0, 0, '16');";

   // Now execute this statement.
   PGresult* res = PQexec(db_connection, sql_buffer.c_str());

   addRelation(db_connection, imageID, "image", nextEntityId, "com_poi");
} 

// -------------------------------------------------------------------
void SKSlabel::addNoteComment (PGconn* db_connection, 
                               const char* imageID, 
                               const char* noteName, 
                               const char* noteText,
                               int poiId)
{
   char nextEntityId[kEntityIdBufSize];  
   memset(nextEntityId, 0, kEntityIdBufSize);
  
   // Get the next id. 
   getNextEntityId(db_connection, nextEntityId, kEntityIdBufSize);

   int next_id = getNextCommentId(db_connection, imageID, poiId, 
                                  comment_type_note);
 
   string sql_buffer = insert_comment_preamble;

   sql_buffer += sql_quote + string(nextEntityId) + sql_quote 
      + string(",");  // -- identifier 
   sql_buffer += "now(),";
   sql_buffer += "'',"; 	    //  --source_url 
   sql_buffer += "'',";	    //  --data_url
   sql_buffer += "'',";	    //  --metadata_url
   sql_buffer += "'',";      //  --mimetype 
   sql_buffer += "'com_note',";  // --comment_typ
   sql_buffer += sql_quote + string(noteName) + sql_quote 
      + string(",");  // --name 
   sql_buffer += sql_quote + string(noteText) + sql_quote 
      + string(",");
   sql_buffer += "'',"; 	    //  --labeltextx
   sql_buffer += "'',"; 	    //  --labeltexty
   sql_buffer += "'',"; 	    //  --labeltype 
   sql_buffer += "'',"; 	    //  --linecolor 	
   sql_buffer += "'',"; 	    //  --moviecip 

   char temp_buffer[64];
   sprintf(temp_buffer, "%d", next_id); 
   sql_buffer += sql_quote + temp_buffer + sql_quote + sql_comma; 

   sprintf(temp_buffer, "%d", poiId); 
   sql_buffer += sql_quote + temp_buffer + sql_quote + sql_comma; 
   // -- poi_id
   sql_buffer += "'',"; 	    //  --textbackgroundcolor
   sql_buffer += "'',"; 	    //  --textcolor
   sql_buffer += "'',"; 	    //  --url 	
   sql_buffer += "'',";      //  --username 
   sql_buffer += "0,";       //  --x 
   sql_buffer += "0,";       //  --y 
   sql_buffer += "0,";       //  --xscale
   sql_buffer += "0,";       //  --yscale 
   sql_buffer += "'16');";       // --zoom

   // Now execute this statement.
   PGresult* res = PQexec(db_connection, sql_buffer.c_str());

   addRelation(db_connection, imageID, "image", nextEntityId, 
               "com_note");
}

// -------------------------------------------------------------------
void SKSlabel::addLabelComment (PGconn* db_connection, 
                                const char* imageID, 
                                int poi_id, 
                                const char* labelName,
                                double pixelX,
                                double pixelY)
{
   // Get the next id
   // Add the label comment
   // Set up the relation in the e_relation table. 
   char nextEntityId[kEntityIdBufSize];  
   memset(nextEntityId, 0, kEntityIdBufSize);
  
   // Get the next id. 
   getNextEntityId(db_connection, nextEntityId, kEntityIdBufSize);

   int label_id = getNextCommentId(db_connection, 
                                   imageID,
                                   poi_id,
                                   comment_type_label);
			       
   string sql_buffer = insert_comment_preamble;

   sql_buffer += sql_quote + string(nextEntityId) + sql_quote 
      + string(",");  // -- identifier 
   sql_buffer += "now(),";
   sql_buffer += "'',"; 	    //  --source_url 
   sql_buffer += "'',";	    //  --data_url
   sql_buffer += "'',";	    //  --metadata_url
   sql_buffer += "'',";      //  --mimetype 
   sql_buffer += "'com_label',";  // --comment_typ
   sql_buffer += sql_quote + string(labelName) + sql_quote 
      + string(",");  // --name 
   sql_buffer += "'',";      //  --text
   sql_buffer += "'0',"; 	    //  --labeltextx
   sql_buffer += "'0',"; 	    //  --labeltexty
   sql_buffer += "'STATIC',"; 	    //  --labeltype 
   sql_buffer += "'0',"; 	    //  --linecolor 	
   sql_buffer += "'label%5FCircle%5Fmc',"; 	    //  --movieclip 


   char numberBuffer[64];
   sprintf(numberBuffer, "%d", label_id);
   sql_buffer += sql_quote + numberBuffer + sql_quote + sql_comma; 

   sprintf(numberBuffer, "%d", poi_id);

   sql_buffer += sql_quote + numberBuffer + sql_quote + sql_comma;     
   //  --poi_id
   sql_buffer += "'16777215',"; 	    //  --textbackgroundcolor
   sql_buffer += "'0',"; 	    //  --textcolor
   sql_buffer += "'',"; 	    //  --url 	
   sql_buffer += "'',";      //  --username 


   sprintf(numberBuffer, "%f,", pixelX);
   sql_buffer +=  numberBuffer;// "0,";       //  --x 

   sprintf(numberBuffer, "%f,", pixelY);

   sql_buffer += numberBuffer; // "0,";       //  --y 

// On 10/19/06, Ben empirically determined that adjusting the
// following x and y scale values for Zoomify comments yields circular
// red dots which are roughly 10 meters in size on our single
// Cambridge aerial image.  We'll live with this awful hack for now...

   sql_buffer += " 10, ";       //  --xscale
   sql_buffer += " 10, ";       //  --yscale /
   sql_buffer += "'16');";       // --zoom

   cout << sql_buffer << endl;

         // Now execute this statement.
   PGresult* res = PQexec(db_connection, sql_buffer.c_str());

   addRelation(db_connection, imageID, "image", nextEntityId, 
               "com_label");
}

// -------------------------------------------------------------------
string& SKSlabel::addZoomifyComment (PGconn* db_connection, 
                                     const char* imageEntityId, 
                                     const char* poiName,
                                     const char* noteName,
                                     const char* noteText,
                                     const char* labelName,
                                     double pixelX,
                                     double pixelY)
{
   int existing_poi_id = 0; 

   if (!imageHasWholeImagePOI(db_connection, 
                              imageEntityId, 
                              existing_poi_id))
   {

      // If we programmatically add a POI comment, it will
      // be ID zero. 
      addWholeImagePoiComment(db_connection, imageEntityId);
   }

   // TODO - Add the ability to add linked POI, label note
   // pairs.  This has to be done with the id field in the POI
   // comment and the poi_id in the note and label comments.
   // addPoiComment(db_connection, imageEntityId, poiName);
    
   addNoteComment(db_connection, imageEntityId, noteName, noteText, 
                  existing_poi_id);
    
   addLabelComment(db_connection, imageEntityId, existing_poi_id, 
                   labelName, pixelX, pixelY);

   return SKSlabel_ID;
}

// -------------------------------------------------------------------
//  QueryImagesContainingLatLon

//  Get the image and geometry of any images containing the specified
//  latitude and longitude.

//  Place the result in the vector of image ids and geometries. 

bool SKSlabel::QueryImagesContainingLatLon (
   double lat, double lon, vector<ImageGeometry>& ig)
{
   const char* query_template = 
      "select identifier,AsText(geometry),width,height from image where Contains(geometry, GeomFromText('POINT(%f %f)',4326));";

   char szQueryBuffer[256];
   memset(szQueryBuffer, 0, sizeof(szQueryBuffer));
   snprintf(szQueryBuffer, 256, query_template,  lon, lat);

   PGresult* res = PQexec(g_connection, szQueryBuffer);
    
   // There shouldn't be a NULL result unless something really dire happens.
   // libpq will only return NULL If we're out of memory.  Nevertheless,
   // we should indicate to the program that there was a failure. 
   if (!res)
      return false; 


   int nrows = PQntuples(res);
   int nfields = PQnfields(res);
        
   // There should be at least four columns: 
   // id, wkt geometry, pixel width, pixel height

   if (nfields < 2)
      return false; 

   ImageGeometry temp_ig;
   for (int i = 0; i < nrows; i++)
   {
      temp_ig.set_imageID(PQgetvalue(res, i, 0));
      temp_ig.set_wktGeometry(PQgetvalue(res, i, 1));
      temp_ig.set_pixelWidth(atoi(PQgetvalue(res, i, 2)));
      temp_ig.set_pixelHeight(atoi(PQgetvalue(res, i, 3)));
      ig.push_back(temp_ig);
   }
    
   return true;
}

// -------------------------------------------------------------------
// Ben Landon's original method for relating (lat,long) pairs to
// Zoomify pixel locations.  Testing indicated that this approach is
// inaccurate.

// Zoomify operates in normalized coordinates, so we don't have to
// worry about selecting an actual pixel location

void SKSlabel::LatLongToPixelLocation (float x, float y, 
                                       const char* wktGeometry, 
                                       double& pixelX, double& pixelY)
{
   // Set pixelX and pixelY to default values of 
   // 0, which to Zoomify means the center of the image.
   // We can leave them set as these defaults and 
   // return from this function if anything goes wrong.

   pixelX = 0;
   pixelY = 0; 
    
   assert(wktGeometry != NULL);
    
   // First, convert the wktGeometry to a GEOSGeometry object. 
   GEOSGeom wktGeom = GEOSGeomFromWKT(wktGeometry);

   int wktGeomId = GEOSGeomTypeId(wktGeom);

         // Get the boundary of the specified geometry. 
   GEOSGeom boundary = GEOSBoundary(wktGeom);

   int boundaryGeomId = GEOSGeomTypeId(boundary);

   GEOSCoordSeq boundary_coordSeq = GEOSGeom_getCoordSeq(boundary);

   GEOSGeom centroid = GEOSGetCentroid(wktGeom);
   int centroidId = GEOSGeomTypeId(centroid);
    
   GEOSCoordSeq centroid_coordSeq = GEOSGeom_getCoordSeq(centroid);

   double centroid_x = 0.0;
   double centroid_y = 0.0;

   int rc = 0;
   unsigned int num_vertices = 0;
    
   rc = GEOSCoordSeq_getSize(centroid_coordSeq, &num_vertices);
    
   if (num_vertices >= 1)
   {
      rc = GEOSCoordSeq_getX(centroid_coordSeq, 0, &centroid_x);
      rc = GEOSCoordSeq_getY(centroid_coordSeq, 0, &centroid_y);
   }
   else
   {
      // If the centroid doesn't have at least one point, then
      // there's not much we can do.

            // FIXME - Fix the failure mode here. 
      assert(false && "lacked enough points in centroid");
      return;
   }

   rc = GEOSCoordSeq_getSize(boundary_coordSeq, &num_vertices);

   if (num_vertices < 4) 
   {
      assert(false && "Not enough vertices in the polygon");
      return;
   }
    
   double v0x = 0.0;
   double v0y = 0.0;
   
   double v1x = 0.0;
   double v1y = 0.0;
    
   double v2x = 0.0;
   double v2y = 0.0;
    
   double v3x = 0.0;
   double v3y = 0.0;

   rc = GEOSCoordSeq_getX(boundary_coordSeq, 0, &v0x);
   rc = GEOSCoordSeq_getY(boundary_coordSeq, 0, &v0y);

   rc = GEOSCoordSeq_getX(boundary_coordSeq, 1, &v1x);
   rc = GEOSCoordSeq_getY(boundary_coordSeq, 1, &v1y);

   rc = GEOSCoordSeq_getX(boundary_coordSeq, 2, &v2x);
   rc = GEOSCoordSeq_getY(boundary_coordSeq, 2, &v2y);

   rc = GEOSCoordSeq_getX(boundary_coordSeq, 3, &v3x);
   rc = GEOSCoordSeq_getY(boundary_coordSeq, 3, &v3y);

   // Now compute the delta x and delta y from the given latitude
   // and longitude compared to the centroid. 
    
   double left_edge_midpoint_x = (v2x + v3x) / 2.0;
   double left_edge_midpoint_y = (v2y + v3y) / 2.0;
    
   double top_edge_midpoint_x = (v0x + v3x) / 2.0;
   double top_edge_midpoint_y = (v0y + v3y) / 2.0;

   double horz_vec_x = left_edge_midpoint_x - centroid_x; 
   double horz_vec_y = left_edge_midpoint_y - centroid_y;

   double vert_vec_x = top_edge_midpoint_x - centroid_x;
   double vert_vec_y = top_edge_midpoint_y - centroid_y;

   double point_of_interest_vec_x = x - centroid_x; 
   double point_of_interest_vec_y = y - centroid_y; 

   // Normalize these vectors so we can 
   // do the projections

   double horz_vec_length = sqrt((horz_vec_x * horz_vec_x) + 
                                 (horz_vec_y * horz_vec_y));

   double vert_vec_length = sqrt((vert_vec_x * vert_vec_x) + 
                                 (vert_vec_y * vert_vec_y));

   double point_of_interest_length = 
      sqrt((point_of_interest_vec_x * point_of_interest_vec_x) + 
           (point_of_interest_vec_y * point_of_interest_vec_y));

   // horz_vec_length is pathologically short.  We have to bail
   // out. 
   if (fabs(horz_vec_length) < 1e-5)
      return;

         // Similarly, if vert_vec_length is pathologically short,
         // we will have to return. 
   if (fabs(vert_vec_length) < 1e-5)
      return; 
    
   if (point_of_interest_length < 1e-5)
      return;

         // Normalization
   horz_vec_x /= horz_vec_length;
   horz_vec_y /= horz_vec_length;

   vert_vec_x /= vert_vec_length;
   vert_vec_y /= vert_vec_length;

   point_of_interest_vec_x /= point_of_interest_length;
   point_of_interest_vec_y /= point_of_interest_length;

         // Now take the dot product of the point_of_interest 
         // (the desired coordinates with) the two basis vectors.
    
   double lambda_horz = 
      point_of_interest_vec_x * horz_vec_x  + 
      point_of_interest_vec_y * horz_vec_y;

   double lambda_vert = 
      point_of_interest_vec_x * vert_vec_x + 
      point_of_interest_vec_y * vert_vec_y;

         // Now, finally, since Zoomify uses (0, 0) to mean the
         // center of the image, subtract the center of the image.
   lambda_horz *= 0.5;
   lambda_vert *= 0.5;

   pixelX = lambda_horz; 
   pixelY = lambda_vert;

         // We should not destroy the CoordSeq since this is owned
         // by the geometry object. 
   GEOSGeom_destroy(centroid);
   GEOSGeom_destroy(boundary);
}

// -------------------------------------------------------------------
// Peter Cho's overloaded version of LatLongToPixelLocation using a
// linear transformation approach to map (lat,long) pairs to Zoomify
// pixel locations.

// Note added on 10/16/06: For testing purposes only, we hardwire into
// this method the 2x2 Minverse matrix and twovector trans derived for
// one single special Cambridge aerial nadir photo.  We'll worry later
// about allowing Minverse and trans to be modified outside this
// method...

void SKSlabel::LatLongToPixelLocation (
   double longitude,double latitude,double& Zoomify_X,double& Zoomify_Y)
{
   genmatrix Minverse(2,2);
   twovector trans;

   Minverse.put(0,0,-0.0005641163309);
   Minverse.put(0,1,1.809481865e-05);
   Minverse.put(1,0,1.933797412e-05);
   Minverse.put(1,1,0.0006523343271);

   trans.put(0,328075.0313);
   trans.put(1,4692200.75);

// First convert input lat,long pair into UTM easting & northing
// coords:
         
   string UTM_zone;
   double northing,easting;
   latlongfunc::LLtoUTM(latitude,longitude,UTM_zone,northing,easting);

   twovector EN(easting,northing);
   twovector Z=Minverse*(EN-trans);
   Zoomify_X=Z.get(0);
   Zoomify_Y=Z.get(1);

   cout << "Zoomify coords = " << Z << endl;
}

// -------------------------------------------------------------------
//  LatLonToPixelCoordsForImage 

//  Given the latitude, longitude or a desired point, and given the
//  pixel width, pixel height, and lat/lon corners of an image,
//  compute the corresponding pixel locations for that lat/lon with
//  respect to the image.

/*
  IN lat - desired latitude

  IN lon - desired longitude 

  IN wktGeometry - string in WKT (well known text) geometry format that 
  describes the bounding polygon of the image in lat/lon coordinates. 

  IN pixelWidth - width of the image in pixels.
  
  IN pixelHeight - height of the image in pixels.

  OUT pixelX - x coordinate of the specified lat/lon with respect
  to that image.

  OUT pixelY - y coordinate of the specified lat/lon with respect to
  that image.
*/

bool SKSlabel::LatLonToPixelCoordsForImage (double lat, double lon, 
                                            double& pixelX, double& pixelY)
{
   LatLongToPixelLocation (lon,lat,pixelX,pixelY);
   return true;
}

bool SKSlabel::LatLonToPixelCoordsForImage (double lat, double lon, 
                                            const string& wktGeometry, 
                                            double& pixelX, double& pixelY)
{
   // For the purposes of this demonstration, the POLYGON format
   // is (upper_right, lower_right, lower_left, upper_left,
   // upper_right)
   LatLongToPixelLocation (lon, lat,
                           wktGeometry.c_str(), 
                           pixelX,
                           pixelY);
   return true;
}

// -------------------------------------------------------------------
bool SKSlabel::AddCommentAtPixelCoordinates (string& imageID, 
                                             string comment, 
                                             int pixelX, 
                                             int pixelY)
{
   addZoomifyComment(g_connection, 
                     imageID.c_str(),
                     comment.c_str(), // poi name
                     comment.c_str(), // note name
                     comment.c_str(), // note text - FIXME 
                     comment.c_str(), // label name
                     pixelX,
                     pixelY);
    
   // FIXME - Figure out some way of determining if addZoomifyComment succeeded or failed. 
   return true; 
}

// -------------------------------------------------------------------
void SKSlabel::GetGeometryCentroid (
   const string& wktGeometry, double& cx, double& cy)
{
   GEOSGeom wktGeom = GEOSGeomFromWKT(wktGeometry.c_str());
    
   GEOSGeom centroid = GEOS_DLL GEOSGetCentroid(wktGeom);

   GEOSCoordSeq centroid_coordSeq = GEOSGeom_getCoordSeq(centroid);

   double centroid_x = 0.0;
   double centroid_y = 0.0;

   int rc = 0;
   unsigned int num_vertices = 0;
    
   rc = GEOSCoordSeq_getSize(centroid_coordSeq, &num_vertices);
    
   if (num_vertices >= 1)
   {
      rc = GEOSCoordSeq_getX(centroid_coordSeq, 0, &centroid_x);
      rc = GEOSCoordSeq_getY(centroid_coordSeq, 0, &centroid_y);
   }
   else
   {
      // If the centroid doesn't have at least one point, then
      // there's not much we can do.

      // FIXME - Fix the failure mode here. 
      assert(false && "lacked enough points in centroid");
      return;
   }

   GEOSGeom_destroy(wktGeom);

   cx = centroid_x;
   cy = centroid_y;
}

void SKSlabel::Cleanup (void)
{
   PQfinish(g_connection);

   // Terminate GEOS
   finishGEOS();
}

// -------------------------------------------------------------------
//  TestResultsForImageId
  
//  Given a vector of ImageGeometry vectors look for the image id
//  specified by the imageID parameter.

//  Return true if that imageID is found, otherwise return false. 
  
bool SKSlabel::TestResultsForImageId (const vector<ImageGeometry>& ig, 
                                      const string& imageID)
{
   vector<ImageGeometry>::const_iterator iter = ig.begin();
   vector<ImageGeometry>::const_iterator end = ig.end();

   while (iter != end)
   {
      const ImageGeometry& imag_geom = *iter++;
      if (imag_geom.get_imageID() == imageID)
         return true; 
   }
    
   return false; 
}

// -------------------------------------------------------------------
bool SKSlabel::TestQueryForImageCentroids (void)
{
   vector<ImageGeometry> ig; 

   char szQueryBuffer[] = "select identifier,AsText(geometry),width,height from image where geometry is not null;";

   PGresult* res = PQexec(g_connection, szQueryBuffer);
    
   // There shouldn't be a NULL result unless something really dire happens.
   // libpq will only return NULL If we're out of memory.  Nevertheless,
   // we should indicate to the program that there was a failure. 

   if (!res)
      return false; 

   int nrows = PQntuples(res);
   int nfields = PQnfields(res);
        
   // There should be at least four columns: 
   // id, wkt geometry, pixel width, pixel height

   if (nfields < 2)
      return false; 

   ImageGeometry temp_ig;

   for (int i = 0; i < nrows; i++)
   {
      temp_ig.set_imageID(PQgetvalue(res, i, 0));
      temp_ig.set_wktGeometry(PQgetvalue(res, i, 1));
      temp_ig.set_pixelWidth(atoi(PQgetvalue(res, i, 2)));
      temp_ig.set_pixelHeight(atoi(PQgetvalue(res, i, 3)));
      ig.push_back(temp_ig);
   }
    
   // Now compute the image centroid for each image. 
   // We can do this from the geometry
    
   vector<ImageGeometry>::iterator iter = ig.begin();
   vector<ImageGeometry>::iterator end = ig.end();
   vector<ImageGeometry> test_results;

   printf("Found %d images with geometry\n", int(ig.size()));
    
   while (iter != end)
   {
      test_results.clear();

      // Test every image, it should contain it's own centroid as
      // far as PostGIS is concerned. 
      const ImageGeometry& imag_geom = *iter++;
      double cx = 0;
      double cy = 0;
      string wktGeometry = imag_geom.get_wktGeometry();
      GetGeometryCentroid(imag_geom.get_wktGeometry(), cx, cy);

      // Now query for the list of images that contain
      // the lat/long point (cx, cy) and make sure that they 
      // contain their own image identifier. 

      QueryImagesContainingLatLon(cx, cy, test_results);
	
      printf("This image's centroid is contained in %d images\n", int(test_results.size()));
	
      if (!TestResultsForImageId(test_results, imag_geom.get_imageID()))
      {
         printf("Warning - %s - did not contain it's centroid\n", 
                imag_geom.get_imageID().c_str());
      }
   }

   return true;
}

// -------------------------------------------------------------------
string& SKSlabel::AddSksAnnotation (double lat, 
                                    double lon,
                                    const char* poiName,
                                    const char* noteName,
                                    const char* noteText,
                                    const char* labelName)
{
   SKSlabel_ID="";

   if (!Setup())
   {
      printf("Database connection failed\n");
      printf("Exiting ...\n");
      return SKSlabel_ID;
   }
    
   vector<ImageGeometry> results; 
   results.clear();
   assert(results.size() == 0);

   QueryImagesContainingLatLon(lat, lon,results);
    
   printf("Found %d results\n", int(results.size()));
    
   vector<ImageGeometry>::iterator iter = results.begin();
   vector<ImageGeometry>::iterator end = results.end();
    
   while (iter != end)
   {
      ImageGeometry& ig = *iter++;
      printf("Found: %s\n", ig.get_imageID().c_str());
	
      double pixelX = 0.0;
      double pixelY = 0;

      LatLonToPixelCoordsForImage (lat, lon, pixelX, pixelY);

//            LatLonToPixelCoordsForImage (lat, lon, 
//                                         ig.get_wktGeometry(),
//                                         pixelX, 
//                                         pixelY);

      printf("Computed Zoomify coordinates x: %f y: %f\n",
             pixelX,
             pixelY);

      SKSlabel_ID=addZoomifyComment (g_connection, 
                                     ig.get_imageID().c_str(),
                                     poiName, 
                                     noteName,
                                     noteText,
                                     labelName, 
                                     pixelX,
                                     pixelY);
      
   }

   results.clear();
   Cleanup();
   return SKSlabel_ID;
}

