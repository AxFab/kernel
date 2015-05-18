# Kernel Sources

  This folder contains the source code of the kernel. The kernel is build from 4 static libraries.

  - `libka.a` -- This library contains all the platform-specifics routines.
  - `libkfs.a` -- Easy enough this library contains all the file system and drivers to embed on the kernel.
  - `libkrn.a` -- The core kernel library contains all the kernel main algorithms.
  - `libaxb.a` -- This is an external library coming from the repository __axlibc__.

## The four fragments

  The external library is a small subset of the standard c library. The static library _ax basics_ embed only routines that are standalone and doesn't require a underlying system. It creates a small runtime for common C routines like `str...`,`mem...`, `vfprintf`, `alloc`...

  The core library is fixed, build from the sources contains at the root of `src/` (actually `src/kern & src/scall`). The core source files may used internal headers to shared information about structure and API.

  The file system library is the most volatile. Anyone should be able to compile the kernel by selecting which driver should be embedded or not. _Currently this option is limited. Note that the TMPFS driver is mandatory in order to make the kernel work, and be sure to get enough to mount at least the booting file-system._

  The last library depends of the chosen platform. The library should be able to initialized a stable but minimal environment for the kernel and call the `kernel_start()` method from the core fragment. Note that some functions are mandatory for this library usually starting by `cpu_` or `mmu_`. See [src/_um/README.md #New Architectures](#) to know more.

## Syscalls

  The kernel is similar to an external library that can be accessed through a defined API, the __sycalls__ defined this API. As an original design, SmokeOs kernel defined it's own interface which, even if inspired by Unix an NT, doesn't matches the one of other systems.

  Syscalls routines already supported are:

  - `void sys_exit(int status, int pid)`
  - `int sys_exec(const char *exec, SMK_StartInfo_t *info)`
  - `int sys_start(const char*name, size_t entry, size_t param)`
  - `int sys_stop()`
  - `int sys_wait(int reason, int param, int timeout)`
  - `int sys_open(const char *path, int dirFd, int flags, int mode)`
  - `int sys_close(int fd)`
  - `ssize_t sys_read(int fd, const void *buf, size_t lg, size_t off)`
  - `ssize_t sys_write(int fd, void *buf, size_t lg, size_t off)`

## Modules

  On the core fragment we can identify several modules which may be study separately, even if strongly inter-dependent.

  - We have the _virtual file system_ which handle devices and inodes. This is the one in charge of communicating with the drivers and file systems.

  - The memory module is really important, it's used everywhere. Most of the memory issue are resolved using _memory mapping_. The kernel is build for platform which provide virtual memory.

  - _Streams_  are split in several file type, we've got divergent behaviors for pipe, terminal, block files...

  - _User and security managements_ is an important concern even if still limited at the stage.

  - The tasks module regroup all the management of _Threads & processes_. Note that the scheduler is part of this module (I've tried to split them but it doesn't come easy).

