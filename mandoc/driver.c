/*
 * Created 190223 lynnl
 *
 * Compile:
 *  gcc -Wall -Wextra driver.c -L. -lmandoc2html
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "mandoc2html.h"

#define LOG(fmt, ...)       printf(fmt "\n", ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)   fprintf(stderr, "[ERR] " fmt "\n", ##__VA_ARGS__)
#ifdef DEBUG
#define LOG_DBG(fmt, ...)   LOG("[DBG] " fmt, ##__VA_ARGS__)
#else
#define LOG_DBG(fmt, ...)   (void) ((void) 0, ##__VA_ARGS__)
#endif

#define assert_nonnull(p)   assert(p != NULL)

/**
 * Get length of a file stream
 * @return      0 if success  -1 otherwise(errno will be set)
 *              file position will at beginning if success
 *              otherwise its position is unknown
 */
static long ftell2(FILE *fp)
{
    long size;

    size = fseek(fp, 0L, SEEK_END);
    if (size != 0) {
        LOG_ERR("fseek(3) SEEK_END fail  errno: %d", errno);
        goto out_exit;
    }

    size = ftell(fp);
    if (size < 0) {
        LOG_ERR("ftell(3) fail  errno: %d", errno);
        goto out_exit;
    }

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        size = -1;
        LOG_ERR("fseek(3) SEEK_SET fail  errno: %d", errno);
    }

out_exit:
    return size;
}

/**
 * Read file content into buffer
 * @buffp       Pointer to buffer
 * @return      0 if success  -1 otherwise(errno will be set)
 *              you're responsible to free(3) *buffp if success
 */
static int read2buffer(const char *path, char **buffp)
{
    int e = -1;
    FILE *fp;
    long size;
    char *buffer;
    long size_read;

    assert_nonnull(path);
    assert_nonnull(buffp);
    assert(*buffp == NULL);

    fp = fopen(path, "r");
    if (fp == NULL) {
        LOG_ERR("fopen(3) fail  path: %s errno: %d", path, errno);
        goto out_exit;
    }

    size = ftell2(fp);
    if (size < 0) goto out_close;

    buffer = (char *) malloc((size + 1) * sizeof(char));
    if (buffer == NULL) {
        LOG_ERR("malloc(3) fail  size: %zu errno: %d", size + 1, errno);
        goto out_close;
    }

    size_read = fread(buffer, sizeof(char), size, fp);
    if (ferror(fp) == 0) {
        e = 0;
        *buffp = buffer;
        buffer[size_read++] = '\0';
    } else {
        LOG_ERR("fread(3) fail  read: %ld vs %ld errno: %d", size_read, size, errno);
        free(buffer);
    }

out_close:
    (void) fclose(fp);
out_exit:
    return e;
}

/**
 * Convert man page into HTML
 *
 * @buffp       Pointer to buffer
 * @return      0 is success  error code otherwise
 *              you're responsible to free(3) *buffp if success
 *
 * see:
 *  http://c-faq.com/stdio/undofreopen.html
 *  http://kaskavalci.com/redirecting-stdout-to-array-and-restoring-it-back-in-c/
 *  https://www.experts-exchange.com/questions/20420198/How-to-return-to-stdout-after-freopen.html
 */
int mandoc2html_buffer(const char *path, char **buffp)
{
    char template[] = "/tmp/.ManPageQL-XXXXXXXX-XXXXXXXX";
    char *tmp;
    int stdout_fileno;
    FILE *fp;
    int e;

    assert_nonnull(path);
    assert_nonnull(buffp);
    assert(*buffp == NULL);

    /* mktemp(3)'s template must on heap  otherwise you got bus error: 10 */
    tmp = mktemp(template);
    if (tmp == NULL){
        LOG_ERR("mktemp(3) fail  errno: %d", errno);
        e = -1;
        goto out_exit;
    }

    stdout_fileno = dup(STDOUT_FILENO);
    if (stdout_fileno == -1) {
        LOG_ERR("dup(2) fail  errno: %d", errno);
        e = -2;
        goto out_exit;
    }

    fp = freopen(tmp, "w", stdout);
    if (fp == NULL) {
        LOG_ERR("freopen(3) fail  path: %s errno: %d", tmp, errno);
        e = -3;
        (void) close(stdout_fileno);    /* Close just duped stdout */
        goto out_exit;
    }

    e = mandoc2html(path);

    /* Restore stdout ASAP */
    (void) fclose(stdout);
    stdout = fdopen(stdout_fileno, "w");
    if (stdout == NULL) {
        LOG_ERR("fdopen(3) fail, stdout goes haywire  fd: %d errno: %d", stdout_fileno, errno);
        (void) close(stdout_fileno);    /* Prevent rsrcleak */
        stdout = stderr;                /* Bad luck: fallback */
    }

    if (e == M2H_ERR_SUCCESS) {
        if ((e = read2buffer(tmp, buffp)) != 0) {
            e = -4;  /* Reassign an error code */
        }
    }

    /* see: https://www.gnu.org/software/libc/manual/html_node/Deleting-Files.html */
    if (unlink(tmp) != 0) LOG_ERR("unlink(2) fail  path: %s errno: %d", tmp, errno);

out_exit:
    return e;
}

void usage(void)
{
    fprintf(stderr, "usage: file\n");
    exit(-1);
    __builtin_unreachable();
}

int main(int argc, char *argv[])
{
#if 1
    char *buffer = NULL;
    int e;

    if (argc != 2) usage();

    e = mandoc2html_buffer(argv[1], &buffer);
    if (e != 0) {
        LOG_ERR("mandoc2html_buffer() fail  error: %d", e);
    } else {
        assert_nonnull(buffer);
        LOG("%s", buffer);
    }

    return e;
#else
    return mandoc2html(argv[1]);
#endif
}
