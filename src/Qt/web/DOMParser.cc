// ==========================================================================
// DOMPARSER class file
// ==========================================================================
// Last updated on 3/15/08; 3/16/08; 3/17/08
// ==========================================================================

#include <iostream>
#include <QtXml/QtXml>
#include "Qt/web/DOMParser.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void DOMParser::allocate_member_objects()
{
}		       

void DOMParser::initialize_member_objects()
{
}

DOMParser::DOMParser()
{
}

bool DOMParser::read_XML_file_into_DOM(string input_xml_filename)
{
   QFile file( input_xml_filename.c_str() );

   if ( file.open( QIODevice::ReadOnly ) ) 
   {
      QString error_msg;
      if (!doc.setContent(&file, &error_msg))
      {
         qDebug() << "Error reading " << input_xml_filename.c_str() 
                  << ": " << error_msg;
         return false;
      }
   }
   return true;
}

bool DOMParser::read_XML_string_into_DOM(string XML_string)
{
//   cout << "inside DOMParser::read_XML_string_into_DOM()" << endl;
//   cout << "XML_string = " << XML_string << endl;
   QString error_msg;
   if (!doc.setContent(QString(XML_string.c_str()), &error_msg))
   {
      qDebug() << "Error reading input XML string:" 
               << error_msg ;
      return false;
   }
   return true;
}

// ==========================================================================
// General XML parsing member functions
// ==========================================================================

// Member function find_elements recursively walks the DOM tree and
// appends any node's element whose tagname matches the input
// search_tagname to the output STL vector matching_elements.

vector<QDomElement> DOMParser::find_elements(const string search_tagname)
{
//   cout << "inside DOMParser::find_elements()" << endl;
//   cout << "search_tagname = " << search_tagname << endl;

   vector<QDomElement> matching_elements;

   QDomNodeList nodes=
      doc.documentElement().elementsByTagName(QString(
         search_tagname.c_str()));
   for (int i=0; i<nodes.length(); i++)
   {
      QDomNode curr_node=nodes.item(i);
      QDomElement curr_Element=curr_node.toElement();
      matching_elements.push_back(curr_Element);
   }
   
   return matching_elements;
}

// ---------------------------------------------------------------------
// Member function extract_text_fields_from_element takes in a
// QDomElement and returns an STL vector containing any text strings
// within its immediate children (i.e. <foo> bar </foo> would yield bar).

vector<string> DOMParser::extract_text_fields_from_element(
   QDomElement& curr_element)
{
//   cout << "inside DOMParser::extract_text_field_from_element()" << endl;

   vector<string> text_fields;
   for (QDomNode child = curr_element.firstChild(); !child.isNull(); 
        child = child.nextSibling() ) 
   {
      if (child.isText())
      {
         string curr_text_field=child.toText().data().toStdString();
//         cout << "curr_text_field = " << curr_text_field << endl;
         text_fields.push_back(curr_text_field);
      }
   } // loop over child QDomNodes

   return text_fields;
}

// ---------------------------------------------------------------------
// Boolean member function element_has_specified_attribute_key returns
// true if the input QDomElement has an attribute with the specified
// key.

bool DOMParser::element_has_specified_attribute_key(
   QDomElement& input_element,const string key)
{
//   cout << "inside DOMParser::element_has_specified_attribute_key()" << endl;
   QDomAttr attribute=input_element.attributeNode(QString(key.c_str()));
   return !(attribute.isNull());
}

// ---------------------------------------------------------------------
// Boolean member function
// element_has_specified_attribute_key_value_pair returns true if the
// input QDomElement has an attribute with the specified key & value.

bool DOMParser::element_has_specified_attribute_key_value_pair(
   QDomElement& input_element,const string key,const string value)
{
//   cout << "inside DOMParser::element_has_specified_attribute_key_value_pair()" << endl;

   QDomAttr attribute=input_element.attributeNode(QString(key.c_str()));
   if (attribute.isNull()) return false;

   if (value != attribute.value().toStdString())
   {
      return false;
   }
   
   return true;
}

// ---------------------------------------------------------------------
// Member function extract_attribute_from_element first checks whether
// the input QDomElement has an attribute labeled by a specified key.
// If so, this boolean method returns true as well as the string value
// corresponding to the input key.

bool DOMParser::extract_attribute_value_from_element(
   QDomElement& input_element,string key,string& value)
{
//   cout << "inside DOMParser::extract_attribute_value_from_element()" << endl;

   if (!element_has_specified_attribute_key(input_element,key))
   {
      return false;
   }

   QDomAttr attribute=input_element.attributeNode(QString(key.c_str()));
   value=attribute.value().toStdString();

//   cout << "key = " << key << " value = " << value << endl;
   return true;
}

// ---------------------------------------------------------------------
// Member function extract_attribute_values_from_children_elements
// takes in a parent QDomElement and iterates over all its children
// whose tagNames match the input parameter.  This method then
// searches for attributes within the matching children whose input
// key matches the input parameter.  It fills and returns the output
// STL vector values with the string values corresponding to the input
// key.

void DOMParser::extract_attribute_values_from_children_elements(
   QDomElement& parentElement,QString tagName,
   std::string key,std::vector<std::string>& values)
{
//   cout << "inside DOMParser::extract_attribute_values_from_children_elements()" << endl;

   QDomElement childElement=parentElement.firstChildElement(tagName);
   while(!childElement.isNull())
   {
      string curr_value;
      if (extract_attribute_value_from_element(
         childElement,key,curr_value))
      {
         values.push_back(curr_value);
//         cout << "curr_value = " << curr_value << endl;
      }
      childElement = childElement.nextSiblingElement(tagName);
   }
}
