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


void lua_pushobject(lua_State* L, const ExitGames::Common::Object* o)
{
    int type = o->getType();
    if (type == ExitGames::Common::TypeCode::BYTE ||
        type == ExitGames::Common::TypeCode::SHORT ||
        type == ExitGames::Common::TypeCode::INTEGER)
    {
        int intval = ((ExitGames::Common::ValueObject<int>)*o).getDataCopy();
        lua_pushnumber(L, intval);
    }
    else if (type == ExitGames::Common::TypeCode::FLOAT ||
             type == ExitGames::Common::TypeCode::DOUBLE)
    {
        double doubleval = ((ExitGames::Common::ValueObject<double>)*o).getDataCopy();
        lua_pushnumber(L, doubleval);
    }
    else if (type == ExitGames::Common::TypeCode::BOOLEAN)
    {
        bool boolval = ((ExitGames::Common::ValueObject<bool>)*o).getDataCopy();
        lua_pushboolean(L, boolval);
    }
    else if (type == ExitGames::Common::TypeCode::STRING)
    {
        const char* stringval = ((ExitGames::Common::ValueObject<ExitGames::Common::JString>)*o).getDataCopy().UTF8Representation().cstr();
        lua_pushstring(L, stringval);
    }
    else
    {
        luaL_error(L, "Unhandled object type %d", o->getType());
    }
}

void table_pushstring(lua_State* L, const char* key, const char* value)
{
    lua_pushstring(L, key);
    lua_pushstring(L, value);
    lua_rawset(L, -3);
}

void table_pushint(lua_State* L, const char* key, int value)
{
    lua_pushstring(L, key);
    lua_pushinteger(L, value);
    lua_rawset(L, -3);
}


void table_pushhashtable(lua_State* L, const char* key, const ExitGames::Common::Hashtable& ht)
{
    lua_pushstring(L, key);
    lua_newtable(L);

    const ExitGames::Common::JVector<ExitGames::Common::Object> keys = ht.getKeys();
    for (unsigned int i = 0; i < keys.getSize(); ++i)
    {
        ExitGames::Common::Object key = keys[i];
        lua_pushobject(L, &key);
        const ExitGames::Common::Object* value = ht.getValue(key);
        lua_pushobject(L, value);
        lua_rawset(L, -3);
    }
    lua_rawset(L, -3);
}

void table_pushstringdictionary(lua_State* L, const char* key, const ExitGames::Common::Dictionary<ExitGames::Common::JString, ExitGames::Common::Object>& d)
{
    lua_pushstring(L, key);
    lua_newtable(L);

    const ExitGames::Common::JVector<ExitGames::Common::JString> keys = d.getKeys();
    for (unsigned int i = 0; i < keys.getSize(); ++i)
    {
        ExitGames::Common::JString key = keys[i];
        lua_pushstring(L, key.UTF8Representation().cstr());
        const ExitGames::Common::Object* value = d.getValue(key);
        lua_pushobject(L, value);
        lua_rawset(L, -3);
    }
    lua_rawset(L, -3);
}
void table_pushobjectdictionary(lua_State* L, const char* key, const ExitGames::Common::Dictionary<ExitGames::Common::Object, ExitGames::Common::Object>& d)
{
    lua_pushstring(L, key);
    lua_newtable(L);

    const ExitGames::Common::JVector<ExitGames::Common::Object> keys = d.getKeys();
    for (unsigned int i = 0; i < keys.getSize(); ++i)
    {
        ExitGames::Common::Object key = keys[i];
        lua_pushobject(L, &key);
        const ExitGames::Common::Object* value = d.getValue(key);
        lua_pushobject(L, value);
        lua_rawset(L, -3);
    }
    lua_rawset(L, -3);
}

void table_pushobject(lua_State* L, const char* key, const ExitGames::Common::Object* o)
{
    lua_pushstring(L, key);
    lua_pushobject(L, o);
    lua_rawset(L, -3);
}

void table_pushboolean(lua_State* L, const char* key, bool b)
{
    lua_pushstring(L, key);
    lua_pushboolean(L, b);
    lua_rawset(L, -3);
}

void lua_pushroom(lua_State* L, ExitGames::LoadBalancing::Room* room)
{
    lua_newtable(L);
    table_pushstring(L, "name", room->getName().UTF8Representation().cstr());
    table_pushint(L, "max_players", room->getMaxPlayers());
    table_pushboolean(L, "is_open", room->getIsOpen());
    table_pushint(L, "direct_mode", room->getDirectMode());
    table_pushhashtable(L, "custom_properties", room->getCustomProperties());
}

void table_pushlobbystatsresponse(lua_State* L, const char* key, const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats)
{
    lua_pushstring(L, key);
    lua_newtable(L);
    for (unsigned int i = 0; i < lobbyStats.getSize(); i++)
    {
        ExitGames::LoadBalancing::LobbyStatsResponse r = lobbyStats[i];
        // push LobbyStatsResponse as an array of tables
        lua_pushinteger(L, i + 1);
        lua_newtable(L);
        {
            table_pushstring(L, "name", r.getName().UTF8Representation().cstr());
            table_pushint(L, "type", r.getType());
            table_pushint(L, "peer_count", r.getPeerCount());
            table_pushint(L, "room_count", r.getRoomCount());
        }
        lua_rawset(L, -3);
    }
    lua_rawset(L, -3);
}

void table_pushplayer(lua_State* L, const char* key, const ExitGames::LoadBalancing::Player& player)
{
    lua_pushstring(L, key);
    lua_newtable(L);

    table_pushstring(L, "name", player.getName().UTF8Representation().cstr());
    table_pushstring(L, "user_id", player.getUserID().UTF8Representation().cstr());
    table_pushhashtable(L, "custom_properties", player.getCustomProperties());
    table_pushboolean(L, "is_inactive", player.getIsInactive());
    table_pushboolean(L, "is_master_client", player.getIsMasterClient());

    lua_rawset(L, -3);
}

void table_pushstringvector(lua_State* L, const char* key, const ExitGames::Common::JVector<ExitGames::Common::JString>& vec)
{
    lua_pushstring(L, key);
    lua_newtable(L);
    for (unsigned int i = 0; i < vec.getSize(); i++)
    {
        ExitGames::Common::JString s = vec[i];
        lua_pushinteger(L, i + 1);
        lua_pushstring(L, s.UTF8Representation().cstr());
        lua_rawset(L, -3);
    }
    lua_rawset(L, -3);
}

void table_pushintvector(lua_State* L, const char* key, const ExitGames::Common::JVector<int>& vec)
{
    lua_pushstring(L, key);
    lua_newtable(L);
    for (unsigned int i = 0; i < vec.getSize(); i++)
    {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, vec[i]);
        lua_rawset(L, -3);
    }
    lua_rawset(L, -3);
}
