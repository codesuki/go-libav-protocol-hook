#include <stdio.h>

#include <libavformat/avio.h>

#include "hook.h"

#define MAX_HOOKS 50

typedef struct Hook {
  char *name;
  callback_open originalOpen;
  callback_close originalClose;
  callback_read originalRead;
  callback_write originalWrite;
  callback_seek originalSeek;
} Hook;

static Hook hooks[MAX_HOOKS];

Hook* getHookForProtocol(char* name) {
  for (int i = 0; i < MAX_HOOKS; ++i) {
    if (hooks[i].name == NULL) {
      continue;
    }
    if (!strcmp(hooks[i].name, name)) {
      return &hooks[i];
    }
  }
  return NULL;
}

Hook* addHookForProtocol(char* name) {
  for (int i = 0; i < MAX_HOOKS; ++i) {
    if (hooks[i].name == NULL) {
      hooks[i].name = strdup(name);
      return &hooks[i];
    }
  }
  return NULL;
}

void removeHook(Hook* hook) {
  hook->name = NULL;
  hook->originalOpen = NULL;
  hook->originalClose = NULL;
  hook->originalRead = NULL;
  hook->originalWrite = NULL;
  hook->originalSeek = NULL;
}

void installOpenHook(Hook* hook, URLProtocol* protocol) {
  hook->originalOpen = protocol->url_open;
  protocol->url_open = cOpenHook;
}

void installCloseHook(Hook* hook, URLProtocol* protocol) {
  hook->originalClose = protocol->url_close;
  protocol->url_close = cCloseHook;
}

void installReadHook(Hook* hook, URLProtocol* protocol) {
  hook->originalRead = protocol->url_read;
  protocol->url_read = cReadHook;
}

void installWriteHook(Hook* hook, URLProtocol* protocol) {
  hook->originalWrite = protocol->url_write;
  protocol->url_write = cWriteHook;
}

void installSeekHook(Hook* hook, URLProtocol* protocol) {
  hook->originalSeek = protocol->url_seek;
  protocol->url_seek = cSeekHook;
}

void uninstallOpenHook(Hook* hook, URLProtocol* protocol) {
  protocol->url_open = hook->originalOpen;
}

void uninstallCloseHook(Hook* hook, URLProtocol* protocol) {
  protocol->url_close = hook->originalClose;
}

void uninstallReadHook(Hook* hook, URLProtocol* protocol) {
  protocol->url_read = hook->originalRead;
}

void uninstallWriteHook(Hook* hook, URLProtocol* protocol) {
  protocol->url_write = hook->originalWrite;
}

void uninstallSeekHook(Hook* hook, URLProtocol* protocol) {
  protocol->url_seek = hook->originalSeek;
}

void installHook(Hook* hook, URLProtocol* protocol) {
  installOpenHook(hook, protocol);
  installCloseHook(hook, protocol);
  installReadHook(hook, protocol);
  installWriteHook(hook, protocol);
  installSeekHook(hook, protocol);
}

void uninstallHook(Hook* hook, URLProtocol* protocol) {
  uninstallOpenHook(hook, protocol);
  uninstallCloseHook(hook, protocol);
  uninstallReadHook(hook, protocol);
  uninstallWriteHook(hook, protocol);
  uninstallSeekHook(hook, protocol);
}

URLProtocol* getProtocolByName(char *name) {
  void *opaque = NULL;
  const char *currentName;
  while ((currentName = avio_enum_protocols(&opaque, 1))) {
    if (!strcmp(currentName, name)) {
         printf("Found protocol: %s\n", currentName);
         return (URLProtocol*)opaque;
    }
  }
  return NULL;
}

int installHookForProtocol(char *name) {
  URLProtocol *protocol = getProtocolByName(name);
  if (protocol == NULL) {
    printf("Protocol not found protocol: %s\n", name);
    return -1;
  }
  Hook* hook = getHookForProtocol(name);
  if (hook != NULL) {
    printf("Hook already installed for protocol: %s\n", name);
    return -1;
  }
  hook = addHookForProtocol(name);
  if (hook == NULL) {
    printf("Could not add new hook for protocol: %s\n", name);
    return -1;
  }
  installHook(hook, protocol);
  return 0;
}

int uninstallHookForProtocol(char *name) {
  URLProtocol *protocol = getProtocolByName(name);
  if (protocol == NULL) {
    printf("Protocol not found protocol: %s\n", name);
    return -1;
  }
  Hook* hook = getHookForProtocol(name);
  if (hook == NULL) {
    printf("No hook installed for protocol: %s\n", name);
    return -1;
  }
  uninstallHook(hook, protocol);
  return 0;
}


int cOpenHook(URLContext *h, const char *url, int flags) {
  printf("C.cOpenHook called with filename %s\n", url);
  int ret = go_open(h, url, flags);
  printf("go_open returned %d\n", ret);
  return ret;
}

int cCloseHook(URLContext *h) {
  printf("C.cCloseHook called with filename %s\n", h->filename);
  int ret = go_close(h);
  printf("go_close returned %d\n", ret);
  return ret;
}

int cReadHook(URLContext *h, unsigned char *buf, int size) {
  printf("C.cReadHook called with filename %s\n", h->filename);
  int ret = go_read(h, buf, size);
  printf("go_read returned %d\n", ret);
  return ret;
}

int cWriteHook(URLContext *h, const unsigned char *buf, int size) {
  printf("C.cWriteHook called with filename %s\n", h->filename);
  int ret = go_write(h, buf, size);
  printf("go_write returned %d\n", ret);
  return ret;
}

int64_t cSeekHook(URLContext *h, int64_t pos, int whence) {
  printf("C.cSeekHook called with filename %s\n", h->filename);
  int ret = go_seek(h, pos, whence);
  printf("go_seek returned %d\n", ret);
  return ret;
}
