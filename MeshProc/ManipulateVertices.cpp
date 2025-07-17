#include "ManipulateVertices.h"

#include "lua/CallbackFunction.h"
#include "lua/LuaUtilities.h"
#include "lua/GlmVec3Type.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

ManipulateVertices::ManipulateVertices(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Callback>("Callback", m_callback);
}

bool ManipulateVertices::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_callback)
	{
		Log().Error("Callback is not set");
		return false;
	}

	for (glm::vec3& v : m_mesh->vertices)
	{
		m_callback->Push();
		lua::GlmVec3Type::Push(m_callback->Lua(), v);
		m_callback->Call(1, 1);
		if (lua_isnil(m_callback->Lua(), -1))
		{
			// keep v as is
			lua_pop(m_callback->Lua(), 1);
			continue;
		}
		glm::vec3 r;
		if (lua::GlmVec3Type::TryGet(m_callback->Lua(), -1, r))
		{
			v = r;
		}
		else
		{
			Log().Error("Lua callback returned not a XVec3");
			// fail
			return false;
		}
	}

	return true;
}
