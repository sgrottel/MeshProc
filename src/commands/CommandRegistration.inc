
#define _str(a) _inner_str(a)
#define _inner_str(a) #a

#define REGISTER_COMMAND_ARG_AS_NAME1(a)
#define REGISTER_COMMAND_ARG_AS_NAME2(a,b)
#define REGISTER_COMMAND_ARG_AS_NAME3(a,b,c) c
#define REGISTER_COMMAND_ARG_AS_NAME4(a,b,c,d) c##.##d
#define REGISTER_COMMAND_ARG_AS_NAME5(a,b,c,d,e) c##.##d##.##e

#define REGISTER_COMMAND_ARG_AS_NAME(...) _str(REGISTER_COMMAND_EVALNAME(REGISTER_COMMAND_SELECT(__VA_ARGS__, REGISTER_COMMAND_ARG_AS_NAME5, REGISTER_COMMAND_ARG_AS_NAME4, REGISTER_COMMAND_ARG_AS_NAME3, REGISTER_COMMAND_ARG_AS_NAME2, REGISTER_COMMAND_ARG_AS_NAME1)(__VA_ARGS__)))

namespace
{
	template<int i>
	struct CommandRegistrationHelper;

	template<int i>
	struct CommandRegistrationHelper {
		static constexpr const int VAL = i;
		static constexpr const char* NAME = nullptr;
		using TYPE = void;
	};
}

#undef REGISTER_COMMAND
#define REGISTER_COMMAND(...) template<>												\
	struct CommandRegistrationHelper<__COUNTER__ + 1> {									\
		static constexpr const int VAL = __COUNTER__;									\
		static constexpr const char* NAME = REGISTER_COMMAND_ARG_AS_NAME(__VA_ARGS__);	\
		using TYPE = REGISTER_COMMAND_ARG_AS_TYPE(__VA_ARGS__);							\
	};
