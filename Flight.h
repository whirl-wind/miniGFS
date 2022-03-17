#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include "minigfs_client.h"
#include "Shadow_Directory.h"
#include "Shadow_Replica.h"
#include <algorithm>

using namespace jsonrpc;
using namespace std;

class Flight
{
public:
    Flight(std::string flight_ID, std::string from, std::string to) {
        gfs_master = Shadow_Directory{ "http://127.0.0.1:8384", "1234567890", "Directory", "00000000" };
        fhandle = std::string("00000002");
        
        flight_id = flight_ID;
        _from = from;
        _to = to;
        pkg_arr.clear();
    }

    void landed();
    void set_destination(std::string position);
    void load_package(std::string pkg_id);
    void check_data();

    Shadow_Directory gfs_master = Shadow_Directory{ "http://127.0.0.1:8384", "1234567890", "Directory", "00000000" };
    std::string fhandle;

    std::string flight_id;
    std::vector<std::string> pkg_arr;
    std::string _from;
    std::string _to;
};

void Flight::set_destination(std::string position){
    _to = position;
}

void Flight::check_data(){
    std::cout<<"\n===============Check Data: Flight"<< flight_id<< " "<<_from<<"->"<<_to<<"==============="<<std::endl;
    std::cout<<"Loaded PKG: ";
    for(int i=0;i<pkg_arr.size();i++){
        std::cout<<pkg_arr[i];
        if(i!=pkg_arr.size()-1) std::cout<<", ";
    }
    if(pkg_arr.size()==0) std::cout<<"None";
    std::cout<<std::endl;

    Json::Value result, result_P, result_A, result_B;
    result = gfs_master.ObtainChunkURL("my_ecs251_file", fhandle, "0");
    while((result["status"]).asString() != "URLpassed"){
        result = gfs_master.ObtainChunkURL("my_ecs251_file", fhandle, "0");
    }

    std::string url_primary = (result["primary"]).asString();
    Shadow_Replica gfs_primary
    { url_primary, "1234567890", "Replica", "00000001" };

    std::string url_secondary_A = (result["secondary_A"]).asString();
    Shadow_Replica gfs_secondary_A
    { url_secondary_A, "1234567890", "Replica", "00000002" };

    std::string url_secondary_B = (result["secondary_B"]).asString();
    Shadow_Replica gfs_secondary_B
    { url_secondary_B, "1234567890", "Replica", "00000003" };

    result = *gfs_primary.dumpJ();
    if((result["data"]).isString()) {
        std::string data = (result["data"]).asString();
        std::stringstream ss;
        ss.clear();
        ss.str(data);
        std::string line;
        std::cout<<"\n........................."<<std::endl;
        while (1)
        {
            std::getline(ss,line);
            if(line==""||line=="\n") break;
            std::cout<<"pkg "<<std::setw(4)<<line<<" : ";
            std::getline(ss,line);
            std::cout<<std::setw(4)<<line<<" -> ";
            std::getline(ss,line);
            std::cout<<std::setw(4)<<line<<std::endl;

            if ( ss.fail() ) break;
        }
        std::cout<<".........................\n"<<std::endl;
        
        // std::cout<<"---------------Checked Data: Flight"<< flight_id<< " "<<_from<<"->"<<_to<<"---------------\n"<<std::endl;
    }
    else std::cout<<"------------!!!Check Data: Flight"<< flight_id<< " "<<_from<<"->"<<_to<<"!!!------------\n"<<std::endl;
}

void Flight::load_package(std::string ld_pkg_id){
    std::cout<<"\n===============Flight"<< flight_id<< " "<<_from<<"->"<<_to<<" Loading PKG==============="<<std::endl;

    Json::Value result, result_P, result_A, result_B;
    result = gfs_master.ObtainChunkURL("my_ecs251_file", fhandle, "0");
    while((result["status"]).asString() != "URLpassed"){
        result = gfs_master.ObtainChunkURL("my_ecs251_file", fhandle, "0");
    }

    std::string url_primary = (result["primary"]).asString();
    Shadow_Replica gfs_primary
    { url_primary, "1234567890", "Replica", "00000001" };

    std::string url_secondary_A = (result["secondary_A"]).asString();
    Shadow_Replica gfs_secondary_A
    { url_secondary_A, "1234567890", "Replica", "00000002" };

    std::string url_secondary_B = (result["secondary_B"]).asString();
    Shadow_Replica gfs_secondary_B
    { url_secondary_B, "1234567890", "Replica", "00000003" };

    result = *gfs_primary.dumpJ();

    if((result["data"]).isString()) {
        std::string data = (result["data"]).asString();
        std::stringstream ss;
        ss.clear();
        ss.str(data);
        std::map<std::string, std::string> pkg_map;
        while (1)
        {
            std::string pkg_id, position, destination;
            std::getline(ss,pkg_id);
            std::getline(ss,position);
            std::getline(ss,destination);

            // std::cout<<pkg_id<<std::endl;
            // std::cout<<position<<std::endl;
            
            if(pkg_id==""||pkg_id=="\n") break;
            if(!(pkg_map.insert(std::pair<std::string, std::string>(pkg_id, position))).second){
                std::cout<<"Chunk Error"<<std::endl;
                std::cout<<"------------!!!Flight"<< flight_id<< " "<<_from<<"->"<<_to<<" fail loading!!!------------\n"<<std::endl;
                return; 
            }
            
            if ( ss.fail() ) break;
        }

        auto iter = pkg_map.find(ld_pkg_id);
 
        if(iter != pkg_map.end()){
            if(iter->second!=_from){
                std::cout<<"Pkg "<< ld_pkg_id <<"\'s position is not here!"<<std::endl;
                std::cout<<"------------!!!Flight"<< flight_id<< " "<<_from<<"->"<<_to<<" fail loading!!!------------\n"<<std::endl;
                return; 
            }
        }
        else{
            // Load package!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
    }
    else {
        std::cout<<"------------!!!Flight"<< flight_id<< " "<<_from<<"->"<<_to<<" fail loading!!!------------\n"<<std::endl;
        return;
    }

    if (find(pkg_arr.begin(), pkg_arr.end(), ld_pkg_id) == pkg_arr.end()){
        pkg_arr.push_back(ld_pkg_id);
        std::cout<<"Good! Successfully loaded package: "<< ld_pkg_id <<std::endl;
    }
    else {
        std::cout<<"Bad!! Already loaded this package: "<< ld_pkg_id <<std::endl;
        std::cout<<"------------!!!Flight"<< flight_id<< " "<<_from<<"->"<<_to<<" fail loading!!!------------\n"<<std::endl;
        return;
    }

    //std::cout<<"---------------Flight"<< flight_id<< " "<<_from<<"->"<<_to<<" PKG loaded---------------\n"<<std::endl;
}

void Flight::landed(){
    std::cout<<"\n===============Flight"<< flight_id<< " "<<_from<<"->"<<_to<<" Took Off==============="<<std::endl;
    string position = _to;
    _to = _from;
    _from = position;

    Json::Value result, result_P, result_A, result_B;
    result = gfs_master.ObtainChunkURL("my_ecs251_file", fhandle, "0");
    while((result["status"]).asString() != "URLpassed"){
        result = gfs_master.ObtainChunkURL("my_ecs251_file", fhandle, "0");
    }

    std::string url_primary = (result["primary"]).asString();
    Shadow_Replica gfs_primary
    { url_primary, "1234567890", "Replica", "00000001" };

    std::string url_secondary_A = (result["secondary_A"]).asString();
    Shadow_Replica gfs_secondary_A
    { url_secondary_A, "1234567890", "Replica", "00000002" };

    std::string url_secondary_B = (result["secondary_B"]).asString();
    Shadow_Replica gfs_secondary_B
    { url_secondary_B, "1234567890", "Replica", "00000003" };

    std::string my_chunk_data = "Flight" + flight_id+"\n"+_to + "\n" + position + "\n";
    for(int i=0;i<pkg_arr.size();i++){
        my_chunk_data += pkg_arr[i] + "\n";
    }

    // std::cout<<"\n........................................"<<std::endl;
    std::cout<<"\n* Flight "<< flight_id<< ": "<<_from<<"->"<<_to<<"\n* Loaded PKG: ";
    for(int i=0;i<pkg_arr.size();i++){
        std::cout<<pkg_arr[i];
        if(i!=pkg_arr.size()-1) std::cout<<", ";
    }
    if(pkg_arr.size()==0) std::cout<<"None";
    std::cout<<"\n"<<std::endl;
    // std::cout<<"........................................\n"<<std::endl;

    while(true){
        //Step3
        result_P = gfs_primary.PushChunk2Replica("my_ecs251_file", fhandle, "0", my_chunk_data);
        result_A = gfs_secondary_A.PushChunk2Replica("my_ecs251_file", fhandle, "0", my_chunk_data);
        result_B = gfs_secondary_B.PushChunk2Replica("my_ecs251_file", fhandle, "0", my_chunk_data);

        //Step4
        if (((result_P["vote"]).asString() == "commit") &&
        ((result_A["vote"]).asString() == "commit") &&
        ((result_B["vote"]).asString() == "commit"))
        {
            //Step6-7
            result_P = gfs_primary.CommitAbort("my_ecs251_file", fhandle, "0", "commit");
            result_A = gfs_secondary_A.CommitAbort("my_ecs251_file", fhandle, "0", "commit");
            result_B = gfs_secondary_B.CommitAbort("my_ecs251_file", fhandle, "0", "commit");
            if(((result_P["status"]).asString() == "committed") &&
            ((result_A["status"]).asString() == "committed") &&
            ((result_B["status"]).asString() == "committed")) {
                pkg_arr.clear();
                // std::cout<<"---------------Flight"<< flight_id<< " "<<_from<<"->"<<_to<<" Landed---------------\n"<<std::endl;
                break;
            }
        }
        else{
            // if(((result_P["Error"]).asString() == "Chunk Error") ||
            // ((result_A["Error"]).asString() == "Chunk Error") ||
            // ((result_B["Error"]).asString() == "Chunk Error") || 
            // ((result_P["Error"]).asString() == "Chunk Error") ||
            // ((result_A["Error"]).asString() == "Chunk Error") ||
            // ((result_B["Error"]).asString() == "Chunk Error")) break;
            if(!(result_P["Error"]).isNull() || !(result_P["Error"]).isNull() || !(result_P["Error"]).isNull()) {
                std::cout<<"------------!!!Flight"<< flight_id<< " "<<_from<<"->"<<_to<<" Landing!!!------------\n"<<std::endl;
                break;
            }
        }
    }
}
