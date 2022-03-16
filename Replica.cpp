
#include "Replica.h"
#include <sstream>
#include <iomanip>
#include <map>
#include <tuple>

Chunk::Chunk()
{
  this->data = "100\nThis is a test!\n101\nThis is a test too!\n";
}

Chunk::Chunk(std::string arg_data)
{
  this->data = arg_data;
}

Json::Value *
Chunk::dumpJ()
{
  Json::Value * result_ptr = new Json::Value();

  if (this->data != "")
    {
      (*result_ptr)["data"] = this->data;
    }

  return result_ptr;
}

bool
Chunk::Jdump(Json::Value *input_json_ptr)
{
  if ((input_json_ptr == NULL) ||
      ((*input_json_ptr).isNull() == true) ||
      ((*input_json_ptr).isObject() != true))
    {
      return false;
    }

  if ((((*input_json_ptr)["data"]).isNull() == true) ||
      (((*input_json_ptr)["data"]).isString() != true))
    {
      return false;
    }

  this->data = ((*input_json_ptr)["data"]).asString();

  return true;
}

Replica::Replica
(std::string core_arg_host_url, std::string core_arg_owner_vsID,
 std::string core_arg_class_id, std::string core_arg_object_id)
  : Core { core_arg_host_url, core_arg_owner_vsID,
    core_arg_class_id, core_arg_object_id }
{
  // std::cout << "a shadow has been created" << std::endl;
}

Replica::Replica
(std::string core_arg_host_url, std::string core_arg_owner_vsID,
 std::string core_arg_class_id, std::string core_arg_object_id,
 std::string arg_data)
  : Core { core_arg_host_url, core_arg_owner_vsID,
    core_arg_class_id, core_arg_object_id }
{
  (this->committed_data).data = arg_data;
}

Json::Value
Replica::CommitAbort
(std::string arg_name, std::string arg_fhandle, std::string arg_chunk_index,
 std::string arg_commitorabort)
{
  Json::Value result;
  //这里要加
  if (arg_commitorabort == "commit"){
    (this->committed_data).data  = (this->uncommitted_data).data;
    result["status"] = "committed";
    std::string data = (this->committed_data).data;
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
  } 

  return result;
}

Json::Value
Replica::PushChunk2Replica
(std::string arg_name, std::string arg_fhandle, std::string arg_chunk_index, std::string arg_chunk)
{
  Json::Value result;
  // (this->uncommitted_data).data = arg_chunk;
  if((this->committed_data).data==""&&false) {
    result["vote"] = "abort"; 
    result["Error"] = "Chunk Error";
    std::cout<<"Chunk Error"<<std::endl;
    return result;
  }
  else{
      std::stringstream ss;
      ss.clear();
      ss.str((this->committed_data).data);
      std::map<std::string, std::tuple<std::string, std::string> > pkg_map;
      while (1)
      {
          std::string pkg_id, position, destination;
          std::getline(ss,pkg_id);
          std::getline(ss,position);
          std::getline(ss,destination);

          // std::cout<<pkg_id<<std::endl;
          // std::cout<<position<<std::endl;
          
          if(pkg_id==""||pkg_id=="\n") break;
          if(!(pkg_map.insert(std::pair<std::string, std::tuple<std::string, std::string> >(pkg_id, std::tuple<std::string, std::string>(position, destination)))).second){
              result["vote"] = "abort";
              result["Error"] = "Chunk Error";
              std::cout<<"Chunk Error"<<std::endl;
              return result; 
          }
          
          if ( ss.fail() ) break;
      }
      //pkg_map.insert(std::pair<std::string, std::string>("100", "position"));

      ss.clear();
      ss.str(arg_chunk);
      std::string ind;
      std::getline(ss,ind);
      if(ind[0]=='F'){
        std::string start, end;
        std::getline(ss,start);
        std::getline(ss,end);
        while (1)
        {
            std::string pkg_id;
            std::getline(ss,pkg_id);
            if(pkg_id==""||pkg_id=="\n") break;
            // std::cout<<pkg_id<<std::endl;

            auto iter = pkg_map.find(pkg_id);
  
            if(iter != pkg_map.end()){
                if(std::get<0>(iter->second)!=start){
                    result["vote"] = "abort";
                    result["Error"] = "Pkg position Error";
                    std::cout<<"Pkg position Error"<<std::endl;
                    return result; 
                }
                if(end == std::get<1>(iter->second)) //arrive destination
                {
                    pkg_map.erase(iter);
                }
                else std::get<0>(iter->second) = end;
            }
            // else{
            //     result["vote"] = "abort";
            //     result["Error"] = "Flight pkg Error";
            //     return result; 
            // }

            // std::cout<<pkg_id<<std::endl;
            // std::cout<<position<<std::endl;
            
            if ( ss.fail() ) break;
        }
      }
      else if(ind[0]=='S'){
            std::string pkg_id, pos, tar;
            std::getline(ss,pkg_id);
            std::getline(ss,pos);
            std::getline(ss,tar);

            auto iter = pkg_map.find(pkg_id);

            if(iter != pkg_map.end()){
                if(std::get<0>(iter->second)!=pos){
                    result["vote"] = "abort";
                    result["Error"] = "Pkg position Error";
                    std::cout<<"Pkg position Error"<<std::endl;
                    return result; 
                }
                std::get<1>(iter->second) = tar;
            }
            else{
                pkg_map.insert(std::pair<std::string, std::tuple<std::string, std::string> >(pkg_id, std::tuple<std::string, std::string>(pos, tar)));
            }
      }
      
      std::string save_data = "";
      std::map<std::string, std::tuple<std::string, std::string> > ::iterator iter;
      iter = pkg_map.begin();
      while(iter != pkg_map.end()) {
          save_data += iter->first + "\n" + std::get<0>(iter->second) + "\n" + std::get<1>(iter->second) +"\n";
          iter++;
      }
      (this->uncommitted_data).data = save_data;
  }
  result["vote"] = "commit"; 
  return result;
}

Json::Value *
Replica::dumpJ()
{
  Json::Value * result_ptr = this->committed_data.dumpJ();

  if (this->name != "")
    {
      (*result_ptr)["name"] = this->name;
    }

  if (this->fhandle != "")
    {
      (*result_ptr)["fhandle"] = this->fhandle;
    }

  if (this->chunk_index != "")
    {
      (*result_ptr)["chunk_index"] = this->chunk_index;
    }

  return result_ptr;
}

bool
Replica::Jdump(Json::Value *input_json_ptr)
{
  if ((input_json_ptr == NULL) ||
      ((*input_json_ptr).isNull() == true) ||
      ((*input_json_ptr).isObject() != true))
    {
      return false;
    }

  if ((((*input_json_ptr)["name"]).isNull() == true) ||
      (((*input_json_ptr)["fhandle"]).isNull() == true) ||
      (((*input_json_ptr)["name"]).isString() != true) ||
      (((*input_json_ptr)["fhandle"]).isString() != true))
    {
      return false;
    }

  this->name    = ((*input_json_ptr)["name"]).asString();
  this->fhandle = ((*input_json_ptr)["fhandle"]).asString();
  return true;
}

