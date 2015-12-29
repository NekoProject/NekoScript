#ifndef NEKO_MINIMAL
#include "Resource.h"
#include "Resource.generated.cpp"

const char * NekoResGet(int index) {
	return _NekoRes_Payload + _NekoRes_Meta[index].offset;
}

unsigned int NekoResLength(int index) {
	return _NekoRes_Meta[index].length;
}

#endif