#include "CommandType.h"

#include "CallbackFunction.h"
#include "GlmMat4Type.h"
#include "GlmVec3Type.h"
#include "ListOfVec3Type.h"
#include "LuaUtilities.h"
#include "MeshType.h"
#include "MultiMeshType.h"
#include "MultiIndicesType.h"
#include "SceneType.h"
#include "Shape2DType.h"
#include "IndicesType.h"

#include "AbstractCommand.h"
#include "utilities/StringUtilities.h"

#include <SimpleLog/SimpleLog.hpp>

#include <lua.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <array>

using namespace meshproc;
using namespace meshproc::lua;

namespace
{
	template<ParamType PT>
	struct LuaParamMapping;

	template<>
	struct LuaParamMapping<ParamType::UInt32>
	{
		static void PushVal(lua_State* lua, const uint32_t& v)
		{
			lua_pushinteger(lua, v);
		}
		static bool GetVal(lua_State* lua, uint32_t& tar)
		{
			return GetLuaUint32(lua, 3, tar) == GetResult::Ok;
		}
	};

	template<>
	struct LuaParamMapping<ParamType::Float>
	{
		static void PushVal(lua_State* lua, const float& v)
		{
			lua_pushnumber(lua, v);
		}
		static bool GetVal(lua_State* lua, float& tar)
		{
			if (lua_isnumber(lua, 3))
			{
				tar = static_cast<float>(lua_tonumber(lua, 3));
				return true;
			}
			if (lua_isinteger(lua, 3))
			{
				tar = static_cast<float>(lua_tointeger(lua, 3));
				return true;
			}
			return false;
		}
	};

	template<>
	struct LuaParamMapping<ParamType::String>
	{
		static void PushVal(lua_State* lua, const std::wstring& v)
		{
			lua_pushstring(lua, ToUtf8(v).c_str());
		}
		static bool GetVal(lua_State* lua, std::wstring& tar)
		{
			if (lua_isstring(lua, 3))
			{
				const char* str = lua_tostring(lua, 3);
				tar = FromUtf8(str);
				return true;
			}
			return false;
		}
	};

	template<typename WRAPPEDTYPE, typename NATIVETYPE>
	struct LuaWrappedParamMapping
	{
		static void PushVal(lua_State* lua, const std::shared_ptr<NATIVETYPE>& v)
		{
			WRAPPEDTYPE::LuaPush(lua, v);
		}
		static bool GetVal(lua_State* lua, std::shared_ptr<NATIVETYPE>& tar)
		{
			auto m = WRAPPEDTYPE::LuaGet(lua, 3);
			if (m)
			{
				tar = m;
				return true;
			}
			return false;
		}
	};

	template<>
	struct LuaParamMapping<ParamType::Mesh> : LuaWrappedParamMapping<MeshType, data::Mesh> {};

	template<>
	struct LuaParamMapping<ParamType::MultiMesh> : LuaWrappedParamMapping<MultiMeshType, std::vector<std::shared_ptr<data::Mesh>>> {};

	template<>
	struct LuaParamMapping<ParamType::Scene> : LuaWrappedParamMapping<SceneType, data::Scene> {};

	template<>
	struct LuaParamMapping<ParamType::Shape2D> : LuaWrappedParamMapping<Shape2DType, data::Shape2D> {};

	template<>
	struct LuaParamMapping<ParamType::Indices> : LuaWrappedParamMapping<IndicesType, std::vector<uint32_t>> {};

	template<>
	struct LuaParamMapping<ParamType::MultiIndices> : LuaWrappedParamMapping<MultiIndicesType, std::vector<std::shared_ptr<std::vector<uint32_t>>>> {};

	template<>
	struct LuaParamMapping<ParamType::Mat4>
	{
		static void PushVal(lua_State* lua, const glm::mat4& v)
		{
			GlmMat4Type::Push(lua, v);
		}

		static bool GetVal(lua_State* lua, glm::mat4& tar)
		{
			return GlmMat4Type::TryGet(lua, 3, tar);
		}
	};

	template<>
	struct LuaParamMapping<ParamType::Vec3>
	{
		static void PushVal(lua_State* lua, const glm::vec3& v)
		{
			GlmVec3Type::Push(lua, v);
		}

		static bool GetVal(lua_State* lua, glm::vec3& tar)
		{
			return GlmVec3Type::TryGet(lua, 3, tar);
		}
	};

	template<>
	struct LuaParamMapping<ParamType::Callback>
	{
		static void PushVal(lua_State* lua, const std::shared_ptr<lua::CallbackFunction>& v)
		{
			if (v)
			{
				v->Push();
			}
			else
			{
				lua_pushnil(lua);
			}
		}

		static bool GetVal(lua_State* lua, std::shared_ptr<lua::CallbackFunction>& tar)
		{
			if (!lua_isfunction(lua, 3))
			{
				tar.reset();
				return false;
			}
			tar = std::make_shared<lua::CallbackFunction>(lua, 3);
			return true;
		}
	};

	template<>
	struct LuaParamMapping<ParamType::ListOfVec3> : LuaWrappedParamMapping<ListOfVec3Type, std::vector<glm::vec3>> {};

	template<ParamType PT>
	static int LuaTryPushVal(lua_State* lua, std::shared_ptr<ParameterBinding::ParamBindingBase> param, sgrottel::ISimpleLog& log)
	{
		const ParamTypeInfo_t<PT>* v = ParameterBinding::GetValueSource<PT>(param.get());
		if (v == nullptr)
		{
			log.Error("Parameter value type mismatch");
			return 0;
		}
		LuaParamMapping<PT>::PushVal(lua, *v);
		return 1;
	}

	template<ParamType PT>
	static bool LuaTryLoadVal(lua_State* lua, std::shared_ptr<ParameterBinding::ParamBindingBase> param, sgrottel::ISimpleLog& log)
	{
		ParamTypeInfo_t<PT>* v = ParameterBinding::GetValueTarget<PT>(param.get());
		if (v == nullptr)
		{
			log.Error("Parameter value type mismatch");
			return false;
		}
		if (!LuaParamMapping<PT>::GetVal(lua, *v))
		{
			log.Error("Failed to set parameter value; likely type mismatch");
			return false;
		}
		return true;
	}

	template <size_t... Es>
	consteval auto MakeLuaTryPushValTableValues(std::integer_sequence<size_t, Es...>) {
		return std::array<int(*)(lua_State*, std::shared_ptr<ParameterBinding::ParamBindingBase>, sgrottel::ISimpleLog&), sizeof...(Es)>{
			&LuaTryPushVal<static_cast<ParamType>(Es)>...
		};
	}

	consteval auto MakeLuaTryPushValTable() {
		auto seq = []<size_t... I>(std::index_sequence<I...>) {
			return std::integer_sequence<size_t, static_cast<size_t>(I)...>{};
		}(std::make_index_sequence<static_cast<size_t>(ParamType::LAST)>{});
		return MakeLuaTryPushValTableValues(seq);
	}

	template <size_t... Es>
	consteval auto MakeLuaTryLoadValTableValues(std::integer_sequence<size_t, Es...>) {
		return std::array<bool(*)(lua_State *, std::shared_ptr<ParameterBinding::ParamBindingBase>, sgrottel::ISimpleLog&), sizeof...(Es)>{
			&LuaTryLoadVal<static_cast<ParamType>(Es)>...
		};
	}

	consteval auto MakeLuaTryLoadValTable() {
		auto seq = []<size_t... I>(std::index_sequence<I...>) {
			return std::integer_sequence<size_t, static_cast<size_t>(I)...>{};
		}(std::make_index_sequence<static_cast<size_t>(ParamType::LAST)>{});
		return MakeLuaTryLoadValTableValues(seq);
	}

}

bool CommandType::Init()
{
	static const struct luaL_Reg commandObjectLib_memberFuncs[] = {
		{"__tostring", &CommandType::CallbackCommandToString},
		{"__gc", &CommandType::CallbackDelete},
		{"invoke", &CommandType::CallbackCommandInvoke},
		{"set", &CommandType::CallbackCommandSet},
		{"get", &CommandType::CallbackCommandGet},
		{nullptr, nullptr}
	};
	return InitImpl(commandObjectLib_memberFuncs);
}

int CommandType::CallbackCommandToString(lua_State* lua)
{
	auto cmd = CommandType::LuaGet(lua, 1);
	if (!cmd)
	{
		lua_pushstring(lua, "nil");
		return 1;
	}
	lua_pushstring(lua, cmd->TypeName().c_str());
	return 1;
}

int CommandType::CallbackCommandInvoke(lua_State* lua)
{
	return CallLuaImpl(&CommandType::InvokeImpl, lua);
}

int CommandType::CallbackCommandGet(lua_State* lua)
{
	return CallLuaImpl(&CommandType::GetImpl, lua);
}

int CommandType::CallbackCommandSet(lua_State* lua)
{
	return CallLuaImpl(&CommandType::SetImpl, lua);
}

int CommandType::InvokeImpl(lua_State* lua)
{
	auto cmd = CommandType::LuaGet(lua, 1);
	if (!cmd)
	{
		return 0;
	}

	const char* name = cmd->TypeName().c_str();
	try
	{
		Log().Detail("Invoking %s", name);
		bool rv = cmd->Invoke();
		if (!rv)
		{
			Log().Warning("Invoking %s returned unsuccessful", name);
		}
		lua_pushboolean(lua, rv ? 1 : 0);
		return 1;
	}
	catch (std::exception& ex)
	{
		Log().Error("Exception trying to invoke %s: %s", name, ex.what());
	}
	catch (...)
	{
		Log().Error("Unknown exception trying to invoke %s", name);
	}
	return 0;
}

int CommandType::GetImpl(lua_State* lua)
{
	static constexpr auto functable = MakeLuaTryPushValTable();

	auto cmd = CommandType::LuaGet(lua, 1);
	if (!cmd)
	{
		return 0;
	}

	int nargs = lua_gettop(lua);
	if (nargs != 2)
	{
		Log().Error("Field name argument missing");
		return 0;
	}
	if (!lua_isstring(lua, 2))
	{
		Log().Error("Field name argument of wrong type");
		return 0;
	}

	size_t len;
	std::string name = luaL_tolstring(lua, 2, &len); // copy type string

	std::shared_ptr<ParameterBinding::ParamBindingBase> param = cmd->GetParam(name);
	if (!param)
	{
		Log().Error("Field name %s not found", name.c_str());
		return 0;
	}

	size_t functableIndex = static_cast<size_t>(param->m_type);
	if (functableIndex < functable.size())
	{
		return functable[functableIndex](lua, param, Log());
	}

	Log().Error("Getting field %s value of type %s is not supported", name.c_str(), GetParamTypeName(param->m_type));
	return 0;
}

int CommandType::SetImpl(lua_State* lua)
{
	static constexpr auto functable = MakeLuaTryLoadValTable();

	auto cmd = CommandType::LuaGet(lua, 1);
	if (!cmd)
	{
		return 0;
	}

	int nargs = lua_gettop(lua);
	if (nargs != 3)
	{
		Log().Error("Field name argument missing");
		return 0;
	}
	if (!lua_isstring(lua, 2))
	{
		Log().Error("Field name argument of wrong type");
		return 0;
	}

	size_t len;
	std::string name = luaL_tolstring(lua, 2, &len); // copy type string

	std::shared_ptr<ParameterBinding::ParamBindingBase> param = cmd->GetParam(name);
	if (!param)
	{
		Log().Error("Field name %s not found", name.c_str());
		return 0;
	}
	if (param->m_mode == ParamMode::Out)
	{
		Log().Error("Field name %s is read only", name.c_str());
		return 0;
	}

	size_t functableIndex = static_cast<size_t>(param->m_type);
	if (functableIndex >= functable.size())
	{
		Log().Error("Setting field %s value of type %s is not supported", name.c_str(), GetParamTypeName(param->m_type));
		return 0;
	}

	functable[functableIndex](lua, param, Log());
	return 0;
}
