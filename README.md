Corbenik
==============================

This is (yet another) CFW for the 3DS. Unlike other CFWs, this was mostly written from scratch for fun, and because I'm a control freak.

Conceptually, and in operation, it is most similar to mid-kid/CakesForeveryWan out of the bunch. That is, it uses external patches from the filesystem. Unlike cakes, patches are dynamically offset - the same way Luma3DS and ReiNand do things. But like cakes, they're software-defined.

More importantly - for arm9 programmers who don't like headaches - I have an implementation of console printf you may be interested in, and a very close imitation of stdio around fatfs in the `std` folder.

The version of loader in this repo is very tightly tied to the operation of Corbenik. It takes care of all binary changes on initialization. It should not be used with other versions of loader.

If you want to know how it sizes up to other CFWs - here's a quicklist of things it does:

Signature patches
--------------------------

Done via standard search and replace.

FIRM Protection
--------------------------

Done similarly to Reinand - replacing the 'exe' string with 'prt'

System module replacement
--------------------------

Overrides the complete module - can't change size.

ExeFs replacement
--------------------------

Overrides the whole exefs.

Locale Emulation
--------------------------

Different from existing solutions - a single text file of the format:
```
<TITLEID> <REGION> <LANGUAGE>
```
This is technically superior because it doesn't involve large and unloadable directory trees. Corbenik comes with an example locale configuration built automatically from single-region single-language games on 3DSDB.

ARM9 control thread
--------------------------

Because...well, I hate NTR with a passion. This is one of the primary features differentiating Corbenik from other CFWs. Hitting X+Y pops open a menu and allows configuring any loader-based patches, as well as taking screenshots and performing memory dumps. I will not add plugins created in ARM9 code, but I may embed an interpreter someday.

