#include <allegro5/allegro5.h>
#include <stdio.h>
#include "networking.h"
#ifdef _WIN32
#include <winsock2.h>
#endif

_Bool SetSocketBlocking(int fd, _Bool blocking){
#ifdef _WIN32
   unsigned long mode = (unsigned long)!blocking;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0);
#else
   int flags = blocking ? 0 : O_NONBLOCK;
   return (fcntl(fd, F_SETFL, flags) == 0);
#endif
}

void error_message(const char* text){
	int error_code;
#ifdef _WIN32
	error_code = WSAGetLastError();
#else
	error_code = (int)errno;
#endif
	fprintf(stderr, "Failed %s. Error code: %d\n", text, error_code);
	return;
}
