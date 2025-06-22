#include "VertexSelectionType.h"

using namespace meshproc;
using namespace meshproc::lua;

bool VertexSelectionType::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &VertexSelectionType::CallbackToString},
		{"__gc", &VertexSelectionType::CallbackDelete},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}
