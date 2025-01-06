#include "stdafx.h"
#include "RemoteClient.h"

unordered_map<RemoteClient*, shared_ptr<RemoteClient>> RemoteClient::remoteClients;