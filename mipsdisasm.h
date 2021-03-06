#ifndef MIPSDISASM_H_
#define MIPSDISASM_H_

#include "config.h"

// defines

#define MAX_PROCEDURES (32 * 1024)

// typedefs

typedef struct _local_table
{
   unsigned int *offsets;
   int count;
} local_table;

typedef struct _procedure
{
   unsigned int start;
   unsigned int end;
   local_table locals;
} procedure;

typedef struct _proc_table
{
   procedure procedures[MAX_PROCEDURES];
   int count;
} proc_table;


// convert from RAM address to ROM address
// config: ROM configuration
// ram_addr: RAM address to convert from
// returns ROM address that maps to the RAM address
unsigned int ram_to_rom(rom_config *config, unsigned int ram_addr);

// convert from ROM offset to RAM address
// config: ROM configuration
// rom_addr: ROM address to convert from
// returns RAM address that maps to the ROM address
unsigned int rom_to_ram(rom_config *config, unsigned int rom_addr);

// generate string label for address based on config sections and labels
// config: ROM configuration
// addr: ROM or RAM address to use
// label: output string
// hint: 0=start/1=end
// returns 0 if default type, 1 if computation
int fill_addr_label(rom_config *config, unsigned int addr, char *label, int hint);

// add procedures from config labels
// procs: procedures list to add labels to
// config: ROM configuration
// file_len: file length for error checking
void mipsdisasm_add_procs(proc_table *procs, rom_config *config, long file_len);

// first pass of disassembler - collects procedures called and sorts them
// data: buffer containing raw MIPS assembly
// datalen: length of 'data'
// procs: procedure table
// config: ROM configuration
void mipsdisasm_pass1(unsigned char *data, long datalen, proc_table *procs, rom_config *config);

// disassemble a given procedure
// out: stream to output data to
// data: buffer containing raw MIPS assembly
// datalen: length of 'data'
// proc: the procedure to disassemble
// config: ROM configuration
// merge_pseudo: if non-zero, merge pseudoinstructions
// returns the RAM address of the end of disassembly
unsigned int disassemble_proc(FILE *out, unsigned char *data, long datalen, procedure *proc, rom_config *config, int merge_pseudo);

// get version string of raw disassembler
const char *disasm_get_version(void);

#endif // MIPSDISASM_H_
