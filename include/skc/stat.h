
#ifndef _SKC_STAT_H
#define _SKC_STAT_H 1

#define S_IRUSR     (0400)
#define S_IWUSR     (0200)
#define S_IXUSR     (0100)
#define S_IRWXU     (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRGRP     (S_IRUSR >> 3)
#define S_IWGRP     (S_IWUSR >> 3)
#define S_IXGRP     (S_IXUSR >> 3)
#define S_IRWXG     (S_IRWXU >> 3)
#define S_IROTH     (S_IRGRP >> 3)
#define S_IWOTH     (S_IWGRP >> 3)
#define S_IXOTH     (S_IXGRP >> 3)
#define S_IRWXO     (S_IRWXG >> 3)

#define S_ISUID     (01000)
#define S_ISGID     (02000)
#define S_ISVTX     (04000)

#define S_IALLUGO   (0777)


#define S_IFREG     0x1000
#define S_IFDIR     0x2000
#define S_IFLNK     0x3000
#define S_IFBLK     0x4000
#define S_IFCHR     0x5000
#define S_IFIFO     0x6000
#define S_IFSOCK    0x7000
#define S_IFTTY     0x8000
#define S_IFMT      0xf000

#define S_ISBLK(m)    (((m) & S_IFMT) == S_IFBLK)
#define S_ISCHR(m)    (((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m)    (((m) & S_IFMT) == S_IFDIR)
#define S_ISFIFO(m)   (((m) & S_IFMT) == S_IFIFO)
#define S_ISREG(m)    (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m)    (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m)   (((m) & S_IFMT) == S_IFSOCK)
#define S_ISTTY(m)    (((m) & S_IFMT) == S_IFTTY)


#define S_TYPEISMQ(buf)  (S_ISFIFO(buf->st_mode) && 1)  /**< Test for a message queue */
#define S_TYPEISSEM(buf) (S_ISFIFO(buf->st_mode) && 1)  /**< Test for a semaphore */
#define S_TYPEISSHM(buf) (S_ISFIFO(buf->st_mode) && 1)  /**< Test for a shared memory object */


#endif  /* _SKC_STAT_H */
