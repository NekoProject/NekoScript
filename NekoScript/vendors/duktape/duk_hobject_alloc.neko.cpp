#include "duk_internal.h"

namespace SeiranBlackTech {
	template<typename T>
	class SizeOf {
	public:
		static const size_t size = sizeof(T);
	};

	template<>
	class SizeOf<duk_hthread>{
	public:
		static const size_t size = sizeof(duk_hthread) + sizeof(void *);
	};
}

#define sizeof(obj) (SeiranBlackTech::SizeOf<obj>::size)
#include "duk_hobject_alloc.cpp"
#undef sizeof
