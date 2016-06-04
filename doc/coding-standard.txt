There's only *one* real hard rule: run `make reformat` before you commit.

Format is a modified mozilla standard - four spaces, not two.

Also, there's a few hard-enforced rules you need to obey if you want to submit code:

  1) Avoid trigraphs at all costs. Only use them if it actually helps readability.

  2) Any `#pragma` macros are banned. You're using linux and GCC, so stop using MSVCisms.
     There's another few quirks about #pragma once that mean I have to use guards instead.
     Use `__atrribute__` instead.

  3) If by some odd chance you use inline assembly; use `__asm__`, not `asm`.

  4) Do not remove `-Werror`. It's there for a reason. Your code should be squeaky clean;
     if not, find a way to make your code correct. This is mandatory.
