// miniNFS
// for Json::value
#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>
#include <json/value.h>
#include <string>

// for JsonRPCCPP
#include <iostream>
#include "minigfs_client.h"
#include <stdio.h>

// ecs251
#include "Core.h"
#include "Directory.h"
#include "Replica.h"
#include "Shadow_Directory.h"
#include <time.h>

using namespace jsonrpc;
using namespace std;

class Flight : public minigfs_Client
{
public:
  Flight(AbstractServerConnector &connector, serverVersion_t type);
  //加函数
};