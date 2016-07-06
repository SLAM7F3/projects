// ==========================================================================
// Header file for TrackList class
// ==========================================================================
// Last modified on 2/24/08
// ==========================================================================

#ifndef _TRACKLIST_H
#define _TRACKLIST_H

#include <list>
#include <memory>
#include <string>
#include "track/trackitem.h"

typedef std::list<TrackItem *> TrackItems;
typedef std::list<TrackItem *>::iterator TrackItemsIterator;

class TrackList
{
  public:
   TrackList();
   ~TrackList();

   // Retrieve the list of TrackItems
   std::list<TrackItem *> &getTrack();
  
   // Add a TrackItem to the list
   void addItem( TrackItem *item );

  private:
   std::list<TrackItem *> m_trackList;
};

#endif
