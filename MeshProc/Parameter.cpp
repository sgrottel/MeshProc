#include "Parameter.h"

#include "AbstractCommand.h"

#include <stdexcept>
#include <array>

using namespace meshproc;

namespace
{
	template<ParamType TYPE>
	consteval const char* GetName()
	{
		return ParamTypeInfo<TYPE>::name;
	}

	template<ParamMode MODE>
	consteval const char* GetName()
	{
		return ParamModeInfo<MODE>::name;
	}

	template <typename TENUM, size_t... Es >
	consteval auto MakeNameTableValues(std::integer_sequence<size_t, Es...>) {
		return std::array<const char*, sizeof...(Es)>{ GetName<static_cast<TENUM>(Es)>()... };
	}

	template <typename TENUM>
	consteval auto MakeNameTable() {
		auto seq = []<size_t... I>(std::index_sequence<I...>) {
			return std::integer_sequence<size_t, static_cast<size_t>(I)...>{};
		}(std::make_index_sequence<static_cast<size_t>(TENUM::LAST)>{});
		return MakeNameTableValues<TENUM>(seq);
	}

}

const char* meshproc::GetParamTypeName(ParamType pt)
{
	static constexpr auto nametable = MakeNameTable<ParamType>();
	const size_t idx = static_cast<size_t>(pt);
	if (idx >= nametable.size())
	{
		return nullptr;
	}
	return nametable[idx];
}

const char* meshproc::GetParamModeName(ParamMode pm)
{
	static constexpr auto nametable = MakeNameTable<ParamMode>();
	const size_t idx = static_cast<size_t>(pm);
	if (idx >= nametable.size())
	{
		return nullptr;
	}
	return nametable[idx];
}
