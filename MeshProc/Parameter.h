#pragma once

#include <stdexcept>
#include <cassert>

namespace meshproc
{

	enum class ParamType {
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

	protected:
		ParameterBase() = default;

	private:
	};

	template<ParamType PT>
	class LockableParameterBase;

	template<typename T, ParamType PT>
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
				throw std::runtime_error("Parameter is locked for read-only");
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

	private:
		T m_value{};
	};

	template<>
	class LockableParameterBase<ParamType::In> {
	protected:
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
	class LockableParameterBase<ParamType::InOut> {
	protected:
		static constexpr bool const isWritable{ true };
		inline void ImplPreInvoke() {}
		inline void ImplPostInvoke() {}
	};

	template<>
	class LockableParameterBase<ParamType::Out> {
	protected:
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

}
