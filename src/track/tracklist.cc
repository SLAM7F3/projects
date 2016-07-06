// ==========================================================================
// TrackList class member function definitions
// ==========================================================================
// Last modified on 2/24/08
// ==========================================================================

#include "track/tracklist.h"

using std::list;

TrackList::TrackList()
{
   // blank for now
}
  
TrackList::~TrackList()
{
   // Manually delete each contained TrackItem before the list gets destroyed
   list<TrackItem *>::iterator i;
   for ( i = m_trackList.begin(); i != m_trackList.end(); ++i ) 
   {
      delete *i;
   }
}

// Retrieve the list of TrackItems
list<TrackItem *> &TrackList::getTrack() 
{ 
   return m_trackList;
}
  
// Add a TrackItem to the list
void TrackList::addItem( TrackItem *item ) 
{
   m_trackList.push_back( item );
}
