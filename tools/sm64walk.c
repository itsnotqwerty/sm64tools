#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libsm64.h"
#include "../utils.h"

#define SM64WALK_VERSION "0.1"

static void print_usage(void)
{
   ERROR("Usage: sm64walk [-v] FILE\n"
         "\n"
         "sm64walk v" SM64WALK_VERSION ": Super Mario 64 script walker\n"
         "\n"
         "Optional arguments:\n"
         " -v           verbose progress output\n"
         "\n"
         "File arguments:\n"
         " FILE        input ROM file\n");
   exit(EXIT_FAILURE);
}

// parse command line arguments
static void parse_arguments(int argc, char *argv[], char *in_filename)
{
   int i;
   int file_count = 0;
   if (argc < 2) {
      print_usage();
   }
   for (i = 1; i < argc; i++) {
      if (argv[i][0] == '-') {
         switch (argv[i][1]) {
            case 'v':
               g_verbosity = 1;
               break;
            default:
               print_usage();
               break;
         }
      } else {
         switch (file_count) {
            case 0:
               strcpy(in_filename, argv[i]);
               break;
            default: // too many
               print_usage();
               break;
         }
         file_count++;
      }
   }
   if (file_count < 1) {
      print_usage();
   }
}

typedef struct
{
   unsigned start;
   unsigned end;
} level_t;

static void add_level(level_t levels[], unsigned *lcount, unsigned start, unsigned end)
{
   unsigned i;
   for (i = 0; i < *lcount; i++) {
      if (levels[i].start == start) {
         return;
      }
   }
   levels[*lcount].start = start;
   levels[*lcount].end = end;
   (*lcount)++;
}

static void decode_level(unsigned char *data, level_t levels[], unsigned int l, unsigned *lcount)
{
   unsigned int ptr_start;
   unsigned int ptr_end;
   unsigned int dst;
   unsigned int a;
   int i;

   printf("Decoding level script %X\n", levels[l].start);

   a = levels[l].start;
   // length = 0 ends level script
   while (a < levels[l].end && data[a+1] != 0) {
      switch (data[a]) {
         case 0x00: printf("LoadJump0"); break; // load and jump from ROM into a RAM segment
         case 0x01: printf("LoadJump1"); break; // load and jump from ROM into a RAM segment
         case 0x03: printf("Delay3   "); break; // delay frames
         case 0x04: printf("Delay4   "); break; // delay frames and signal end
         case 0x05: printf("JumpSeg  "); break; // jump to level script at segmented address
         case 0x06: printf("PushJump "); break; // push script stack and jump to segmented address
         case 0x07: printf("PopScript"); break; // pop script stack, return to prev 0x06 or 0x0C
         case 0x0B: printf("CondPop  "); break; // conditional stack pop
         case 0x0C: printf("CondJump "); break; // conditional jump to segmented address
         case 0x11: printf("CallFunc1"); break; // call function
         case 0x12: printf("CallFuncM"); break; // call function
         case 0x16: printf("LoadASM  "); break; // load ASM into RAM
         case 0x17: printf("CopyData "); break; // copy uncompressed data from ROM to a RAM segment
         case 0x18: printf("CopyMIO0 "); break; // decompress MIO0 data from ROM and copy it into a RAM segment
         case 0x1A: printf("CopyMIO0 "); break; // decompress MIO0 data from ROM and copy it into a RAM segment (for texture only segments?)
         case 0x1B: printf("StartLoad"); break; // start RAM loading sequence (before 17, 18, 1A)
         case 0x1D: printf("EndLoad  "); break; // end RAM loading sequence (before 17, 18, 1A)
         case 0x1F: printf("StartArea"); break; // start of an area
         case 0x20: printf("EndArea  "); break; // end of an area
         case 0x21: printf("LoadPoly "); break; // load polygon data without geo layout
         case 0x22: printf("LdPolyGeo"); break; // load polygon data with geo layout
         case 0x24: printf("LoadObjct"); break; // load object with behavior
         case 0x25: printf("LoadMario"); break; // load mario object with behavior
         case 0x26: printf("ConctWarp"); break; // connect warps
         case 0x27: printf("PaintWarp"); break; // level warps for paintings
         case 0x2B: printf("MarioStrt"); break; // Mario's default position
         case 0x2E: printf("Collision"); break; // load collision data
         case 0x31: printf("Terrain  "); break; // terrain type
         case 0x34: printf("Blackout "); break; // blackout screen
         case 0x36: printf("Music    "); break; // music
         case 0x39: printf("MulObject"); break; // multiple objects from main level segment
         default:   printf("         "); break;
      }
      printf(" %02X %02X %02X%02X ", data[a], data[a+1], data[a+2], data[a+3]);
      switch (data[a]) {
         case 0x00: // load and jump from ROM into a RAM segment
         case 0x01: // load and jump from ROM into a RAM segment
            ptr_start = read_u32_be(&data[a+4]);
            ptr_end = read_u32_be(&data[a+8]);
            printf("%08X %08X %08X\n", ptr_start, ptr_end, read_u32_be(&data[a+0xc]));
            add_level(levels, lcount, ptr_start, ptr_end);
            break;
         case 0x17: // copy uncompressed data from ROM to a RAM segment
         case 0x18: // decompress MIO0 data from ROM and copy it into a RAM segment
         case 0x1A: // decompress MIO0 data from ROM and copy it into a RAM segment (for texture only segments?)
            ptr_start = read_u32_be(&data[a+4]);
            ptr_end = read_u32_be(&data[a+8]);
            printf("%08X %08X\n", ptr_start, ptr_end);
            break;
         case 0x11: // call function
         case 0x12: // call function
            ptr_start = read_u32_be(&data[a+0x4]);
            printf("%08X %08X\n", read_u32_be(&data[a]), ptr_start);
            break;
         case 0x16: // load ASM into RAM
            dst       = read_u32_be(&data[a+0x4]);
            ptr_start = read_u32_be(&data[a+0x8]);
            ptr_end   = read_u32_be(&data[a+0xc]);
            printf("%08X %08X %08X\n", dst, ptr_start, ptr_end);
            break;
         case 0x25: // load mario object with behavior
         case 0x24: // load object with behavior
            printf("%08X", read_u32_be(&data[a]));
            for (i = 4; i < data[a+1]-4; i+=4) {
               printf(" %08X", read_u32_be(&data[a+i]));
            }
            dst = read_u32_be(&data[a+i]);
            printf(" %08X\n", dst);
            break;
         default:
            for (i = 4; i < data[a+1]; i+=4) {
               printf("%08X ", read_u32_be(&data[a+i]));
            }
            printf("\n");
            break;
      }
      a += data[a+1];
   }
   printf("Done %X\n\n", levels[l].start);
}

static void walk_scripts(unsigned char *data)
{
   level_t levelscripts[100];
   unsigned lcount = 0;
   unsigned l = 0;
   // add main entry level script
   levelscripts[0].start = 0x108A10;
   levelscripts[0].end   = 0x108A40;
   lcount++;
   while (l < lcount) {
      decode_level(data, levelscripts, l, &lcount);
      l++;
   }
}

int main(int argc, char *argv[])
{
   char in_filename[FILENAME_MAX];
   unsigned char *in_buf = NULL;
   long in_size;
   int rom_type;

   // get configuration from arguments
   parse_arguments(argc, argv, in_filename);

   // read input file into memory
   in_size = read_file(in_filename, &in_buf);
   if (in_size <= 0) {
      ERROR("Error reading input file \"%s\"\n", in_filename);
      exit(EXIT_FAILURE);
   }

   // confirm valid SM64
   rom_type = sm64_rom_type(in_buf, in_size);
   if (rom_type < 1) {
      ERROR("This does not appear to be a valid SM64 ROM\n");
      exit(EXIT_FAILURE);
   } else if (rom_type == 0) {
      ERROR("This ROM is already extended!\n");
      exit(EXIT_FAILURE);
   } else if (rom_type == 1) {
      // byte-swapped BADC format, swap to big-endian ABCD format for processing
      INFO("Byte-swapping ROM\n");
      swap_bytes(in_buf, in_size);
   }

   // walk those scripts
   walk_scripts(in_buf);

   // cleanup
   free(in_buf);

   return EXIT_SUCCESS;
}