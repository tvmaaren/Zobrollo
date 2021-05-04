#ifdef __cplusplus
#define _Bool bool
extern "C"{
#endif
_Bool SetSocketBlocking(int fd, _Bool blocking);
void error_message(const char* text);
#ifdef __cplusplus
}
#endif

