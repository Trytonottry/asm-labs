// packer.c
// gcc -O2 -o packer packer.c -lz
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <errno.h>

void die(const char *m){
    perror(m);
    exit(1);
}

int main(int argc, char **argv){
    if(argc!=3){
        fprintf(stderr,"Usage: %s <input_binary> <output_packed>\n", argv[0]);
        return 1;
    }
    const char *infile = argv[1];
    const char *outfile = argv[2];

    FILE *f = fopen(infile, "rb");
    if(!f) die("fopen input");
    struct stat st;
    if(fstat(fileno(f), &st) < 0) die("fstat");
    size_t inlen = st.st_size;
    unsigned char *in = malloc(inlen);
    if(!in) die("malloc input");
    if(fread(in,1,inlen,f) != inlen) die("fread");
    fclose(f);

    uLong bound = compressBound(inlen);
    unsigned char *out = malloc(bound);
    if(!out) die("malloc out");
    uLong outlen = bound;
    if(compress(out, &outlen, in, inlen) != Z_OK) die("compress");

    FILE *fo = fopen(outfile, "wb");
    if(!fo) die("fopen output");

    // header: 8 bytes magic + 8 bytes orig len (LE) + 8 bytes comp len (LE)
    const char magic[8] = "PACKED01";
    if(fwrite(magic,1,8,fo)!=8) die("write magic");
    uint64_t orig = (uint64_t)inlen;
    uint64_t comp = (uint64_t)outlen;
    if(fwrite(&orig, sizeof(orig), 1, fo)!=1) die("write orig");
    if(fwrite(&comp, sizeof(comp), 1, fo)!=1) die("write comp");
    if(fwrite(out,1,outlen,fo) != outlen) die("write blob");
    fclose(fo);

    fprintf(stdout, "Packed: %s -> %s (orig=%lu comp=%lu)\n", infile, outfile, (unsigned long)inlen, (unsigned long)outlen);

    free(in); free(out);
    return 0;
}
