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
	files map[*int]*os.File
}

func NewExampleFileHook() *ExampleFileHook {
	return &ExampleFileHook{files: make(map[*int]*os.File)}
}

func (h *ExampleFileHook) Open(handle *int, filename string) int {
	fmt.Printf("Opening file %s\n", filename)
	if _, ok := h.files[handle]; ok {
		fmt.Printf("File already open: %s\n", filename)
		return -1
	}
	file, err := os.OpenFile(filename, os.O_RDWR|os.O_CREATE, 0666)
	if err != nil {
		return -1
	}
	h.files[handle] = file
	return 0
}

func (h *ExampleFileHook) Close(handle *int) int {
	fmt.Printf("Closing file %d\n", handle)
	if _, ok := h.files[handle]; !ok {
		fmt.Printf("File not open: %d\n", handle)
		return -1
	}
	err := h.files[handle].Close()
	if err != nil {
		return -1
	}
	delete(h.files, handle)
	return 0
}

func (h *ExampleFileHook) Read(handle *int, buf []byte, size int) int {
	fmt.Printf("Reading from file %d\n", handle)
	if _, ok := h.files[handle]; !ok {
		fmt.Printf("File not open: %d\n", handle)
		return -1
	}
	n, err := h.files[handle].Read(buf)
	if err != nil && err != io.EOF {
		return -1
	}
	fmt.Printf("go read %d bytes\n", n)
	return n
}

func (h *ExampleFileHook) Write(handle *int, buf []byte, size int) int {
	fmt.Printf("Writing to file %d\n", handle)
	if _, ok := h.files[handle]; !ok {
		fmt.Printf("File not open: %d\n", handle)
		return -1
	}
	n, err := h.files[handle].Write(buf)
	if err != nil {
		return -1
	}
	return n
}

func (h *ExampleFileHook) Seek(handle *int, pos int64, whence int) int64 {
	fmt.Printf("Seeking in file %d\n", handle)
	if _, ok := h.files[handle]; !ok {
		fmt.Printf("File not open: %d\n", handle)
		return -1
	}
	newPos, err := h.files[handle].Seek(pos, whence)
	if err != nil {
		return -1
	}
	return newPos
}

// TODO: maybe remove size parameter since its useless in go
type ProtocolHook interface {
	Open(handle *int, filename string) int
	Close(handle *int) int
	Read(handle *int, buf []byte, size int) int
	Write(handle *int, buf []byte, size int) int
	Seek(handle *int, pos int64, whence int) int64
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
	handle := (*int)(unsafe.Pointer(h))
	protocolName := C.GoString(h.prot.name)
	filename := C.GoString(cFilename)
	if _, ok := hooks[protocolName]; !ok {
		return -1
	}
	return hooks[protocolName].Open(handle, filename)
}

//export go_close
func go_close(h *C.URLContext) int {
	handle := (*int)(unsafe.Pointer(h))
	protocolName := C.GoString(h.prot.name)
	if _, ok := hooks[protocolName]; !ok {
		return -1
	}
	return hooks[protocolName].Close(handle)
}

//export go_read
func go_read(h *C.URLContext, buf *C.uchar, size C.int) int {
	handle := (*int)(unsafe.Pointer(h))
	protocolName := C.GoString(h.prot.name)
	if _, ok := hooks[protocolName]; !ok {
		return -1
	}
	goBuffer := (*[1 << 30]byte)(unsafe.Pointer(buf))[:size:size]
	return hooks[protocolName].Read(handle, goBuffer, int(size))
}

//export go_write
func go_write(h *C.URLContext, buf unsafe.Pointer, size C.int) int {
	handle := (*int)(unsafe.Pointer(h))
	protocolName := C.GoString(h.prot.name)
	if _, ok := hooks[protocolName]; !ok {
		return -1
	}
	goBuffer := C.GoBytes(buf, size)
	return hooks[protocolName].Write(handle, goBuffer, int(size))
}

//export go_seek
func go_seek(h *C.URLContext, pos C.int64_t, whence C.int) int64 {
	handle := (*int)(unsafe.Pointer(h))
	protocolName := C.GoString(h.prot.name)
	if _, ok := hooks[protocolName]; !ok {
		return -1
	}
	return hooks[protocolName].Seek(handle, int64(pos), int(whence))
}
