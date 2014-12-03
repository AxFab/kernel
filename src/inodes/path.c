/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Methods used to build pathname and display
 */
#include <kernel/vfs.h>
#include <kernel/info.h>
#include <kernel/cpu.h>

// ===========================================================================

int kfs_plink (kInode_t* ino, char* ptr, size_t length)
{
  int ret;
  char uri [PATH_MAX];
  uri[0] = ptr[0] = '\0';

  if (!ino)
    return __seterrno (EINVAL);

  for (;;) {
    if (ino->parent_ == NULL) {
      return __noerror ();
    }

    ret = snprintf (uri, PATH_MAX, "/%s%s", ino->name_, ptr);
    strncpy (ptr, uri, length);

    if (ret < 0 || ret > PATH_MAX) {
      return __seterrno(ENAMETOOLONG);
    }

    ino = ino->parent_;
  }
}

// --------------------------------------------------------------------------
/** */
int kfs_puri (kInode_t* ino, char* ptr, size_t length)
{
#define ISVOL(ino) (S_ISDIR(ino->stat_.mode_) && ino->dev_ != ino->parent_->dev_)
  int ret;
  char uri [PATH_MAX];
  uri[0] = ptr[0] = '\0';

  if (!ino)
    return __seterrno (EINVAL);

  for (;;) {
    if (ino == NULL) {
      ret = snprintf (uri, PATH_MAX, "%s:%s", "Sys", ptr);

    } else if (ISVOL(ino)) {
      ret = snprintf (uri, PATH_MAX, "%s:%s", ino->name_, ptr);

    } else {
      ret = snprintf (uri, PATH_MAX, "\\%s%s", ino->name_, ptr);
    }

    strncpy (ptr, uri, length);

    if (ret < 0 || ret > PATH_MAX) {
      return __seterrno(ENAMETOOLONG);
    }

    if (ino == NULL || ISVOL(ino)) {
      return __noerror ();
    }

    ino = ino->parent_;
  }
}


// // --------------------------------------------------------------------------
// void kfs_log_ino (kInode_t* ino, int depth)
// {
//   int i;
//   char type;
//   char* rights[] = {
//     "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"
//   };
//   // char uri [PATH_MAX];
//   int o = ino->stat_.mode_ & 7;
//   int g = (ino->stat_.mode_ >> 3) & 7;
//   int u = (ino->stat_.mode_ >> 6) & 7;

//   switch (ino->stat_.mode_ & S_IFMT) {
//     case S_IFBLK:
//       type = 'b';
//       break;
//     case S_IFDIR:
//       type = 'd';
//       break;
//     case S_IFLNK:
//       type = 'l';
//       break;
//     case S_IFREG:
//       type = '-';
//       break;
//     case S_IFCHR:
//       type = 'c';
//       break;
//     case S_IFIFO:
//       type = 'p';
//       break;
//     case S_IFSOCK:
//       type = 's';
//       break;
//     default:
//       type = '?';
//       break;
//   }

//   kprintf("%c%s%s%s %3d", type, rights[u], rights[g], rights[o], 1);
//   kprintf(" %-5s", ino->stat_.uid_ == ROOT_UID ? "root" : " -");
//   // kprintf(" %-5s", ino->stat_.gid_ == ROOT_UID ? "root" : " -");
//   //kprintf(" %-12s", kSizeFormat(ino->stat_._length));
//   kprintf(" <%s>", kpsize(ino->stat_.length_));

//   for (i = 0; i < depth; ++i)
//     kprintf("  ");

//   //kfs_plink (ino, uri, PATH_MAX);
//   //kprintf(" %s\n", uri);
//   kprintf(" '%s'\n", ino->name_);
// }

// // --------------------------------------------------------------------------
// void kfs_log_dir (kInode_t* ino, int depth)
// {
//   ino = ino->child_;

//   while (ino) {
//     kfs_log_ino (ino, depth);

//     if (ino->child_) {
//       kfs_log_dir (ino, depth + 1);
//     }

//     ino = ino->next_;
//   }
// }

// // --------------------------------------------------------------------------
// void kfs_log_all ()
// {
//   kprintf ("Vfs debug display -----\n");
//   kInode_t* ino = kSYS.rootNd_;
//   kfs_log_ino (ino, 0);
//   kfs_log_dir (ino, 1);
// }

