#pragma once

#include "lua/Runner.h"

#include <lua.hpp>

#include <memory>

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			template<typename TVAR, typename TIMPL>
			class AbstractType : public Runner::Component<TIMPL>
			{
			public:

				static int LuaPush(lua_State* lua, std::shared_ptr<TVAR> val);
				static std::shared_ptr<TVAR> LuaGet(lua_State* lua, int idx);

				AbstractType(Runner& owner)
					: Runner::Component<TIMPL>{ owner }
				{};

			protected:

				static int CallbackDelete(lua_State* lua);
				static int CallbackToString(lua_State* lua);

				bool InitImpl(const struct luaL_Reg memberFunc[], const char* const TYPE_NAME = TIMPL::LUA_TYPE_NAME);

			private:

				struct wrapped
				{
					std::shared_ptr<TVAR> val;
				};

				static wrapped* GetWrappedObject(lua_State* lua, int idx)
				{
					void* ud = luaL_checkudata(lua, idx, TIMPL::LUA_TYPE_NAME);
					return reinterpret_cast<wrapped*>(ud);
				}
			};

			template<typename TVAR, typename TIMPL>
			int AbstractType<TVAR, TIMPL>::LuaPush(lua_State* lua, std::shared_ptr<TVAR> val)
			{
				wrapped* w = (wrapped*)lua_newuserdata(lua, sizeof(wrapped)); // put on stack
				memset(w, 0, sizeof(wrapped));
				luaL_getmetatable(lua, TIMPL::LUA_TYPE_NAME);
				lua_setmetatable(lua, -2);
				w->val = val;
				return 1;
			}

			template<typename TVAR, typename TIMPL>
			std::shared_ptr<TVAR> AbstractType<TVAR, TIMPL>::LuaGet(lua_State* lua, int idx)
			{
				wrapped* w = GetWrappedObject(lua, idx);
				if (w != nullptr)
				{
					return w->val;
				}
				return 0;
			}

			template<typename TVAR, typename TIMPL>
			int AbstractType<TVAR, TIMPL>::CallbackDelete(lua_State* lua)
			{
				wrapped* w = GetWrappedObject(lua, 1);
				if (w != nullptr)
				{
					w->val.reset();
				}
				return 0;
			}

			template<typename TVAR, typename TIMPL>
			int AbstractType<TVAR, TIMPL>::CallbackToString(lua_State* lua)
			{
				wrapped* w = GetWrappedObject(lua, 1);
				if (w == nullptr || !w->val)
				{
					lua_pushstring(lua, "nil");
					return 1;
				}
				lua_pushstring(lua, TIMPL::LUA_TYPE_NAME);
				return 1;
			}

			template<typename TVAR, typename TIMPL>
			bool AbstractType<TVAR, TIMPL>::InitImpl(const struct luaL_Reg memberFuncs[], const char* const TYPE_NAME)
			{
				if (!Runner::Component<TIMPL>::AssertStateReady()) return false;

				// The member functions for the MeshObject
				luaL_newmetatable(Runner::Component<TIMPL>::lua(), TYPE_NAME);

				bool hasIndexDispatcher = false;
				for (const struct luaL_Reg* func = memberFuncs; func->func != nullptr; func++)
				{
					if (strncmp(func->name, "__index", 8) == 0)
					{
						hasIndexDispatcher = true;
						break;
					}
				}

				if (!hasIndexDispatcher)
				{
					// default __index dispatcher just references the metatable
					lua_pushvalue(Runner::Component<TIMPL>::lua(), -1);
					lua_setfield(Runner::Component<TIMPL>::lua(), -2, "__index");

					// lua_pushstring(Runner::Component<TIMPL>::lua(), "__index");
					// lua_pushvalue(Runner::Component<TIMPL>::lua(), -2);	// pushes the metatable
					// lua_settable(Runner::Component<TIMPL>::lua(), -3);	// metatable.__index = metatable
				}

				luaL_setfuncs(Runner::Component<TIMPL>::lua(), memberFuncs, 0);

				lua_pop(Runner::Component<TIMPL>::lua(), 1);

				return true;
			}

		}
	}
}
