#pragma once

#include <filesystem>
#include <memory>

// forward declaration
typedef struct lua_State lua_State;

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{

	class LuaRunner
	{
	public:
		LuaRunner(sgrottel::ISimpleLog& log);

		bool Init();
		bool LoadScript(const std::filesystem::path& script);
		bool RunScript();

	private:
		static LuaRunner* GetThis(lua_State* lua);

		static int CallbackLogWrite(lua_State* lua);
		static int CallbackLogWarn(lua_State* lua);
		static int CallbackLogError(lua_State* lua);

		bool AssertStateReady();
		bool RegisterLogFunctions();
		void CallbackLogImpl(lua_State* lua, uint32_t flags);

		sgrottel::ISimpleLog& m_log;
		std::shared_ptr<lua_State> m_state;
	};

}
