#pragma once

#include "AbstractType.h"

#include "lua/LuaUtilities.h"

#include <vector>

namespace meshproc
{
	namespace lua
	{
		namespace types
		{
			template<typename TINNERVAR, typename TIMPL>
			class AbstractListType : public AbstractType<std::vector<TINNERVAR>, TIMPL>
			{
			public:

			protected:
				using MyAbstractType = AbstractType<std::vector<TINNERVAR>, TIMPL>;

				static int CallbackCtor(lua_State* lua)
				{
					MyAbstractType::LuaPush(lua, std::make_shared<std::vector<TINNERVAR>>());
					return 1;
				}

				static int CallbackLength(lua_State* lua)
				{
					// args of the "#" operator is twice the object instance
					const int argcnt = lua_gettop(lua);
					if (argcnt != 2)
					{
						// DumpLuaStack(lua);
						return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", argcnt);
					}

					const auto list = MyAbstractType::LuaGet(lua, 1);
					if (!list)
					{
						return luaL_error(lua, "Pre-First argument expected to be a IndexList");
					}

					// ignoring second arg

					lua_pushinteger(lua, list->size());
					return 1;
				}

				static int CallbackDispatchGet(lua_State* lua)
				{
					const int argcnt = lua_gettop(lua);
					if (argcnt != 2)
					{
						return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", argcnt);
					}

					const auto list = MyAbstractType::LuaGet(lua, 1);
					if (!list)
					{
						return luaL_error(lua, "Pre-First argument expected to be a IndexList");
					}

					uint32_t idx = 0;

					if (lua_isstring(lua, 2))
					{
						// try to load function callback
						size_t len;
						std::string name = luaL_tolstring(lua, 2, &len); // copy type string

						luaL_getmetatable(lua, TIMPL::LUA_TYPE_NAME);

						lua_getfield(lua, -1, name.c_str());

						if (lua_isnil(lua, -1))
						{
							lua_pop(lua, 2); // pop nil and metatable and try with string as index
						}
						else
						{
							lua_remove(lua, -2); // remove metatable, keep function pointer value
							return 1;
						}
					}

					auto res = GetLuaUint32(lua, 2, idx);
					if (res == GetResult::Ok)
					{
						// numeric index -> return list element
						if (idx == 0 || idx > list->size())
						{
							lua_pushnil(lua);
							return 1;
						}

						TIMPL::LuaPushElementValue(lua, *list, idx - 1);
						return 1;
					}

					return luaL_error(lua, "Failed to check agument %d", res);
				}

				static int CallbackSet(lua_State* lua)
				{
					const int argcnt = lua_gettop(lua);
					if (argcnt != 3)
					{
						return luaL_error(lua, "Arguments number mismatch: must be 3, is %d", argcnt);
					}

					const auto list = MyAbstractType::LuaGet(lua, 1);
					if (!list)
					{
						return luaL_error(lua, "Pre-First argument expected to be a IndexList");
					}

					uint32_t idx;
					if (GetLuaUint32(lua, 2, idx) != GetResult::Ok)
					{
						return luaL_error(lua, "Failed to get insert index argument integer");
					}
					if (idx == 0 || idx > list->size())
					{
						return luaL_error(lua, "Invalid insert index argument integer, %d", idx);
					}

					TINNERVAR val;
					if (!TIMPL::LuaGetElement(lua, 3, val))
					{
						return luaL_error(lua, "Failed to get value argument");
					}

					list->at(idx - 1) = val;
					return 0;
				}

				static int CallbackInsert(lua_State* lua)
				{
					const int argcnt = lua_gettop(lua);
					if ((argcnt != 2) && (argcnt != 3))
					{
						return luaL_error(lua, "Arguments number mismatch: must be 2 or 3, is %d", argcnt);
					}

					const auto list = MyAbstractType::LuaGet(lua, 1);
					if (!list)
					{
						return luaL_error(lua, "Pre-First argument expected to be a IndexList");
					}

					TINNERVAR val;
					if (!TIMPL::LuaGetElement(lua, -1, val))
					{
						return luaL_error(lua, "Failed to get value argument");
					}

					if (argcnt == 2)
					{
						list->push_back(val);
						return 0;
					}

					uint32_t idx;
					if (GetLuaUint32(lua, 2, idx) != GetResult::Ok)
					{
						return luaL_error(lua, "Failed to get insert index argument integer");
					}
					if (idx == 0 || idx > list->size() + 1)
					{
						return luaL_error(lua, "Invalid insert index argument integer, %d", idx);
					}
					if (idx == list->size() + 1)
					{
						list->push_back(val);
						return 0;
					}

					auto where = list->cbegin();
					std::advance(where, idx - 1);
					list->insert(where, val);

					return 0;
				}

				static int CallbackRemove(lua_State* lua)
				{
					const int argcnt = lua_gettop(lua);
					if ((argcnt != 1) && (argcnt != 2))
					{
						return luaL_error(lua, "Arguments number mismatch: must be 1 or 2, is %d", argcnt);
					}

					const auto list = MyAbstractType::LuaGet(lua, 1);
					if (!list)
					{
						return luaL_error(lua, "Pre-First argument expected to be a IndexList");
					}

					if (argcnt == 1)
					{
						list->pop_back();
						return 0;
					}

					uint32_t idx;
					if (GetLuaUint32(lua, 2, idx) != GetResult::Ok)
					{
						return luaL_error(lua, "Failed to get insert index argument integer");
					}
					if (idx == 0 || idx > list->size() + 1)
					{
						return luaL_error(lua, "Invalid insert index argument integer, %d", idx);
					}

					auto where = list->cbegin();
					std::advance(where, idx - 1);
					list->erase(where);

					return 0;
				}

				static int CallbackResize(lua_State* lua)
				{
					const int argcnt = lua_gettop(lua);
					if (argcnt != 2)
					{
						return luaL_error(lua, "Arguments number mismatch: must be 2, is %d", argcnt);
					}

					const auto list = MyAbstractType::LuaGet(lua, 1);
					if (!list)
					{
						return luaL_error(lua, "Pre-First argument expected to be a IndexList");
					}

					uint32_t newlen;
					if (GetLuaUint32(lua, 2, newlen) != GetResult::Ok)
					{
						return luaL_error(lua, "Failed to get insert index argument integer");
					}

					list->resize(newlen, TIMPL::GetInvalidValue());

					return 0;
				}

			};
		}
	}
}
