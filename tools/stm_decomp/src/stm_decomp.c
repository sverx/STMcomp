/* decompressor for STMcompr files
 *
 * usage: stm_decomp <input stmcompr file> <output binary data file>
 *
 * the decompressor will list each run on the standard output, you can pipe that to a text file if needed
 *
 *
*/

#include <stdio.h>
#include <stdbool.h>

FILE *fIN;
FILE *fOUT;
unsigned char in_data;
unsigned char HH, old_HH;
unsigned char run_len;
unsigned short out_data;
bool is_temp;

void restore_HH (void) {
  // this will make sure that HH is restored to its old value if the last run was under a 'temp HH' flag
  if (is_temp) {
    is_temp=false;
    HH = old_HH;
  }
}

int main(int argc, char const* *argv) {

  if (argc<3) {
    printf("Fatal: unrecognized arguments. Usage: stm_decomp <input stmcompr file> <output binary data file>\n");
    return(1);
  }

  fIN=fopen(argv[1],"rb");
  if (!fIN) {
    printf("Fatal: can't open input stmcompr file\n");
    return(1);
  }

  fOUT=fopen(argv[2],"wb");
  if (!fOUT) {
    printf("Fatal: can't open output binary file\n");
    return(1);
  }

  fread (&in_data, 1, 1, fIN);
  printf("Map width (tiles):%d\n",in_data);

  while (!feof(fIN)) {

    fread (&in_data, 1, 1, fIN);

    switch (in_data & 0x03) {
      case 0:
        // run of different values
        run_len = in_data >> 2;
        if (run_len==0) {
          printf("End of data\n");
          return(0);
        }

        printf("Run of %d different values:",run_len);

        while (run_len>0) {
          fread (&in_data, 1, 1, fIN);
          printf("%02x ",in_data);

          out_data=HH*256+in_data;
          fwrite (&out_data, 1, 2, fOUT);
          run_len--;
        }

        printf ("\n");

        restore_HH();
        break;

      case 1:
        // run of same word values
        run_len = 2+(in_data >> 2);
        fread (&in_data, 1, 1, fIN);
        printf("Run of %d same values:%02x %02x\n",run_len, in_data, HH);

        while (run_len>0) {
          out_data=HH*256+in_data;
          fwrite (&out_data, 1, 2, fOUT);
          run_len--;
        }

        restore_HH();
        break;

      case 3:
        // run of successive word values
        run_len = 2+(in_data >> 2);
        fread (&in_data, 1, 1, fIN);
        printf("Run of %d successive values from %02x %02x\n",run_len, in_data, HH);

        out_data=HH*256+in_data;

        while (run_len>0) {
          fwrite (&out_data, 1, 2, fOUT);
          out_data++;
          run_len--;
        }

        // this might have set HH to a new value, take note of that!
        HH = out_data>>8;

        restore_HH();
        break;

      case 2:
        // set new HH value
        old_HH = HH;
        HH=(in_data >> 3);
        is_temp=((in_data & 0x04)!=0);
        printf("Upper byte value set to %02x, %s\n", HH, (is_temp)?"temporarily":"permanently");
        break;
    }
  }

  return(0);
}
