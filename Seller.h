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

class Seller
{
public:
    Seller(std::string seller_ID) {
        gfs_master = Shadow_Directory{ "http://127.0.0.1:8384", "1234567890", "Directory", "00000000" };
        fhandle = std::string("00000002");

        seller_id = seller_ID;
        selling_pkgarr.clear();
        selling_cusarr.clear();
    }

    void package_pkg(std::string pkg_ID, std::string custom_ID, std::string pos, std::string tar);
    void check_data();

    Shadow_Directory gfs_master = Shadow_Directory{ "http://127.0.0.1:8384", "1234567890", "Directory", "00000000" };
    std::string fhandle;

    std::string seller_id;
    std::vector<std::string> selling_pkgarr;
    std::vector<std::string> selling_cusarr;
};

void Seller::check_data(){
    std::cout<<"\n===============Check Data: Seller "<< seller_id<< "==============="<<std::endl;
    std::vector<std::string> check_arr;

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
            check_arr.push_back(line);
            std::getline(ss,line);
            std::cout<<std::setw(4)<<line<<" -> ";
            std::getline(ss,line);
            std::cout<<std::setw(4)<<line<<std::endl;

            if ( ss.fail() ) break;
        }
        std::cout<<".........................\n"<<std::endl;
        
        // std::cout<<"---------------Checked Data: Flight"<< flight_id<< " "<<_from<<"->"<<_to<<"---------------\n"<<std::endl;
    }
    else std::cout<<"------------!!!Check Data: Seller "<< seller_id<< "!!!------------\n"<<std::endl;

    //update selling pkg
    std::vector<std::string>::iterator check_iter;
    bool s = true;
    while(s){
        for(int i=0;i<selling_pkgarr.size();i++){
            check_iter = find(check_arr.begin(), check_arr.end(), selling_pkgarr[i]);
            if (check_iter == check_arr.end()) {
                selling_pkgarr.erase(selling_pkgarr.begin() + i);
                break;
            }
            if(i==selling_pkgarr.size()-1) s = false;
        }
        if(selling_pkgarr.size()== 0) break;
    }
    
    std::cout<<"Delivering PKG: ";
    for(int i=0;i<selling_pkgarr.size();i++){
        std::cout<<selling_pkgarr[i];
        if(i!=selling_pkgarr.size()-1) std::cout<<", ";
    }
    if(selling_pkgarr.size()==0) std::cout<<"None";
    std::cout<<std::endl;
}

void Seller::package_pkg(std::string pkg_ID, std::string custom_ID, std::string pos, std::string tar){
    std::cout<<"\n===============Seller "<< seller_id<< " adding PKG "<< pkg_ID <<"==============="<<std::endl;

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

    std::string my_chunk_data = "Seller" + seller_id+"\n" + pkg_ID + "\n" + pos + "\n" + tar + "\n";

    while(true){
        //Step3
        std::cout << "Push data to replica 1" << std::endl;
        result_P = gfs_primary.PushChunk2Replica("my_ecs251_file", fhandle, "0", my_chunk_data);
        std::cout << "Push data to replica 2" << std::endl;
        result_A = gfs_secondary_A.PushChunk2Replica("my_ecs251_file", fhandle, "0", my_chunk_data);
        std::cout << "Push data to replica 3" << std::endl;
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
                selling_pkgarr.push_back(pkg_ID);
                selling_cusarr.push_back(custom_ID);
                // pkg_arr.clear();
                // std::cout<<"\n........................................"<<std::endl;
                std::cout<<"\n* Seller "<< seller_id<<"\n* Delivering PKG: ";
                for(int i=0;i<selling_pkgarr.size();i++){
                    std::cout<<selling_pkgarr[i];
                    if(i!=selling_pkgarr.size()-1) std::cout<<", ";
                }
                if(selling_pkgarr.size()==0) std::cout<<"None";
                std::cout<<"\n"<<std::endl;
                // std::cout<<"........................................\n"<<std::endl;
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
            // std::cout<<"\n........................................"<<std::endl;
            std::cout<<"\n* Seller "<< seller_id<<"\n* Delivering PKG: ";
            for(int i=0;i<selling_pkgarr.size();i++){
                std::cout<<selling_pkgarr[i];
                if(i!=selling_pkgarr.size()-1) std::cout<<", ";
            }
            if(selling_pkgarr.size()==0) std::cout<<"None";
            std::cout<<"\n"<<std::endl;
            // std::cout<<"........................................\n"<<std::endl;
            if(!(result_P["Error"]).isNull() || !(result_P["Error"]).isNull() || !(result_P["Error"]).isNull()) {
                std::cout<<"------------!!!Seller "<< seller_id<< " adding PKG!!!------------\n"<<std::endl;
                break;
            }
        }
    }
}