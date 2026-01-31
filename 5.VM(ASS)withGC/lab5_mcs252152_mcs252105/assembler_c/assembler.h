#ifndef ASM_H
#define ASM_H

#include <stdint.h>

int lookup_opcode(char *word, uint8_t *opcode);

int assemble(char *infile, char *outfile);


#endif
