#include "PlaceMesh.h"

using namespace meshproc;

PlaceMesh::PlaceMesh(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParam("Scene", Scene);
	AddParam("Mesh", Mesh);
	AddParam("Mat", Mat);
	Mat.Put() = glm::mat4{ 1 };
}

bool PlaceMesh::Invoke()
{
	if (!Scene.Put())
	{
		Scene.Put() = std::make_shared<data::Scene>();
	}
	Scene.Put()->m_meshes.push_back({ Mesh.Get(), Mat.Get() });
	return true;
}
