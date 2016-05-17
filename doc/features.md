Feature graph
----------------

+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|CFW     |Firms?           |Patch Method     |Supplies Officially           |Available  |Optimization[1]|Focus?             |Notes  |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|CakesFW |Dec/Enc (SD)     |Loadable, Fixed  |Sig,Emu(M),Twl,Agb,Sys,Ptc    |Mis,Bck,Mod|Speed/Read     |Devs/Advanced Users| [2]   |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|Luma3DS |Enc(NAND) Dec(SD)|Builtin, Dynamic |Sig,Emu(2),Sys,Mod,Bck,Mis,Ptc|N/A        |Giant Mess     |"Noob-proof"       | [3]   |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|ReiNAND |Meme(SD)         |Builtin, Dynamic |Sig,Emu,Sys,Mod,Ptc           |N/A        |Readability    |Minimalist         | [4]   |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|Corbenik|Dec/Enc (SD)     |Builtin, Dynamic |Sig,Ptc,Sys,Svc               |N/A        |Read/Speed     |Advanced Devs      | [5]   |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|NTR     |N/A              |Executable       |Mis                           |Mis        |Douchebaggery  |Shilling Closed Src| [6]   |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+

 * Sig: Signature Patch.
 * Twl: TWL Signature Patch
 * Agb: AGB Signature Patch
 * Ptc: FIRM protection
 * Emu: Emunand.
   * (M) - Can have many (unlimited)
   * (X) - Can have X emunands.
 * Sys: Can boot sysnand from a9lh.
 * Mod: Loader replacement
 * Bck: Load svcBackdoor on 11.0
 * Mis: Other misc fixes and alterations including:
   * UNITINFO patch
   * GBA bios screen

Misc features by CFW:
 * Cakes:
   * mid-kid/lgy_cakes
     * TWL patches
     * AGB patches
   * 3ds_injector
     * You have multiple choices. Wolfvak's, mid-kid's, mine, etc...
   * Wolfvak/icing
     * UNITINFO
     * svcBackdoor (incomplete)
 * Luma
   * Loader replacement. CPU speed/language emulation
 * Corbenik
   * Arbitrary service injection to empty slots.
     Similar results can be achieved in cakes, but with much pain.

[1] I'm not just referring to speed; I'm referring to purpose-based optimization. For example;
    do you want something well documented? Would you choose a slower algorithm that can be more
    easily debugged at the expense of speed? Would you use preprocessor macros to use different implementations?

    CakesFW is Speed/Read. Speed first, keep readability if possible whenever possible. Which is a good
    approach.

    No, I don't call Luma a complete mess lightly. Go look thorugh the code. You back? Good. Tell me,
    are the comments helpful and do you understand what the blatant pointer abuse in patches.c is
    actually ref'ing? No? I had to decode that. It's referencing the exefs's code section. Which was
    impossible to tell without ten minutes staring at it. Moving on now...

    ReiNAND is cruft-free and well documented. It doesn't do a lot, but what it does do, it does well.
    Unix philosphy in a nutshell. The only complaint is memekey.

    My focus will always be readability over speed, unless choosing speed and adding additional documentation
    suffices. As an aside; if you see a single trigraph in my code, please report it. Trigraphs are by design a bug.

[2] CakesFW uses a patch format that has static offsets. They're loaded off FS and in theory are impervious
    to updates. In practice, updating patches is a pain and better done by offset patches like Luma3DS/Rei.

    Either way, Cakes allows doing some things the other firmware authors would probably call idiotic; I call them
    smart since I get a choice. Cakes is lacking a few things from Luma3DS mainly because (in my opinion) the patch
    format is incapable of accomodating some changes in a sane manner that doesn't require excessive RE or on console
    one off offset checks.

[3] Luma is a weird beast - "Noob-proof" as github states very perfectly describes it for better or worse. Want to load
    encrypted SD FIRM? Nope. Load SD firm with SysNAND? Nope. And many options aren't exposed to the user, period. Oh,
    I suppose there's no legitimate reason to disable firmprot anyways, either. Detection of Emunand #2 tends to be
    faulty. I have four.

[4] Reisyukaku has fallen behind a bit; his firmware is still in use and rather slim. He does, however, do some fancy
    ass magic to load a re-encrypted copy (?) of the nintendo firmware using a key refered to as the 'memekey'. How
    encrypting data differently makes it any less illegal to rehost I have no clue.

[5] Yes, Corbenik currently lacks quite a few things from this graph. Quite. A. Few. This will change with time.

[6] Okay, first - I could rant about how NTR is harming us long run for hours. I won't rant for hours, but I will
    explain some of the rationale here:

    NTR is a secondary CFW; it's designed to be used with a primary one. It consists of a proprietary blob, ntr.bin
    which loads into memory and takes over the debug service and subsequently the arm11 kernel.

    Normally, the functionality would be useful, but when you consider he's using techniques he hasn't bothered to doc
    on 3dbrew, you should by now know his goal is lock-in. From BootNTR it's apparent he's a shitty coder[7]. He
    won't document it, because someone else will create a better optimized version. Hm...that sounds a lot like a
    certain Japanese company who developed a cons- oh wait.

    The point is; NTR is not your friend. Stop using it.

[7] Tell me that giant switch statement couldn't be implemented as a dynamic patcher on the 3DS itself, considering
    all of the python offset extractor could be rewritten in C. Dare you. He also can't be bothered to port to 10.4
    NATIVE_FIRM even though there's literally no difference that should break NTR.
