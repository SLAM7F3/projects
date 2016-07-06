// ==========================================================================
// Header file for NETLINK base class
// ==========================================================================
// Last modified on 4/1/04
// ==========================================================================

#ifndef NETLINK_H
#define NETLINK_H

class netlink
{

  public:

// Initialization, constructor and destructor functions:

   netlink();
   netlink(int ID);
   netlink(int ID,double s);
   netlink(const netlink& r);
   virtual ~netlink();
   netlink& operator= (const netlink& r);
   bool operator== (const netlink& r) const;

   friend std::ostream& operator<< 
      (std::ostream& outstream,const netlink& r);

// Set & get member functions:

   void set_ID(const int id);
   void set_score(const double s);
   int get_ID() const;
   double get_score() const;

  private:

   int neighbor_ID;
   double score;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const netlink& r);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void netlink::set_ID(const int id)
{
   neighbor_ID=id;
}

inline void netlink::set_score(const double s)
{
   score=s;
}

inline int netlink::get_ID() const
{
   return neighbor_ID;
}

inline double netlink::get_score() const
{
   return score;
}

// Two road links are regarded as equivalent provided their
// neighbor_ID fields are equal, independent of their score values:

inline bool netlink::operator== (const netlink& r) const
{
   return (neighbor_ID==r.neighbor_ID);
}

#endif // netlink.h



