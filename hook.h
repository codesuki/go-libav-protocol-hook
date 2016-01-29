#include "url.h"

typedef int (*callback_open)(URLContext *h, const char *url, int flags);
typedef int (*callback_close)(URLContext *h);
typedef int (*callback_write)(URLContext *h, const unsigned char *buf, int size);
typedef int (*callback_read)(URLContext *h, unsigned char *buf, int size);
typedef int64_t (*callback_seek)(URLContext *h, int64_t pos, int whence);

int installHookForProtocol(char *name);
int uninstallHookForProtocol(char *name);

int cOpenHook(URLContext *h, const char *url, int flags);
int cCloseHook(URLContext *h);
int cReadHook(URLContext *h, unsigned char *buf, int size);
int cWriteHook(URLContext *h, const unsigned char *buf, int size);
int64_t cSeekHook(URLContext *h, int64_t pos, int whence);
