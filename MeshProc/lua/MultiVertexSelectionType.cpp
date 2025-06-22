#include "MultiVertexSelectionType.h"

using namespace meshproc;
using namespace meshproc::lua;

bool MultiVertexSelectionType::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &MultiVertexSelectionType::CallbackToString},
		{"__gc", &MultiVertexSelectionType::CallbackDelete},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}
