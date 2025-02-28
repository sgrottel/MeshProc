#include "ParameterValue.h"

using namespace meshproc;

template<ParamType PT>
class ParameterValue::StorageImpl : public ParameterValue::Storage
{
public:
	StorageImpl()
		: Storage(PT)
	{
	}
	virtual ~StorageImpl() = default;

	ParamTypeInfo_t<PT> m_value{};
};

template<unsigned int VAL>
class ParameterValue::Pusher : public Pusher<VAL - 1>
{
public:
	using StorageImpl_t = StorageImpl<static_cast<ParamType>(VAL)>;

	template<ParamMode PM>
	bool TrySet(std::shared_ptr<StorageImpl_t> target, ParameterBinding::ParamBindingBase& source)
	{
		if (source.m_mode == PM)
		{
			auto& s = dynamic_cast<ParameterBinding::ParamBinding<PM, static_cast<ParamType>(VAL)>&>(source);
			target->m_value = s.m_var;
			return true;
		}
		return false;
	}

	std::shared_ptr<Storage> Push(ParameterBinding::ParamBindingBase& source)
	{
		if (VAL == static_cast<unsigned int>(source.m_type))
		{
			auto v = std::make_shared<StorageImpl_t>();
			if (TrySet<ParamMode::InOut>(v, source))
			{
				return v;
			}
			if (TrySet<ParamMode::Out>(v, source))
			{
				return v;
			}
			return nullptr;
		}
		return Pusher<VAL - 1>::Push(source);
	}
};

template<>
class ParameterValue::Pusher<0> : public ParameterBinding
{
public:
	using StorageImpl_t = StorageImpl<static_cast<ParamType>(0)>;

	template<ParamMode PM>
	bool TrySet(std::shared_ptr<StorageImpl_t> target, ParameterBinding::ParamBindingBase& source)
	{
		if (source.m_mode == PM)
		{
			auto& s = dynamic_cast<ParameterBinding::ParamBinding<PM, static_cast<ParamType>(0)>&>(source);
			target->m_value = s.m_var;
			return true;
		}
		return false;
	}

	std::shared_ptr<Storage> Push(ParameterBinding::ParamBindingBase& source)
	{
		if (0 == static_cast<unsigned int>(source.m_type))
		{
			auto v = std::make_shared<StorageImpl_t>();
			if (TrySet<ParamMode::InOut>(v, source))
			{
				return v;
			}
			if (TrySet<ParamMode::Out>(v, source))
			{
				return v;
			}
			return nullptr;
		}
		return nullptr;
	}
};

template<unsigned int VAL>
class ParameterValue::Puller : public Puller<VAL - 1>
{
public:
	using StorageImpl_t = StorageImpl<static_cast<ParamType>(VAL)>;

	template<ParamMode PM>
	bool TryPut(const std::shared_ptr<StorageImpl_t> source, ParameterBinding::ParamBindingBase& target)
	{
		if (target.m_mode == PM)
		{
			auto& t = dynamic_cast<ParameterBinding::ParamBinding<PM, static_cast<ParamType>(VAL)>&>(target);
			t.m_var = source->m_value;
			return true;
		}
		return false;
	}

	bool Pull(const std::shared_ptr<Storage>& source, ParameterBinding::ParamBindingBase& target)
	{
		if (VAL == static_cast<unsigned int>(target.m_type))
		{
			auto s = std::dynamic_pointer_cast<StorageImpl_t>(source);
			if (!s)
			{
				return false;
			}
			if (TryPut<ParamMode::InOut>(s, target))
			{
				return true;
			}
			if (TryPut<ParamMode::In>(s, target))
			{
				return true;
			}
		}
		return Puller<VAL - 1>::Pull(source, target);
	}

};

template<>
class ParameterValue::Puller<0> : public ParameterBinding
{
public:
	using StorageImpl_t = StorageImpl<static_cast<ParamType>(0)>;

	template<ParamMode PM>
	bool TryPut(const std::shared_ptr<StorageImpl_t> source, ParameterBinding::ParamBindingBase& target)
	{
		if (target.m_mode == PM)
		{
			auto& t = dynamic_cast<ParameterBinding::ParamBinding<PM, static_cast<ParamType>(0)>&>(target);
			t.m_var = source->m_value;
			return true;
		}
		return false;
	}

	bool Pull(const std::shared_ptr<Storage>& source, ParameterBinding::ParamBindingBase& target)
	{
		if (0 == static_cast<unsigned int>(target.m_type))
		{
			auto s = std::dynamic_pointer_cast<StorageImpl_t>(source);
			if (!s)
			{
				return false;
			}
			if (TryPut<ParamMode::InOut>(s, target))
			{
				return true;
			}
			if (TryPut<ParamMode::In>(s, target))
			{
				return true;
			}
		}
		return false;
	}

};

ParamType ParameterValue::GetType()
{
	return m_value ? m_value->m_type : ParamType::LAST;
}

bool ParameterValue::Push(ParameterBinding::ParamBindingBase& source)
{
	Pusher<static_cast<unsigned int>(ParamType::LAST) - 1> pusher;
	m_value = pusher.Push(source);
	return static_cast<bool>(m_value);
}

bool ParameterValue::Pull(ParameterBinding::ParamBindingBase& target) const
{
	Puller<static_cast<unsigned int>(ParamType::LAST) - 1> puller;
	return puller.Pull(m_value, target);
}
