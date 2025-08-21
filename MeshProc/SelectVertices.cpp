#include "SelectVertices.h"

#include "lua/CallbackFunction.h"
#include "lua/LuaUtilities.h"
#include "lua/GlmVec3Type.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

SelectVertices::SelectVertices(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Callback>("Callback", m_callback);
	AddParamBinding<ParamMode::Out, ParamType::Indices>("Selection", m_selection);
}

bool SelectVertices::Invoke()
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

	m_selection = std::make_shared<std::vector<uint32_t>>();

	for (size_t i = 0; i < m_mesh->vertices.size(); ++i)
	{
		const glm::vec3& v = m_mesh->vertices.at(i);

		m_callback->Push();
		lua::GlmVec3Type::Push(m_callback->Lua(), v);
		m_callback->Call(1, 1);

		if (!lua_isboolean(m_callback->Lua(), -1))
		{
			// keep v as is
			lua_pop(m_callback->Lua(), 1);
			continue;
		}

		int b = lua_toboolean(m_callback->Lua(), -1);
		if (b)
		{
			m_selection->push_back(static_cast<uint32_t>(i));
		}
	}

	Log().Detail("Selected %d / %d vertices", static_cast<int>(m_selection->size()), static_cast<int>(m_mesh->vertices.size()));

	return true;
}
