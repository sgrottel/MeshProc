#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>

// forward declaration
typedef struct lua_State lua_State;

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{
	namespace commands
	{
		class CommandFactory;
	}

	namespace lua
	{

		class Runner
		{
		public:

			template<typename IMPL>
			class Component
			{
			public:
				Component(Runner& owner)
					: m_owner(owner)
				{ }

			protected:

				template<typename FN, typename... ARGS>
				static int CallLuaImpl(FN fn, lua_State* lua, ARGS... args)
				{
					auto that = Runner::GetThis(lua);
					if (that == nullptr) return 0;
					auto comp = that->GetComponent<IMPL>();
					if (comp == nullptr) return 0;
					return (comp->*fn)(lua, args...);
				}

				inline bool AssertStateReady() const
				{
					return m_owner.AssertStateReady();
				}

				inline lua_State* lua() const
				{
					return m_owner.m_state.get();
				}

				inline sgrottel::ISimpleLog& Log() const
				{
					return m_owner.m_log;
				}

				template<typename T>
				T* GetComponent() const
				{
					return m_owner.GetComponent<T>();
				}

			private:
				Runner& m_owner;
			};

			Runner(sgrottel::ISimpleLog& log, commands::CommandFactory& factory);

			bool RegisterCommands();

			bool Init();
			bool LoadScript(const std::filesystem::path& script);
			bool SetArgs(const std::unordered_map<std::wstring_view, std::wstring_view>& args);
			bool RunScript();

		private:
			class Components;

			static Runner* GetThis(lua_State* lua);

			static int LuaLibLoader(lua_State* lua);

			template<typename T>
			T* GetComponent() const;
			bool AssertStateReady();


			sgrottel::ISimpleLog& m_log;
			std::shared_ptr<lua_State> m_state;
			std::shared_ptr<Components> m_components;
			std::filesystem::path m_workingDirectory;
		};

	}
}
