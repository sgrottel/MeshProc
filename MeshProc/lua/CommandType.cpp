#include "CommandType.h"

#include "MeshType.h"
#include "MultiMeshType.h"
#include "MultiVertexSelectionType.h"
#include "SceneType.h"
#include "VertexSelectionType.h"

#include "AbstractCommand.h"
#include "utilities/StringUtilities.h"

#include <SimpleLog/SimpleLog.hpp>

#include <lua.hpp>

#include <glm/gtc/type_ptr.hpp>

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
		static bool SetVal(lua_State* lua, uint32_t& tar)
		{
			if (lua_isnumber(lua, 3))
			{
				tar = static_cast<uint32_t>(lua_tonumber(lua, 3));
				return true;
			}
			if (lua_isinteger(lua, 3))
			{
				tar = static_cast<uint32_t>(lua_tointeger(lua, 3));
				return true;
			}
			return false;
		}
	};

	template<>
	struct LuaParamMapping<ParamType::Float>
	{
		static void PushVal(lua_State* lua, const float& v)
		{
			lua_pushnumber(lua, v);
		}
		static bool SetVal(lua_State* lua, float& tar)
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
		static bool SetVal(lua_State* lua, std::wstring& tar)
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
		static bool SetVal(lua_State* lua, std::shared_ptr<NATIVETYPE>& tar)
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
	struct LuaParamMapping<ParamType::VertexSelection> : LuaWrappedParamMapping<VertexSelectionType, std::vector<uint32_t>> {};

	template<>
	struct LuaParamMapping<ParamType::MultiVertexSelection> : LuaWrappedParamMapping<MultiVertexSelectionType, std::vector<std::shared_ptr<std::vector<uint32_t>>>> {};

	template<>
	struct LuaParamMapping<ParamType::Mat4>
	{
		static void PushVal(lua_State* lua, const glm::mat4& v)
		{
			glm::mat4 m = glm::transpose(v);
			auto values = glm::value_ptr(m);

			// Step 1: Create the table
			lua_createtable(lua, 16, 0); // Preallocate array part with 16 elements
			for (int i = 0; i < 16; ++i) {
				lua_pushnumber(lua, values[i]);
				lua_rawseti(lua, -2, i + 1); // Lua is 1-indexed
			}

			// Step 2: Set the XMat4 metatable
			lua_getfield(lua, LUA_REGISTRYINDEX, "XMat4_mt");
			if (!lua_istable(lua, -1)) {
				lua_pop(lua, 2); // cleanup table and non-table
				luaL_error(lua, "XMat4_mt not found in registry. Is xyz_math loaded?");
				return;
			}
			lua_setmetatable(lua, -2); // Set metatable on the matrix table
		}

		static bool SetVal(lua_State* lua, glm::mat4& tar)
		{
			luaL_checktype(lua, 3, LUA_TTABLE);
			auto tableLen = lua_rawlen(lua, 3);
			if (tableLen == 16)
			{ // assume 4x4 matrix
				lua_getfield(lua, LUA_REGISTRYINDEX, "XMat4_mt"); // push expected metatable
				if (!lua_getmetatable(lua, 3)) { // push actual metatable of value
					return luaL_error(lua, "Expected XMat4, but value has no metatable");
				}
				if (!lua_rawequal(lua, -1, -2)) {
					return luaL_error(lua, "Expected XMat4, incorrect metatable");
				}
				lua_pop(lua, 2); // pop both metatables

				// Extract matrix
				glm::mat4 m;
				float* mat = glm::value_ptr(m);
				for (int i = 0; i < 16; ++i) {
					lua_rawgeti(lua, 3, i + 1);
					mat[i] = (float)luaL_checknumber(lua, -1);
					lua_pop(lua, 1);
				}
				tar = glm::transpose(m);
				return true;
			}
			else if (tableLen == 9)
			{ // assume 3x3 matrix
				lua_getfield(lua, LUA_REGISTRYINDEX, "XMat3_mt"); // push expected metatable
				if (!lua_getmetatable(lua, 3)) { // push actual metatable of value
					return luaL_error(lua, "Expected XMat4, but value has no metatable");
				}
				if (!lua_rawequal(lua, -1, -2)) {
					return luaL_error(lua, "Expected XMat4, incorrect metatable");
				}
				lua_pop(lua, 2); // pop both metatables

				// Extract matrix
				glm::mat3 m;
				float* mat = glm::value_ptr(m);
				for (int i = 0; i < 9; ++i) {
					lua_rawgeti(lua, 3, i + 1);
					mat[i] = (float)luaL_checknumber(lua, -1);
					lua_pop(lua, 1);
				}
				tar = glm::transpose(m);
				return true;
			}
			else
			{
				return luaL_error(lua, "Expected XMat4, but value has wrong size");
			}

			return false;
		}
	};

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
	static bool LuaTrySetVal(lua_State* lua, std::shared_ptr<ParameterBinding::ParamBindingBase> param, sgrottel::ISimpleLog& log)
	{
		ParamTypeInfo_t<PT>* v = ParameterBinding::GetValueTarget<PT>(param.get());
		if (v == nullptr)
		{
			log.Error("Parameter value type mismatch");
			return false;
		}
		if (!LuaParamMapping<PT>::SetVal(lua, *v))
		{
			log.Error("Failed to set parameter value; likely type mismatch");
			return false;
		}
		return true;
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

	switch (param->m_type)
	{
	case ParamType::UInt32:
		return LuaTryPushVal<ParamType::UInt32>(lua, param, Log());
	case ParamType::Float:
		return LuaTryPushVal<ParamType::Float>(lua, param, Log());
	case ParamType::String:
		return LuaTryPushVal<ParamType::String>(lua, param, Log());
		//	Vec3, ==> Look at library https://github.com/xyz-ai-dev/xyz_math
	case ParamType::Mat4:
		return LuaTryPushVal<ParamType::Mat4>(lua, param, Log());
	case ParamType::Mesh:
		return LuaTryPushVal<ParamType::Mesh>(lua, param, Log());
	case ParamType::MultiMesh:
		return LuaTryPushVal<ParamType::MultiMesh>(lua, param, Log());
	case ParamType::Scene:
		return LuaTryPushVal<ParamType::Scene>(lua, param, Log());
	case ParamType::VertexSelection:
		return LuaTryPushVal<ParamType::VertexSelection>(lua, param, Log());
	case ParamType::MultiVertexSelection:
		return LuaTryPushVal<ParamType::MultiVertexSelection>(lua, param, Log());
	default:
		Log().Error("Getting field %s value of type %s is not supported", name.c_str(), GetParamTypeName(param->m_type));
		return 0;
	}

	return 1;
}

int CommandType::SetImpl(lua_State* lua)
{
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

	switch (param->m_type)
	{
	case ParamType::UInt32:
		LuaTrySetVal<ParamType::UInt32>(lua, param, Log());
		break;
	case ParamType::Float:
		LuaTrySetVal<ParamType::Float>(lua, param, Log());
		break;
	case ParamType::String:
		LuaTrySetVal<ParamType::String>(lua, param, Log());
		break;
		//	Vec3, ==> Look at library https://github.com/xyz-ai-dev/xyz_math
	case ParamType::Mat4:
		LuaTrySetVal<ParamType::Mat4>(lua, param, Log());
		break;
	case ParamType::Mesh:
		LuaTrySetVal<ParamType::Mesh>(lua, param, Log());
		break;
	case ParamType::MultiMesh:
		LuaTrySetVal<ParamType::MultiMesh>(lua, param, Log());
		break;
	case ParamType::Scene:
		LuaTrySetVal<ParamType::Scene>(lua, param, Log());
		break;
	case ParamType::VertexSelection:
		LuaTrySetVal<ParamType::VertexSelection>(lua, param, Log());
		break;
	case ParamType::MultiVertexSelection:
		LuaTrySetVal<ParamType::MultiVertexSelection>(lua, param, Log());
		break;
	default:
		Log().Error("Setting field %s value of type %s is not supported", name.c_str(), GetParamTypeName(param->m_type));
		return 0;
	}

	return 0;
}
