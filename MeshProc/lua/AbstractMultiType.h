#pragma once

#include "AbstractType.h"

#include "LuaUtilities.h"

#include <lua.hpp>

#include <memory>
#include <vector>

namespace meshproc
{
	namespace lua
	{
		template<typename TINNERVAR, typename TIMPL>
		class AbstractMultiType : public AbstractType<std::vector<std::shared_ptr<TINNERVAR>>, TIMPL>
		{
		protected:
			AbstractMultiType(Runner& owner)
				: AbstractType<std::vector<std::shared_ptr<TINNERVAR>>, TIMPL>{ owner }
			{};

		protected:
			static int CallbackSize(lua_State* lua);
			template<typename TINNERIMPL>
			static int CallbackGetImpl(lua_State* lua);
		};

		template<typename TINNERVAR, typename TIMPL>
		int AbstractMultiType<TINNERVAR, TIMPL>::CallbackSize(lua_State* lua)
		{
			int size = lua_gettop(lua);
			if (size != 1)
			{
				return luaL_error(lua, "Arguments number mismatch: must be 1, is %d", size);
			}
			auto val = AbstractType<std::vector<std::shared_ptr<TINNERVAR>>, TIMPL>::LuaGet(lua, 1);
			if (!val)
			{
				return luaL_error(lua, "Pre-First argument expected to be a %s", TIMPL::LUA_TYPE_NAME);
			}

			lua_pushinteger(lua, val->size());
			return 1;
		}

		template<typename TINNERVAR, typename TIMPL>
		template<typename TINNERIMPL>
		int AbstractMultiType<TINNERVAR, TIMPL>::CallbackGetImpl(lua_State* lua)
		{
			int size = lua_gettop(lua);
			if (size != 2)
			{
				return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", size);
			}
			auto val = AbstractType<std::vector<std::shared_ptr<TINNERVAR>>, TIMPL>::LuaGet(lua, 1);
			if (!val)
			{
				return luaL_error(lua, "Pre-First argument expected to be a %s", TIMPL::LUA_TYPE_NAME);
			}
			uint32_t idx;
			if (GetResult::Ok != GetLuaUint32(lua, 2, idx))
			{
				return luaL_error(lua, "First argument expected to be an integer");
			}
			if (idx == 0 || idx > val->size())
			{
				return luaL_error(lua, "Argument out of bounds expected [1..%d], got %d", static_cast<int>(val->size()), static_cast<int>(idx));
			}

			TINNERIMPL::LuaPush(lua, val->at(idx - 1));
			return 1;
		}

	}
}
