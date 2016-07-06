// ==========================================================================
// Header file for SKSlabel class
// ==========================================================================
// Last updated on 10/19/06
// ==========================================================================

#ifndef __SKS_LABEL_H__
#define __SKS_LABEL_H__

#include <geos_c.h>
#include "libpq-fe.h"
#include <iostream>
#include <string>
#include <vector>
#include "math/genmatrix.h"
#include "isds/ImageGeometry.h"
#include "math/twovector.h"

class SKSlabel 
{

  public:

   enum CommentType
   {
      comment_type_poi,comment_type_label,comment_type_note
   };

// Initialization, constructor and destructor functions:

   SKSlabel();
   virtual ~SKSlabel();
   friend std::ostream& operator<< (
      std::ostream& outstream,const SKSlabel& s);

   std::string& AddSksAnnotation (double lat, 
                                  double lon,
                                  const char* poiName,
                                  const char* noteName,
                                  const char* noteText,
                                  const char* labelName);

  private:

   std::string insert_comment_preamble,insert_relation_preamble;
   std::string sql_quote,sql_comma;
   std::string SKSlabel_ID;

   int kEntityIdBufSize;

   void allocate_member_objects();
   void initialize_member_objects();
   void initialize_string_constants();

   void initEntityIdSequence (
      PGconn* db_connection, char* result, int bufsize);
//   void noticeMessageHandler (const char* fmt, ...);
//   void errorMessageHandler (const char* fmt, ...);
   bool Setup (void);
   void getNextEntityId (
      PGconn* db_connection, char* nextEntityId, int bufsize);
   void addRelation (PGconn* db_connection, 
                     const char* srcEntityId, 
                     const char* srcEntityType, 
                     const char* tgtEntityId,
                     const char* tgtEntityType);
   int getNextCommentId (PGconn* db_connection, 
                         const char* imageId, 
                         int poi_id,
                         int comment_type);
   bool imageHasWholeImagePOI (
      PGconn* db_connection, const char* imageId, int& id);
   void addWholeImagePoiComment (
      PGconn* db_connection, const char* imageId);
   void addPoiComment (PGconn* db_connection, 
                       const char* imageId, 
                       const char* poiName);
   void addNoteComment (PGconn* db_connection, 
                        const char* imageId, 
                        const char* noteName, 
                        const char* noteText,
                        int poiId);
   void addLabelComment (PGconn* db_connection, 
                         const char* imageId, 
                         int poi_id, 
                         const char* labelName,
                         double pixelX,
                         double pixelY);
   std::string& addZoomifyComment (PGconn* db_connection, 
                                   const char* imageEntityId, 
                                   const char* poiName,
                                   const char* noteName,
                                   const char* noteText,
                                   const char* labelName,
                                   double pixelX,
                                   double pixelY);
   bool QueryImagesContainingLatLon (
      double lat, double lon, std::vector<ImageGeometry>& ig);
   void LatLongToPixelLocation (float x, float y, 
                                const char* wktGeometry, 
                                double& pixelX, double& pixelY);
   void LatLongToPixelLocation (
      double longitude,double latitude,double& Zoomify_X,double& Zoomify_Y);
   bool LatLonToPixelCoordsForImage (double lat, double lon, 
                                     double& pixelX, double& pixelY);
   bool LatLonToPixelCoordsForImage (double lat, double lon, 
                                     const std::string& wktGeometry, 
                                     double& pixelX, double& pixelY);
   bool AddCommentAtPixelCoordinates (std::string& imageId, 
                                      std::string comment, 
                                      int pixelX, 
                                      int pixelY);
   void GetGeometryCentroid (
      const std::string& wktGeometry, double& cx, double& cy);
   void Cleanup (void);
   
  
   bool TestResultsForImageId (const std::vector<ImageGeometry>& ig, 
                               const std::string& imageId);
   bool TestQueryForImageCentroids (void);
   
};

#endif // SKSlabel.h
