#ifndef OSEMU_HELPFUNCTIONS_H_
#define OSEMU_HELPFUNCTIONS_H_

bool cs_malloc(void *result, size_t size);

#define NULLFREE(X) {if (X) {void *tmpX=X; X=NULL; free(tmpX); }}

void cs_sleepms(uint32_t msec);

void cs_log_txt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
char *cs_hexdump(int32_t m, const uint8_t *buf, int32_t n, char *target, int32_t len);
void cs_log_hex(const uint8_t *buf, int32_t n, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

#define cs_log(fmt, params...)							cs_log_txt(fmt, ##params)
#define cs_log_dump(buf, n, fmt, params...)				cs_log_hex(buf,  n, fmt, ##params)
#define cs_log_dbg(a, fmt, params...)					do { if (debuglog) cs_log_txt(fmt, ##params); } while(0)
#define cs_log_dump_dbg(buf, n, fmt, params...)			do { if (debuglog) cs_log_hex(buf , n, fmt, ##params); } while(0)

int32_t boundary(int32_t exp, int32_t n);

uint32_t b2i(int32_t n, const uint8_t *b);
uint8_t *i2b_buf(int32_t n, uint32_t i, uint8_t *b);

void cs_strncpy(char *destination, const char *source, size_t num);

void get_random_bytes_init(void);
void get_random_bytes(uint8_t *dst, uint32_t dst_len);

uint32_t crc32(uint32_t crc, const unsigned char *buf, uint32_t len);
uint32_t fletcher_crc32(uint8_t *data, uint32_t len);

void aes_set_key(struct aes_keys *aes, char *key);
void aes_decrypt(struct aes_keys *aes, uint8_t *buf, int32_t n);
void aes_encrypt_idx(struct aes_keys *aes, uint8_t *buf, int32_t n);

#ifdef IPV6SUPPORT
#define IN_ADDR_T struct in6_addr
#define SOCKADDR sockaddr_storage
#define ADDR_ANY in6addr_any
#define DEFAULT_AF AF_INET6
#else
#define IN_ADDR_T in_addr_t
#define SOCKADDR sockaddr_in
#define ADDR_ANY INADDR_ANY
#define DEFAULT_AF AF_INET
#endif

#ifdef IPV6SUPPORT
#define GET_IP() *(struct in6_addr *)pthread_getspecific(getip)
#define IP_ISSET(a) !cs_in6addr_isnull(&a)
#define IP_EQUAL(a, b) cs_in6addr_equal(&a, &b)
#define IP_ASSIGN(a, b) cs_in6addr_copy(&a, &b)
#define SIN_GET_ADDR(a) ((struct sockaddr_in6 *)&a)->sin6_addr
#define SIN_GET_PORT(a) ((struct sockaddr_in6 *)&a)->sin6_port
#define SIN_GET_FAMILY(a) ((struct sockaddr_in6 *)&a)->sin6_family
#else
#define GET_IP() *(in_addr_t *)pthread_getspecific(getip)
#define IP_ISSET(a) (a)
#define IP_EQUAL(a, b) (a == b)
#define IP_ASSIGN(a, b) (a = b)
#define SIN_GET_ADDR(a) (a.sin_addr.s_addr)
#define SIN_GET_PORT(a) (a.sin_port)
#define SIN_GET_FAMILY(a) (a.sin_family)
#endif

void cs_resolve(const char *hostname, IN_ADDR_T *ip, struct SOCKADDR *sock, socklen_t *sa_len);

#define SAFE_PTHREAD_1ARG(a, b, c) { \
	int32_t pter = a(b); \
	if(pter != 0) \
	{ \
		c("FATAL ERROR: %s() failed in %s with error %d %s\n", #a, __func__, pter, strerror(pter)); \
		exit_oscam = 1;\
	} }

#define SAFE_ATTR_INIT(a)			SAFE_PTHREAD_1ARG(pthread_attr_init, a, cs_log)
#define SAFE_MUTEX_LOCK(a)			SAFE_PTHREAD_1ARG(pthread_mutex_lock, a, cs_log)
#define SAFE_MUTEX_UNLOCK(a)		SAFE_PTHREAD_1ARG(pthread_mutex_unlock, a, cs_log)

#define SAFE_PTHREAD_2ARG(a, b, c, d) { \
	int32_t pter = a(b, c); \
	if(pter != 0) \
	{ \
		d("FATAL ERROR: %s() failed in %s with error %d %s\n", #a, __func__, pter, strerror(pter)); \
		exit_oscam = 1;\
	} }

#define SAFE_MUTEX_INIT(a,b)		SAFE_PTHREAD_2ARG(pthread_mutex_init, a, b, cs_log)

#define SAFE_ATTR_SETSTACKSIZE(a, b) { \
	int32_t pter = pthread_attr_setstacksize(a, b); \
	if(pter != 0) \
	{ \
		cs_log_dbg(0, "WARNING: pthread_attr_setstacksize() failed in %s with error %d %s\n", __func__, pter, strerror(pter)); \
	} }


typedef void* (*thread_func)(void *arg);
int32_t start_thread(const char *nameroutine, thread_func startroutine, void *arg, pthread_t *pthread, int8_t detach, int8_t modify_stacksize);

size_t b64encode(const char *in, size_t inlen, char **out);
void base64_encode(const char *in, size_t inlen, char *out, size_t outlen);

char *strtolower(char *txt);
char *strtoupper(char *txt);

#endif
