# Kernel

 This is the kernel of SmokeOS. This project is an hobbyist kernel start I
 started some years ago in order to improve my skill on low-level architecture.
 I pursue this project since hopping that one day this system would be stable
 and big enough to be used for my daily work.



## Roadmap


- Create and make usage of rwlock
- Clean memory space code
- Debug multi-thread code
- Check all functions for synchronization in order to activate multi-CPU features.
- Split inode and dirent in current VFS modules.
- Update spinlock to be a recursive lock.
- Document and fix design of heap-allocation implementation.
- Create proper stream module.
- Analyze and fix Tty issues.




# Various information

## Compilation flags

 - `__AX_STR_EX`: Prevent the compiler that extended string functions (like strdup of strcmpi) are implemented on standard headers.


