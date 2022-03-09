
// miniNFS

// for Json::value
#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>
#include <json/value.h>
#include <string>

// for JsonRPCCPP
#include <iostream>
#include "minigfs_server.h"
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <stdio.h>

// ecs251
#include "Core.h"
#include "Directory.h"
#include "Replica.h"
#include "Shadow_Directory.h"
#include "Shadow_Replica.h"
#include <time.h>

using namespace jsonrpc;
using namespace std;

class Myminigfs_Server : public minigfs_Server
{
public:
  Myminigfs_Server(AbstractServerConnector &connector, serverVersion_t type);
  virtual Json::Value ObtainChunkURL(const std::string& action, const std::string& arguments, const std::string& chunkindex, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID);
  virtual Json::Value PushChunk2Replica(const std::string& action, const std::string& arguments, const std::string& chunk, const std::string& chunkindex, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID);
  virtual Json::Value CommitAbort(const std::string& action, const std::string& arguments, const std::string& chunkindex, const std::string& class_id, const std::string& commitorabort, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID);
  virtual Json::Value LookUp(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID);
  virtual Json::Value Create(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& created_class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID, const std::string& sattr);
  virtual Json::Value dumpJ(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID);
};

Myminigfs_Server::Myminigfs_Server(AbstractServerConnector &connector, serverVersion_t type)
  : minigfs_Server(connector, type)
{
  std::cout << "Myminigfs_Server Object created" << std::endl;
}

Replica *mounted;
// primary与gfs_secondary_A, gfs_secondary_B优先级不同
Shadow_Replica *gfs_secondary_A;
Shadow_Replica *gfs_secondary_B;

// member function

Json::Value
Myminigfs_Server::ObtainChunkURL
(const std::string& action, const std::string& arguments, const std::string& chunkindex, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID)
{
  Json::Value result;
  //
  return result;
}

Json::Value
Myminigfs_Server::PushChunk2Replica
(const std::string& action, const std::string& arguments, const std::string& chunk, const std::string& chunkindex, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID)
{
  Json::Value result;
  
  std::cout<<"Prime_Replica PushChunk2Replica!"<<std::endl;

  if (fhandle != "00000002") // inode 2 is the root
    {
      result["status"] = "GFSERR_STALE";
      result["vote"] = "abort";
    }
  else
    {
      result = mounted->PushChunk2Replica(filename, fhandle, chunkindex, chunk);
    }

  return result;
}

Json::Value
Myminigfs_Server::CommitAbort
(const std::string& action, const std::string& arguments, const std::string& chunkindex, const std::string& class_id, const std::string& commitorabort, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID)
{
  Json::Value result;
  
  std::cout<<"Prime_Replica CommitAbort!"<<std::endl;

  if (fhandle != "00000002") // inode 2 is the root
    {
      result["status"] = "GFSERR_STALE";
    }
  else
    {
      // Step5-6
      Json::Value result_P = mounted->CommitAbort(filename, fhandle, chunkindex, commitorabort);
      // result_P["status"] = "Bad";
      result["status_P"] = (result_P["status"]).asString(); // 保证primary在线
      if(((result_P["status"]).asString() == "committed")){ // Step5：primary传递“写请求”到secondaryAB
        Json::Value result_A = gfs_secondary_A->CommitAbort(filename, fhandle, chunkindex, commitorabort);
        Json::Value result_B = gfs_secondary_B->CommitAbort(filename, fhandle, chunkindex, commitorabort);
        result["status_A"] = (result_A["status"]).asString();
        result["status_B"] = (result_B["status"]).asString();
        if(((result_A["status"]).asString() == "committed")&&((result_B["status"]).asString() == "committed")) result["status"]="committed"; // Step6：secondaryA,B告诉primary“写操作”成功的结果
      }
    }

  return result;
}

Json::Value
Myminigfs_Server::LookUp(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID)
{
  Json::Value result;
  std::cout << action << " " << arguments << " " << owner_vsID << std::endl;

  std::cout << "SFelixWu receiving LookUp" << std::endl;

  if (fhandle != "00000002") // inode 2 is the root
    {
      result["status"] = "NFSERR_STALE";
    }
  else
    {
      // result = mounted->LookUp(fhandle, filename); //这我感觉整个都没有用！
    }

  return result;
}

Json::Value
Myminigfs_Server::Create(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& created_class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID, const std::string& sattr)
{
  Json::Value result;
  std::cout << action << " " << arguments << " " << owner_vsID << std::endl;

  std::cout << "SFelixWu receiving Create" << std::endl;

  if (fhandle != "00000002")
    {
      result["status"] = "NFSERR_STALE";
    }
  else
    {
      // result = mounted->Create(fhandle, filename, sattr); //这我感觉整个都没有用！
    }

  return result;
}

Json::Value
Myminigfs_Server::dumpJ(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID)
{
  Json::Value result;
  std::cout << action << " " << arguments << " " << owner_vsID << std::endl;

  std::cout << "SFelixWu receiving dumpJ" << std::endl;

  if (object_id != "00000002")
    {
      result["status"] = "NFSERR_STALE";
    }
  else
    {
      Json::Value *myv_ptr = mounted->dumpJ();
      if (myv_ptr != NULL)
	{
	  result = *myv_ptr;
	  result["status"] = "NFS_OK";
	}
      else
	{
	  result["status"] = "NFSERR_STALE";
	}
    }

  return result;
}

int
main() 
{
  // Directory NFS_root
  // { "http://169.237.6.102", "1234567890", "Directory", "00000000", "root", "00000002" };

  // mounted = (&NFS_root);
  
  Replica GFS_primaryReplica
  { "http://169.237.6.102", "1234567890", "Replica", "00000001", "This is Prime_Replica!"};

  mounted = (&GFS_primaryReplica);

  // SecondaryA,B
  std::string url_secondary_A = "http://127.0.0.1:8301";
  gfs_secondary_A = new Shadow_Replica{ url_secondary_A, "1234567890", "Replica", "00000002"};

  std::string url_secondary_B = "http://127.0.0.1:8302";
  gfs_secondary_B = new Shadow_Replica{ url_secondary_B, "1234567890", "Replica", "00000003" };


  HttpServer httpserver(8300);
  Myminigfs_Server s(httpserver,
		JSONRPC_SERVER_V1V2); // hybrid server (json-rpc 1.0 & 2.0)
  s.StartListening();
  std::cout << "Hit enter to stop the server" << endl;
  getchar();

  s.StopListening();
  return 0;
}
