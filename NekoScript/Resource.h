#ifndef NEKO_MINIMAL
struct _NekoRes_Meta_t {
	int index;
	unsigned int offset;
	unsigned int length;
};

extern const _NekoRes_Meta_t _NekoRes_Meta[];
extern const char _NekoRes_Payload[];

const char * NekoResGet(int index);
unsigned int NekoResLength(int index);

#include "Resource.generated.h"
#endif