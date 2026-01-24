#include "MultiMeshType.h"

#include "MeshType.h"

using namespace meshproc;
using namespace meshproc::lua;

bool MultiMeshType::Init()
{
	static const struct luaL_Reg memberFuncs[] = {
		{"__tostring", &MultiMeshType::CallbackToString},
		{"__gc", &MultiMeshType::CallbackDelete},
		{"size", &AbstractMultiType<data::Mesh, MultiMeshType>::CallbackSize},
		{"get", &MultiMeshType::CallbackGet},
		{nullptr, nullptr}
	};
	return InitImpl(memberFuncs);
}

int MultiMeshType::CallbackGet(lua_State* lua)
{
	return AbstractMultiType<data::Mesh, MultiMeshType>::CallbackGetImpl<MeshType>(lua);
}
