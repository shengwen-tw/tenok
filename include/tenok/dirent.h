/**
 * @file
 */
#ifndef __DIRENT_H__
#define __DIRENT_H__

#include <stdint.h>
#include <sys/limits.h>

#include "kconfig.h"

/* Return type of the opendir() */
typedef struct dirstream {
    struct inode *inode_dir;       /* Directory inode */
    struct list_head *dentry_list; /* List pointer of the dentry to return */
} DIR;

/* Return type of the readdir() */
struct dirent {
    char d_name[NAME_MAX]; /* File name */
    uint32_t d_ino;        /* The inode of the file */
    uint8_t d_type;        /* File type */
};

/**
 * @brief  Open a directory stream corresponding to the directory name,
 *         and returns a pointer to the directory stream
 * @param  name: The file path to the directory.
 * @param  dir: The directory stream to return.
 * @retval int: 0 on success and nonzero error number on error.
 */
int opendir(const char *name, DIR *dir);

/**
 * @brief  Return a pointer to a dirent structure representing the next
 *         directory entry in the directory stream pointed to by dirp
 * @param  dirp: Pointer to the dirent object.
 * @param  dirent: The structure of file entries under the directory to
 *         return.
 * @retval int: 0 on success and nonzero error number on error.
 */
int readdir(DIR *dirp, struct dirent *dirent);

#endif
