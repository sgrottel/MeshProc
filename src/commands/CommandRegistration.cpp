#include "CommandRegistration.h"

#ifdef REGISTER_COMMAND
#error REGISTER_COMMAND must not be registed yet!
#endif

#include "AbstractCommand.h"
#include "CommandRegistration.inc"

#ifndef REGISTER_COMMAND
#error REGISTER_COMMAND failed to register
#endif

// Include all commands here:

#include "compute/LinearColorMap.h"
#include "compute/OpenBorder.h"
#include "compute/SplitByEdges.h"
#include "compute/VertexEdgeDistance.h"
#include "compute/VertexEdgeDistanceToCut.h"
#include "compute/VertexNormals.h"
#include "edit/CloseLoopWithPin.h"
#include "edit/CutHalfSpace.h"
#include "edit/CutPlaneLoop.h"
#include "edit/DisplacementNoise.h"
#include "edit/InvertVertexSelection.h"
#include "edit/SelectConnectedComponentVertices.h"
#include "edit/SimpleSmooth.h"
#include "edit/Subdivision.h"
#include "edit/VertexColorDyeThrough.h"
#include "generator/Cuboid.h"
#include "generator/Icosahedron.h"
#include "generator/Octahedron.h"
#include "generator/SphereIco.h"
#include "io/Model3mfReader.h"
#include "io/ObjReader.h"
#include "io/ObjWriter.h"
#include "io/PlyReader.h"
#include "io/PlyWriter.h"
#include "io/StlReader.h"
#include "io/StlWriter.h"
#include "io/VertexColorReader.h"
#include "io/VertexColorWriter.h"
#include "util/TriggerMeshLabRefresh.h"

// Now, included commands will be wrapped in the factory

#include "CommandFactory.h"

#include <SimpleLog/SimpleLog.hpp>

static_assert(::meshproc::commands::_utils::Guard<__COUNTER__>::val == 0);

namespace
{

	using FinalRegistrationHelper = CommandRegistrationHelper<__COUNTER__>;

	template<typename TCMD>
	struct RegisterAction
	{
		static bool RegisterCommandType(meshproc::commands::CommandFactory& factory, const char* name)
		{
			return factory.Register<TCMD>(name);
		}
	};

	template<>
	struct RegisterAction<void>
	{
		static bool RegisterCommandType([[maybe_unused]] meshproc::commands::CommandFactory& factory, [[maybe_unused]] const char* name)
		{
			return true;
		}
	};

	template<int I>
	struct RegisterCountedCommand
	{
		static bool RegisterCommandType(meshproc::commands::CommandFactory& factory, const sgrottel::ISimpleLog& log)
		{
			const bool preSucc = RegisterCountedCommand<I - 1>::RegisterCommandType(factory, log);
			if (!preSucc)
			{
				return false;
			}

			const bool succ = RegisterAction<CommandRegistrationHelper<I>::TYPE>::RegisterCommandType(factory, CommandRegistrationHelper<I>::NAME);
			if (!succ)
			{
				log.Error("Failed to register command (%d) \"%s\"", CommandRegistrationHelper<I>::VAL, CommandRegistrationHelper<I>::NAME);
				return false;
			}

			return true;
		}
	};

	template<>
	struct RegisterCountedCommand<-1>
	{
		static bool RegisterCommandType(
			[[maybe_unused]] meshproc::commands::CommandFactory& factory,
			[[maybe_unused]] const sgrottel::ISimpleLog& log)
		{
			return true;
		}
	};

}

bool meshproc::commands::CommandRegistration(CommandFactory& factory, const sgrottel::ISimpleLog& log)
{
	log.Detail("Populating CommandFactory");

	const bool succ = RegisterCountedCommand<FinalRegistrationHelper::VAL>::RegisterCommandType(factory, log);

	// add code here, which cannot be handled by the `RegisterCommandHelper` construct:
	//factory.HideCommand("DevPlayground");

	log.Detail("Populated CommandFactory: %s", succ ? "success" : "failed");

	return succ;
}
