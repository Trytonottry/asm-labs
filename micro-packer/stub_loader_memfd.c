// stub_loader_memfd.c
// gcc -O2 -o stub_memfd stub_loader_memfd.c -lz
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <zlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>

#ifndef MFD_CLOEXEC
#define MFD_CLOEXEC 0x0001U
#endif

static int my_memfd_create(const char *name, unsigned int flags){
#ifdef SYS_memfd_create
    return syscall(SYS_memfd_create, name, flags);
#else
    errno = ENOSYS;
    return -1;
#endif
}

void die(const char *m){ perror(m); exit(1); }

int main(int argc, char **argv){
    const char *packed_path = NULL;
    if(argc >= 2) packed_path = argv[1];
    else packed_path = argv[0];

    FILE *f = fopen(packed_path, "rb");
    if(!f) die("fopen packed");

    char magic[8];
    if(fread(magic,1,8,f)!=8) die("read magic");
    if(memcmp(magic,"PACKED01",8)!=0){
        fprintf(stderr,"Not a packed blob\n");
        fclose(f);
        return 1;
    }
    uint64_t orig, comp;
    if(fread(&orig,sizeof(orig),1,f)!=1) die("read orig");
    if(fread(&comp,sizeof(comp),1,f)!=1) die("read comp");
    unsigned char *cmpbuf = malloc(comp);
    if(!cmpbuf) die("malloc cmp");
    if(fread(cmpbuf,1,comp,f)!=comp) die("read blob");
    fclose(f);

    unsigned char *out = malloc(orig);
    if(!out) die("malloc out");
    uLong dest = (uLong)orig;
    if(uncompress(out, &dest, cmpbuf, comp) != Z_OK) die("uncompress");

    // memfd_create
    int fd = my_memfd_create("payload_memfd", MFD_CLOEXEC);
    if(fd < 0) die("memfd_create");
    ssize_t w = write(fd, out, orig);
    if((size_t)w != orig) die("write memfd");

    // Set executable permission on the memfd via fchmod
    if(fchmod(fd, 0700) < 0) die("fchmod memfd");

    // exec via /proc/self/fd/<fd>
    char fdpath[64];
    snprintf(fdpath, sizeof(fdpath), "/proc/self/fd/%d", fd);

    // Build new argv: pass through remaining args (we can strip first)
    char **newargv = malloc(sizeof(char*) * (argc + 1));
    newargv[0] = fdpath;
    for(int i=1;i<argc;i++) newargv[i] = argv[i];
    newargv[argc] = NULL;

    if(execv(fdpath, newargv) < 0){
        perror("execv memfd");
        close(fd);
        exit(1);
    }
    return 0;
}
