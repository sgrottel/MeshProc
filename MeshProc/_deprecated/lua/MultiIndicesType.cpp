#include "MultiIndicesType.h"

#include "IndicesType.h"

using namespace meshproc;
using namespace meshproc::lua;

bool MultiIndicesType::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &MultiIndicesType::CallbackToString},
		{"__gc", &MultiIndicesType::CallbackDelete},
		{"size", &AbstractMultiType<std::vector<uint32_t>, MultiIndicesType>::CallbackSize},
		{"get", &MultiIndicesType::CallbackGet},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}

int MultiIndicesType::CallbackGet(lua_State* lua)
{
	return AbstractMultiType<std::vector<uint32_t>, MultiIndicesType>::CallbackGetImpl<IndicesType>(lua);
}
