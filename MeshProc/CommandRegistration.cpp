#include "CommandRegistration.h"

#include "CommandFactory.h"

#include "CloseLoopWithPin.h"
#include "Convex2DHull.h"
#include "CollapseTriangles.h"
#include "CutHalfSpace.h"
#include "DevPlayground.h"
#include "DisplacementNoise.h"
#include "Extract2DLoops.h"
#include "ExtractSubMesh.h"
#include "FlatSkirt.h"
#include "LinearExtrude.h"
#include "generator/CrystalGrain.h"
#include "generator/Cuboid.h"
#include "generator/Icosahedron.h"
#include "generator/LinearExtrude2DMesh.h"
#include "generator/RotateExtrude2DMesh.h"
#include "generator/SphereIco.h"
#include "generator/VertexNormals.h"
#include "io/CsvShape2DReader.h"
#include "io/CsvShape2DWriter.h"
#include "io/ObjReader.h"
#include "io/PlyReader.h"
#include "io/PlyWriter.h"
#include "io/StlReader.h"
#include "io/StlWriter.h"
#include "io/SvgShape2DWriter.h"
#include "ManipulateVertices.h"
#include "MeasureBoundingBox.h"
#include "OpenBorder.h"
#include "SelectBottomTriangles.h"
#include "SelectRandomTriangles.h"
#include "Subdivision.h"

#include <SimpleLog/SimpleLog.hpp>

bool meshproc::CommandRegistration(class CommandFactory& factory, const sgrottel::ISimpleLog& log)
{
	bool succ = true;
	log.Detail("Populating CommandFactory");

	succ &= factory.Register<CloseLoopWithPin>("CloseLoopWithPin");
	succ &= factory.Register<Convex2DHull>("Convex2DHull");
	succ &= factory.Register<CollapseTriangles>("CollapseTriangles");
	succ &= factory.Register<CutHalfSpace>("CutHalfSpace");
	succ &= factory.Register<DevPlayground>("DevPlayground");
	factory.HideCommand("DevPlayground");
	succ &= factory.Register<DisplacementNoise>("DisplacementNoise");
	succ &= factory.Register<Extract2DLoops>("Extract2DLoops");
	succ &= factory.Register<ExtractSubMesh>("ExtractSubMesh");
	succ &= factory.Register<FlatSkirt>("FlatSkirt");
	succ &= factory.Register<LinearExtrude>("LinearExtrude");
	succ &= factory.Register<generator::CrystalGrain>("generator.CrystalGrain");
	succ &= factory.Register<generator::Cuboid>("generator.Cuboid");
	succ &= factory.Register<generator::Icosahedron>("generator.Icosahedron");
	succ &= factory.Register<generator::LinearExtrude2DMesh>("generator.LinearExtrude2DMesh");
	succ &= factory.Register<generator::RotateExtrude2DMesh>("generator.RotateExtrude2DMesh");
	succ &= factory.Register<generator::SphereIco>("generator.SphereIco");
	succ &= factory.Register<generator::VertexNormals>("generator.VertexNormals");
	succ &= factory.Register<io::CsvShape2DReader>("io.CsvShape2DReader");
	succ &= factory.Register<io::CsvShape2DWriter>("io.CsvShape2DWriter");
	succ &= factory.Register<io::ObjReader>("io.ObjReader");
	succ &= factory.Register<io::PlyReader>("io.PlyReader");
	succ &= factory.Register<io::PlyWriter>("io.PlyWriter");
	succ &= factory.Register<io::StlReader>("io.StlReader");
	succ &= factory.Register<io::StlWriter>("io.StlWriter");
	succ &= factory.Register<io::SvgShape2DWriter>("io.SvgShape2DWriter");
	succ &= factory.Register<ManipulateVertices>("ManipulateVertices");
	succ &= factory.Register<MeasureBoundingBox>("MeasureBoundingBox");
	succ &= factory.Register<OpenBorder>("OpenBorder");
	succ &= factory.Register<SelectBottomTriangles>("SelectBottomTriangles");
	succ &= factory.Register<SelectRandomTriangles>("SelectRandomTriangles");
	succ &= factory.Register<Subdivision>("Subdivision");

	// Add further Commands to this registration function

	log.Detail("Populated CommandFactory: %s", succ ? "success" : "failed");
	return succ;
}
