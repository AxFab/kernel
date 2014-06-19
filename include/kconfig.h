#ifndef KCONFIG_H__
#define KCONFIG_H__


#define MAX_STRING_LENGTH   256
#define MAX_LOOP_STREAM     32
#define MAX_LOOP_BUCKET     512


#define MAX_TMPFILE_NAME    32
#define MAX_SYMLINK_LOOP  64

#define PTR_POISON  ((void*)0xdeaddead)

#define USR_SPACE_BASE  0x00400000
#define USR_SPACE_LIMIT 0xD0000000

#define ROOT_UID 0x1593

#define FILENAME_SEPARATOR "/\\"
#define VOLUME_SEPARATOR ":"

#define HEAP_START ((void*)(256 * _Mb_))
#define STACK_DEFAULT (1 * _Mb_)

#define FILE_MAP_SIZE  (8 * _Kb_)

#endif /* KCONFIG_H__ */
