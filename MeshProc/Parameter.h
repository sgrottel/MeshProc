#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace meshproc
{
	namespace data
	{
		class Mesh;
		class Scene;
		class Shape2D;
	}
	namespace lua
	{
		class CallbackFunction;
	}

	enum class ParamType
	{
		UInt32,
		Float,
		String,
		Vec3,
		Mat4,
		Mesh,
		MultiMesh,
		Scene,
		Shape2D,
		VertexSelection, // e.g. also edges/loops
		MultiVertexSelection,
		Callback,

		LAST
	};

	const char* GetParamTypeName(ParamType pt);

	template<ParamType PT>
	struct ParamTypeInfo;

	template<>
	struct ParamTypeInfo<ParamType::UInt32>
	{
		static constexpr const char* name = "UInt";
		typedef uint32_t type;
	};

	template<>
	struct ParamTypeInfo<ParamType::Float>
	{
		static constexpr const char* name = "Float";
		typedef float type;
	};

	template<>
	struct ParamTypeInfo<ParamType::String>
	{
		static constexpr const char* name = "String";
		typedef std::wstring type;
	};

	template<>
	struct ParamTypeInfo<ParamType::Vec3>
	{
		static constexpr const char* name = "Vec3";
		typedef glm::vec3 type;
	};

	template<>
	struct ParamTypeInfo<ParamType::Mat4>
	{
		static constexpr const char* name = "Mat4";
		typedef glm::mat4 type;
	};

	template<>
	struct ParamTypeInfo<ParamType::Mesh>
	{
		static constexpr const char* name = "Mesh";
		typedef std::shared_ptr<data::Mesh> type;
	};

	template<>
	struct ParamTypeInfo<ParamType::MultiMesh>
	{
		static constexpr const char* name = "MultiMesh";
		typedef std::shared_ptr<std::vector<std::shared_ptr<data::Mesh>>> type;
	};

	template<>
	struct ParamTypeInfo<ParamType::Scene>
	{
		static constexpr const char* name = "Scene";
		typedef std::shared_ptr<data::Scene> type;
	};

	template<>
	struct ParamTypeInfo<ParamType::Shape2D>
	{
		static constexpr const char* name = "Shape2D";
		typedef std::shared_ptr<data::Shape2D> type;
	};

	template<>
	struct ParamTypeInfo<ParamType::VertexSelection>
	{
		static constexpr const char* name = "VertexSelection";
		typedef std::shared_ptr<std::vector<uint32_t>> type;
	};

	template<>
	struct ParamTypeInfo<ParamType::MultiVertexSelection>
	{
		static constexpr const char* name = "MultiVertexSelection";
		typedef std::shared_ptr<std::vector<std::shared_ptr<std::vector<uint32_t>>>> type;
	};

	template<>
	struct ParamTypeInfo<ParamType::Callback>
	{
		static constexpr const char* name = "Callback";
		typedef std::shared_ptr<lua::CallbackFunction> type;
	};

	template<ParamType PT>
	using ParamTypeInfo_t = typename ParamTypeInfo<PT>::type;

	enum class ParamMode
	{
		In,
		InOut,
		Out,

		LAST
	};

	template<ParamMode PT>
	struct ParamModeInfo;

	template<>
	struct ParamModeInfo<ParamMode::In>
	{
		static constexpr const char* name = "In";
	};

	template<>
	struct ParamModeInfo<ParamMode::InOut>
	{
		static constexpr const char* name = "InOut";
	};

	template<>
	struct ParamModeInfo<ParamMode::Out>
	{
		static constexpr const char* name = "Out";
	};

	const char* GetParamModeName(ParamMode pm);

}
