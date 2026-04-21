/** Photon Realtime
 * Functions and constants for interacting with Photon Realtime
 * @document
 * @namespace realtime
 */

#define EXTENSION_NAME Realtime
#define LIB_NAME "Realtime"
#define MODULE_NAME "realtime"

#ifndef DLIB_LOG_DOMAIN
#define DLIB_LOG_DOMAIN "realtime"
#endif

#include <dmsdk/sdk.h>

#include "photon_extension_defines.h"

#if PHOTON_PLATFORM_SUPPORTED

#include "LoadBalancing-cpp/inc/Client.h"
#include "listener.h"
#include "helper.h"


static ExitGames::LoadBalancing::Client* g_PhotonClient;
static PhotonListener* g_PhotonListener = 0x0;



struct JoinRoomOptions
{
    ExitGames::Common::Hashtable customRoomProperties;
    nByte maxPlayers;
    nByte matchmakingMode;
    ExitGames::Common::JString lobbyName;
    nByte lobbyType;
    ExitGames::Common::JString sqlLobbyFilter;
    ExitGames::Common::JVector<ExitGames::Common::JString> expectedUsers;
    bool rejoin;
    int cacheSliceIndex;

    JoinRoomOptions()
    {
        customRoomProperties = ExitGames::Common::Hashtable();
        maxPlayers = 0;
        matchmakingMode = ExitGames::LoadBalancing::MatchmakingMode::FILL_ROOM;
        lobbyName = ExitGames::Common::JString();
        lobbyType = ExitGames::LoadBalancing::LobbyType::DEFAULT;
        sqlLobbyFilter = ExitGames::Common::JString();
        expectedUsers = ExitGames::Common::JVector<ExitGames::Common::JString>();
        rejoin = false;
        cacheSliceIndex = 0;
    }
};




static void check_stringarray(lua_State* L, int index, ExitGames::Common::JVector<ExitGames::Common::JString>* out)
{
    if (lua_type(L, index) == LUA_TTABLE)
    {
        lua_pushvalue(L, index);
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            int i = luaL_checknumber(L, -2);
            const char* value = luaL_checkstring(L, -1);
            out->setElementAt(value, i);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
}


static void check_hashtable(lua_State* L, int index, ExitGames::Common::Hashtable* out)
{
    if (lua_type(L, index) == LUA_TTABLE)
    {
        lua_pushvalue(L, index);
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            const char* key = luaL_checkstring(L, -2);
            const char* value = luaL_checkstring(L, -1);
            out->put(key, value);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
}

static void check_roomoptions(lua_State* L, int index, ExitGames::LoadBalancing::RoomOptions* out)
{
    if (lua_type(L, index) == LUA_TTABLE)
    {
        lua_pushvalue(L, index);
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            const char* key = luaL_checkstring(L, -2);
            if (dmStrCaseCmp(key, "is_visible") == 0)
            {
                out->setIsVisible(lua_toboolean(L, -1));
                dmLogInfo("IsVisible %d", out->getIsVisible());
            }
            else if (dmStrCaseCmp(key, "is_open") == 0)
            {
                out->setIsOpen(lua_toboolean(L, -1));
            }
            else if (dmStrCaseCmp(key, "supress_room_events") == 0)
            {
                out->setSuppressRoomEvents(lua_toboolean(L, -1));
            }
            else if (dmStrCaseCmp(key, "max_players") == 0)
            {
                out->setMaxPlayers(luaL_checknumber(L, -1));
            }
            else if (dmStrCaseCmp(key, "player_ttl") == 0)
            {
                out->setPlayerTtl(luaL_checknumber(L, -1));
            }
            else if (dmStrCaseCmp(key, "empty_room_ttl") == 0)
            {
                out->setEmptyRoomTtl(luaL_checknumber(L, -1));
            }
            else if (dmStrCaseCmp(key, "lobby_type") == 0)
            {
                out->setLobbyType(luaL_checknumber(L, -1));
            }
            else if (dmStrCaseCmp(key, "lobby_name") == 0)
            {
                out->setLobbyName(luaL_checkstring(L, -1));
            }
            else if (dmStrCaseCmp(key, "publish_user_id") == 0)
            {
                out->setPublishUserID(lua_toboolean(L, -1));
            }
            else if (dmStrCaseCmp(key, "direct_mode") == 0)
            {
                out->setDirectMode(luaL_checknumber(L, -1));
            }
            else if (dmStrCaseCmp(key, "props_listed_in_lobby") == 0)
            {
                ExitGames::Common::JVector<ExitGames::Common::JString> propsListedInLobby = ExitGames::Common::JVector<ExitGames::Common::JString>();
                check_stringarray(L, -1, &propsListedInLobby);
                out->setPropsListedInLobby(propsListedInLobby);
            }
            else if (dmStrCaseCmp(key, "custom_room_properties") == 0)
            {
                ExitGames::Common::Hashtable customRoomProperties = ExitGames::Common::Hashtable();
                check_hashtable(L, -1, &customRoomProperties);
                out->setCustomRoomProperties(customRoomProperties);
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
}


static void check_joinroomoptions(lua_State* L, int index, JoinRoomOptions* out)
{
    if (lua_type(L, index) == LUA_TTABLE)
    {
        lua_pushvalue(L, index);
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            const char* key = luaL_checkstring(L, -2);
            if (dmStrCaseCmp(key, "custom_room_properties") == 0)
            {
                check_hashtable(L, -1, &out->customRoomProperties);
            }
            else if (dmStrCaseCmp(key, "max_players") == 0)
            {
                out->maxPlayers = luaL_checknumber(L, -1);
            }
            else if (dmStrCaseCmp(key, "matchmaking_mode") == 0)
            {
                out->matchmakingMode = luaL_checknumber(L, -1);
            }
            else if (dmStrCaseCmp(key, "lobby_name") == 0)
            {
                out->lobbyName = luaL_checkstring(L, -1);
            }
            else if (dmStrCaseCmp(key, "lobby_type") == 0)
            {
                out->lobbyType = luaL_checknumber(L, -1);
            }
            else if (dmStrCaseCmp(key, "sql_lobby_filter") == 0)
            {
                out->sqlLobbyFilter = luaL_checkstring(L, -1);
            }
            else if (dmStrCaseCmp(key, "expected_users") == 0)
            {
                check_stringarray(L, -1, &out->expectedUsers);
            }
            else if (dmStrCaseCmp(key, "rejoin") == 0)
            {
                out->rejoin = lua_toboolean(L, -1);
            }
            else if (dmStrCaseCmp(key, "cache_slice_index") == 0 == 0)
            {
                out->cacheSliceIndex = luaL_checknumber(L, -1);
            }
            else
            {
                dmLogInfo("Join room options contains unknown key %s", key);
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
}


static void check_int_array(lua_State* L, int index, int* out)
{
    if (lua_type(L, index) == LUA_TTABLE)
    {
        lua_pushvalue(L, index);
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            const int index = luaL_checknumber(L, -2);
            const int value = luaL_checknumber(L, -1);
            out[index] = value;
        }
    }
}

static void check_raiseeventoptions(lua_State* L, int index, ExitGames::LoadBalancing::RaiseEventOptions* out)
{
    if (lua_type(L, index) == LUA_TTABLE)
    {
        lua_pushvalue(L, index);
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            const char* key = luaL_checkstring(L, -2);
            if (dmStrCaseCmp(key, "channel_id") == 0)
            {
                out->setChannelID(luaL_checknumber(L, -1));
            }
            else if (dmStrCaseCmp(key, "event_caching") == 0)
            {
                out->setEventCaching(luaL_checknumber(L, -1));
            }
            else if (dmStrCaseCmp(key, "receiver_group") == 0)
            {
                out->setReceiverGroup(luaL_checknumber(L, -1));
            }
            else if (dmStrCaseCmp(key, "interest_group") == 0)
            {
                out->setInterestGroup(luaL_checknumber(L, -1));
            }
            else if (dmStrCaseCmp(key, "cache_slice_index") == 0)
            {
                out->setCacheSliceIndex(luaL_checknumber(L, -1));
            }
            else if (dmStrCaseCmp(key, "target_players") == 0)
            {
                const size_t size = lua_objlen(L, -1);
                int array[size];
                check_int_array(L, -1, array);
                out->setTargetPlayers(array, size);
            }
            else if (dmStrCaseCmp(key, "web_flags") == 0)
            {
                ExitGames::LoadBalancing::WebFlags flags;
                flags.setFlags(luaL_checknumber(L, -1));
                out->setWebFlags(flags);
            }
            else if (dmStrCaseCmp(key, "encrypt") == 0)
            {
                out->setEncrypt(lua_toboolean(L, -1));
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
}



/*******************************************
 * LIFECYCLE
 *******************************************/

/** Initialize Realtime by creating a load balanced client
 * @name init
 * @string app_id
 * @string app_version
 * @function listener
 */
static int Init(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    dmLogInfo("init");

    const char* appId = luaL_checkstring(L, 1);
    const char* appVersion = luaL_checkstring(L, 2);

    if (g_PhotonListener)
    {
        delete g_PhotonListener;
    }
    g_PhotonListener = new PhotonListener(L, 3);

    ExitGames::LoadBalancing::ClientConstructOptions options = ExitGames::LoadBalancing::ClientConstructOptions();
    if (g_PhotonClient)
    {
        delete g_PhotonClient;
    }
    g_PhotonClient = new ExitGames::LoadBalancing::Client(*g_PhotonListener, appId, appVersion, options);

    return 0;
}

/** Connect to the server.
 * @name connect
 * @table options Table with connection options
 * @field username string
 * @field use_datagram_encryption boolean
 * @field server_type number See SERVER_TYPE_* enums
 * @field server_address string The IP address or domain name and optionally the port number to connect to. IP addresses can be in IPv4 or IPv6 format
 * @field auth_data string Sets the HTTP POST data, that will be forwarded to the authentication service, to the provided data.
 * @field auth_parameters string The HTTP GET parameters that will be forwarded to the authentication service to the provided parameters.
 * @field auth_type number The type of the "Custom Authentication" service that will be used. See AUTH_* enums
 */
static int Connect(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    ExitGames::LoadBalancing::ConnectOptions connectOptions = ExitGames::LoadBalancing::ConnectOptions();
    ExitGames::LoadBalancing::AuthenticationValues authValues = ExitGames::LoadBalancing::AuthenticationValues();
    if (lua_type(L, 1) == LUA_TTABLE)
    {
        lua_pushvalue(L, 1);
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            const char* key = luaL_checkstring(L, -2);
            if (dmStrCaseCmp(key, "username") == 0)
            {
                const char* username = luaL_checkstring(L, -1);
                connectOptions.setUsername(username);
                authValues.setUserID(username);
            }
            else if (dmStrCaseCmp(key, "use_datagram_encryption") == 0)
            {
                bool useDatagramEncryption = lua_toboolean(L, -1);
                connectOptions.setTryUseDatagramEncryption(useDatagramEncryption);
            }
            else if (dmStrCaseCmp(key, "server_type") == 0)
            {
                uint32_t serverType = luaL_checknumber(L, -1);
                connectOptions.setServerType(serverType);
            }
            else if (dmStrCaseCmp(key, "server_address") == 0)
            {
                const char* serverAddress = luaL_checkstring(L, -1);
                connectOptions.setServerAddress(serverAddress);
            }
            else if (dmStrCaseCmp(key, "auth_data") == 0)
            {
                const char* authData = luaL_checkstring(L, -1);
                authValues.setData(authData);
            }
            else if (dmStrCaseCmp(key, "auth_parameters") == 0)
            {
                const char* authParameters = luaL_checkstring(L, -1);
                authValues.setParameters(authParameters);
            }
            else if (dmStrCaseCmp(key, "auth_type") == 0)
            {
                uint32_t authType = luaL_checknumber(L, -1);
                authValues.setType(authType);
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }

    // do not use a background thread for sending and receiving data
    // if we ever make this user controllable then we need to synchronise
    // callbacks using a queue and only call Lua code on the main thread
    connectOptions.setUseBackgroundSendReceiveThread(false);
    connectOptions.setAuthenticationValues(authValues);
    g_PhotonClient->connect(connectOptions);
    return 0;
}

/** Update Realtime.
 * Call this from a script component.
 * @name update
 */
static int Update(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    g_PhotonClient->service();
    return 0;
}

/** Disconnect from server.
 * @name disconnect
 */
static int Disconnect(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    g_PhotonClient->disconnect();
    return 0;
}


/** Join lobby.
 * @name join_lobby
 * @string lobby_name
 * @number lobby_type
 */
static int JoinLobby(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char* lobbyName = luaL_checkstring(L, 1);
    nByte lobbyType = luaL_checknumber(L, 2);
    g_PhotonClient->opJoinLobby(
        lobbyName,
        lobbyType);

    return 0;
}

/** Leave lobby.
 * @name leave_lobby
 */
static int LeaveLobby(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    g_PhotonClient->opLeaveLobby();

    return 0;
}

/** Create room.
 * @name create_room
 * @string game_id The name to create a room with. Must be unique and not in use or the room can't be created. If this is an empty string, then the server will assign a GUID as name.
 * @table room_options
 * @field is_visible boolean
 * @field is_open boolean
 * @field supress_room_events boolean
 * @field max_players number
 * @field player_ttl number
 * @field empty_room_ttl number
 * @field lobby_type number realtime.LOBBYTYPE_* enum
 * @field lobby_name string
 * @field publish_user_id string
 * @field direct_mode numbmer realtime.DIRECTMODE_* enum
 * @field props_listed_in_lobby table
 * @field custom_room_properties table
 * @table expected_users
 * 
 * @example
 *   local game_id = "1234"
 *   local room_options = {
 *      is_visible = false,
 *      is_open = false,
 *      supress_room_events = false,
 *      max_players = 4,
 *      player_ttl = 0,
 *      empty_room_ttl = 0,
 *      lobby_type = realtime.LOBBYTYPE_DEFAULT,
 *      lobby_name = "",
 *      publish_user_id = "",
 *      direct_mode = realtime.DIRECTMODE_NONE,
 *      props_listed_in_lobby = {},
 *      custom_room_properties = {},
 *   }
 *   local expected_users = {}
 *   realtime.create_room(game_id, room_options, expected_users)
 */
static int CreateRoom(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char* gameID = luaL_checkstring(L, 1);
    ExitGames::LoadBalancing::RoomOptions roomOptions = ExitGames::LoadBalancing::RoomOptions();
    check_roomoptions(L, 2, &roomOptions);
    ExitGames::Common::JVector<ExitGames::Common::JString> expectedUsers = ExitGames::Common::JVector<ExitGames::Common::JString>();
    check_stringarray(L, 3, &expectedUsers);

    g_PhotonClient->opCreateRoom(
        gameID,
        roomOptions,
        expectedUsers);

    return 0;
}


/** Join room.
 * @name join_room
 * @string game_id The name to create a room with. Must be unique and not in use or the room can't be created. If this is an empty string, then the server will assign a GUID as name.
 * @table join_options
 * @field rejoin boolean
 * @field cache_slice_index number
 * @field expected_users table
 * 
 * @example
 *   local game_id = "1234"
 *   local join_options = {
 *      rejoin = false,
 *      cache_slice_index = 0,
 *      expected_users = {},
 *   }
 *   realtime.join_room(game_id, join_options)
 */
static int JoinRoom(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char* gameID = luaL_checkstring(L, 1);
    JoinRoomOptions joinRoomOptions = JoinRoomOptions();
    check_joinroomoptions(L, 2, &joinRoomOptions);
            
    g_PhotonClient->opJoinRoom(
        gameID,
        joinRoomOptions.rejoin,
        joinRoomOptions.cacheSliceIndex,
        joinRoomOptions.expectedUsers);

    return 0;
}

/** Join or create room.
 * @name join_or_create_room
 * @string game_id The name to create a room with. Must be unique and not in use or the room can't be created. If this is an empty string, then the server will assign a GUID as name.
 * @table room_options
 * @field is_visible boolean
 * @field is_open boolean
 * @field supress_room_events boolean
 * @field max_players number
 * @field player_ttl number
 * @field empty_room_ttl number
 * @field lobby_type number realtime.LOBBYTYPE_* enum
 * @field lobby_name string
 * @field publish_user_id string
 * @field direct_mode numbmer realtime.DIRECTMODE_* enum
 * @field props_listed_in_lobby table
 * @field custom_room_properties table
 * @table join_options
 * @field custom_room_properties table
 * @field max_players number
 * @field matchmaking_mode number realtime.MATCHMAKINGMODE_* enum
 * @field lobby_name string
 * @field lobby_type number realtime.LOBBYTYPE_* enum
 * @field sql_lobby_filter string
 * @field expected_users table
 * 
 * @example
 *   local game_id = "1234"
 *   local room_options = {
 *      is_visible = false,
 *      is_open = false,
 *      supress_room_events = false,
 *      max_players = 4,
 *      player_ttl = 0,
 *      empty_room_ttl = 0,
 *      lobby_type = realtime.LOBBYTYPE_DEFAULT,
 *      lobby_name = "",
 *      publish_user_id = "",
 *      direct_mode = realtime.DIRECTMODE_NONE,
 *      props_listed_in_lobby = {},
 *      custom_room_properties = {},
 *   }
 *   local join_options = {
 *      custom_room_properties = {},
 *      max_players = 0,
 *      matchmaking_mode = realtime.MATCHMAKINGMODE_FILL_ROOM,
 *      lobby_name = "",
 *      lobby_type = realtime.LOBBYTYPE_DEFAULT,
 *      sql_lobby_filter = "",
 *      expected_users = {},
 *   }
 */
static int JoinOrCreateRoom(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char* gameID = luaL_checkstring(L, 1);
    ExitGames::LoadBalancing::RoomOptions roomOptions = ExitGames::LoadBalancing::RoomOptions();
    check_roomoptions(L, 2, &roomOptions);
    JoinRoomOptions joinRoomOptions = JoinRoomOptions();
    check_joinroomoptions(L, 3, &joinRoomOptions);

    g_PhotonClient->opJoinRandomOrCreateRoom(
        gameID,
        roomOptions,
        joinRoomOptions.customRoomProperties,
        joinRoomOptions.maxPlayers,
        joinRoomOptions.matchmakingMode,
        joinRoomOptions.lobbyName,
        joinRoomOptions.lobbyType,
        joinRoomOptions.sqlLobbyFilter,
        joinRoomOptions.expectedUsers);

    return 0;
}


/** Join or create random room.
 * @name join_or_create_random_room
 * @string game_id The name to create a room with. Must be unique and not in use or the room can't be created. If this is an empty string, then the server will assign a GUID as name.
 * @table room_options
 * @field is_visible boolean
 * @field is_open boolean
 * @field supress_room_events boolean
 * @field max_players number
 * @field player_ttl number
 * @field empty_room_ttl number
 * @field lobby_type number realtime.LOBBYTYPE_* enum
 * @field lobby_name string
 * @field publish_user_id string
 * @field direct_mode numbmer realtime.DIRECTMODE_* enum
 * @field props_listed_in_lobby table
 * @field custom_room_properties table
 * @table join_options
 * @field custom_room_properties table
 * @field max_players number
 * @field matchmaking_mode number realtime.MATCHMAKINGMODE_* enum
 * @field lobby_name string
 * @field lobby_type number realtime.LOBBYTYPE_* enum
 * @field sql_lobby_filter string
 * @field expected_users table
 * 
 * @example
 *   local game_id = "1234"
 *   local room_options = {
 *      is_visible = false,
 *      is_open = false,
 *      supress_room_events = false,
 *      max_players = 4,
 *      player_ttl = 0,
 *      empty_room_ttl = 0,
 *      lobby_type = realtime.LOBBYTYPE_DEFAULT,
 *      lobby_name = "",
 *      publish_user_id = "",
 *      direct_mode = realtime.DIRECTMODE_NONE,
 *      props_listed_in_lobby = {},
 *      custom_room_properties = {},
 *   }
 *   local join_options = {
 *      custom_room_properties = {},
 *      max_players = 0,
 *      matchmaking_mode = realtime.MATCHMAKINGMODE_FILL_ROOM,
 *      lobby_name = "",
 *      lobby_type = realtime.LOBBYTYPE_DEFAULT,
 *      sql_lobby_filter = "",
 *      expected_users = {},
 *   }
 *   realtime.join_or_create_random_room(game_id, room_options, join_options)
 */
static int JoinOrCreateRandomRoom(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char* gameID = luaL_checkstring(L, 1);
    ExitGames::LoadBalancing::RoomOptions roomOptions = ExitGames::LoadBalancing::RoomOptions();
    check_roomoptions(L, 2, &roomOptions);
    JoinRoomOptions joinRoomOptions = JoinRoomOptions();
    check_joinroomoptions(L, 3, &joinRoomOptions);

    g_PhotonClient->opJoinRandomOrCreateRoom(
        gameID,
        roomOptions,
        joinRoomOptions.customRoomProperties,
        joinRoomOptions.maxPlayers,
        joinRoomOptions.matchmakingMode,
        joinRoomOptions.lobbyName,
        joinRoomOptions.lobbyType,
        joinRoomOptions.sqlLobbyFilter,
        joinRoomOptions.expectedUsers);

    return 0;
}


/** Join random room.
 * @name join_random_room
 * @table join_options
 * @field custom_room_properties table
 * @field max_players number
 * @field matchmaking_mode number realtime.MATCHMAKINGMODE_* enum
 * @field lobby_name string
 * @field lobby_type number realtime.LOBBYTYPE_* enum
 * @field sql_lobby_filter string
 * @field expected_users table
 * 
 * @example
 *   local join_options = {
 *      custom_room_properties = {},
 *      max_players = 0,
 *      matchmaking_mode = realtime.MATCHMAKINGMODE_FILL_ROOM,
 *      lobby_name = "",
 *      lobby_type = realtime.LOBBYTYPE_DEFAULT,
 *      sql_lobby_filter = "",
 *      expected_users = {},
 *   }
 *   realtime.join_random_room(join_options)
 */
static int JoinRandomRoom(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    JoinRoomOptions joinRoomOptions = JoinRoomOptions();
    check_joinroomoptions(L, 1, &joinRoomOptions);

    g_PhotonClient->opJoinRandomRoom(
        joinRoomOptions.customRoomProperties,
        joinRoomOptions.maxPlayers,
        joinRoomOptions.matchmakingMode,
        joinRoomOptions.lobbyName,
        joinRoomOptions.lobbyType,
        joinRoomOptions.sqlLobbyFilter,
        joinRoomOptions.expectedUsers);

    return 0;
}

/** Leave room.
 * @name leave_room
 * @boolean will_come_back
 * @boolean send_auth_cookie
 */
static int LeaveRoom(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    bool willComeBack = lua_toboolean(L, 1);
    bool sendAuthCookie = lua_toboolean(L, 2);

    g_PhotonClient->opLeaveRoom(
        willComeBack,
        sendAuthCookie);

    return 0;
}


/** Get room list.
 * @name get_room_list
 * @return room_list
 */
static int GetRoomList(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    lua_newtable(L);
    const ExitGames::Common::JVector<ExitGames::LoadBalancing::Room*> roomList = g_PhotonClient->getRoomList();
    for (unsigned int i = 0; i < roomList.getSize(); ++i)
    {
        lua_pushnumber(L, i + 1);
        lua_pushroom(L, roomList[i]);
        lua_rawset(L, -3);
    }

    return 1;
}


/** Sends in-game data to other players in the game.
 * @name raise_event
 * @boolean reliable
 * @number parameter
 * @number event_code
 * @table options (channel_id, event_caching, receiver_group, interest_group, cache_slice_index, target_players, web_flags, encrypt)
 * @field channel_id number
 * @field event_caching number
 * @field receiver_group number
 * @field interest_group number
 * @field cache_slice_index number
 * @field target_players number
 * @field web_flags number
 * @field encrypt boolean
 * @treturn boolean 
 */
static int RaiseEvent(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    bool reliable = lua_toboolean(L, 1);

    lua_Number parameters = luaL_checknumber(L, 2);

    lua_Number eventCode = luaL_checknumber(L, 3);

    ExitGames::LoadBalancing::RaiseEventOptions options = ExitGames::LoadBalancing::RaiseEventOptions();
    if (lua_type(L, 4) == LUA_TTABLE)
    {
        check_raiseeventoptions(L, 4, &options);
    }

    bool success = g_PhotonClient->opRaiseEvent(reliable, parameters, eventCode, options);
    lua_pushboolean(L, success);
    return 1;
}

/** Set the auto join lobby flag.
 * The value of the autoJoinLobby flag determines if the client will
 * automatically join the default lobby whenever it has successfully connected
 * and whenever it leaves a game room.
 * @name set_auto_join_lobby
 * @boolean auto_join
 */
static int SetAutoJoinLobby(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    bool autoJoin = lua_toboolean(L, 1);
    g_PhotonClient->setAutoJoinLobby(autoJoin);

    return 0;
}


static const luaL_reg Module_methods[] = {
    { "init", Init },
    { "connect", Connect },
    { "disconnect", Disconnect },
    { "update", Update },
    { "join_lobby", JoinLobby },
    { "leave_lobby", LeaveLobby },
    { "join_or_create_random_room", JoinOrCreateRandomRoom },
    { "join_or_create_room", JoinOrCreateRoom },
    { "join_random_room", JoinRandomRoom },
    { "join_room", JoinRoom },
    { "create_room", CreateRoom },
    { "leave_room", LeaveRoom },
    { "get_room_list", GetRoomList },
    { "set_auto_join_lobby", SetAutoJoinLobby },
    { "raise_event", RaiseEvent },
    { 0, 0 }
};


static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);

    #define SETCONSTANT(name, val) \
    lua_pushnumber(L, (lua_Number) val); \
    lua_setfield(L, -2, #name);

    /**
     * EVENT_CONNECTIONERRORRETURN
     * @field EVENT_CONNECTIONERRORRETURN
     */
    SETCONSTANT(EVENT_CONNECTIONERRORRETURN, PhotonListener::EVENT_CONNECTIONERRORRETURN)

    /**
     * EVENT_CLIENTERRORRETURN
     * @field EVENT_CLIENTERRORRETURN
     */
     SETCONSTANT(EVENT_CLIENTERRORRETURN, PhotonListener::EVENT_CLIENTERRORRETURN)

    /**
     * EVENT_WARNINGRETURN
     * @field EVENT_WARNINGRETURN
     */
     SETCONSTANT(EVENT_WARNINGRETURN, PhotonListener::EVENT_WARNINGRETURN)

    /**
     * EVENT_SERVERERRORRETURN
     * @field EVENT_SERVERERRORRETURN
     */
     SETCONSTANT(EVENT_SERVERERRORRETURN, PhotonListener::EVENT_SERVERERRORRETURN)

    /**
     * EVENT_JOINROOMEVENTACTION
     * @field EVENT_JOINROOMEVENTACTION
     */
     SETCONSTANT(EVENT_JOINROOMEVENTACTION, PhotonListener::EVENT_JOINROOMEVENTACTION)

    /**
     * EVENT_LEAVEROOMEVENTACTION
     * @field EVENT_LEAVEROOMEVENTACTION
     */
     SETCONSTANT(EVENT_LEAVEROOMEVENTACTION, PhotonListener::EVENT_LEAVEROOMEVENTACTION)

    /**
     * EVENT_CUSTOMEVENTACTION
     * @field EVENT_CUSTOMEVENTACTION
     */
     SETCONSTANT(EVENT_CUSTOMEVENTACTION, PhotonListener::EVENT_CUSTOMEVENTACTION)

    /**
     * EVENT_CONNECTRETURN
     * @field EVENT_CONNECTRETURN
     */
     SETCONSTANT(EVENT_CONNECTRETURN, PhotonListener::EVENT_CONNECTRETURN)

    /**
     * EVENT_DISCONNECTRETURN
     * @field EVENT_DISCONNECTRETURN
     */
     SETCONSTANT(EVENT_DISCONNECTRETURN, PhotonListener::EVENT_DISCONNECTRETURN)

    /**
     * EVENT_CREATEROOMRETURN
     * @field EVENT_CREATEROOMRETURN
     */
     SETCONSTANT(EVENT_CREATEROOMRETURN, PhotonListener::EVENT_CREATEROOMRETURN)

    /**
     * EVENT_JOINORCREATEROOMRETURN
     * @field EVENT_JOINORCREATEROOMRETURN
     */
     SETCONSTANT(EVENT_JOINORCREATEROOMRETURN, PhotonListener::EVENT_JOINORCREATEROOMRETURN)

    /**
     * EVENT_JOINRANDOMORCREATEROOMRETURN
     * @field EVENT_JOINRANDOMORCREATEROOMRETURN
     */
     SETCONSTANT(EVENT_JOINRANDOMORCREATEROOMRETURN, PhotonListener::EVENT_JOINRANDOMORCREATEROOMRETURN)

    /**
     * EVENT_JOINROOMRETURN
     * @field EVENT_JOINROOMRETURN
     */
     SETCONSTANT(EVENT_JOINROOMRETURN, PhotonListener::EVENT_JOINROOMRETURN)

    /**
     * EVENT_JOINRANDOMROOMRETURN
     * @field EVENT_JOINRANDOMROOMRETURN
     */
     SETCONSTANT(EVENT_JOINRANDOMROOMRETURN, PhotonListener::EVENT_JOINRANDOMROOMRETURN)

    /**
     * EVENT_LEAVEROOMRETURN
     * @field EVENT_LEAVEROOMRETURN
     */
     SETCONSTANT(EVENT_LEAVEROOMRETURN, PhotonListener::EVENT_LEAVEROOMRETURN)

    /**
     * EVENT_JOINLOBBYRETURN
     * @field EVENT_JOINLOBBYRETURN
     */
     SETCONSTANT(EVENT_JOINLOBBYRETURN, PhotonListener::EVENT_JOINLOBBYRETURN)

    /**
     * EVENT_LEAVELOBBYRETURN
     * @field EVENT_LEAVELOBBYRETURN
     */
     SETCONSTANT(EVENT_LEAVELOBBYRETURN, PhotonListener::EVENT_LEAVELOBBYRETURN)

    /**
     * EVENT_ONFINDFRIENDSRESPONSE
     * @field EVENT_ONFINDFRIENDSRESPONSE
     */
     SETCONSTANT(EVENT_ONFINDFRIENDSRESPONSE, PhotonListener::EVENT_ONFINDFRIENDSRESPONSE)

    /**
     * EVENT_ONLOBBYSTATSRESPONSE
     * @field EVENT_ONLOBBYSTATSRESPONSE
     */
     SETCONSTANT(EVENT_ONLOBBYSTATSRESPONSE, PhotonListener::EVENT_ONLOBBYSTATSRESPONSE)

    /**
     * EVENT_WEBRPCRETURN
     * @field EVENT_WEBRPCRETURN
     */
     SETCONSTANT(EVENT_WEBRPCRETURN, PhotonListener::EVENT_WEBRPCRETURN)

    /**
     * EVENT_ONROOMLISTUPDATE
     * @field EVENT_ONROOMLISTUPDATE
     */
     SETCONSTANT(EVENT_ONROOMLISTUPDATE, PhotonListener::EVENT_ONROOMLISTUPDATE)

    /**
     * EVENT_ONROOMPROPERTIESCHANGE
     * @field EVENT_ONROOMPROPERTIESCHANGE
     */
     SETCONSTANT(EVENT_ONROOMPROPERTIESCHANGE, PhotonListener::EVENT_ONROOMPROPERTIESCHANGE)

    /**
     * EVENT_ONPLAYERPROPERTIESCHANGE
     * @field EVENT_ONPLAYERPROPERTIESCHANGE
     */
     SETCONSTANT(EVENT_ONPLAYERPROPERTIESCHANGE, PhotonListener::EVENT_ONPLAYERPROPERTIESCHANGE)

    /**
     * EVENT_ONAPPSTATSUPDATE
     * @field EVENT_ONAPPSTATSUPDATE
     */
     SETCONSTANT(EVENT_ONAPPSTATSUPDATE, PhotonListener::EVENT_ONAPPSTATSUPDATE)

    /**
     * EVENT_ONLOBBYSTATSUPDATE
     * @field EVENT_ONLOBBYSTATSUPDATE
     */
     SETCONSTANT(EVENT_ONLOBBYSTATSUPDATE, PhotonListener::EVENT_ONLOBBYSTATSUPDATE)

    /**
     * EVENT_ONCACHESLICECHANGED
     * @field EVENT_ONCACHESLICECHANGED
     */
     SETCONSTANT(EVENT_ONCACHESLICECHANGED, PhotonListener::EVENT_ONCACHESLICECHANGED)

    /**
     * EVENT_ONMASTERCLIENTCHANGED
     * @field EVENT_ONMASTERCLIENTCHANGED
     */
     SETCONSTANT(EVENT_ONMASTERCLIENTCHANGED, PhotonListener::EVENT_ONMASTERCLIENTCHANGED)

    /**
     * EVENT_ONCUSTOMAUTHENTICATIONINTERMEDIATESTEP
     * @field EVENT_ONCUSTOMAUTHENTICATIONINTERMEDIATESTEP
     */
     SETCONSTANT(EVENT_ONCUSTOMAUTHENTICATIONINTERMEDIATESTEP, PhotonListener::EVENT_ONCUSTOMAUTHENTICATIONINTERMEDIATESTEP)

    /**
     * EVENT_ONAVAILABLEREGIONS
     * @field EVENT_ONAVAILABLEREGIONS
     */
     SETCONSTANT(EVENT_ONAVAILABLEREGIONS, PhotonListener::EVENT_ONAVAILABLEREGIONS)

    /**
     * EVENT_ONSECRETRECEIVAL
     * @field EVENT_ONSECRETRECEIVAL
     */
     SETCONSTANT(EVENT_ONSECRETRECEIVAL, PhotonListener::EVENT_ONSECRETRECEIVAL)

    /**
     * EVENT_ONDIRECTCONNECTIONESTABLISHED
     * @field EVENT_ONDIRECTCONNECTIONESTABLISHED
     */
     SETCONSTANT(EVENT_ONDIRECTCONNECTIONESTABLISHED, PhotonListener::EVENT_ONDIRECTCONNECTIONESTABLISHED)

    /**
     * EVENT_ONDIRECTCONNECTIONFAILEDTOESTABLISH
     * @field EVENT_ONDIRECTCONNECTIONFAILEDTOESTABLISH
     */
     SETCONSTANT(EVENT_ONDIRECTCONNECTIONFAILEDTOESTABLISH, PhotonListener::EVENT_ONDIRECTCONNECTIONFAILEDTOESTABLISH)

    /**
     * EVENT_ONDIRECTMESSAGE
     * @field EVENT_ONDIRECTMESSAGE
     */
     SETCONSTANT(EVENT_ONDIRECTMESSAGE, PhotonListener::EVENT_ONDIRECTMESSAGE)

    /**
     * EVENT_ONCUSTOMOPERATIONRESPONSE
     * @field EVENT_ONCUSTOMOPERATIONRESPONSE
     */
     SETCONSTANT(EVENT_ONCUSTOMOPERATIONRESPONSE, PhotonListener::EVENT_ONCUSTOMOPERATIONRESPONSE)

    /**
     * EVENT_ONGETROOMLISTRESPONSE
     * @field EVENT_ONGETROOMLISTRESPONSE
     */
     SETCONSTANT(EVENT_ONGETROOMLISTRESPONSE, PhotonListener::EVENT_ONGETROOMLISTRESPONSE)

    /**
     * Use a custom authentication service.
     * @field AUTH_CUSTOM
     */
    SETCONSTANT(AUTH_CUSTOM, ExitGames::LoadBalancing::CustomAuthenticationType::CUSTOM)
    /**
     * Authenticates users by their Steam Account. Pass L"ticket=[ticket]" to setParameters().
     * @field AUTH_STEAM
     */
    SETCONSTANT(AUTH_STEAM, ExitGames::LoadBalancing::CustomAuthenticationType::STEAM)
    /**
     * Authenticates users by their Facebook Account. Pass L"token=[token]" to setParameters().
     * @field AUTH_FACEBOOK
     */
    SETCONSTANT(AUTH_FACEBOOK, ExitGames::LoadBalancing::CustomAuthenticationType::FACEBOOK)
    /**
     * Authenticates users by their Oculus Account. Pass L"userid=[userid]&nonce=[nonce]" to setParameters().
     * @field AUTH_OCULUS
     */
    SETCONSTANT(AUTH_OCULUS, ExitGames::LoadBalancing::CustomAuthenticationType::OCULUS)
    /**
     * Authenticates users by their PSN Account. Pass L"token=[token]&env=[env]&userName=[userName]" to setParameters().
     * @field AUTH_PLAYSTATION_4
     */
    SETCONSTANT(AUTH_PLAYSTATION_4, ExitGames::LoadBalancing::CustomAuthenticationType::PLAYSTATION_4)
    /**
     * Authenticates users by their XBox Network Account. Pass the XSTS token to setData().
     * @field AUTH_XBOX
     */
    SETCONSTANT(AUTH_XBOX, ExitGames::LoadBalancing::CustomAuthenticationType::XBOX)
    /**
     * Authenticates users by their HTC Viveport Account and user token. Pass L"userToken=[userToken]" to setParameters().
     * @field AUTH_VIVEPORT
     */
    SETCONSTANT(AUTH_VIVEPORT, ExitGames::LoadBalancing::CustomAuthenticationType::VIVEPORT)
    /**
     * Authenticates users by their Nintendo Account. Pass L"token=[token]&appversion=[appversion]" to setParameters(). The appversion is optional.
     * @field AUTH_NINTENDO_SWITCH
     */
    SETCONSTANT(AUTH_NINTENDO_SWITCH, ExitGames::LoadBalancing::CustomAuthenticationType::NINTENDO_SWITCH)
    /**
     * Authenticates users by their PSN Account. Pass L"token=[token]&env=[env]&userName=[userName]" to setParameters().
     * @field AUTH_PLAYSTATION_5
     */
    SETCONSTANT(AUTH_PLAYSTATION_5, ExitGames::LoadBalancing::CustomAuthenticationType::PLAYSTATION_5)
    /**
     * Authenticates users by their Epic Online Services (EOS) Account. Pass L"token=[token]&ownershipToken=[ownershipToken]" to setParameters(). The ownershipToken is optional.
     * @field AUTH_EPIC
     */
    SETCONSTANT(AUTH_EPIC, ExitGames::LoadBalancing::CustomAuthenticationType::EPIC)
    /**
     * Authenticates users by their Facebook Account. Pass L"token=[token]" to setParameters().
     * @field AUTH_FACEBOOK_GAMING
     */
    SETCONSTANT(AUTH_FACEBOOK_GAMING, ExitGames::LoadBalancing::CustomAuthenticationType::FACEBOOK_GAMING)
    /**
     * Disables custom authentication.
     * @field AUTH_NONE
     */
    SETCONSTANT(AUTH_NONE, ExitGames::LoadBalancing::CustomAuthenticationType::NONE)

    /**
     * Photon Cloud and for self-hosted Photon 5 or higher Server instances
     * @field SERVER_TYPE_NAME_SERVER
     */
    SETCONSTANT(SERVER_TYPE_NAME_SERVER, ExitGames::LoadBalancing::ServerType::NAME_SERVER)

    /**
     * Self-hosted Photon 4 Server instances
     * @field SERVER_TYPE_MASTER_SERVER
     */
    SETCONSTANT(SERVER_TYPE_MASTER_SERVER, ExitGames::LoadBalancing::ServerType::MASTER_SERVER)



    /**
     * Fills up rooms (oldest first) to get players together as fast as possible. Default. Makes most sense with MaxPlayers > 0 and games that can only start with more players.
     * @field MATCHMAKINGMODE_FILL_ROOM
     */
    SETCONSTANT(MATCHMAKINGMODE_FILL_ROOM, ExitGames::LoadBalancing::MatchmakingMode::FILL_ROOM)
    /**
     * Distributes players across available rooms sequentially but takes filters into account. Without filters, rooms get players evenly distributed.
     * @field MATCHMAKINGMODE_SERIAL_MATCHING
     */
    SETCONSTANT(MATCHMAKINGMODE_SERIAL_MATCHING, ExitGames::LoadBalancing::MatchmakingMode::SERIAL_MATCHING)
    /**
     * Joins a (fully) random room. Expected properties must match, but aside from this, any available room might be selected.
     * @field MATCHMAKINGMODE_RANDOM_MATCHING
     */
    SETCONSTANT(MATCHMAKINGMODE_RANDOM_MATCHING, ExitGames::LoadBalancing::MatchmakingMode::RANDOM_MATCHING)


    /**
     * This lobby type is used unless another lobby type is specified. Room lists will be sent and Client::opJoinRandomRoom() can filter by matching properties.
     * @field LOBBYTYPE_DEFAULT
     */
    SETCONSTANT(LOBBYTYPE_DEFAULT, ExitGames::LoadBalancing::LobbyType::DEFAULT);
    /**
     * This lobby type lists rooms like type DEFAULT but  SQL-like "where" clauses for filtering can be used with Client::opJoinRandomRoom(). This allows 'bigger', 'less', 'or' and 'and' combinations.
     * @field LOBBYTYPE_SQL_LOBBY
     */
    SETCONSTANT(LOBBYTYPE_SQL_LOBBY, ExitGames::LoadBalancing::LobbyType::SQL_LOBBY);
    /**
     * This lobby does not send room lists. It is only used for Client::opJoinRandomRoom(). It keeps rooms available for matchmaking for a while even when there are only inactive users left.
     * @field LOBBYTYPE_ASYNC_RANDOM_LOBBY
     */
    SETCONSTANT(LOBBYTYPE_ASYNC_RANDOM_LOBBY, ExitGames::LoadBalancing::LobbyType::ASYNC_RANDOM_LOBBY);


    /**
     * Do not create any 2p2 connections between the clients. This is the default.
     * @field DIRECTMODE_NONE
     */
    SETCONSTANT(DIRECTMODE_NONE, ExitGames::LoadBalancing::DirectMode::NONE)
#ifndef DM_PLATFORM_HTML5
    /**
     * Each client establishes a direct connection with every other client inside the room.
     * @field DIRECTMODE_ALL_TO_OTHERS
     */
    SETCONSTANT(DIRECTMODE_ALL_TO_OTHERS, ExitGames::LoadBalancing::DirectMode::ALL_TO_OTHERS);
    /**
     * The master client establishes a direct connection with every other client inside the room. All other clients only establish a direct connection with the master client but not with each other.
     * @field DIRECTMODE_MASTER_TO_OTHERS
     */
    SETCONSTANT(DIRECTMODE_MASTER_TO_OTHERS, ExitGames::LoadBalancing::DirectMode::MASTER_TO_OTHERS);
    /**
     * Each client establishes a direct connection with every client inside the room, including itself.
     * @field DIRECTMODE_ALL_TO_ALL
     */
    SETCONSTANT(DIRECTMODE_ALL_TO_ALL, ExitGames::LoadBalancing::DirectMode::ALL_TO_ALL);
    /**
     * The master client establishes a direct connection with every client inside the room, including itself. All other clients only establish a direct connection with the master client but not with each other.
     * @field DIRECTMODE_MASTER_TO_ALL
     */
    SETCONSTANT(DIRECTMODE_MASTER_TO_ALL, ExitGames::LoadBalancing::DirectMode::MASTER_TO_ALL);
#endif



    #undef SETCONSTANT

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeRealtime(dmExtension::AppParams* params)
{
    ExitGames::Common::Base::setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS));
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeRealtime(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    dmLogInfo("Registered %s Extension", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeRealtime(dmExtension::AppParams* params)
{
    dmLogInfo("AppFinalizeRealtime");
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeRealtime(dmExtension::Params* params)
{
    dmLogInfo("FinalizeRealtime");
    if (g_PhotonListener)
    {
        delete g_PhotonListener;
        g_PhotonListener = 0;
    }

    if (g_PhotonClient)
    {
        delete g_PhotonClient;
        g_PhotonClient = 0;
    }
    return dmExtension::RESULT_OK;
}

#else

static dmExtension::Result AppInitializeRealtime(dmExtension::AppParams* params)
{
    dmLogWarning("Registered %s (null) Extension", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeRealtime(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeRealtime(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeRealtime(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

#endif

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeRealtime, AppFinalizeRealtime, InitializeRealtime, 0, 0, FinalizeRealtime)
