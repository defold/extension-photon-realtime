#include <dmsdk/sdk.h>
#include "Common-cpp/inc/Object.h"
#include "Common-cpp/inc/JString.h"
#include "Common-cpp/inc/Enums/TypeCode.h"
#include "Common-cpp/inc/Containers/Dictionary.h"
#include "Common-cpp/inc/Containers/JVector.h"
#include "Common-cpp/inc/Containers/Hashtable.h"
#include "LoadBalancing-cpp/inc/Listener.h"
#include "LoadBalancing-cpp/inc/Player.h"
#include "LoadBalancing-cpp/inc/Room.h"
#include "LoadBalancing-cpp/inc/LobbyStatsResponse.h"

#include "listener.h"
#include "helper.h"


#define PHOTONCALLBACK_STACKCHECK(_diff_) DM_LUA_STACK_CHECK(dmScript::GetCallbackLuaContext(m_Callback), _diff_);





lua_State* PhotonListener::setupCallback(int event)
{
    if (!dmScript::IsCallbackValid(m_Callback))
    {
        dmLogError("Callback is invalid.");
        return 0;
    }

    if (!dmScript::SetupCallback(m_Callback))
    {
        dmLogError("Failed to setup callback.");
        return 0;
    }

    lua_State* L = dmScript::GetCallbackLuaContext(m_Callback);
    lua_pushinteger(L, event);
    return L;
}

void PhotonListener::invokeCallback(lua_State* L)
{
    // self, id, data
    int ret = dmScript::PCall(L, 3, 0);
    (void)ret;

    dmScript::TeardownCallback(m_Callback);
}

void PhotonListener::destroyCallback()
{
    if (m_Callback)
    {
        dmScript::DestroyCallback(m_Callback);
        m_Callback = 0x0;
    }
}


PhotonListener::PhotonListener(lua_State* L, int index) {
    m_Callback = dmScript::CreateCallback(L, index);

}

PhotonListener::~PhotonListener() {
    destroyCallback();
}

// receive and print out debug out here
void PhotonListener::debugReturn(int debugLevel, const ExitGames::Common::JString& string)
{
    switch (debugLevel)
    {
    case ExitGames::Common::DebugLevel::OFF:
        break;
    case ExitGames::Common::DebugLevel::ERRORS:
        dmLogError("%ls", string.cstr());
        break;
    case ExitGames::Common::DebugLevel::WARNINGS:
        dmLogWarning("%ls", string.cstr());
        break;
    case ExitGames::Common::DebugLevel::INFO:
        dmLogInfo("%ls", string.cstr());
        break;
    case ExitGames::Common::DebugLevel::ALL:
    default:
        dmLogDebug("%ls", string.cstr());
        break;
    }
}


// implement your error-handling here
void PhotonListener::connectionErrorReturn(int errorCode)
{
    dmLogError("connectionErrorReturn");
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_CONNECTIONERRORRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        invokeCallback(L);
    }
}

void PhotonListener::clientErrorReturn(int errorCode)
{
    dmLogError("clientErrorReturn");
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_CLIENTERRORRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        invokeCallback(L);
    }
}

void PhotonListener::warningReturn(int warningCode)
{
    dmLogWarning("warningReturn");
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_WARNINGRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "warning_code", warningCode);
        invokeCallback(L);
    }
}

void PhotonListener::serverErrorReturn(int errorCode)
{
    dmLogError("serverErrorReturn");
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_SERVERERRORRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        invokeCallback(L);
    }
}


// events, triggered by certain operations of all players in the same room
void PhotonListener::joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player)
{
    dmLogInfo("joinRoomEventAction");
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_JOINROOMEVENTACTION))
    {
        lua_newtable(L);
        table_pushint(L, "player_nr", playerNr);
        table_pushintvector(L, "player_nrs", playernrs);
        table_pushplayer(L, "player", player);
        invokeCallback(L);
    }
}

void PhotonListener::leaveRoomEventAction(int playerNr, bool isInactive)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_LEAVEROOMEVENTACTION))
    {
        lua_newtable(L);
        table_pushint(L, "player_nr", playerNr);
        table_pushboolean(L, "is_inactive", isInactive);
        invokeCallback(L);
    }
}


void PhotonListener::customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_CUSTOMEVENTACTION))
    {
        lua_newtable(L);
        table_pushint(L, "player_nr", playerNr);
        table_pushint(L, "event_code", eventCode);
        table_pushobject(L, "event_content", &eventContent);
        invokeCallback(L);
    }
}


// callbacks for operations on the server
void PhotonListener::connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_CONNECTRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        table_pushstring(L, "error_string", errorString.UTF8Representation().cstr());
        table_pushstring(L, "region", region.UTF8Representation().cstr());
        table_pushstring(L, "cluster", cluster.UTF8Representation().cstr());
        invokeCallback(L);
    }
}

void PhotonListener::disconnectReturn(void)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_DISCONNECTRETURN))
    {
        lua_newtable(L);
        invokeCallback(L);
    }
}

void PhotonListener::createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_CREATEROOMRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        table_pushstring(L, "error_string", errorString.UTF8Representation().cstr());
        table_pushint(L, "local_player_nr", localPlayerNr);
        table_pushhashtable(L, "room_properties", roomProperties);
        table_pushhashtable(L, "player_properties", playerProperties);
        invokeCallback(L);
    }
}

void PhotonListener::joinOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_JOINORCREATEROOMRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        table_pushstring(L, "error_string", errorString.UTF8Representation().cstr());
        table_pushint(L, "local_player_nr", localPlayerNr);
        table_pushhashtable(L, "room_properties", roomProperties);
        table_pushhashtable(L, "player_properties", playerProperties);
        invokeCallback(L);
    }
}

void PhotonListener::joinRandomOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_JOINRANDOMORCREATEROOMRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        table_pushstring(L, "error_string", errorString.UTF8Representation().cstr());
        table_pushint(L, "local_player_nr", localPlayerNr);
        table_pushhashtable(L, "room_properties", roomProperties);
        table_pushhashtable(L, "player_properties", playerProperties);
        invokeCallback(L);
    }
}

void PhotonListener::joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_JOINROOMRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        table_pushstring(L, "error_string", errorString.UTF8Representation().cstr());
        table_pushint(L, "local_player_nr", localPlayerNr);
        table_pushhashtable(L, "room_properties", roomProperties);
        table_pushhashtable(L, "player_properties", playerProperties);
        invokeCallback(L);
    }
}

void PhotonListener::joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString)
{
    dmLogInfo("joinRandomRoomReturn");
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_JOINRANDOMROOMRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        table_pushstring(L, "error_string", errorString.UTF8Representation().cstr());
        table_pushint(L, "local_player_nr", localPlayerNr);
        table_pushhashtable(L, "room_properties", roomProperties);
        table_pushhashtable(L, "player_properties", playerProperties);
        invokeCallback(L);
    }
}

void PhotonListener::leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_LEAVEROOMRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        table_pushstring(L, "error_string", errorString.UTF8Representation().cstr());
        invokeCallback(L);
    }
}

void PhotonListener::joinLobbyReturn(void)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_JOINLOBBYRETURN))
    {
        lua_newtable(L);
        invokeCallback(L);
    }
}

void PhotonListener::leaveLobbyReturn(void)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_LEAVELOBBYRETURN))
    {
        lua_newtable(L);
        invokeCallback(L);
    }
}

void PhotonListener::onFindFriendsResponse(void)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONFINDFRIENDSRESPONSE))
    {
        lua_newtable(L);
        invokeCallback(L);
    }
}

void PhotonListener::onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONLOBBYSTATSRESPONSE))
    {
        lua_newtable(L);
        table_pushlobbystatsresponse(L, "lobby_stats", lobbyStats);
        invokeCallback(L);
    }
}

void PhotonListener::webRpcReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& uriPath, int resultCode, const ExitGames::Common::Dictionary<ExitGames::Common::Object, ExitGames::Common::Object>& returnData)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_WEBRPCRETURN))
    {
        lua_newtable(L);
        table_pushint(L, "error_code", errorCode);
        table_pushstring(L, "error_string", errorString.UTF8Representation().cstr());
        table_pushstring(L, "uri_path", uriPath.UTF8Representation().cstr());
        table_pushint(L, "result_code", resultCode);
        table_pushobjectdictionary(L, "return_data", returnData);
        invokeCallback(L);
    }
}


// info, that certain values have been updated
void PhotonListener::onRoomListUpdate(void)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONROOMLISTUPDATE))
    {
        lua_newtable(L);
        invokeCallback(L);
    }
}

void PhotonListener::onRoomPropertiesChange(const ExitGames::Common::Hashtable& changes)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONROOMPROPERTIESCHANGE))
    {
        lua_newtable(L);
        table_pushhashtable(L, "changes", changes);
        invokeCallback(L);
    }
}

void PhotonListener::onPlayerPropertiesChange(int playerNr, const ExitGames::Common::Hashtable& changes)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONPLAYERPROPERTIESCHANGE))
    {
        lua_newtable(L);
        table_pushint(L, "player_nr", playerNr);
        table_pushhashtable(L, "changes", changes);
        invokeCallback(L);
    }
}

void PhotonListener::onAppStatsUpdate(void)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONAPPSTATSUPDATE))
    {
        lua_newtable(L);
        invokeCallback(L);
    }
}

void PhotonListener::onLobbyStatsUpdate(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONLOBBYSTATSUPDATE))
    {
        lua_newtable(L);
        table_pushlobbystatsresponse(L, "lobby_stats", lobbyStats);
        invokeCallback(L);
    }
}

void PhotonListener::onCacheSliceChanged(int cacheSliceIndex)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONPLAYERPROPERTIESCHANGE))
    {
        lua_newtable(L);
        table_pushint(L, "cache_slice_index", cacheSliceIndex);
        invokeCallback(L);
    }
}

void PhotonListener::onMasterClientChanged(int id, int oldID)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONMASTERCLIENTCHANGED))
    {
        lua_newtable(L);
        table_pushint(L, "id", id);
        table_pushint(L, "old_id", oldID);
        invokeCallback(L);
    }
}


// custom authentication
void PhotonListener::onCustomAuthenticationIntermediateStep(const ExitGames::Common::Dictionary<ExitGames::Common::JString, ExitGames::Common::Object>& customResponseData)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONCUSTOMAUTHENTICATIONINTERMEDIATESTEP))
    {
        lua_newtable(L);
        table_pushstringdictionary(L, "custom_response_data", customResponseData);
        invokeCallback(L);
    }
}


// receive the available server regions during the connect workflow (if you have specified in the constructor, that you want to select a region)
void PhotonListener::onAvailableRegions(const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegions, const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegionServers)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONAVAILABLEREGIONS))
    {
        lua_newtable(L);
        table_pushstringvector(L, "available_regions", availableRegions);
        table_pushstringvector(L, "available_region_servers", availableRegionServers);
        invokeCallback(L);
    }
}



void PhotonListener::onSecretReceival(const ExitGames::Common::JString& secret)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONSECRETRECEIVAL))
    {
        lua_newtable(L);
        table_pushstring(L, "secret", secret.UTF8Representation().cstr());
        invokeCallback(L);
    }
}


void PhotonListener::onDirectConnectionEstablished(int remoteID)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONDIRECTCONNECTIONESTABLISHED))
    {
        lua_newtable(L);
        table_pushint(L, "remote_id", remoteID);
        invokeCallback(L);
    }
}

void PhotonListener::onDirectConnectionFailedToEstablish(int remoteID)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONDIRECTCONNECTIONFAILEDTOESTABLISH))
    {
        lua_newtable(L);
        table_pushint(L, "remote_id", remoteID);
        invokeCallback(L);
    }
}

void PhotonListener::onDirectMessage(const ExitGames::Common::Object& msg, int remoteID, bool relay)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONDIRECTMESSAGE))
    {
        lua_newtable(L);
        table_pushstring(L, "msg", msg.toString().UTF8Representation().cstr());
        table_pushint(L, "remote_id", remoteID);
        table_pushboolean(L, "relay", relay);
        invokeCallback(L);
    }
}


void PhotonListener::onCustomOperationResponse(const ExitGames::Photon::OperationResponse& operationResponse)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONCUSTOMOPERATIONRESPONSE))
    {
        lua_newtable(L);
        table_pushint(L, "operation_code", operationResponse.getOperationCode());
        table_pushint(L, "return_code", operationResponse.getReturnCode());
        invokeCallback(L);
    }
}


void PhotonListener::onGetRoomListResponse(const ExitGames::Common::JVector<ExitGames::Common::Helpers::SharedPointer<ExitGames::LoadBalancing::Room> >& roomList, const ExitGames::Common::JVector<ExitGames::Common::JString>& roomNameList)
{
    PHOTONCALLBACK_STACKCHECK(0);
    if (lua_State* L = setupCallback(EVENT_ONGETROOMLISTRESPONSE))
    {
        lua_newtable(L);

        lua_pushstring(L, "room_list");
        lua_newtable(L);
        for (unsigned int i = 0; i < roomList.getSize(); i++)
        {
            const ExitGames::Common::Helpers::SharedPointer<ExitGames::LoadBalancing::Room> room = roomList[i];
            lua_pushinteger(L, i + 1);
            lua_newtable(L);
            {
                table_pushstring(L, "name", room->getName().UTF8Representation().cstr());
                table_pushint(L, "max_players", room->getMaxPlayers());
                table_pushboolean(L, "is_open", room->getIsOpen());
                table_pushint(L, "direct_mode", room->getDirectMode());
            }
            lua_rawset(L, -3);
        }
        lua_rawset(L, -3);

        table_pushstringvector(L, "room_name_list", roomNameList);
        invokeCallback(L);
    }
}
