#include <allegro5/allegro5.h>
#include <stdio.h>
#include "networking.h"
#ifdef _WIN32
#include <winsock2.h>
#endif

//got this code from https://stackoverflow.com/questions/1543466/how-do-i-change-a-tcp-socket-to-be-non-blocking
_Bool SetSocketBlockingEnabled(int fd, _Bool blocking)
{
   if (fd < 0) return 0;

#ifdef _WIN32
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? 1 : 0;
#else
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return 0;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? 1 : 0;
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
