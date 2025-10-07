#pragma once

#include "LoadBalancing-cpp/inc/Listener.h"


class PhotonListener : public ExitGames::LoadBalancing::Listener
{
private:
    dmScript::LuaCallbackInfo* m_Callback;

    void destroyCallback();
    lua_State* setupCallback(int event);
    void invokeCallback(lua_State* L);

public:
    static const uint32_t EVENT_CONNECTIONERRORRETURN = 0;
    static const uint32_t EVENT_CLIENTERRORRETURN = 1;
    static const uint32_t EVENT_WARNINGRETURN = 2;
    static const uint32_t EVENT_SERVERERRORRETURN = 3;
    static const uint32_t EVENT_JOINROOMEVENTACTION = 4;
    static const uint32_t EVENT_LEAVEROOMEVENTACTION = 5;
    static const uint32_t EVENT_CUSTOMEVENTACTION = 6;
    static const uint32_t EVENT_CONNECTRETURN = 7;
    static const uint32_t EVENT_DISCONNECTRETURN = 8;
    static const uint32_t EVENT_CREATEROOMRETURN = 9;
    static const uint32_t EVENT_JOINORCREATEROOMRETURN = 10;
    static const uint32_t EVENT_JOINRANDOMORCREATEROOMRETURN = 11;
    static const uint32_t EVENT_JOINROOMRETURN = 12;
    static const uint32_t EVENT_JOINRANDOMROOMRETURN = 13;
    static const uint32_t EVENT_LEAVEROOMRETURN = 14;
    static const uint32_t EVENT_JOINLOBBYRETURN = 15;
    static const uint32_t EVENT_LEAVELOBBYRETURN = 16;
    static const uint32_t EVENT_ONFINDFRIENDSRESPONSE = 17;
    static const uint32_t EVENT_ONLOBBYSTATSRESPONSE = 18;
    static const uint32_t EVENT_WEBRPCRETURN = 19;
    static const uint32_t EVENT_ONROOMLISTUPDATE = 20;
    static const uint32_t EVENT_ONROOMPROPERTIESCHANGE = 21;
    static const uint32_t EVENT_ONPLAYERPROPERTIESCHANGE = 22;
    static const uint32_t EVENT_ONAPPSTATSUPDATE = 23;
    static const uint32_t EVENT_ONLOBBYSTATSUPDATE = 24;
    static const uint32_t EVENT_ONCACHESLICECHANGED = 25;
    static const uint32_t EVENT_ONMASTERCLIENTCHANGED = 26;
    static const uint32_t EVENT_ONCUSTOMAUTHENTICATIONINTERMEDIATESTEP = 27;
    static const uint32_t EVENT_ONAVAILABLEREGIONS = 28;
    static const uint32_t EVENT_ONSECRETRECEIVAL = 29;
    static const uint32_t EVENT_ONDIRECTCONNECTIONESTABLISHED = 30;
    static const uint32_t EVENT_ONDIRECTCONNECTIONFAILEDTOESTABLISH = 31;
    static const uint32_t EVENT_ONDIRECTMESSAGE = 32;
    static const uint32_t EVENT_ONCUSTOMOPERATIONRESPONSE = 33;
    static const uint32_t EVENT_ONGETROOMLISTRESPONSE = 34;

    PhotonListener(lua_State* L, int index);
    ~PhotonListener();

    //
    // ExitGames::LoadBalancing::Listener
    //
    // receive and print out debug out here
    void debugReturn(int debugLevel, const ExitGames::Common::JString& string);

    // implement your error-handling here
    void connectionErrorReturn(int errorCode);
    void clientErrorReturn(int errorCode);
    void warningReturn(int warningCode);
    void serverErrorReturn(int errorCode);

    // events, triggered by certain operations of all players in the same room
    void joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player);
    void leaveRoomEventAction(int playerNr, bool isInactive);

    void customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent);

    // callbacks for operations on the server
    void connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster);
    void disconnectReturn(void);
    void createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
    void joinOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
    void joinRandomOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
    void joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
    void joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
    void leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString);
    void joinLobbyReturn(void);
    void leaveLobbyReturn(void);
    void onFindFriendsResponse(void);
    void onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats);
    void webRpcReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& uriPath, int resultCode, const ExitGames::Common::Dictionary<ExitGames::Common::Object, ExitGames::Common::Object>& returnData);

    // info, that certain values have been updated
    void onRoomListUpdate(void);
    void onRoomPropertiesChange(const ExitGames::Common::Hashtable& changes);
    void onPlayerPropertiesChange(int playerNr, const ExitGames::Common::Hashtable& changes);
    void onAppStatsUpdate(void);
    void onLobbyStatsUpdate(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats);
    void onCacheSliceChanged(int cacheSliceIndex);
    void onMasterClientChanged(int id, int oldID);

    // custom authentication
    void onCustomAuthenticationIntermediateStep(const ExitGames::Common::Dictionary<ExitGames::Common::JString, ExitGames::Common::Object>& customResponseData);

    // receive the available server regions during the connect workflow (if you have specified in the constructor, that you want to select a region)
    void onAvailableRegions(const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegions, const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegionServers);

    void onSecretReceival(const ExitGames::Common::JString& secret);

    void onDirectConnectionEstablished(int remoteID);
    void onDirectConnectionFailedToEstablish(int remoteID);
    void onDirectMessage(const ExitGames::Common::Object& msg, int remoteID, bool relay);

    void onCustomOperationResponse(const ExitGames::Photon::OperationResponse& operationResponse);

    void onGetRoomListResponse(const ExitGames::Common::JVector<ExitGames::Common::Helpers::SharedPointer<ExitGames::LoadBalancing::Room> >& roomList, const ExitGames::Common::JVector<ExitGames::Common::JString>& roomNameList);
};
