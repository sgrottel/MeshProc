#include "SceneType.h"

using namespace meshproc;
using namespace meshproc::lua;

bool SceneType::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &SceneType::CallbackToString},
		{"__gc", &SceneType::CallbackDelete},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}
