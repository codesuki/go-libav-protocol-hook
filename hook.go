package main

/*
#cgo pkg-config: libavformat libavutil

#include "hook.h"
*/
import "C"

import (
	"fmt"
	"io"
	"os"
	"unsafe"
)

type ExampleFileHook struct {
	files map[string]*os.File
}

func NewExampleFileHook() *ExampleFileHook {
	return &ExampleFileHook{files: make(map[string]*os.File)}
}

func (h *ExampleFileHook) Open(filename string) int {
	fmt.Printf("Opening file %s\n", filename)
	if _, ok := h.files[filename]; ok {
		fmt.Printf("File already open: %s\n", filename)
		return -1
	}
	file, err := os.OpenFile(filename, os.O_RDWR|os.O_CREATE, 0666)
	if err != nil {
		return -1
	}
	h.files[filename] = file
	return 0
}

func (h *ExampleFileHook) Close(filename string) int {
	fmt.Printf("Closing file %s\n", filename)
	if _, ok := h.files[filename]; !ok {
		fmt.Printf("File not open: %s\n", filename)
		return -1
	}
	err := h.files[filename].Close()
	if err != nil {
		return -1
	}
	return 0
}

func (h *ExampleFileHook) Read(filename string, buf []byte, size int) int {
	fmt.Printf("Reading from file %s\n", filename)
	if _, ok := h.files[filename]; !ok {
		fmt.Printf("File not open: %s\n", filename)
		return -1
	}
	n, err := h.files[filename].Read(buf)
	if err != nil && err != io.EOF {
		return -1
	}
	fmt.Printf("go read %d bytes\n", n)
	return n
}

func (h *ExampleFileHook) Write(filename string, buf []byte, size int) int {
	fmt.Printf("Writing to file %s\n", filename)
	if _, ok := h.files[filename]; !ok {
		fmt.Printf("File not open: %s\n", filename)
		return -1
	}
	n, err := h.files[filename].Write(buf)
	if err != nil {
		return -1
	}
	return n
}

// TODO: maybe remove size parameter since its useless in go
type ProtocolHook interface {
	Open(filename string) int
	Close(filename string) int
	Read(filename string, buf []byte, size int) int
	Write(filename string, buf []byte, size int) int
}

var hooks map[string]ProtocolHook = make(map[string]ProtocolHook)

func main() {

}

func InstallHookForProtocol(name string, hook ProtocolHook) {
	if _, ok := hooks[name]; ok {
		fmt.Printf("Hook already registered for protocol: %s\n", name)
		return
	}
	ret := C.installHookForProtocol(C.CString(name))
	if ret == -1 {
		fmt.Printf("Could not find protocol: %s\n", name)
	}
	hooks[name] = hook
	fmt.Printf("Installed hook for protocol: %s\n", name)
}

func UninstallHookForProtocol(name string) {
	if _, ok := hooks[name]; !ok {
		fmt.Printf("No hook registered for protocol: %s\n", name)
		return
	}
	C.uninstallHookForProtocol(C.CString(name))
	fmt.Printf("Uninstalled hook for protocol: %s\n", name)
}

//export go_open
func go_open(h *C.URLContext, cFilename *C.char) int {
	protocolName := C.GoString(h.prot.name)
	filename := C.GoString(cFilename)
	if _, ok := hooks[protocolName]; !ok {
		return -1
	}
	return hooks[protocolName].Open(filename)
}

//export go_close
func go_close(h *C.URLContext) int {
	protocolName := C.GoString(h.prot.name)
	filename := C.GoString(h.filename)
	if _, ok := hooks[protocolName]; !ok {
		return -1
	}
	return hooks[protocolName].Close(filename)
}

//export go_read
func go_read(h *C.URLContext, buf unsafe.Pointer, size C.int) int {
	protocolName := C.GoString(h.prot.name)
	filename := C.GoString(h.filename)
	if _, ok := hooks[protocolName]; !ok {
		return -1
	}
	goBuffer := C.GoBytes(buf, size)
	return hooks[protocolName].Read(filename, goBuffer, int(size))
}

//export go_write
func go_write(h *C.URLContext, buf unsafe.Pointer, size C.int) int {
	protocolName := C.GoString(h.prot.name)
	filename := C.GoString(h.filename)
	if _, ok := hooks[protocolName]; !ok {
		return -1
	}
	goBuffer := C.GoBytes(buf, size)
	return hooks[protocolName].Write(filename, goBuffer, int(size))
}
