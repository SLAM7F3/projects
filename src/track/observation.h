// ==========================================================================
// Header file for observatin class 
// ==========================================================================
// Last modified on 2/24/08; 6/26/08; 7/7/08
// ==========================================================================

#ifndef _OBSERVATION_H
#define _OBSERVATION_H

class Observation
{
  public:

   Observation();
   ~Observation();

// Set & get methods

   void set_time(long long t);
   long long get_time() const;

   void set_longitude(double l);
   double get_longitude() const;

   void set_latitude(double l);
   double get_latitude() const;

   void set_speed(double s);
   double get_speed() const;

   void set_azimuth(double a);
   double get_azimuth() const;

  private:

   long long time;
   double longitude,latitude,speed,azimuth;

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void Observation::set_time(long long t)
{
   time=t;
}

inline long long Observation::get_time() const
{
   return time;
}

inline void Observation::set_longitude(double l)
{
   longitude=l;
}

inline double Observation::get_longitude() const
{
   return longitude;
}

inline void Observation::set_latitude(double l)
{
   latitude=l;
}

inline double Observation::get_latitude() const
{
   return latitude;
}

inline void Observation::set_speed(double s)
{
   speed=s;
}

inline double Observation::get_speed() const
{
   return speed;
}

inline void Observation::set_azimuth(double a)
{
   azimuth=a;
}

inline double Observation::get_azimuth() const
{
   return azimuth;
}

#endif
