#include "CommandRegistration.h"

#include "CommandFactory.h"

#include "CloseLoopWithPin.h"
#include "Convex2DHull.h"
#include "DevPlayground.h"
#include "Extract2DLoops.h"
#include "ExtractSubMesh.h"
#include "FlatSkirt.h"
#include "generator/CrystalGrain.h"
#include "generator/Cube.h"
#include "generator/Icosahedron.h"
#include "generator/LinearExtrude2DMesh.h"
#include "generator/SphereIco.h"
#include "io/CsvShape2DWriter.h"
#include "io/ObjReader.h"
#include "io/PlyReader.h"
#include "io/PlyWriter.h"
#include "io/StlReader.h"
#include "io/StlWriter.h"
#include "io/SvgShape2DWriter.h"
#include "OpenBorder.h"
#include "SelectBottomTriangles.h"
#include "SelectVertexSelection.h"

#include <SimpleLog/SimpleLog.hpp>

bool meshproc::CommandRegistration(class CommandFactory& factory, const sgrottel::ISimpleLog& log)
{
	bool succ = true;
	log.Detail("Populating CommandFactory");

	succ &= factory.Register<CloseLoopWithPin>("CloseLoopWithPin");
	succ &= factory.Register<Convex2DHull>("Convex2DHull");
	succ &= factory.Register<DevPlayground>("DevPlayground");
	factory.HideCommand("DevPlayground");
	succ &= factory.Register<Extract2DLoops>("Extract2DLoops");
	succ &= factory.Register<ExtractSubMesh>("ExtractSubMesh");
	succ &= factory.Register<FlatSkirt>("FlatSkirt");
	succ &= factory.Register<generator::CrystalGrain>("generator.CrystalGrain");
	succ &= factory.Register<generator::Cube>("generator.Cube");
	succ &= factory.Register<generator::Icosahedron>("generator.Icosahedron");
	succ &= factory.Register<generator::LinearExtrude2DMesh>("generator.LinearExtrude2DMesh");
	succ &= factory.Register<generator::SphereIco>("generator.SphereIco");
	succ &= factory.Register<io::CsvShape2DWriter>("io.CsvShape2DWriter");
	succ &= factory.Register<io::ObjReader>("io.ObjReader");
	succ &= factory.Register<io::PlyReader>("io.PlyReader");
	succ &= factory.Register<io::PlyWriter>("io.PlyWriter");
	succ &= factory.Register<io::StlReader>("io.StlReader");
	succ &= factory.Register<io::StlWriter>("io.StlWriter");
	succ &= factory.Register<io::SvgShape2DWriter>("io.SvgShape2DWriter");
	succ &= factory.Register<OpenBorder>("OpenBorder");
	succ &= factory.Register<SelectBottomTriangles>("SelectBottomTriangles");
	succ &= factory.Register<SelectVertexSelection>("SelectVertexSelection");

	// Add further Commands to this registration function

	log.Detail("Populated CommandFactory: %s", succ ? "success" : "failed");
	return succ;
}
