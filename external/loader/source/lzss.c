#include <3ds.h>

int lzss_decompress(u8 *buffer)
{
  // This WAS originally a decompilation in @yifan_lu's repo; it was rewritten for readability following ctrtool's namings.
  // You can thank me for making it more readable if you'd like; I don't really care. Did it for myself.
  unsigned int decompSize, v15;
  u8 *compressEndOff, *index, *stopIndex;
  char control;
  int v9, v13, v14, v16;
  int ret = 0;

  if ( !buffer ) // Return immediately when buffer is invalid.
    return 0;

  // v1=decompressedSize, v2=compressedSize, v3=index, v4=stopIndex
  decompSize     = *((u32 *)buffer - 2);
  compressEndOff = &buffer[*((u32 *)buffer - 1)];
  index          = &buffer[-(decompSize >> 24)]; // FIXME - The integer negation is due to a compiler optimization. It's probably okay, but...
  stopIndex      = &buffer[-(decompSize & 0xFFFFFF)];

  while ( index > stopIndex ) // index > stopIndex
  {
    control = *(index-- - 1); // control (just scoping though)
    for (int i=0; i<8; i++)
    {
      if ( control & 0x80 ) // control & 0x80
      {
        v13 = *(index - 1);
        v14 = *(index - 2);
        index -= 2;
        v15 = ((v14 | (v13 << 8)) & 0xFFFF0FFF) + 2;
        v16 = v13 + 32;
        do
        {
          ret = compressEndOff[v15];
          *(compressEndOff-- - 1) = ret;
          v16 -= 16;
        }
        while ( !(v16 < 0) );
      }
      else
      {
        v9 = *(index-- - 1);
        ret = v9;
        *(compressEndOff-- - 1) = v9;
      }
      control *= 2;
      if ( index <= stopIndex )
        return ret;
    }
  }

  return ret;
}
