// stub_loader.c
// gcc -O2 -o stub_loader stub_loader.c -lz
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>

void die(const char *m){ perror(m); exit(1); }

int main(int argc, char **argv){
    // В режиме self-executing stub: packed blob идёт после бинарника (concat),
    // либо можно запускать stub_loader payload.packed
    const char *packed_path = NULL;
    if(argc >= 2){
        packed_path = argv[1];
    } else {
        // если нет аргумента — пытаемся читать собственный файл argv[0]
        packed_path = argv[0];
    }

    FILE *f = fopen(packed_path, "rb");
    if(!f) die("fopen packed");

    // Если stub и blob конкатенированы, нужно найти смещение: в нашем простом методе
    // мы предполагаем, что файл - это чистый payload.packed (магия в начале).
    char magic[8];
    if(fread(magic,1,8,f)!=8) die("read magic");
    if(memcmp(magic,"PACKED01",8)!=0){
        fprintf(stderr,"Not a packed blob (magic mismatch)\n");
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

    // temp file, write, fchmod, unlink, exec
    char tmpfn[] = "/tmp/payloadXXXXXX";
    int fd = mkstemp(tmpfn);
    if(fd < 0) die("mkstemp");
    // Удаляем имя файла — он останется доступен по fd, но не будет виден в FS
    if(unlink(tmpfn) < 0) die("unlink tmp");

    ssize_t wrote = write(fd, out, orig);
    if((size_t)wrote != orig) die("write tmp");
    if(fchmod(fd, 0700) < 0) die("fchmod");

    // Получаем путь /proc/self/fd/<fd> для exec
    char fdpath[64];
    snprintf(fdpath, sizeof(fdpath), "/proc/self/fd/%d", fd);

    // Составляем argv для exec: передаём оставшиеся аргументы пользователю
    // Соберём newargv: newargv[0]=fdpath, затем argv[1..] либо none
    int newargc = argc; // можно пробросить аргументы
    char **newargv = malloc(sizeof(char*) * (newargc + 1));
    newargv[0] = fdpath;
    for(int i=1;i<argc;i++) newargv[i] = argv[i];
    newargv[argc] = NULL;

    // Выполняем (будет читаться через /proc/self/fd/<fd>)
    if(execv(fdpath, newargv) < 0){
        perror("execv");
        close(fd);
        exit(1);
    }
    return 0;
}
