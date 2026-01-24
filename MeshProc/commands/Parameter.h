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
		class HalfSpace;
		class Mesh;
		class Scene;
		class Shape2D;
	}

	namespace commands
	{

		enum class ParamType
		{
			UInt32,
			Float,
			FloatList,
			String,
			Vec3,
			Vec3List,
			Vec3ListList,
			Mat4,
			Mesh,
//			MultiMesh,
			Scene,
//			Shape2D,
			IndexList, // e.g. vertices, also edges/loops, or triangles
			IndexListList,
			HalfSpace,

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
			static constexpr bool canSetNil = false;
		};

		template<>
		struct ParamTypeInfo<ParamType::Float>
		{
			static constexpr const char* name = "Float";
			typedef float type;
			static constexpr bool canSetNil = false;
		};

		template<>
		struct ParamTypeInfo<ParamType::FloatList>
		{
			static constexpr const char* name = "FloatList";
			typedef std::shared_ptr<std::vector<float>> type;
			static constexpr bool canSetNil = true;
			static type NilVal() { return nullptr; }
		};

		template<>
		struct ParamTypeInfo<ParamType::String>
		{
			static constexpr const char* name = "String";
			typedef std::wstring type;
			static constexpr bool canSetNil = true;
			static type NilVal() { return L""; }
		};

		template<>
		struct ParamTypeInfo<ParamType::Vec3>
		{
			static constexpr const char* name = "Vec3";
			typedef glm::vec3 type;
			static constexpr bool canSetNil = false;
		};

		template<>
		struct ParamTypeInfo<ParamType::Mat4>
		{
			static constexpr const char* name = "Mat4";
			typedef glm::mat4 type;
			static constexpr bool canSetNil = false;
		};

		template<>
		struct ParamTypeInfo<ParamType::Mesh>
		{
			static constexpr const char* name = "Mesh";
			typedef std::shared_ptr<data::Mesh> type;
			static constexpr bool canSetNil = true;
			static type NilVal() { return nullptr; }
		};

		//template<>
		//struct ParamTypeInfo<ParamType::MultiMesh>
		//{
		//	static constexpr const char* name = "MultiMesh";
		//	typedef std::shared_ptr<std::vector<std::shared_ptr<data::Mesh>>> type;
		//	static constexpr bool canSetNil = true;
		//	static type NilVal() { return nullptr; }
		//};

		template<>
		struct ParamTypeInfo<ParamType::Scene>
		{
			static constexpr const char* name = "Scene";
			typedef std::shared_ptr<data::Scene> type;
			static constexpr bool canSetNil = true;
			static type NilVal() { return nullptr; }
		};

		//template<>
		//struct ParamTypeInfo<ParamType::Shape2D>
		//{
		//	static constexpr const char* name = "Shape2D";
		//	typedef std::shared_ptr<data::Shape2D> type;
		//	static constexpr bool canSetNil = true;
		//	static type NilVal() { return nullptr; }
		//};

		template<>
		struct ParamTypeInfo<ParamType::IndexList>
		{
			static constexpr const char* name = "IndexList";
			typedef std::shared_ptr<std::vector<uint32_t>> type;
			static constexpr bool canSetNil = true;
			static type NilVal() { return nullptr; }
		};

		template<>
		struct ParamTypeInfo<ParamType::IndexListList>
		{
			static constexpr const char* name = "IndexListList";
			typedef std::shared_ptr<std::vector< ParamTypeInfo<ParamType::IndexList>::type >> type;
			static constexpr bool canSetNil = true;
			static type NilVal() { return nullptr; }
		};

		template<>
		struct ParamTypeInfo<ParamType::Vec3List>
		{
			static constexpr const char* name = "Vec3List";
			typedef std::shared_ptr<std::vector<glm::vec3>> type;
			static constexpr bool canSetNil = true;
			static type NilVal() { return nullptr; }
		};

		template<>
		struct ParamTypeInfo<ParamType::Vec3ListList>
		{
			static constexpr const char* name = "Vec3ListList";
			typedef std::shared_ptr<std::vector< typename ParamTypeInfo<ParamType::Vec3List>::type >> type;
			static constexpr bool canSetNil = true;
			static type NilVal() { return nullptr; }
		};

		template<>
		struct ParamTypeInfo<ParamType::HalfSpace>
		{
			static constexpr const char* name = "HalfSpace";
			typedef std::shared_ptr<data::HalfSpace> type;
			static constexpr bool canSetNil = true;
			static type NilVal() { return nullptr; }
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
}
