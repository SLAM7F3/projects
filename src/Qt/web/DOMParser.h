// ========================================================================
// DOMPARSER header file
// ========================================================================
// Last updated on 3/15/08; 3/16/08; 3/17/08; 4/28/08
// ========================================================================

#ifndef DOMPARSER_H
#define DOMPARSER_H

#include <string>
#include <vector>
#include <QDomElement>
#include "math/threevector.h"

class DOMParser
{

  public:

   DOMParser();
   bool read_XML_file_into_DOM(std::string input_xml_filename);
   bool read_XML_string_into_DOM(std::string input_xml_string);

// Set & get member functions:

   QDomDocument& get_doc();
   const QDomDocument& get_doc() const;

// General XML parsing member functions:

   std::vector<QDomElement> find_elements(const std::string search_tagname);
   std::vector<std::string> extract_text_fields_from_element(
      QDomElement& curr_element);

   bool element_has_specified_attribute_key(
      QDomElement& input_element,const std::string key);
   bool element_has_specified_attribute_key_value_pair(
      QDomElement& input_element,const std::string key,
      const std::string value);
   bool extract_attribute_value_from_element(
      QDomElement& input_element,std::string key,std::string& value);
   void extract_attribute_values_from_children_elements(
      QDomElement& parent_element,QString tagName,
      std::string key,std::vector<std::string>& values);

  private:

   QDomDocument doc;

   void allocate_member_objects();
   void initialize_member_objects();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline QDomDocument& DOMParser::get_doc()
{
   return doc;
}

inline const QDomDocument& DOMParser::get_doc() const
{
   return doc;
}

#endif // DOMPARSER_H

