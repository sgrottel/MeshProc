#pragma once

namespace meshproc
{

	template<typename T>
	class Parameter
	{
	public:

		T const& Get() const noexcept
		{
			return m_value;
		}

		T& Put()
		{
			return m_value;
		}

	private:
		T m_value;
	};

}
