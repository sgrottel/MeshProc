#include "MeshProgram.h"

#include "AbstractCommand.h"
#include "CommandFactory.h"
#include "Parameter.h"

#include <SimpleLog/SimpleLog.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

using namespace meshproc;

MeshProgram::MeshProgram(const sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

void MeshProgram::Load(CommandFactory const& factory)
{
	m_log.Warning("The current implementation of `MeshProgram::Load` is a hard-coded demo and not the final function");
	// TODO: Implement the real thing

	m_program.push_back({ "generator.Cube", factory.Instantiate("generator.Cube", m_log) });
	m_program.back().setConstParam["SizeX"] = "2";
	m_program.back().getVarParam["Mesh"] = "cube";

	m_program.push_back({ "PlaceMesh", factory.Instantiate("PlaceMesh", m_log) });
	m_program.back().setVarParam["Mesh"] =  "cube";
	m_program.back().setConstParam["Mat"] =  "0";
	m_program.back().getVarParam["Scene"] =  "scene";

	m_program.push_back({ "generator.SphereIco", factory.Instantiate("generator.SphereIco", m_log) });
	m_program.back().setConstParam["Iterations"] = "4";
	m_program.back().getVarParam["Mesh"] = "cube";

	m_program.push_back({ "PlaceMesh", factory.Instantiate("PlaceMesh", m_log) });
	m_program.back().setVarParam["Scene"] = "scene";
	m_program.back().setVarParam["Mesh"] = "cube";
	m_program.back().setConstParam["Mat"] = "1";
	m_program.back().getVarParam["Scene"] = "scene";

	m_program.push_back({ "io.StlWriter", factory.Instantiate("io.StlWriter", m_log) });
	m_program.back().setVarParam["Scene"] =  "scene";
	m_program.back().setConstParam["Path"] =  "cube.stl";

}

namespace
{

	//bool DemoConstSetter(ParameterBase& p, const std::string& s)
	//{
	//	auto* flt1 = dynamic_cast<Parameter<float, ParamMode::In>*>(&p);
	//	if (flt1 != nullptr)
	//	{
	//		flt1->Put() = static_cast<float>(std::atof(s.c_str()));
	//		return true;
	//	}

	//	auto* mat1 = dynamic_cast<Parameter<glm::mat4, ParamMode::In>*>(&p);
	//	if (mat1 != nullptr)
	//	{
	//		if (s == "0")
	//		{
	//			mat1->Put() = glm::mat4{ 1.0f };
	//			return true;
	//		}
	//		if (s == "1")
	//		{
	//			mat1->Put() = glm::translate(glm::vec3(0.0f, 0.0f, 1.0f));
	//			return true;
	//		}
	//	}

	//	auto* uint1 = dynamic_cast<Parameter<uint32_t, ParamMode::In>*> (&p);
	//	if (uint1 != nullptr)
	//	{
	//		uint1->Put() = static_cast<uint32_t>(std::atoi(s.c_str()));
	//		return true;
	//	}

	//	auto* path1 = dynamic_cast<Parameter<std::filesystem::path, ParamMode::In>*> (&p);
	//	if (path1 != nullptr)
	//	{
	//		path1->Put() = s;
	//		return true;
	//	}

	//	return false;
	//}
}

void MeshProgram::Execution() const
{
	m_log.Message("Executing Mesh Program:");
	int pidx = 0;

	// std::unordered_map<std::string, std::shared_ptr<ParameterBase>> variables;

	while (pidx < m_program.size())
	{
		Instruction const& i = m_program[pidx];

		// TODO: special instrucations for:
		//  - control flow - if, loops, jumps, etc. (think assembler)
		//  - variable release (any better way to do that? jumps make thinks complicated)

		if (!i.cmd)
		{
			m_log.Critical("[%d]: command null", pidx + 1);
			return;
		}

	// 		for (auto const& s : i.setConstParam)
	// 		{
	// 			auto p = i.cmd->AccessParam(s.first);
	// 			if (p == nullptr)
	// 			{
	// 				m_log.Error("Cannot set parameter \"%s\": not found", s.first.c_str());
	// 				continue;
	// 			}
	// 			if (!p->IsWritable())
	// 			{
	// 				m_log.Error("Cannot set parameter \"%s\": not writable", s.first.c_str());
	// 				continue;
	// 			}
	// 
	// 			// TODO: Implement the real thing
	// 
	// 			if (!DemoConstSetter(*p, s.second))
	// 			{
	// 				m_log.Error("Failed to set parameter \"%s\"", s.first.c_str());
	// 				continue;
	// 			}
	// 
	// 		}

	// 		for (auto const& s : i.setVarParam)
	// 		{
	// 			auto p = i.cmd->AccessParam(s.first);
	// 			if (p == nullptr)
	// 			{
	// 				m_log.Error("Cannot set parameter \"%s\": not found", s.first.c_str());
	// 				continue;
	// 			}
	// 			if (!p->IsWritable())
	// 			{
	// 				m_log.Error("Cannot set parameter \"%s\": not writable", s.first.c_str());
	// 				continue;
	// 			}
	// 
	// 			auto v = variables.find(s.second);
	// 			if (v == variables.end())
	// 			{
	// 				m_log.Error("Cannot set parameter \"%s\": variable not found", s.first.c_str());
	// 				continue;
	// 			}
	// 
	// 			p->SetVariable(v->second);
	// 		}

		bool invokeResult = false;
		try
		{
			m_log.Detail("%s.Invoke()", i.name.c_str());
			invokeResult = i.cmd->Invoke();
		}
		catch (std::exception const& ex)
		{
			m_log.Critical("[%d]: command threw %s", pidx + 1, ex.what());
			invokeResult = false;
		}
		catch (...)
		{
			m_log.Critical("[%d]: command threw unknown exception", pidx + 1);
			invokeResult = false;
		}
		if (!invokeResult)
		{
			m_log.Error("[%d]: command failed", pidx + 1);
			return;
		}

		for (auto const& g : i.getVarParam)
		{
			auto p = i.cmd->AccessParam(g.first);
			if (!p)
			{
				m_log.Error("Cannot get parameter \"%s\": not found", g.first.c_str());
				continue;
			}

	// 			auto v = p->GetVariable();
	// 			if (!v)
	// 			{
	// 				m_log.Error("Cannot get parameter \"%s\": failed to get variable", g.first.c_str());
	// 				continue;
	// 			}
	// 
	// 			variables[g.second] = v;
		}

		pidx++;

		break; // THIS IS DEBUGGING! REMOVE ME!

	}

	// 	m_log.Message("Mesh Program execution completed.");

	m_log.Critical("Not implemented!");
}
