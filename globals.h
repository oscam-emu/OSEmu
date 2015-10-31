#ifndef OSEMU_GLOBALS_H_
#define OSEMU_GLOBALS_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE //needed for PTHREAD_MUTEX_RECURSIVE on some plattforms and maybe other things; do not remove
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
 
#include "cscrypt/aes.h"
#include "cscrypt/des.h"
#include "cscrypt/md5.h"

#ifdef UNUSED
#elif __GNUC__ >= 3 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

//ECM rc codes:
#define E_FOUND         0
///////above is all found
#define E_NOTFOUND  4  //for selection of found, use < E_NOTFOUND
#define E_FAKE          7
#define E_INVALID       8
#define E_STOPPED       13 //for selection of error, use <= E_STOPPED and exclude selection of found
///////above is all notfound, some error or problem
#define E_UNHANDLED 100 //for selection of unhandled, use >= E_UNHANDLED

#define SCT_LEN(sct) (3+((sct[1]&0x0f)<<8)+sct[2])

#define MAX_ECM_SIZE 1024
#define MAX_EMM_SIZE 1024

extern int8_t debuglog;
extern int8_t havelogfile;
extern char*  logfile;
extern int bg;
extern int32_t exit_oscam;

struct aes_keys {
	AES_KEY			aeskey_encrypt;		// encryption key needed by monitor and used by camd33, camd35
	AES_KEY			aeskey_decrypt;		// decryption key needed by monitor and used by camd33, camd35
};

typedef struct ecm_request_t {
	uint8_t			cw[16];
	int16_t			ecmlen;
	uint16_t		caid;
	uint16_t		srvid;
	uint32_t		prid;
	uint16_t        pid;
	int8_t			rc;
} ECM_REQUEST;

#include "helpfunctions.h"

#endif
