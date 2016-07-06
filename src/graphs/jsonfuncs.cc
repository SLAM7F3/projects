// ==========================================================================
// Jsonfuncs namespace method definitions
// ==========================================================================
// Last modified on 10/31/13; 11/3/13; 11/4/13; 4/5/14
// ==========================================================================

#include <iostream>

#include "math/constants.h"
#include "general/filefuncs.h"
#include "postgres/gis_database.h"
#include "graphs/jsonfuncs.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

namespace jsonfunc
{

// ==========================================================================
// JSON file export methods
// ==========================================================================

// Method indent_spaces()

      string indent_spaces(unsigned int n_indent)
         {
            string value;
            for (unsigned int n=0; n<n_indent; n++)
            {
               value += " ";
            }
            return value;
         }

// ---------------------------------------------------------------------
      void write_json_file(string json_filename,const string& value)
         {
            ofstream outstream;
            filefunc::openfile(json_filename,outstream);
            outstream << value << endl;
            filefunc::closefile(json_filename,outstream);
         }

// ---------------------------------------------------------------------
// Method output_edge_GraphML() generates an output string
// source and target node indices along with edge weight and color
// information.  The output string is returned by this method.

      string output_edge_GraphML(
         int n_indent,int i,int j,double edge_weights,
         double r,double g,double b,double relative_thickness,
         bool terminal_edge_flag)
         {
//   cout << "inside jsonfunc::output_edge_GraphML()" << endl;

            string edge_value=indent_spaces(n_indent-2);
            edge_value += "{ \n";

            edge_value +=  output_GraphML_key_value_pair(
               n_indent,"source",stringfunc::number_to_string(i));
            edge_value +=  output_GraphML_key_value_pair(
               n_indent,"target",stringfunc::number_to_string(j));

            edge_value += output_data_GraphML(
               n_indent,edge_weights,r,g,b,relative_thickness);

            edge_value += indent_spaces(n_indent-2)+"}";
            if (!terminal_edge_flag) edge_value += ",";
            edge_value += "\n";

            return edge_value;
         }

// ---------------------------------------------------------------------
// Member function output_data_GraphML() exports the current graph's
// nodes content to an output string in JSON format.

      string output_data_GraphML(
         int n_indent,double edge_weights,double r,double g,double b,
         double relative_size)
         {
            string data_type="";
            int time_stamp=-1;
            string URL="";
            int Npx=-1;
            int Npy=-1;
            int parent_ID=-1;
            vector<int> children_IDs;
            string thumbnail_URL="";
            return output_data_GraphML(
               n_indent,data_type,time_stamp,
               URL,Npx,Npy,URL,Npx,Npy,
               edge_weights,parent_ID,children_IDs,
               NEGATIVEINFINITY,NEGATIVEINFINITY,r,g,b,relative_size);
         }

      string output_data_GraphML(
         int n_indent,string data_type,int time_stamp,
         string photo_URL,int photo_Npx,int photo_Npy,
         string thumbnail_URL,int thumbnail_Npx,int thumbnail_Npy,
         double edge_weights,int parent_ID,const vector<int>& children_IDs,
         double gx,double gy,double r,double g,double b,double relative_size)
         {
//   cout << "inside jsonfunc::output_data_GraphML()" << endl;
//   cout << "gx = " << gx << " gy = " << gy << endl;

            string data_value=indent_spaces(n_indent);

            data_value += "\"data\": {\n";
            if (data_type.size() > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"type",data_type);
            }

            if (time_stamp > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"time_stamp",
                  stringfunc::number_to_string(time_stamp));
            }

            if (photo_URL.size() > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"photoURL",photo_URL);
            }
            if (photo_Npx > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"photo_Npx",stringfunc::number_to_string(
                     photo_Npx));
            }
            if (photo_Npy > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"photo_Npy",stringfunc::number_to_string(
                     photo_Npy));
            }

            if (thumbnail_URL.size() > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"thumbnailURL",thumbnail_URL);
            }
            if (thumbnail_Npx > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"thumbnail_Npx",stringfunc::number_to_string(
                     thumbnail_Npx));
            }
            if (thumbnail_Npy > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"thumbnail_Npy",stringfunc::number_to_string(
                     thumbnail_Npy));
            }

            if (edge_weights > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"edge_weights",
                  stringfunc::number_to_string(edge_weights));
            }
            if (parent_ID >= 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"parent_ID",
                  stringfunc::number_to_string(parent_ID));
            }

            if (children_IDs.size() > 0)
            {
               string children_IDs_str;
               for (unsigned int c=0; c<children_IDs.size(); c++)
               {
                  int child_ID=children_IDs[c];
                  children_IDs_str += 
                     stringfunc::number_to_string(child_ID)+" ";
               }

               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"children_ID",children_IDs_str);
            }

            if (gx >= 0.5*NEGATIVEINFINITY)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"U",stringfunc::number_to_string(gx));
            }
            if (gy >= 0.5*NEGATIVEINFINITY)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"V",stringfunc::number_to_string(gy));
            }

            data_value += output_GraphML_key_value_pair(
               n_indent+2,"relativeSize",
               stringfunc::number_to_string(relative_size));
   
            if (r > -0.5 && g > -0.5 && b > -0.5)
            {
               string rgb_str = stringfunc::number_to_string(r)+" "+
                  stringfunc::number_to_string(g)+" "+
                  stringfunc::number_to_string(b);
//                  stringfunc::number_to_string(b)+"\" \n";
               bool terminal_comma_flag=false;
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"rgbColor",rgb_str,terminal_comma_flag);
            }
            data_value += indent_spaces(n_indent);
            data_value += "}\n";

            return data_value;
         }

// ---------------------------------------------------------------------
      string output_geolocation_GraphML(
         int n_indent,double longitude,double latitude)
         {
//   cout << "inside jsonfunc::output_geolocation_GraphML()" << endl;
//   cout << "gx = " << gx << " gy = " << gy << endl;

            string data_value=indent_spaces(n_indent);

            data_value += "\"data\": {\n";

            if (longitude > 0.5*NEGATIVEINFINITY)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"longitude",
                  stringfunc::number_to_string(longitude,7));
            }

            if (latitude > 0.5*NEGATIVEINFINITY)
            {
               bool terminal_comma_flag=false;
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"latitude",
                  stringfunc::number_to_string(latitude,7),
                  terminal_comma_flag);
            }

            data_value += indent_spaces(n_indent);
            data_value += "}\n";

            return data_value;
         }

// ---------------------------------------------------------------------
      string output_metadata_GraphML(
         int n_indent,int npx,int npy,
         string image_filename,string image_timestamp,
         double longitude,double latitude,double zposn,
         double azimuth,double elevation,double roll)
         {
//            cout << "inside jsonfunc::output_metadata_GraphML()" << endl;

            string data_value=indent_spaces(n_indent);

            data_value += "\"data\": {\n";

            if (npx > 0 && npy > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"npx",stringfunc::number_to_string(npx));
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"npy",stringfunc::number_to_string(npy));
            }
            if (image_filename.size() > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"image_filename",
                  filefunc::getbasename(image_filename));
            }
            if (image_timestamp.size() > 0)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"image_timestamp",image_timestamp);
            }

            if (longitude > 0.5*NEGATIVEINFINITY &&
                latitude > 0.5*NEGATIVEINFINITY)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"longitude",
                  stringfunc::number_to_string(longitude,7));
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"latitude",
                  stringfunc::number_to_string(latitude,7));
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"altitude",
                  stringfunc::number_to_string(zposn));
            }
            
            if (azimuth > 0.5*NEGATIVEINFINITY &&
                elevation > 0.5*NEGATIVEINFINITY &&
                roll > 0.5*NEGATIVEINFINITY)
            {
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"azimuth",
                  stringfunc::number_to_string(azimuth));
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"elevation",
                  stringfunc::number_to_string(elevation));
               bool terminal_comma_flag=false;
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"roll",stringfunc::number_to_string(roll),
                  terminal_comma_flag);
            }
            
            data_value += indent_spaces(n_indent);
            data_value += "}\n";

            return data_value;
         }

// ---------------------------------------------------------------------
      string output_neighbor_GraphML(int n_indent,vector<int>& neighbor_IDs)
         {
//            cout << "inside jsonfunc::output_neighbor_GraphML()" << endl;

            string data_value=indent_spaces(n_indent);
            data_value += "\"data\": {\n";

            string neighbor_values;
            for (unsigned int i=0; i<neighbor_IDs.size(); i++)
            {
               neighbor_values += stringfunc::number_to_string(
                  neighbor_IDs[i])+" ";
            }

            bool terminal_comma_flag=false;
            data_value += output_GraphML_key_value_pair(
               n_indent+2,"Neighbors",neighbor_values,terminal_comma_flag);

            data_value += indent_spaces(n_indent);
            data_value += "}\n";

            return data_value;
         }

/*
      string output_neighbor_GraphML(int n_indent,vector<int>& neighbor_IDs)
         {
//            cout << "inside jsonfunc::output_neighbor_GraphML()" << endl;

            string data_value=indent_spaces(n_indent);

            data_value += "\"data\": {\n";

            for (unsigned int i=0; i<neighbor_IDs.size(); i++)
            {
               bool terminal_comma_flag=true;
               if (i==neighbor_IDs.size()-1) terminal_comma_flag=false;
               data_value += output_GraphML_key_value_pair(
                  n_indent+2,"Neighbor_"+stringfunc::number_to_string(i),
                  stringfunc::number_to_string(neighbor_IDs[i]),
                  terminal_comma_flag);
            }
            data_value += indent_spaces(n_indent);
            data_value += "}\n";

            return data_value;
         }
*/

// ==========================================================================
// Key value pair output methods
// ==========================================================================

// Member function output_GraphML_key_value_pair() generates an
// indented line of text containing a key surrounded in double quotes
// separated by a colon from a value surrounded in double quotes.

      string output_GraphML_key_value_pair(
         int n_indent,string key,string value,bool terminal_comma_flag)
         {
//   cout << "inside jsonfunc::output_GraphML_key_value_pair()" << endl;

            string curr_line=indent_spaces(n_indent);
            curr_line += "\""+key+"\": \"";
            curr_line += value;
            if (terminal_comma_flag)
            {
               curr_line += "\", \n";
            }
            else
            {
               curr_line += "\" \n";
//      curr_line += "\n";   
            }

            return curr_line;
         }

// ---------------------------------------------------------------------
// Member function output_key_value_pair() generates an indented line
// of text containing a string key surrounded by double quotes
// separated by a colon from a numerical output value.

      string output_key_value_pair(
         int n_indent,string key,double value,bool terminal_comma_flag)
         {
//            cout << "inside jsonfunc::output_key_value_pair()" << endl;
//            cout << "key = " << key
//                 << " value = " << value << endl;

            string curr_line=indent_spaces(n_indent);
//            curr_line += key+": ";
            curr_line += "\""+key+"\": ";
            curr_line += stringfunc::number_to_string(value);
            if (terminal_comma_flag)
            {
               curr_line += ", ";
            }
            curr_line += "\n";   

            return curr_line;
         }

// ---------------------------------------------------------------------
      string output_key_value_pair(
         int n_indent,string key,string value,bool terminal_comma_flag)
         {
//   cout << "inside jsonfunc::output_key_value_pair()" << endl;

            string curr_line=indent_spaces(n_indent);
//            curr_line += key+": ";
            curr_line += "\""+key+"\": ";
            curr_line += "\""+value+"\"";
            if (terminal_comma_flag)
            {
               curr_line += ", ";
            }
            curr_line += "\n";   

            return curr_line;
         }

// ---------------------------------------------------------------------
      string output_key_value_pair(
         int n_indent,string key,const vector<int>& value,
         bool terminal_comma_flag)
         {
            vector<double> V;
            for (unsigned int i=0; i<value.size(); i++)
            {
               V.push_back(value[i]);
            }
            return output_key_value_pair(n_indent,key,V,terminal_comma_flag);
         }

// ---------------------------------------------------------------------
      string output_key_value_pair(
         int n_indent,string key,const vector<double>& value,
         bool terminal_comma_flag)
         {
//   cout << "inside jsonfunc::output_key_value_pair()" << endl;

            string curr_line=indent_spaces(n_indent);
//            curr_line += key+": [";
            curr_line += "\""+key+"\": [";

            for (unsigned int v=0; v<value.size(); v++)
            {
               curr_line += stringfunc::number_to_string(value[v]);
               if (v < value.size()-1) curr_line += ",";
            }
            curr_line += " ]";

            if (terminal_comma_flag)
            {
               curr_line += ", ";
            }
            curr_line += "\n";   

            return curr_line;
         }

// ==========================================================================
// Node neighbor export methods
// ==========================================================================

      void write_neighbors_JSON_file(
         int requested_node_ID,vector<int>& neighbor_IDs,
         string JSON_filename)
         {
//            cout << "inside videofunc::write_neighbors_JSON_file()" << endl;
            string value=generate_neighbor_JSON_string(
               requested_node_ID,neighbor_IDs);
            jsonfunc::write_json_file(JSON_filename,value);
         }
      
// --------------------------------------------------------------------------
// Method generate_neighbor_JSON_string() takes in some requested
// node ID along with node metadata extracted to several STL vectors
// via a gis database call.  It generates a JSON string containing
// the requested node's neighboring node IDs.

      string generate_neighbor_JSON_string(
         int requested_node_ID,vector<int>& neighbor_IDs)
         {
//            cout << "inside videofunc::generate_neighbor_JSON_string()" << endl;

            string value="\n";
            value="{ \n";
            value += "  \"Node_Neighbors\": { \n";

// Write out neighbor IDs for requested node:

            value += "     \"node\": [ \n";

            bool terminal_node_flag=true;

            value += output_neighbor_GraphML(
               9,requested_node_ID,neighbor_IDs,terminal_node_flag);
            
            value += "    ] \n";
            value += "  } \n";
            value += "} \n";

            return value;
         }

// ---------------------------------------------------------------------
// Member function output_neighbor_GraphML()

      string output_neighbor_GraphML(
         int n_indent,int node_ID,vector<int>& neighbor_IDs,
         bool terminal_node_flag)
         {
//            cout << "inside videofunc::output_neighbor_GraphML()" << endl;

            string node_value;
            if (node_ID < 0) return node_value;

            node_value=jsonfunc::indent_spaces(n_indent-2);
            node_value += "{ \n";

            node_value += jsonfunc::output_GraphML_key_value_pair(
               n_indent,"id",stringfunc::number_to_string(node_ID));

            node_value += jsonfunc::output_neighbor_GraphML(
               n_indent,neighbor_IDs);
            node_value += jsonfunc::indent_spaces(n_indent-2);
            node_value += "}";
            if (!terminal_node_flag) node_value += ",";
            node_value += "\n";

//            cout << "node_value = " << node_value << endl;
            return node_value;
         }

// ---------------------------------------------------------------------
// Method retrieve_node_neighbors_from_database() performs 2 postgres
// queries from the "link" table within the "data_network" database.
// The first is for target_IDs after setting the source_ID equal to
// the input node_ID.  The second is for source_IDs after setting the
// target_ID equal to the input node_ID.  This method returns all
// neighbor IDs within an output STL vector.

      vector<int> retrieve_node_neighbors_from_database(
         gis_database* gis_database_ptr,int node_ID)
         {
            vector<int> neighbor_ID;

            string curr_select_command = 
               "SELECT target_id from link where source_id=";
            curr_select_command += stringfunc::number_to_string(node_ID);
            Genarray<string>* field_array_ptr=gis_database_ptr->
               select_data(curr_select_command);

            for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
            {
               int curr_neighbor_ID=stringfunc::string_to_number(
                  field_array_ptr->get(i,0));
               neighbor_ID.push_back(curr_neighbor_ID);
            } // loop over index i labeling database rows

            curr_select_command = 
               "SELECT source_id from link where target_id=";
            curr_select_command += stringfunc::number_to_string(node_ID);
            field_array_ptr=gis_database_ptr->select_data(curr_select_command);

            for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
            {
               int curr_neighbor_ID=stringfunc::string_to_number(
                  field_array_ptr->get(i,0));
               neighbor_ID.push_back(curr_neighbor_ID);
            } // loop over index i labeling database rows

            return neighbor_ID;
         }

// ---------------------------------------------------------------------
      string write_data_json_string(
         int n_indent,double edge_weight,
         double r,double g,double b,double relative_size)
         {
//   cout << "inside jsonfunc::write_data_json_string()" << endl;

            string data_type="";
            string caption="";
            int time_stamp=-1;
            string URL="";
            int Npx=-1;
            int Npy=-1;
            string thumbnail_URL="";
            int connected_component_ID=-1;
            int parent_ID=-1;
            vector<int> children_IDs;
            double gx=NEGATIVEINFINITY;
            double gy=NEGATIVEINFINITY;

            return write_data_json_string(
               n_indent,data_type,time_stamp,caption,URL,Npx,Npy,
               thumbnail_URL,Npx,Npy,edge_weight,
               connected_component_ID,parent_ID,children_IDs,
               gx,gy,r,g,b,relative_size);
         }

      string write_data_json_string(
         int n_indent,string data_type,double epoch,string caption,
         string photo_URL,int photo_Npx,int photo_Npy,
         string thumbnail_URL,int thumbnail_Npx,int thumbnail_Npy,
         double edge_weight,int connected_component_ID,int parent_ID,
         const vector<int>& children_IDs,
         double gx,double gy,
         double r,double g,double b,double relative_size)
         {
//   cout << "inside jsonfunc::write_data_json_string()" << endl;
//   cout << "gx = " << gx << " gy = " << gy << endl;

            double gx2=NEGATIVEINFINITY;
            double gy2=NEGATIVEINFINITY;
            vector<string> attribute_keys,attribute_values;
            return write_data_json_string(
               n_indent,data_type,epoch,caption,
               photo_URL,photo_Npx,photo_Npy,
               thumbnail_URL,thumbnail_Npx,thumbnail_Npy,
               edge_weight,connected_component_ID,parent_ID,children_IDs,
               gx,gy,gx2,gy2,r,g,b,relative_size,
               attribute_keys,attribute_values);
         }
      
      string write_data_json_string(
         int n_indent,string data_type,double epoch,string caption,
         string photo_URL,int photo_Npx,int photo_Npy,
         string thumbnail_URL,int thumbnail_Npx,int thumbnail_Npy,
         double edge_weight,int connected_component_ID,int parent_ID,
         const vector<int>& children_IDs,
         double gx,double gy,double gx2,double gy2,
         double r,double g,double b,double relative_size,
         const vector<string>& attribute_keys,
         const vector<string>& attribute_values)
         {
//            cout << "inside jsonfunc::write_data_json_string()" << endl;
//            cout << "gx = " << gx << " gy = " << gy << endl;

            string json_string=indent_spaces(n_indent);

            json_string += "\"data\": {\n";
            if (data_type.size() > 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"type",data_type);
            }

            if (epoch > 0)
            {

// As of 2/16/2012, Michael Yee requests that epoch time values be
// reported as integers specifying millseconds since midnight 1 Jan 1970:
               
               string epoch_milliseconds=stringfunc::number_to_string(
                  1000*epoch,0);

//               cout.precision(12);
//               cout << "epoch = " << epoch << endl;
//               cout << "epoch_milliseconds = " << epoch_milliseconds
//                    << endl;
               
               json_string += output_key_value_pair(
                  n_indent+2,"epoch_millisecs",epoch_milliseconds);
            }
            if (caption.size() > 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"caption",caption);
            }

            if (photo_URL.size() > 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"photoURL",photo_URL);
            }
            if (photo_Npx > 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"photo_Npx",photo_Npx);
            }
            if (photo_Npy > 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"photo_Npy",photo_Npy);
            }

            if (thumbnail_URL.size() > 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"thumbnailURL",thumbnail_URL);
            }
            if (thumbnail_Npx > 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"thumbnail_Npx",thumbnail_Npx);
            }
            if (thumbnail_Npy > 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"thumbnail_Npy",thumbnail_Npy);
            }

            if (edge_weight > 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"edge_weight",edge_weight);
            }
            if (connected_component_ID >= 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"connected_component_ID",connected_component_ID);
            }
            if (parent_ID >= 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"parent_ID",parent_ID);
            }

            if (children_IDs.size() > 0)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"children_ID",children_IDs);
            }

            if (gx >= 0.5*NEGATIVEINFINITY)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"gx",gx);
            }
            if (gy >= 0.5*NEGATIVEINFINITY)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"gy",gy);
            }

            if (gx2 >= 0.5*NEGATIVEINFINITY)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"gx2",gx2);
            }
            if (gy2 >= 0.5*NEGATIVEINFINITY)
            {
               json_string += output_key_value_pair(
                  n_indent+2,"gy2",gy2);
            }

            json_string += output_key_value_pair(
               n_indent+2,"relativeSize",relative_size);

            for (unsigned int a=0; a<attribute_keys.size(); a++)
            {
               json_string += output_key_value_pair(
                  n_indent+2,attribute_keys[a],attribute_values[a]);
            } // loop over index a labeling attributes
 
            if (r > -0.5 && g > -0.5 && b > -0.5)
            {
               vector<double> rgb;
               rgb.push_back(r);
               rgb.push_back(g);
               rgb.push_back(b);
//               cout << "r = " << r << " g = " << g << " b = " << b << endl;
               bool terminal_comma_flag=false;
               json_string += output_key_value_pair(
                  n_indent+2,"rgbColor",rgb,terminal_comma_flag);
            }

            json_string += indent_spaces(n_indent);
            json_string += "}\n";

            return json_string;
         }

// ---------------------------------------------------------------------
// Method write_edge_json_string() generates an output string
// source and target node indices along with edge weight and color
// information.  The output string is returned by this method.

      string write_edge_json_string(
         int n_indent,int i,int j,double edge_weight,
         double r,double g,double b,double relative_thickness,
         bool terminal_edge_flag)
         {
//            cout << "inside jsonfunc::write_edge_json_string()" << endl;

            string edge_json_string=indent_spaces(n_indent-2);
            edge_json_string += "{ \n";

            edge_json_string +=  output_key_value_pair(
               n_indent,"source",i);
            edge_json_string +=  output_key_value_pair(
               n_indent,"target",j);

            edge_json_string += write_data_json_string(
               n_indent,edge_weight,r,g,b,relative_thickness);

            edge_json_string += indent_spaces(n_indent-2)+"}";
            if (!terminal_edge_flag) edge_json_string += ",";
            edge_json_string += "\n";

//            cout << "edge_json_string = " << edge_json_string << endl;

            return edge_json_string;
         }

// ==========================================================================
// Node attribute methods
// ==========================================================================

      typedef map<int,vector<int> > KEY_VALUE_IDS_MAP;

      string generate_condition_color_mapper_JSON_string(
         const vector<string>& all_attribute_keys,
         const vector<vector<string> >& all_attribute_values,
         KEY_VALUE_IDS_MAP& selected_key_value_IDs_map,
         colorfunc::RGB inColor_RGB,colorfunc::RGB outColor_RGB)
      {
//         cout << "inside jsonfunc::generate_condition_color_mapper_JSON_string()" << endl;

         string json_string="{ \n";
         if (selected_key_value_IDs_map.size() > 0)
         {
            json_string += "  'condition': { \n";

            unsigned int key_counter=0;
            for (KEY_VALUE_IDS_MAP::iterator iter=
                    selected_key_value_IDs_map.begin();
                 iter != selected_key_value_IDs_map.end(); ++iter)
            {
               int key_ID=iter->first;
               vector<int> value_IDs=iter->second;

               string selected_key=all_attribute_keys[key_ID];
               vector<string> attribute_values=all_attribute_values[key_ID];
                  
               vector<string> curr_attribute_values;
               for (unsigned int v=0; v<value_IDs.size(); v++)
               {
                  int curr_value_ID=value_IDs[v];
                  curr_attribute_values.push_back(
                     attribute_values[curr_value_ID]);
               }

               if (curr_attribute_values.size()==1)
               {
                  json_string += "    '"+selected_key+"': '"+
                     curr_attribute_values[0]+"'";
               }
               else
               {
                  json_string += "    '"+selected_key+"|=': [";
                  for (unsigned int v=0; v<curr_attribute_values.size(); v++)
                  {
                     json_string += "'"+curr_attribute_values[v]+"'";
                     if (v < curr_attribute_values.size()-1) 
                        json_string += ",";
                  } // loop over index v labeling current values
                  json_string += "]";
               }
               if (key_counter < selected_key_value_IDs_map.size()-1) 
                  json_string += ",";
               key_counter++;
               json_string += "\n";
            } // loop over selected_key_values_map iterator
       
            json_string += "  }, \n";
         } // selected_key_value_IDs_map.size() > 0 conditional

// As of 5/5/13, we believe that BOTH inColor and outColor must be
// specified in order for condition color mapping to work in Michael
// Yee's graph viewer.

         json_string += "  'inColor': ["+
            stringfunc::number_to_string(inColor_RGB.first)+","+
            stringfunc::number_to_string(inColor_RGB.second)+","+
            stringfunc::number_to_string(inColor_RGB.third)+"], \n";

         json_string += "  'outColor': ["+
            stringfunc::number_to_string(outColor_RGB.first)+","+
            stringfunc::number_to_string(outColor_RGB.second)+","+
            stringfunc::number_to_string(outColor_RGB.third)+"] \n";

         json_string += "} \n";

         return json_string;
      }

// ---------------------------------------------------------------------
// Method generate_dominant_colorings_JSON_string()

      string generate_dominant_colorings_JSON_string(
	 string primary_color,string secondary_color,string tertiary_color,
         colorfunc::RGB inColor_RGB,colorfunc::RGB outColor_RGB)
      {
//         cout << "inside jsonfunc::generate_dominant_colorings_JSON_string()" 
//              << endl;
//         cout << "primary_color = " << primary_color << endl;
//         cout << "secondary_color = " << secondary_color << endl;
//         cout << "tertiary_color = " << tertiary_color << endl;
         
         string json_string="{ \n";
         json_string += "  'condition': { \n";
         json_string += "      'primary_color': '"+primary_color;
         if (secondary_color != "any")
         {
            json_string += "', \n";
            json_string += "      'secondary_color': '"+secondary_color;
            if (tertiary_color != "any")
            {
               json_string += "', \n";
               json_string += "      'tertiary_color': '"+tertiary_color
                  +"' \n";
            }
            else
            {
               json_string += "' \n";
            }
         }
         else
         {
            json_string += "' \n";
         }
         
         json_string += "     }, \n";
         json_string += "  'inColor': ["+
            stringfunc::number_to_string(inColor_RGB.first)+","+
            stringfunc::number_to_string(inColor_RGB.second)+","+
            stringfunc::number_to_string(inColor_RGB.third)+"], \n";

         json_string += "  'outColor': ["+
            stringfunc::number_to_string(outColor_RGB.first)+","+
            stringfunc::number_to_string(outColor_RGB.second)+","+
            stringfunc::number_to_string(outColor_RGB.third)+"] \n";
         json_string += "} \n";

//         cout << "json_string = " << json_string << endl;
         return json_string;
      }

// ---------------------------------------------------------------------
// Method generate_multi_object_selection_JSON_string() produces a
// "SET_CONDITION_COLOR_MAPPER" JSON string which follows the
// schema specified by Michael Yee in March 2012.  It takes in STL
// vector n_objects which contains one or more number of objects
// to be marked in Michael's graph viewer.  This method uses the "|="
// syntax to indicate "one of " in order to mark nodes containing
// multiple different numbers of objects.

      string generate_multi_object_selection_JSON_string(
         const vector<int>& n_objects,string object_name,
         colorfunc::RGB inColor_RGB,colorfunc::RGB outColor_RGB)
      {
//         cout << "inside jsonfunc::generate_multi_object_selection_JSON_string()" 
//              << endl;

         string json_string="{ \n";
         json_string += "  'condition': { \n";
         json_string += "      '"+object_name+"|=': [";

         for (unsigned int i=0; i<n_objects.size(); i++)
         {
            json_string += "'"+stringfunc::number_to_string(n_objects[i])+"'";
            if (i < n_objects.size()-1) json_string += ",";
         }
         json_string += "] \n";
         
         json_string += "     }, \n";
         json_string += "  'inColor': ["+
            stringfunc::number_to_string(inColor_RGB.first)+","+
            stringfunc::number_to_string(inColor_RGB.second)+","+
            stringfunc::number_to_string(inColor_RGB.third)+"], \n";

         json_string += "  'outColor': ["+
            stringfunc::number_to_string(outColor_RGB.first)+","+
            stringfunc::number_to_string(outColor_RGB.second)+","+
            stringfunc::number_to_string(outColor_RGB.third)+"] \n";
         json_string += "} \n";

//         cout << "json_string = " << json_string << endl;
         return json_string;
      }

// ---------------------------------------------------------------------
// Method generate_discrete_color_mapper_JSON_string() produces a
// "SET_DISCRETE_COLOR_MAPPER" JSON string which follows the
// schema specified by Michael Yee in November 2013. 

      string generate_discrete_color_mapper_JSON_string(
         string attribute_str,colorfunc::RGB& defaultColor_RGB,
         vector<int>& values,vector<colorfunc::RGB>& value_RGBs)
      {
//         cout << "inside jsonfunc::generate_discrete_color_mapper_JSON_string()"
//              << endl;
//         cout << "values.size() = " << values.size() << endl;
//         cout << "value_RGBs.size() = " << value_RGBs.size() << endl;

         string json_string="{ \n";
         json_string += "  'attribute':  '"+attribute_str+"' ,\n";
         json_string += "  'mapping': [ \n";

         unsigned int n_values=values.size();
         for (unsigned int i=0; i<n_values; i++)
         {
            json_string += "      { 'value': "+stringfunc::number_to_string(
               values[i])+", 'color': [";
            json_string += stringfunc::number_to_string(
               value_RGBs[i].first)+", ";
            json_string += stringfunc::number_to_string(
               value_RGBs[i].second)+", ";
            json_string += stringfunc::number_to_string(
               value_RGBs[i].third)+"] }"; 
            if (i < n_values-1) json_string += ",";
            json_string += "\n";
         }
         json_string += "  ], \n";
         json_string += "  'defaultColor': ["+
            stringfunc::number_to_string(defaultColor_RGB.first)+","+
            stringfunc::number_to_string(defaultColor_RGB.second)+","+
            stringfunc::number_to_string(defaultColor_RGB.third)+"] \n";
         json_string += "} \n";

         cout << "json_string = " << json_string << endl;
//         cout << "json_string.size() = " << json_string.size() << endl;
         return json_string;
      }

} // jsonfunc namespace


