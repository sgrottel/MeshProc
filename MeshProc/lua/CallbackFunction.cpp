#include "CallbackFunction.h"

#include "LuaUtilities.h"

#include <cassert>
#include <string>
#include <stdexcept>

using namespace meshproc;
using namespace meshproc::lua;

CallbackFunction::CallbackFunction(lua_State* lua, int idx)
	: m_lua{ lua }
{
	assert(lua_isfunction(m_lua, idx));
	lua_pushvalue(m_lua, idx);
	m_functionRef = luaL_ref(m_lua, LUA_REGISTRYINDEX);
}

CallbackFunction::~CallbackFunction()
{
	if (m_lua != nullptr)
	{
		luaL_unref(m_lua, LUA_REGISTRYINDEX, m_functionRef);
		m_lua = nullptr;
	}
}

bool CallbackFunction::Push() const
{
	if (m_lua == nullptr)
	{
		return false;
	}
	lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_functionRef);

	return true;
}

bool CallbackFunction::Call(int numArgs, int numResults, bool throwOnError) const
{
	if (m_lua == nullptr)
	{
		if (throwOnError)
		{
			throw std::logic_error("CallbackFunction without lua state object");
		}
		return false;
	}
	int rv = lua_pcall(m_lua, numArgs, numResults, 0);
	if (rv != LUA_OK && throwOnError)
	{
		std::string errorMsg = lua_tostring(m_lua, -1);
		throw std::runtime_error(errorMsg);
	}
	return rv == LUA_OK;
}
