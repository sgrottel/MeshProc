#include "CommandRegistration.h"

#include "CommandFactory.h"

#include "CloseLoopWithPin.h"
#include "ExtractSubMesh.h"
#include "FlatSkirt.h"
#include "generator/Cube.h"
#include "generator/Icosahedron.h"
#include "generator/SphereIco.h"
#include "io/ObjReader.h"
#include "io/PlyReader.h"
#include "io/PlyWriter.h"
#include "io/StlReader.h"
#include "io/StlWriter.h"
#include "OpenBorder.h"
#include "PlaceMesh.h"
#include "SelectBottomTriangles.h"
#include "SelectVertexSelection.h"

#include <SimpleLog/SimpleLog.hpp>

bool meshproc::CommandRegistration(class CommandFactory& factory, const sgrottel::ISimpleLog& log)
{
	bool succ = true;
	log.Detail("Populating CommandFactory");

	succ &= factory.Register<CloseLoopWithPin>("CloseLoopWithPin");
	succ &= factory.Register<ExtractSubMesh>("ExtractSubMesh");
	succ &= factory.Register<FlatSkirt>("FlatSkirt");
	succ &= factory.Register<generator::Cube>("generator.Cube");
	succ &= factory.Register<generator::Icosahedron>("generator.Icosahedron");
	succ &= factory.Register<generator::SphereIco>("generator.SphereIco");
	succ &= factory.Register<io::ObjReader>("io.ObjReader");
	succ &= factory.Register<io::PlyReader>("io.PlyReader");
	succ &= factory.Register<io::PlyWriter>("io.PlyWriter");
	succ &= factory.Register<io::StlReader>("io.StlReader");
	succ &= factory.Register<io::StlWriter>("io.StlWriter");
	succ &= factory.Register<OpenBorder>("OpenBorder");
	succ &= factory.Register<PlaceMesh>("PlaceMesh");
	succ &= factory.Register<SelectBottomTriangles>("SelectBottomTriangles");
	succ &= factory.Register<SelectVertexSelection>("SelectVertexSelection");

	// Add further Commands to this registration function

	log.Detail("Populated CommandFactory: %s", succ ? "success" : "failed");
	return succ;
}
