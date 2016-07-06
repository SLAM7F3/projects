// ==========================================================================
// Header file for pixel_cc class
// ==========================================================================
// Last modified on 6/20/16; 6/21/16; 6/22/16; 6/24/16
// ==========================================================================

#ifndef PIXEL_CC_H
#define PIXEL_CC_H

#include "geometry/bounding_box.h"

class pixel_cc
{

  public:

   typedef std::pair<int,int> PIXEL_COORDINATES;

   pixel_cc(int ID, int class_ID);
   pixel_cc(const pixel_cc& cc);
   ~pixel_cc();
   pixel_cc& operator= (const pixel_cc& cc);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const pixel_cc& cc);

// Set and get member functions:

   void set_rejected_flag(bool flag);
   bool get_rejected_flag() const;
   int get_ID() const;
   int get_class_ID() const;
   void set_imagesize(int imagesize);
   int get_imagesize() const;
   void set_score(double score);
   double get_score() const;
   void set_score_threshold(double threshold);
   double get_score_threshold() const;
   void set_quadrant_scores(double tl, double tr, double bl, double br);
   void get_quadrant_scores(double& tl, double& tr, double& bl, double& br);
   void set_bbox(int px_min, int px_max, int py_min, int py_max);
   void set_bbox(const bounding_box& b);
   bounding_box& get_bbox();
   const bounding_box& get_bbox() const;
   void set_center(int px, int py);
   void get_center(int& px, int& py);
   void set_pixel_coords(int px, int py);
   std::vector<PIXEL_COORDINATES>* get_pixel_coords_ptr();
   const std::vector<PIXEL_COORDINATES>* get_pixel_coords_ptr() const;

  private: 

   bool rejected_flag;
   int ID;
   int class_ID;
   int imagesize;  // 0 --> doublesize, 1 --> fullsize,  2 --> halfsize
   double score;
   double score_threshold;
   double tl_score, tr_score, bl_score, br_score;
   bounding_box bbox;
   int px_center, py_center;
   std::vector<PIXEL_COORDINATES> pixel_coords;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const pixel_cc& im);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void pixel_cc::set_rejected_flag(bool flag)
{
   rejected_flag = flag;
}

inline bool pixel_cc::get_rejected_flag() const
{
   return rejected_flag;
}

inline int pixel_cc::get_ID() const
{
   return ID;
}

inline int pixel_cc::get_class_ID() const
{
   return class_ID;
}

inline void pixel_cc::set_imagesize(int imagesize)
{
   this->imagesize = imagesize;
}

inline int pixel_cc::get_imagesize() const
{
   return imagesize;
}

inline void pixel_cc::set_score(double score)
{
   this->score = score;
}

inline double pixel_cc::get_score() const
{
   return score;
}

inline void pixel_cc::set_quadrant_scores(
   double tl, double tr, double bl, double br)
{
   tl_score = tl; // Top left quadrant
   tr_score = tr; // Top right quadrant
   bl_score = bl; // Bottom left quadrant
   br_score = br; // Bottom right quadrant
   
}

inline void pixel_cc::get_quadrant_scores(
   double& tl, double& tr, double& bl, double& br)
{
   tl = tl_score;
   tr = tr_score;
   bl = bl_score;
   br = br_score;
}

inline void pixel_cc::set_score_threshold(double threshold)
{
   score_threshold = threshold;
}

inline double pixel_cc::get_score_threshold() const
{
   return score_threshold;
}

inline void pixel_cc::set_center(int px, int py)
{
   px_center = px;
   py_center = py;
}

inline void pixel_cc::get_center(int& px, int& py)
{
   px = px_center;
   py = py_center;
}

inline void pixel_cc::set_bbox(int px_min, int px_max, int py_min, int py_max)
{
   bbox = bounding_box(px_min, px_max, py_min, py_max);
}

inline void pixel_cc::set_bbox(const bounding_box& b)
{
   bbox = b;
}

inline bounding_box& pixel_cc::get_bbox()
{
   return bbox;
}

inline const bounding_box& pixel_cc::get_bbox() const
{
   return bbox;
}

inline void pixel_cc::set_pixel_coords(int px, int py)
{
   pixel_cc::PIXEL_COORDINATES curr_pixel;
   curr_pixel.first = px;
   curr_pixel.second = py;
   pixel_coords.push_back(curr_pixel);
}

inline std::vector<pixel_cc::PIXEL_COORDINATES>* 
pixel_cc::get_pixel_coords_ptr()
{
   return &pixel_coords;
}

inline const std::vector<pixel_cc::PIXEL_COORDINATES>* 
pixel_cc::get_pixel_coords_ptr() const
{
   return &pixel_coords;
}


#endif  // pixel_cc.h
