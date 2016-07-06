// ==========================================================================
// Header file for purely virtual PICKHANDLER class
// ==========================================================================
// Last modified on 9/12/08; 12/26/10; 2/9/11
// ==========================================================================

#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <osgGA/GUIEventHandler>

class ModeController;

class PickHandler : public osgGA::GUIEventHandler 
{
  public: 

   PickHandler(const int p_ndims,ModeController* MC_ptr);

// Set & get member functions:

   void set_enable_pick_flag(bool flag);
   bool get_enable_pick_flag() const;
   void set_enable_drag_flag(bool flag);
   bool get_enable_drag_flag() const;
   void set_rotation_mode(bool flag);
   void set_scaling_mode(bool flag);
   void set_min_doubleclick_time_spread(double dt);
   void set_max_doubleclick_time_spread(double dt);
   double get_dX() const;
   double get_dY() const;

   bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

  protected:

   bool rotation_mode,scaling_mode;

   virtual ~PickHandler()=0;
   int get_ndims() const;

   virtual bool pick(const osgGA::GUIEventAdapter& ea)=0;
   virtual bool pick_box(
      float oldX, float oldY, const osgGA::GUIEventAdapter& ea)=0;
   virtual bool drag(const osgGA::GUIEventAdapter& ea)=0;
   virtual bool drag_box(
      float oldX,float oldY,const osgGA::GUIEventAdapter& ea)=0;
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea)=0;
   virtual bool release()=0;

   virtual bool scale(const osgGA::GUIEventAdapter& ea)=0;
   virtual bool scale(float oldX,float oldY,
                      const osgGA::GUIEventAdapter& ea)=0;
   virtual bool toggle_scaling_mode()=0;
   virtual bool toggle_rotate_mode()=0;
//   virtual bool rotate(const osgGA::GUIEventAdapter& ea)=0;
   virtual bool rotate(float oldX,float oldY,
   		       const osgGA::GUIEventAdapter& ea)=0;

  private:

   const int ndims;
   bool enable_pick_flag,enable_drag_flag;
   double t_curr_click,t_prev_click;
   double min_doubleclick_time_spread,max_doubleclick_time_spread;
   double curr_X,curr_Y,prev_X,prev_Y,dX,dY,max_delta;
   ModeController* MC_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void PickHandler::set_enable_pick_flag(bool flag)
{
   enable_pick_flag=flag;
}

inline bool PickHandler::get_enable_pick_flag() const
{
   return enable_pick_flag;
}

inline void PickHandler::set_enable_drag_flag(bool flag)
{
   enable_drag_flag=flag;
}

inline bool PickHandler::get_enable_drag_flag() const
{
   return enable_drag_flag;
}

inline void PickHandler::set_rotation_mode(bool flag)
{
   rotation_mode=flag;
}

inline void PickHandler::set_scaling_mode(bool flag)
{
   scaling_mode=flag;
}

inline int PickHandler::get_ndims() const
{
   return ndims;
}

inline void PickHandler::set_min_doubleclick_time_spread(double dt)
{
   min_doubleclick_time_spread=dt;
}

inline void PickHandler::set_max_doubleclick_time_spread(double dt)
{
   max_doubleclick_time_spread=dt;
}

inline double PickHandler::get_dX() const
{
   return dX;
}

inline double PickHandler::get_dY() const
{
   return dY;
}

#endif // Pickhandler.h



