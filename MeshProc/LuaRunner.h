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

	class CommandFactory;

	class LuaRunner
	{
	public:
		LuaRunner(sgrottel::ISimpleLog& log, CommandFactory& factory);

		bool RegisterCommands();

		bool Init();
		bool LoadScript(const std::filesystem::path& script);
		bool RunScript();

	private:
		static LuaRunner* GetThis(lua_State* lua);
		template<typename FN, typename... ARGS>
		static int CallLuaImpl(FN fn, lua_State* lua, ARGS... args);

		static int CallbackLogWrite(lua_State* lua);
		static int CallbackLogWarn(lua_State* lua);
		static int CallbackLogError(lua_State* lua);

		static int CallbackCreateCommand(lua_State* lua);
		static int CallbackCommandDelete(lua_State* lua);
		static int CallbackCommandToString(lua_State* lua);
		static int CallbackCommandInvoke(lua_State* lua);
		static int CallbackCommandGet(lua_State* lua);
		static int CallbackCommandSet(lua_State* lua);

		bool AssertStateReady();
		bool RegisterLogFunctions();
		int CallbackLogImpl(lua_State* lua, uint32_t flags);
		int CallbackCreateCommandImpl(lua_State* lua);
		int CallbackCommandInvokeImpl(lua_State* lua);
		int CallbackCommandGetImpl(lua_State* lua);
		int CallbackCommandSetImpl(lua_State* lua);

		sgrottel::ISimpleLog& m_log;
		CommandFactory& m_factory;
		std::shared_ptr<lua_State> m_state;
	};

}
