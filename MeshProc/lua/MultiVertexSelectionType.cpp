#include "MultiVertexSelectionType.h"

#include "VertexSelectionType.h"

using namespace meshproc;
using namespace meshproc::lua;

bool MultiVertexSelectionType::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &MultiVertexSelectionType::CallbackToString},
		{"__gc", &MultiVertexSelectionType::CallbackDelete},
		{"size", &AbstractMultiType<std::vector<uint32_t>, MultiVertexSelectionType>::CallbackSize},
		{"get", &MultiVertexSelectionType::CallbackGet},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}

int MultiVertexSelectionType::CallbackGet(lua_State* lua)
{
	return AbstractMultiType<std::vector<uint32_t>, MultiVertexSelectionType>::CallbackGetImpl<VertexSelectionType>(lua);
}
