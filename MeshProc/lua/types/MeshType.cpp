#include "MeshType.h"

using namespace meshproc;
using namespace meshproc::lua;
using namespace meshproc::lua::types;

bool MeshType::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &MeshType::CallbackToString},
		{"__gc", &MeshType::CallbackDelete},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}
