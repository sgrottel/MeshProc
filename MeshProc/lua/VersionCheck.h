#pragma once

#include "Runner.h"

namespace meshproc
{
	namespace lua
	{

		class VersionCheck : public Runner::Component<VersionCheck>
		{
		public:
			VersionCheck(Runner& owner)
				: Component<VersionCheck>{ owner }
			{};

			bool Init();

		private:
			static int CallbackToString(lua_State* lua);

			static int CallbackGetVersion(lua_State* lua);

			static int CallbackAssertVersion(lua_State* lua);
			static int CallbackAssertVersionSince(lua_State* lua);
			static int CallbackAssertVersionSinceIncluded(lua_State* lua);
			static int CallbackAssertVersionBefore(lua_State* lua);
			static int CallbackAssertVersionBeforeIncluded(lua_State* lua);

			static int GetVersionParams(lua_State* lua, uint32_t& major, uint32_t& minor, uint32_t& patch, uint32_t& build);
			static int GetVersionParamsCompare(lua_State* lua);
		};

	}
}
