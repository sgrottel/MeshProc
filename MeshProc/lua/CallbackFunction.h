#pragma once

#include <lua.hpp>

#include <memory>

namespace meshproc
{
	namespace lua
	{
		class CallbackFunction final
		{
		public:
			CallbackFunction(lua_State* lua, int idx);
			~CallbackFunction();
			CallbackFunction(const CallbackFunction&) = delete;
			CallbackFunction(CallbackFunction&&) = delete;
			CallbackFunction& operator=(const CallbackFunction&) = delete;
			CallbackFunction& operator=(CallbackFunction&&) = delete;

			// first push function, then push arguments, then call
			bool Push() const;
			bool Call(int numArgs, int numResults, bool throwOnError = true) const;

			inline lua_State* Lua() const noexcept
			{
				return m_lua;
			}

		private:
			lua_State* m_lua;
			int m_functionRef;
		};
	}
}
