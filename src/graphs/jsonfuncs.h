// ==========================================================================
// Header file for jsonfunc namespace
// ==========================================================================
// Last modified on 4/19/12; 5/16/12; 11/3/13; 4/5/14
// ==========================================================================

#ifndef JSONFUNCS_H
#define JSONFUNCS_H

#include <map>
#include <string>
#include <vector>
#include "color/colorfuncs.h"

class gis_database;

namespace jsonfunc
{

// JSON file export methods

    std::string indent_spaces(unsigned int n_indent);
    void write_json_file(std::string json_filename,const std::string& value);

    std::string output_edge_GraphML(
       int n_indent,int i,int j,double edge_weights,
       double r,double g,double b,double relative_thickness,
       bool terminal_edge_flag);

    std::string output_data_GraphML(
       int n_indent,double edge_weights,double r,double g,double b,
       double relative_size);
    std::string output_data_GraphML(
       int n_indent,std::string data_type,int time_stamp,
       std::string photo_URL,int photo_Npx,int photo_Npy,
       std::string thumbnail_URL,int thumbnail_Npx,int thumbnail_Npy,
       double edge_weights,int parent_ID,
       const std::vector<int>& children_IDs,
       double gx,double gy,double r,double g,double b,double relative_size);

    std::string output_geolocation_GraphML(
       int n_indent,double longitude,double latitude);
    std::string output_metadata_GraphML(
       int n_indent,int npx,int npy,
       std::string image_filename,std::string image_timestamp,
       double longitude,double latitude,double zposn,
       double azimuth,double elevation,double roll);
    std::string output_neighbor_GraphML(
       int n_indent,std::vector<int>& neighbor_IDs);

// Key value pair output methods:

    std::string output_GraphML_key_value_pair(
       int n_indent,std::string key,std::string value,
       bool terminal_comma_flag=true);
    std::string output_key_value_pair(
       int n_indent,std::string key,double value,
       bool terminal_comma_flag=true);
    std::string output_key_value_pair(
       int n_indent,std::string key,std::string value,
       bool terminal_comma_flag=true);
    std::string output_key_value_pair(
       int n_indent,std::string key,const std::vector<int>& value,
       bool terminal_comma_flag=true);
    std::string output_key_value_pair(
       int n_indent,std::string key,const std::vector<double>& value,
       bool terminal_comma_flag=true);

// Node neighbor export methods

    void write_neighbors_JSON_file(
       int requested_node_ID,std::vector<int>& neighbor_IDs,
       std::string JSON_filename);
    std::string generate_neighbor_JSON_string(
       int requested_node_ID,std::vector<int>& neighbor_IDs);
    std::string output_neighbor_GraphML(
       int n_indent,int node_ID,std::vector<int>& neighbor_IDs,
       bool terminal_node_flag);

    std::vector<int> retrieve_node_neighbors_from_database(
       gis_database* gis_database_ptr,int node_ID);

    std::string write_data_json_string(
       int n_indent,double edge_weight,
       double r,double g,double b,double relative_size);
    std::string write_data_json_string(
       int n_indent,std::string data_type,double epoch,std::string caption,
       std::string photo_URL,int photo_Npx,int photo_Npy,
       std::string thumbnail_URL,int thumbnail_Npx,int thumbnail_Npy,
       double edge_weight,
       int connected_component_ID,int parent_ID,
       const std::vector<int>& children_IDs,
       double gx,double gy,double r,double g,double b,double relative_size);
    std::string write_data_json_string(
       int n_indent,std::string data_type,double epoch,std::string caption,
       std::string photo_URL,int photo_Npx,int photo_Npy,
       std::string thumbnail_URL,int thumbnail_Npx,int thumbnail_Npy,
       double edge_weight,int connected_component_ID,int parent_ID,
       const std::vector<int>& children_IDs,
       double gx,double gy,double gx2,double gy2,
       double r,double g,double b,double relative_size,
       const std::vector<std::string>& attribute_keys,
       const std::vector<std::string>& attribute_values);

    std::string write_edge_json_string(
       int n_indent,int i,int j,double edge_weight,
       double r,double g,double b,double relative_thickness,
       bool terminal_edge_flag);

// Node attribute methods

    std::string generate_condition_color_mapper_JSON_string(
       const std::vector<std::string>& all_attribute_keys,
       const std::vector<std::vector<std::string> >& all_attribute_values,
       std::map<int,std::vector<int> >& selected_key_value_IDs_map,
       colorfunc::RGB selected_attribute_RGB,
       colorfunc::RGB unselected_attribute_RGB);
    std::string generate_dominant_colorings_JSON_string(
       std::string primary_color,std::string secondary_color,
       std::string tertiary_color,
       colorfunc::RGB inColor_RGB,colorfunc::RGB outColor_RGB);
    std::string generate_multi_object_selection_JSON_string(
       const std::vector<int>& n_objects,std::string object_name,
       colorfunc::RGB inColor_RGB,colorfunc::RGB outColor_RGB);
    std::string generate_discrete_color_mapper_JSON_string(
       std::string attribute_str,colorfunc::RGB& defaultColor_RGB,
       std::vector<int>& values,std::vector<colorfunc::RGB>& value_RGBs);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // jsonfunc namespace

#endif // jsonfuncs.h

