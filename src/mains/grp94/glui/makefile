

#TARGETS=libglui.a
#TARGET_HEADERS=glui.h

TARGET_LIB=libglui.so
TARGET_HEADERS=glui.h

INSTALL_BASE_DIR=/usr/local
INSTALL_LIB_DIR = $(INSTALL_BASE_DIR)/lib
INSTALL_INC_DIR = $(INSTALL_BASE_DIR)/include



default: $(TARGET_LIB)

include makefile.rules





############################################################################


#default: install


GLUI_OBJS = glui_add_controls.o glui.o glui_bitmap_img_data.o glui_bitmaps.o glui_button.o glui_edittext.o glui_checkbox.o glui_node.o glui_radio.o glui_statictext.o glui_panel.o glui_separator.o glui_spinner.o glui_control.o glui_column.o glui_translation.o glui_rotation.o glui_mouse_iaction.o glui_listbox.o glui_rollout.o arcball.o algebra3.o quaternion.o

libglui.a: $(GLUI_OBJS)
	@echo "Creating library"
	$(LINK_LIBRARY)

libglui.so: $(GLUI_OBJS)
	@echo "Creating so"
	$(LINK_CXX_SO)


install: $(TARGET_LIB)
	$(CP) $(TARGET_LIB) $(INSTALL_LIB_DIR)
	$(CP) $(TARGET_HEADERS) $(INSTALL_INC_DIR)

uninstall: $(TARGET_LIB)
	$(RM) $(INSTALL_LIB_DIR)/$(TARGET_LIB) 
	$(RM) $(INSTALL_INC_DIR)/$(TARGET_HEADERS) 


algebra3.o: algebra3.h
arcball.o: arcball.h algebra3.h quaternion.h stdinc.h
example1.o: glui.h
example2.o:   glui.h
example3.o:   glui.h
example4.o:   glui.h
glui.o: glui.h  stdinc.h 
glui_add_controls.o: glui.h  stdinc.h 
glui_bitmap_img_data.o: glui_img_checkbox_0.c glui_img_checkbox_1.c
glui_bitmap_img_data.o: glui_img_radiobutton_0.c glui_img_radiobutton_1.c
glui_bitmap_img_data.o: glui_img_uparrow.c glui_img_downarrow.c
glui_bitmap_img_data.o: glui_img_leftarrow.c glui_img_rightarrow.c
glui_bitmap_img_data.o: glui_img_spinup_1.c glui_img_spinup_0.c
glui_bitmap_img_data.o: glui_img_spindown_1.c glui_img_spindown_0.c
glui_bitmap_img_data.o: glui_img_checkbox_0_dis.c glui_img_checkbox_1_dis.c
glui_bitmap_img_data.o: glui_img_radiobutton_0_dis.c
glui_bitmap_img_data.o: glui_img_radiobutton_1_dis.c glui_img_spinup_dis.c
glui_bitmap_img_data.o: glui_img_spindown_dis.c
glui_bitmaps.o: glui.h stdinc.h 
glui_button.o: glui.h stdinc.h 
glui_checkbox.o: glui.h stdinc.h 
glui_column.o: glui.h stdinc.h 
glui_control.o: glui.h stdinc.h 
glui_edittext.o: glui.h stdinc.h 
glui_node.o: glui.h stdinc.h 
glui_panel.o: glui.h stdinc.h 
glui_radio.o: glui.h stdinc.h 
glui_separator.o: glui.h stdinc.h 
glui_spinner.o: glui.h stdinc.h 
glui_statictext.o: glui.h stdinc.h 
quaternion.o: quaternion.h algebra3.h stdinc.h
glui_translation.o: glui.h stdinc.h  
glui_rotation.o: glui.h stdinc.h  
glui_mouse_iaction.o: glui.h stdinc.h  
glui_listbox.o: glui.h stdinc.h  
glui_rollout.o: glui.h stdinc.h 

