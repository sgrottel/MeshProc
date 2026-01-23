#include "CommandRegistration.h"

namespace
{
	template<int N>
	struct RegisterCommandHelper {};
}

// define the COMMAND_PATH (namespace, class_name)
// then include "CommandRegistration.inc"
//
// this will generate: 
//   #include "namespace/class_name.h"
//   And the template specialization of `RegisterCommandHelper` for class runtime registration logic

#define COMMAND_PATH compute, LinearColorMap
#include "CommandRegistration.inc"
#define COMMAND_PATH compute, OpenBorder
#include "CommandRegistration.inc"
#define COMMAND_PATH compute, VertexNormals
#include "CommandRegistration.inc"
#define COMMAND_PATH edit, CloseLoopWithPin
#include "CommandRegistration.inc"
#define COMMAND_PATH edit, CutHalfSpace
#include "CommandRegistration.inc"
#define COMMAND_PATH edit, DisplacementNoise
#include "CommandRegistration.inc"
#define COMMAND_PATH edit, Subdivision
#include "CommandRegistration.inc"
#define COMMAND_PATH generator, Cuboid
#include "CommandRegistration.inc"
#define COMMAND_PATH generator, Icosahedron
#include "CommandRegistration.inc"
#define COMMAND_PATH generator, Octahedron
#include "CommandRegistration.inc"
#define COMMAND_PATH generator, SphereIco
#include "CommandRegistration.inc"
#define COMMAND_PATH io, ObjReader
#include "CommandRegistration.inc"
#define COMMAND_PATH io, ObjWriter
#include "CommandRegistration.inc"
#define COMMAND_PATH io, PlyReader
#include "CommandRegistration.inc"
#define COMMAND_PATH io, PlyWriter
#include "CommandRegistration.inc"
#define COMMAND_PATH io, StlReader
#include "CommandRegistration.inc"
#define COMMAND_PATH io, StlWriter
#include "CommandRegistration.inc"

// add more commands here

#include "CommandFactory.h"

#include <SimpleLog/SimpleLog.hpp>

namespace
{

	template<int N>
	bool CallRegisterCommandHelper(meshproc::commands::CommandFactory& factory);
	
	template<>
	bool CallRegisterCommandHelper<-1>(meshproc::commands::CommandFactory& factory)
	{
		return true;
	}

	template<int N>
	bool CallRegisterCommandHelper(meshproc::commands::CommandFactory& factory)
	{
		if (!CallRegisterCommandHelper<N - 1>(factory))
		{
			return false;
		}
		return factory.Register<typename RegisterCommandHelper<N>::T>(RegisterCommandHelper<N>::NAME);
	}
}

bool meshproc::commands::CommandRegistration(CommandFactory& factory, const sgrottel::ISimpleLog& log)
{
	bool succ = true;
	log.Detail("Populating CommandFactory");

	CallRegisterCommandHelper<__COUNTER__ - 1>(factory);

	// Here you can add code, which cannot be handled by the `RegisterCommandHelper` construct
	//factory.HideCommand("DevPlayground");

	log.Detail("Populated CommandFactory: %s", succ ? "success" : "failed");
	return succ;
}
