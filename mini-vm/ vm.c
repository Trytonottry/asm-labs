// vm.c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEMSIZE 4096
uint8_t mem[MEMSIZE];
uint8_t regs[8];

void run(uint8_t *code, size_t code_len){
    size_t ip=0;
    while(ip<code_len){
        uint8_t op=code[ip++];
        switch(op){
            case 0x00: break;
            case 0x01: { uint8_t r=code[ip++]; uint8_t imm=code[ip++]; regs[r]=imm; break;}
            case 0x02: { uint8_t r=code[ip++]; uint8_t imm=code[ip++]; regs[r]^=imm; break;}
            case 0x03: { uint8_t r1=code[ip++]; uint8_t r2=code[ip++]; regs[r1]+=regs[r2]; break;}
            case 0x10: { uint16_t addr = code[ip++] | (code[ip++]<<8); uint16_t len = code[ip++] | (code[ip++]<<8);
                         fwrite(&mem[addr],1,len,stdout); break;}
            case 0x20: { uint16_t addr = code[ip++] | (code[ip++]<<8); uint16_t len = code[ip++] | (code[ip++]<<8);
                         for(int i=0;i<len;i++) mem[addr+i]=code[ip++]; break;}
            case 0xFF: return;
            default: fprintf(stderr,"Unknown op %02x\n",op); return;
        }
    }
}

int main(){
    // sample: write "Hello\n" to mem@0x100 then print
    uint8_t program[] = {
        0x20, 0x00,0x01, 0x06,0x00, 'H','e','l','l','o','\n', // STORE bytes at 0x100? we'll interpret addr low-high
        // Actually using addr 0x0100: little-endian
        // but for demo write to 0x0001 due to simplicity
        0x10, 0x01,0x00, 0x06,0x00, // PRINT addr=0x0001 len=6
        0xFF
    };
    // Adjust: fill mem from program op 0x20 handler uses given address; here address=0x0100: 0x00,0x01
    // Simpler: craft properly:
    uint8_t prog2[] = {
        0x20, 0x00,0x01, 0x06,0x00, 'H','e','l','l','o','\n',
        0x10, 0x00,0x01, 0x06,0x00,
        0xFF
    };
    run(prog2, sizeof(prog2));
    return 0;
}
