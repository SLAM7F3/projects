
/*
 * MyApplicationUsage is a subclass of the built-in OSG
 * ApplicationUsage class which is what controls all the help
 * text the user can modify.  This class adds functionality for
 * modes into the existing help framework.
 */

#ifndef MYAPPLICATIONUSAGE_H
#define MYAPPLICATIONUSAGE_H

#include <iostream>
#include <osg/ApplicationUsage>
#include <map>
#include <string>
#include <osg/Export>

namespace osg {

    class OSG_EXPORT MyApplicationUsage : public osg::ApplicationUsage
    {
        public:
        
            //may need constructors
            MyApplicationUsage();
            typedef std::map<std::string,int> KeyToModeMap;

            //very similar to addKeyboardMouseBinding but allows you to add a
            //mode for that key as well
            void add_key_binding_with_mode(
               std::string key, std::string explanation, int mode);

            //get the key to mode bindinds
            KeyToModeMap get_key_to_mode_bindings();

            //given a particular key, return what mode that key is for
            //-1 signals that the key is usable in all modes
            //-2 signals that key is not being used (or wasn't registered)
            int get_mode_for_key(std::string key);
            
        protected:
        
            KeyToModeMap    _keyToModeBindings;
        
    };
}

#endif
