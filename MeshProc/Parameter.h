#pragma once

#include <data/Mesh.h>
#include <data/Scene.h>

#include <glm/glm.hpp>

#include <cassert>
#include <filesystem>
#include <stdexcept>

namespace meshproc
{

	enum class ParamMode {
		In,
		InOut,
		Out
	};

	class ParameterBase
	{
	public:
		virtual ~ParameterBase() = default;
		ParameterBase(const ParameterBase&) = delete;
		ParameterBase(ParameterBase&&) = delete;
		ParameterBase& operator=(const ParameterBase&) = delete;
		ParameterBase& operator=(ParameterBase&&) = delete;

		virtual void PreInvoke() = 0;
		virtual void PostInvoke() = 0;

		virtual const char* TypeStr() const = 0;
		virtual const char* ModeStr() const = 0;

	protected:
		ParameterBase() = default;

	private:
	};

	template<ParamMode PT>
	class LockableParameterBase;

	template<typename T>
	class ParameterTypeInfo;

	template<typename T, ParamMode PT>
	class Parameter : public ParameterBase, public LockableParameterBase<PT>
	{
	public:

		T const& Get() const noexcept
		{
			return m_value;
		}

		T& Put()
		{
			if (!LockableParameterBase<PT>::isWritable)
			{
				throw std::runtime_error("Parameter is locked and read-only");
			}
			return m_value;
		}

	protected:

		void PreInvoke() override
		{
			LockableParameterBase<PT>::ImplPreInvoke();
		}
		void PostInvoke() override
		{
			LockableParameterBase<PT>::ImplPostInvoke();
		}
		const char* TypeStr() const override
		{
			return ParameterTypeInfo<T>{}.TypeStr();
		}
		const char* ModeStr() const override
		{
			return LockableParameterBase<PT>::ModeStr;
		}

	private:
		T m_value{};
	};

	template<>
	class LockableParameterBase<ParamMode::In> {
	protected:
		static constexpr const char* ModeStr = "In";
		bool isWritable{ true };
		inline void ImplPreInvoke()
		{
			assert(isWritable);
			isWritable = false;
		}
		inline void ImplPostInvoke()
		{
			assert(!isWritable);
			isWritable = true;
		}
	};

	template<>
	class LockableParameterBase<ParamMode::InOut> {
	protected:
		static constexpr const char* ModeStr = "InOut";
		static constexpr bool const isWritable{ true };
		inline void ImplPreInvoke() {}
		inline void ImplPostInvoke() {}
	};

	template<>
	class LockableParameterBase<ParamMode::Out> {
	protected:
		static constexpr const char* ModeStr = "Out";
		bool isWritable{ false };
		inline void ImplPreInvoke()
		{
			assert(!isWritable);
			isWritable = true;
		}
		inline void ImplPostInvoke()
		{
			assert(isWritable);
			isWritable = false;
		}
	};

	template<typename T>
	class ParameterTypeInfo
	{
	public:
		inline const char* TypeStr() const
		{
			return typeid(T).name();
		}
	};

	template<>
	class ParameterTypeInfo<float>
	{
	public:
		inline const char* TypeStr() const
		{
			return "float";
		}
	};

	template<>
	class ParameterTypeInfo<uint32_t>
	{
	public:
		inline const char* TypeStr() const
		{
			return "uint32";
		}
	};

	template<>
	class ParameterTypeInfo<std::filesystem::path>
	{
	public:
		inline const char* TypeStr() const
		{
			return "string (path)";
		}
	};

	template<>
	class ParameterTypeInfo<std::shared_ptr<data::Mesh>>
	{
	public:
		inline const char* TypeStr() const
		{
			return "Mesh";
		}
	};

	template<>
	class ParameterTypeInfo<std::shared_ptr<data::Scene>>
	{
	public:
		inline const char* TypeStr() const
		{
			return "Scene";
		}
	};

	template<>
	class ParameterTypeInfo<glm::vec3>
	{
	public:
		inline const char* TypeStr() const
		{
			return "vec3";
		}
	};

	template<>
	class ParameterTypeInfo<glm::mat4>
	{
	public:
		inline const char* TypeStr() const
		{
			return "mat4";
		}
	};

}
