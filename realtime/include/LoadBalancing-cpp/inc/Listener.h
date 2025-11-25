/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Photon-cpp/inc/OperationResponse.h"
#include "LoadBalancing-cpp/inc/LobbyStatsResponse.h"

namespace ExitGames
{
	namespace LoadBalancing
	{
		class Player;
		class Room;

		class Listener: public virtual Common::BaseListener
		{
		public:
			virtual ~Listener(void){}

			// receive and print out debug out here
			virtual void debugReturn(int debugLevel, const Common::JString& string) = 0;

			// implement your error-handling here
			virtual void connectionErrorReturn(int errorCode) = 0;
			virtual void clientErrorReturn(int errorCode) = 0;
			virtual void warningReturn(int warningCode) = 0;
			virtual void serverErrorReturn(int errorCode) = 0;

			// events, triggered by certain operations of all players in the same room
			virtual void joinRoomEventAction(int playerNr, const Common::JVector<int>& playernrs, const Player& player) = 0;
			virtual void leaveRoomEventAction(int playerNr, bool isInactive) = 0;

			virtual void customEventAction(int playerNr, nByte eventCode, const Common::Object& eventContent) = 0;

			// callbacks for operations on the server
			virtual void connectReturn(int errorCode, const Common::JString& errorString, const Common::JString& region, const Common::JString& cluster) = 0;
			virtual void disconnectReturn(void) = 0;
			virtual void createRoomReturn(int EG_UNUSED(localPlayerNr), const Common::Hashtable& EG_UNUSED(roomProperties), const Common::Hashtable& EG_UNUSED(playerProperties), int EG_UNUSED(errorCode), const Common::JString& EG_UNUSED(errorString)) {}
			virtual void joinOrCreateRoomReturn(int EG_UNUSED(localPlayerNr), const Common::Hashtable& EG_UNUSED(roomProperties), const Common::Hashtable& EG_UNUSED(playerProperties), int EG_UNUSED(errorCode), const Common::JString& EG_UNUSED(errorString)) {}
			virtual void joinRandomOrCreateRoomReturn(int EG_UNUSED(localPlayerNr), const Common::Hashtable& EG_UNUSED(roomProperties), const Common::Hashtable& EG_UNUSED(playerProperties), int EG_UNUSED(errorCode), const Common::JString& EG_UNUSED(errorString)) {}
			virtual void joinRoomReturn(int EG_UNUSED(localPlayerNr), const Common::Hashtable& EG_UNUSED(roomProperties), const Common::Hashtable& EG_UNUSED(playerProperties), int EG_UNUSED(errorCode), const Common::JString& EG_UNUSED(errorString)) {}
			virtual void joinRandomRoomReturn(int EG_UNUSED(localPlayerNr), const Common::Hashtable& EG_UNUSED(roomProperties), const Common::Hashtable& EG_UNUSED(playerProperties), int EG_UNUSED(errorCode), const Common::JString& EG_UNUSED(errorString)) {}
			virtual void leaveRoomReturn(int errorCode, const Common::JString& errorString) = 0;
			virtual void joinLobbyReturn(void) {}
			virtual void leaveLobbyReturn(void) {}
			virtual void onFindFriendsResponse(void) {}
			virtual void onLobbyStatsResponse(const Common::JVector<LobbyStatsResponse>& EG_UNUSED(lobbyStats)) {}
			virtual void webRpcReturn(int EG_UNUSED(errorCode), const Common::JString& EG_UNUSED(errorString), const Common::JString& EG_UNUSED(uriPath), int EG_UNUSED(resultCode), const Common::Dictionary<Common::Object, Common::Object>& EG_UNUSED(returnData)) {}
			virtual void onPropertiesChangeFailed(void) {}

			// info, that certain values have been updated
			virtual void onRoomListUpdate(void) {}
			virtual void onRoomPropertiesChange(const Common::Hashtable& EG_UNUSED(changes)) {}
			virtual void onPlayerPropertiesChange(int EG_UNUSED(playerNr), const Common::Hashtable& EG_UNUSED(changes)) {}
			virtual void onAppStatsUpdate(void) {}
			virtual void onLobbyStatsUpdate(const Common::JVector<LobbyStatsResponse>& EG_UNUSED(lobbyStats)) {}
			virtual void onCacheSliceChanged(int EG_UNUSED(cacheSliceIndex)) {}
			virtual void onMasterClientChanged(int EG_UNUSED(id), int EG_UNUSED(oldID)) {}

			// custom authentication
			virtual void onCustomAuthenticationIntermediateStep(const Common::Dictionary<Common::JString, Common::Object>& EG_UNUSED(customResponseData)){}

			// receive the available server regions during the connect workflow (if you have specified in the constructor, that you want to select a region)
			virtual void onAvailableRegions(const Common::JVector<Common::JString>& EG_UNUSED(availableRegions), const Common::JVector<Common::JString>& EG_UNUSED(availableRegionServers)){}

			virtual void onSecretReceival(const Common::JString& EG_UNUSED(secret)){}

			virtual void onDirectConnectionEstablished(int EG_UNUSED(remoteID)){}
			virtual void onDirectConnectionFailedToEstablish(int EG_UNUSED(remoteID)) {}
			virtual void onDirectMessage(const Common::Object& EG_UNUSED(msg), int EG_UNUSED(remoteID), bool EG_UNUSED(relay)) {}

			virtual void onCustomOperationResponse(const Photon::OperationResponse& EG_UNUSED(operationResponse)) {}

			virtual void onGetRoomListResponse(const Common::JVector<Common::Helpers::SharedPointer<Room> >& EG_UNUSED(roomList), const Common::JVector<Common::JString>& EG_UNUSED(roomNameList)) {}
		};
		/** @file */
	}
}