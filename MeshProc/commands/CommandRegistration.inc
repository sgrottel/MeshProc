
#ifndef COMMAND_PATH
#error You must define COMMAND_PATH before including CommandRegistration.i
#endif

#ifndef _I_STUFF
#define _I_STUFF

#define _str(a) _inner_str(a)
#define _inner_str(a) #a

#define ARG_AS_PATH1(a) a
#define ARG_AS_PATH2(a,b) a##/##b
#define ARG_AS_PATH3(a,b,c) a##/##b##/##c

#define ARG_AS_NAME1(a) a
#define ARG_AS_NAME2(a,b) a##.##b
#define ARG_AS_NAME3(a,b,c) a##.##b##.##c

#define ARG_AS_TYPE1(a) a
#define ARG_AS_TYPE2(a,b) a##::##b
#define ARG_AS_TYPE3(a,b,c) a##::##b##::##c

#define SELECT(_1,_2,_3,NAME,...) NAME
#define EVALNAME(a) a

#define ARG_AS_PATH(...) _str(EVALNAME(SELECT(__VA_ARGS__, ARG_AS_PATH3, ARG_AS_PATH2, ARG_AS_PATH1)(__VA_ARGS__))##.h)

#define ARG_AS_NAME(...) _str(EVALNAME(SELECT(__VA_ARGS__, ARG_AS_NAME3, ARG_AS_NAME2, ARG_AS_NAME1)(__VA_ARGS__)))

#define ARG_AS_TYPE(...) EVALNAME(SELECT(__VA_ARGS__, ARG_AS_TYPE3, ARG_AS_TYPE2, ARG_AS_TYPE1)(__VA_ARGS__))

#endif

#include ARG_AS_PATH(COMMAND_PATH)

namespace
{
	template<>
	struct RegisterCommandHelper<__COUNTER__>
	{
		using T = meshproc::commands::ARG_AS_TYPE(COMMAND_PATH);
		static constexpr const char * NAME = ARG_AS_NAME(COMMAND_PATH);
	};
}

#undef COMMAND_PATH
