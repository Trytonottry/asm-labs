// rle_packer.c
// gcc -O2 -o rle_packer rle_packer.c
// Usage: ./rle_packer <input_binary> <output_stub.c>

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

void die(const char *s){ perror(s); exit(1); }

int main(int argc, char **argv){
    if(argc!=3){
        fprintf(stderr, "Usage: %s <input_binary> <output_stub.c>\n", argv[0]);
        return 1;
    }
    const char *infile = argv[1];
    const char *outfile = argv[2];

    FILE *f = fopen(infile, "rb");
    if(!f) die("fopen in");
    struct stat st;
    if(fstat(fileno(f), &st) < 0) die("fstat");
    size_t inlen = st.st_size;
    uint8_t *in = malloc(inlen);
    if(!in) die("malloc in");
    if(fread(in,1,inlen,f) != inlen) die("fread");
    fclose(f);

    // Simple RLE: encode as (count, byte) pairs, count in [1..255]
    uint8_t *out = malloc(inlen * 2 + 16); // worst-case
    size_t outpos = 0;
    size_t i = 0;
    while(i < inlen){
        uint8_t val = in[i];
        uint8_t cnt = 1;
        size_t j = i + 1;
        while(j < inlen && in[j] == val && cnt < 255){
            cnt++; j++;
        }
        out[outpos++] = cnt;
        out[outpos++] = val;
        i = j;
    }

    FILE *fo = fopen(outfile, "w");
    if(!fo) die("fopen out");

    // write a C stub with embedded blob
    fprintf(fo,
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"#include <stdint.h>\n"
"#include <unistd.h>\n"
"#include <sys/syscall.h>\n"
"#include <sys/mman.h>\n"#include <sys/stat.h>\n"
"#include <fcntl.h>\n"
"#include <string.h>\n"
"\n"
"// Generated stub. RLE format: sequence of (uint8_t count, uint8_t value)\n"
"static const unsigned char packed_blob[] = {");
    for(size_t k=0;k<outpos;k++){
        if(k % 16 == 0) fprintf(fo, "\n    ");
        fprintf(fo, "0x%02x, ", out[k]);
    }
    fprintf(fo, "\n};\n\n");
    fprintf(fo, "static const unsigned long packed_len = %zuUL;\n", outpos);
    fprintf(fo, "static const unsigned long orig_len = %zuUL;\n\n", inlen);

    // add memfd_exec routine + unpack + exec
    fprintf(fo,
"static int my_memfd_create(const char *name, unsigned int flags){\n"
"    #ifdef SYS_memfd_create\n"
"    return syscall(SYS_memfd_create, name, flags);\n"
"    #else\n"
"    return -1; // unsupported\n"
"    #endif\n"
"}\n\n"
"void die(const char *s){ perror(s); _exit(1); }\n\n"
"int main(int argc, char **argv){\n"
"    // Decompress RLE into memfd\n"
"    unsigned long est = orig_len;\n"
"    int fd = my_memfd_create(\"packed_exec\", 1 /*MFD_CLOEXEC*/);\n"
"    if(fd < 0) die(\"memfd_create\");\n"
"    unsigned long written = 0;\n"
"    for(unsigned long p=0;p<packed_len; p+=2){\n"
"        unsigned int cnt = packed_blob[p];\n"
"        unsigned char val = packed_blob[p+1];\n" 
"        for(unsigned int c=0;c<cnt;c++){\n"
"            ssize_t w = write(fd, &val, 1);\n"
"            if(w != 1) die(\"write memfd\");\n" 
"            written++;\n" 
"        }\n"
"    }\n"
"    if(written != orig_len) die(\"written mismatch\");\n" 
"    if(fchmod(fd, 0700) < 0) die(\"fchmod\");\n"
"    // Exec via /proc/self/fd/<fd>\n"
"    char fdpath[64];\n"
"    snprintf(fdpath, sizeof(fdpath), \"/proc/self/fd/%d\", fd);\n"
"    // Build new argv: pass through program args\n"
"    int newargc = argc;\n"
"    char **newargv = malloc(sizeof(char*) * (newargc + 1));\n"
"    if(!newargv) die(\"malloc argv\");\n"
"    newargv[0] = fdpath;\n"
"    for(int i=1;i<argc;i++) newargv[i] = argv[i];\n"
"    newargv[argc] = NULL;\n"
"    execv(fdpath, newargv);\n"
"    die(\"execv\");\n"
"    return 0;\n"
"}\n");

    fclose(fo);
    printf("Stub generated to %s — orig=%zu comp=%zu\n", outfile, inlen, outpos);
    free(in); free(out);
    return 0;
}
