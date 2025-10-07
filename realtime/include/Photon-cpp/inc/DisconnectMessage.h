/* Exit Games Photon - C++ Client Lib
* Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
* https://www.photonengine.com
* mailto:developer@photonengine.com
*/

#pragma once

#include "Common-cpp/inc/Common.h"

namespace ExitGames
{
	namespace Photon
	{
		namespace Internal
		{
			class PeerBase;
		}

		class DisconnectMessage
		{
		public:
			~DisconnectMessage(void);

			const Common::Object& operator[](unsigned int index) const;

			Common::JString toString(bool withDebugMessage=false, bool withParameters=false, bool withParameterTypes=false) const;
			Common::Object getParameterForCode(nByte parameterCode) const;

			short getCode(void) const;
			const Common::JString& getDebugMessage(void) const;
			const Common::Dictionary<nByte, Common::Object>& getParameters(void) const;
		private:
			DisconnectMessage(short code, Common::JString debugMessage, const Common::Dictionary<nByte, Common::Object>& parameters);

			short mCode;
			Common::JString mDebugMessage;
			Common::Dictionary<nByte, Common::Object> mParameters;

			friend class Internal::PeerBase;
		};
	}
}