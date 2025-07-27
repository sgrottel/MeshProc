#include "IndicesType.h"

using namespace meshproc;
using namespace meshproc::lua;

bool IndicesType::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &IndicesType::CallbackToString},
		{"__gc", &IndicesType::CallbackDelete},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}
