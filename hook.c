#include <stdio.h>

#include <libavformat/avio.h>

#include "hook.h"

// TODO: save callbacks by name to support hooking multiple protocols
callback_open originalOpen;
callback_close originalClose;
callback_read originalRead;
callback_write originalWrite;

void installOpenHook(URLProtocol* protocol) {
  originalOpen = protocol->url_open;
  protocol->url_open = cOpenHook;
}

void installCloseHook(URLProtocol* protocol) {
  originalClose = protocol->url_close;
  protocol->url_close = cCloseHook;
}

void installReadHook(URLProtocol* protocol) {
  originalRead = protocol->url_read;
  protocol->url_read = cReadHook;
}

void installWriteHook(URLProtocol* protocol) {
  originalWrite = protocol->url_write;
  protocol->url_write = cWriteHook;
}

void uninstallOpenHook(URLProtocol* protocol) {
  protocol->url_open = originalOpen;
}

void uninstallCloseHook(URLProtocol* protocol) {
  protocol->url_close = originalClose;
}

void uninstallReadHook(URLProtocol* protocol) {
  protocol->url_read = originalRead;
}

void uninstallWriteHook(URLProtocol* protocol) {
  protocol->url_write = originalWrite;
}

void installHook(URLProtocol* protocol) {
  installOpenHook(protocol);
  installCloseHook(protocol);
  installReadHook(protocol);
  installWriteHook(protocol);
}

void uninstallHook(URLProtocol* protocol) {
  uninstallOpenHook(protocol);
  uninstallCloseHook(protocol);
  uninstallReadHook(protocol);
  uninstallWriteHook(protocol);
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
  //av_register_all();
  URLProtocol *protocol = getProtocolByName(name);
  if (protocol == NULL) {
    return -1;
  }
  installHook(protocol);
  return 0;
}

int uninstallHookForProtocol(char *name) {
  URLProtocol *protocol = getProtocolByName(name);
  if (protocol == NULL) {
    return -1;
  }
  uninstallHook(protocol);
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
