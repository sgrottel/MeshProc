#include "MultiMeshType.h"

using namespace meshproc;
using namespace meshproc::lua;

bool MultiMeshType::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &MultiMeshType::CallbackToString},
		{"__gc", &MultiMeshType::CallbackDelete},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}
