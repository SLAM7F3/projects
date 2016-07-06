// ========================================================================
// ========================================================================
// Last updated on 8/23/05
// ========================================================================

#include <string>
#include "osg/MyApplicationUsage.h"

using namespace osg;
using std::string;

MyApplicationUsage::MyApplicationUsage()
{
}

// Add the mode binding to the map, and then call the super function
// to add the keyboard and mouse binding as usual

void MyApplicationUsage::add_key_binding_with_mode(
   string key, string explanation, int mode)
{
    _keyToModeBindings[key] = mode;
    addKeyboardMouseBinding(key, explanation);
}

std::map<string,int> MyApplicationUsage::get_key_to_mode_bindings()
{
    return _keyToModeBindings;
}

// Search the map for the key and return its hashed value, return -1
// for all modes

int MyApplicationUsage::get_mode_for_key(string key)
{
    //some keys are not registered using the key_to_mode binding function
    //found in MyApplicationUsage, these are keys that are native to OSG,
    //so we should just add them to all modes, thus return -1

    std::map<string, int>::iterator keyit = _keyToModeBindings.find(key);  
    if (keyit != _keyToModeBindings.end())
    {
        return keyit->second;
    }
    else
        // -1 indicates show key in all help modes
       return -1;
}
