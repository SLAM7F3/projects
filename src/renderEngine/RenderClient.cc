
#include "renderEngine/RenderClient.h"

using std::ios;
using std::cout;
using std::endl;
using std::string;

RenderClient::RenderClient(std::string host, int port_number)
{
    this->host = host;
    this->port_number = port_number;
    the_connection = new ClientSocket(host,port_number);
    the_connection->set_non_blocking( false );
}

bool RenderClient::send_osg_group(osg::Group* group, std::string name, std::string extension)
{
    unsigned int datatype;
    if (extension == "osg")
        datatype = eOSGGroupOSG;
    else if (extension == "ive")
        datatype = eOSGGroupIVE;
    else
    {
        cout << "Error! Invalid extension!" << endl;
        return false;
    }

    osgDB::ReaderWriter* writer = osgDB::Registry::instance()->getReaderWriterForExtension(extension);
    std::stringstream node_output;
    writer->writeNode(*group, node_output);
    node_output.seekg (0, ios::end);
    long size = node_output.tellg();
    node_output.seekg (0, ios::beg);
    char * buffer;
    buffer = new char [size];
    node_output.read (buffer,size);

    // Generate packet header for current pulse:
    packet_header curr_header;
    int n_header_bytes = sizeof(packet_header);
    int n_data_bytes = size;
    curr_header.n_data_bytes = n_data_bytes;
    curr_header.data_type = datatype;
    curr_header.operation = eAdd;
    strcpy(curr_header.name, name.c_str());

    char* header_byte_ptr=reinterpret_cast<char *>(&curr_header);
    char* data_byte_ptr=buffer;
    
    the_connection->write_all_to_socket(header_byte_ptr,n_header_bytes);
    the_connection->write_all_to_socket(data_byte_ptr,n_data_bytes);
    cout << "The osg::Group*: " << name << " sent to RenderServer..." << endl;
    
    return true;
}

bool RenderClient::send_delete_group(std::string name)
{
    packet_header curr_header;
    int n_header_bytes = sizeof(packet_header);
    curr_header.n_data_bytes = 0;
    curr_header.data_type = eOSGGroupOSG;
    curr_header.operation = eDelete;
    strcpy(curr_header.name, name.c_str());

    char* header_byte_ptr=reinterpret_cast<char *>(&curr_header);
    
    the_connection->write_all_to_socket(header_byte_ptr,n_header_bytes);
    cout << "Delete of osg::Group*: " << name << " sent to RenderServer..." << endl;

    return true;
}

void RenderClient::close()
{
    the_connection->disconnect();
}
