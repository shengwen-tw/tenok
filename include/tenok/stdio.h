/**
 * @file
 */
#ifndef __STDIO_H__
#define __STDIO_H__

#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/reent.h>
#include <sys/types.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define fopen _fopen
#define fclose _fclose
#define fread _fread
#define fwrite _fwrite
#define fseek _fseek
#define fileno _fileno

#define __SIZEOF_FILE sizeof(__FILE)

typedef union {
    char __size[__SIZEOF_FILE];
    uint32_t __align;
} FILE;

/**
 * @brief  Open the file whose name is the string pointed to by pathname and
 *         associates a stream with it
 * @param  pathname: The pathname of the file to open.
 * @param  mode: Not used.
 * @retval FILE *: File stream object.
 */
FILE *fopen(const char *pathname, const char *mode);

/**
 * @brief  Close the given file stream
 * @param  stream: The file stream to provide.
 * @retval int: 0 on sucess and nonzero error number on error.
 */
int fclose(FILE *stream);

/**
 * @brief  Read nmemb items of data, each size bytes long, from the stream
 *         pointed to by stream, storing them at the location given by ptr
 * @param  ptr: The memory space for storing the read data.
 * @param  size: The number of nmemb bytes to read.
 * @param  nmemb: The bytes number to read at once.
 * @param  stream: The file stream to provide.
 * @retval int: The read number on sucess and nonzero error number on error.
 */
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

/**
 * @brief  Write nmemb items of data, each size bytes long, to the stream
 *         pointed to by stream, obtaining them  from  the  location given by
 * ptr
 * @param  ptr: The data to write to the file.
 * @param  size: The number of nmemb bytes to write.
 * @param  nmemb: The byte number to write at once.
 * @param  stream: The file stream to provide.
 * @retval int: The write number on sucess and nonzero error number on error.
 */
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/**
 * @brief  Set the file position indicator for the stream pointed to by stream
 * @param  stream: The file stream to provide.
 * @param  offset: The new offset to the position specified by whence.
 * @param  whence: The start position of the new offset.
 * @retval int: The new file position on sucess and nonzero error number on
 * error.
 */
int fseek(FILE *stream, long offset, int whence);

/**
 * @brief  Examine the argument stream and returns the integer file descriptor
 *         used to implement this stream
 * @param  stream: The file stream to provide.
 * @retval int: The file descriptor integer.
 */
int fileno(FILE *stream);

int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
int vsprintf(char *str, const char *format, va_list);
int vsnprintf(char *str, size_t, const char *format, va_list);
int vdprintf(int fd, const char *format, va_list ap);
int dprintf(int fd, const char *format, ...);

#endif
