// ==========================================================================
// TrackItem class member function definitions
// ==========================================================================
// Last modified on 2/24/08
// ==========================================================================

#include "track/trackitem.h"

using std::list;
using std::string;

TrackItem::TrackItem(string entityId, string entityType, 
                     string label, list<Observation *> observationList)
{
   this->entityId = entityId;
   this->entityType = entityType;
   this->label = label;
   this->observationList = observationList;
}

TrackItem::~TrackItem()
{
   // Delete the ptrs in observationList before it gets destroyed
   list<Observation *>::iterator i;
   for( i = observationList.begin(); i != observationList.end(); ++i ) {
      delete *i;
   }
}
