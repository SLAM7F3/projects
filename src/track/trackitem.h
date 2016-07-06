// ==========================================================================
// Header file for TrackItem class
// ==========================================================================
// Last modified on 2/24/08; 4/29/08
// ==========================================================================

#ifndef _TRACKITEM_H
#define _TRACKITEM_H

#include <list>
#include <string>
#include "track/observation.h"

class TrackItem
{
  public:

   // Create a new TrackItem populated with info

   TrackItem(std::string entityId, std::string entityType, std::string label,
             std::list<Observation *> observationList);
   ~TrackItem();

// Set & get member functions:

   std::string get_entityID() const;
   std::string get_entityType() const;
   std::string get_label() const;
   std::list<Observation*>& get_observationList();
   const std::list<Observation*>& get_observationList() const;

   std::string entityId;
   std::string entityType;
   std::string label;
   std::list<Observation*> observationList;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline std::string TrackItem::get_entityID() const
{
   return entityId;
}

inline std::string TrackItem::get_entityType() const
{
   return entityType;
}

inline std::string TrackItem::get_label() const
{
   return label;
}

inline std::list<Observation*>& TrackItem::get_observationList() 
{
   return observationList;
}

inline const std::list<Observation*>& TrackItem::get_observationList() const
{
   return observationList;
}

#endif
