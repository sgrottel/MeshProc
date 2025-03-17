#include "MeshProgram.h"

#include "AbstractCommand.h"
#include "CommandFactory.h"
#include "Parameter.h"
#include "ParameterValue.h"

#include <SimpleLog/SimpleLog.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <cassert>

using namespace meshproc;

MeshProgram::MeshProgram(const sgrottel::ISimpleLog& log)
	: m_log{ log }
{
}

void MeshProgram::Load(std::filesystem::path script, CommandFactory const& factory)
{
	Clear();

	if (!std::filesystem::exists(script))
	{
		m_log.Error(L"Specified script file does not exist: %s", script.wstring().c_str());
		return;
	}

	// TODO: Implement the real thing
	m_log.Warning("The current implementation of `MeshProgram::Load` is a hard-coded demo and not the final function");

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

	template<ParamType PT>
	class ExplicitValue : public ParameterBinding
	{
	public:
		static std::shared_ptr<ParameterBinding::ParamBindingBase> Pack(ParamTypeInfo_t<PT>& value)
		{
			auto p = std::make_shared<ParameterBinding::ParamBinding<ParamMode::Out, PT>>(value);
			return p;
		}
	};

	bool DemoConstSetter(ParameterValue& p, ParamType t, const std::string& s)
	{
		if (t == ParamType::Float)
		{
			float f = static_cast<float>(std::atof(s.c_str()));
			p.Push(*ExplicitValue<ParamType::Float>::Pack(f));
			return true;
		}

		if (t == ParamType::Mat4)
		{
			glm::mat4 m;
			if (s == "0")
			{
				m = glm::mat4{ 1.0f };
				p.Push(*ExplicitValue<ParamType::Mat4>::Pack(m));
				return true;
			}
			if (s == "1")
			{
				m = glm::translate(glm::vec3(0.5f, 0.0f, 1.5f));
				p.Push(*ExplicitValue<ParamType::Mat4>::Pack(m));
				return true;
			}
		}

		if (t == ParamType::UInt32)
		{
			uint32_t u = static_cast<uint32_t>(std::atoi(s.c_str()));
			p.Push(*ExplicitValue<ParamType::UInt32>::Pack(u));
			return true;
		}

		if (t == ParamType::String)
		{
			std::wstring w;
			w.resize(s.size());
			std::transform(s.begin(), s.end(), w.begin(),
				// super-rubbish, but I don't care for now
				[](char c) { return static_cast<wchar_t>(c); }
			);
			p.Push(*ExplicitValue<ParamType::String>::Pack(w));
			return true;
		}

		return false;
	}
}

void MeshProgram::Execution() const
{
	m_log.Message("Executing Mesh Program:");
	int pidx = 0;

	std::unordered_map<std::string, std::shared_ptr<ParameterValue>> variables;

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

		for (auto const& s : i.setConstParam)
		{
			ParamMode pm = i.cmd->GetParamMode(s.first);
			if (pm == ParamMode::LAST)
			{
				m_log.Error("Cannot get parameter \"%s\": not found", s.first.c_str());
				continue;
			}
			if (pm == ParamMode::Out)
			{
				m_log.Error("Cannot get parameter \"%s\": mode is Out (only)", s.first.c_str());
				continue;
			}

			ParamType pt = i.cmd->GetParamType(s.first);
			assert(pt != ParamType::LAST);

			ParameterValue constValue;

			// TODO: Implement the real thing
			if (!DemoConstSetter(constValue, pt, s.second))
			{
				m_log.Error("Cannot set parameter \"%s\": failed to parse const description", s.first.c_str());
				continue;
			}

			if (!i.cmd->PushParamValue(s.first, constValue))
			{
				m_log.Error("Cannot set parameter \"%s\": failed to push variable", s.first.c_str());
				continue;
			}
		}

		for (auto const& s : i.setVarParam)
		{
			ParamMode pm = i.cmd->GetParamMode(s.first);
			if (pm == ParamMode::LAST)
			{
				m_log.Error("Cannot get parameter \"%s\": not found", s.first.c_str());
				continue;
			}
			if (pm == ParamMode::Out)
			{
				m_log.Error("Cannot get parameter \"%s\": mode is Out (only)", s.first.c_str());
				continue;
			}

			auto v = variables.find(s.second);
			if (v == variables.end())
			{
				m_log.Error("Cannot set parameter \"%s\": variable not found", s.first.c_str());
				continue;
			}

			ParamType pt = i.cmd->GetParamType(s.first);
			assert(pt != ParamType::LAST);
			if (v->second->GetType() != pt)
			{
				m_log.Error("Cannot set parameter \"%s\": variable type different", s.first.c_str());
				continue;
			}

			if (!i.cmd->PushParamValue(s.first , *v->second))
			{
				m_log.Error("Cannot set parameter \"%s\": failed to push variable", s.first.c_str());
				continue;
			}
		}

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
			ParamMode pm = i.cmd->GetParamMode(g.first);
			if (pm == ParamMode::LAST)
			{
				m_log.Error("Cannot get parameter \"%s\": not found", g.first.c_str());
				continue;
			}
			if (pm == ParamMode::In)
			{
				m_log.Error("Cannot get parameter \"%s\": mode is in (only)", g.first.c_str());
				continue;
			}

			ParamType pt = i.cmd->GetParamType(g.first);
			assert(pt < ParamType::LAST);

			std::shared_ptr<ParameterValue> v = std::make_shared<ParameterValue>();
			if (!i.cmd->PullParamValue(*v, g.first))
			{
				m_log.Error("Cannot get parameter \"%s\": failed to pull variable", g.first.c_str());
				continue;
			}

			variables[g.second] = v;
		}

		pidx++;

	}

	m_log.Message("Mesh Program execution completed.");
}
