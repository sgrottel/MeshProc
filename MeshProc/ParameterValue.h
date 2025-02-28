#pragma once

#include "Parameter.h"
#include "ParameterBinding.h"

#include <memory>

namespace meshproc
{

	class ParameterValue
	{
	public:
		ParamType GetType();

		bool Push(ParameterBinding::ParamBindingBase& source);

		bool Pull(ParameterBinding::ParamBindingBase& target) const;

	private:

		class Storage {
		public:
			ParamType m_type;
			virtual ~Storage() = default;
		protected:
			Storage(ParamType type)
				: m_type{ type }
			{
			}
		};

		template<ParamType>
		class StorageImpl;

		template<unsigned int VAL>
		class Pusher;

		template<unsigned int VAL>
		class Puller;

		std::shared_ptr<Storage> m_value;
	};

}