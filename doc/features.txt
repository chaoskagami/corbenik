Feature graph
===================

+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|CFW     |Firms?           |Patch Method     |Supplies Officially           |3rd Party  |Optimization[1]|Focus?             |Notes  |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|CakesFW |Dec/Enc (SD)     |Loadable Fixed   |S EN T G Y P R                |L B U I F  |Speed/Read     |Devs/Advanced Users| [2]   |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|Luma3DS |Enc(NAND) Dec(SD)|Builtin Dynamic  |S E2 Y L B P U X R            |N/A        |Speed          |"Noob-proof"       | [3]   |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|ReiNAND |Meme(SD)         |Builtin Dynamic  |S E1 L P F Y R                |N/A        |Readability    |Minimalist         | [4]   |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|Corbenik|Dec/Enc(SD)      |Loadable Bytecode|S P Y L B F U X               |           |Read/Speed     |Advanced Devs      | [5]   |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+
|NTR     |Hook             |ARM11 Binaries   |X F                           |X          |Closed Src?    |Being Dead         | [6]   |
+--------+-----------------+-----------------+------------------------------+-----------+---------------+-------------------+-------+

 * R: Reboot patch
 * S: Basic Signature Patch
 * T: TWL Signature Patch
 * G: AGB Signature Patch
 * P: FIRM protection
 * E: Emunand.
   * N - Can have many (unlimited)
   * X - Can have X emunands.
 * Y: Can boot sysnand from a9lh.
 * L: Loader replacement
 * B: Load svcBackdoor on 11.0
 * U: UNITINFO
 * I: GBA BIOS screen
 * F: Region free
 * X: Extras that are not general features listed above

Misc features by CFW:
 * Cakes
   * mid-kid/lgy_cakes
     * TWL patches
     * AGB patches
   * 3ds_injector
     * You have multiple choices. Wolfvak's, mid-kid's, mine, etc...
   * Wolfvak/icing
     * UNITINFO
     * svcBackdoor (o3DS only)
 * Luma
   * Loader replacement. CPU speed/language emulation
   * ARM11 XN Clear (dev)
   * Exception vector hook (dev)
 * Corbenik
   * Arbitrary service injection to empty slots.
   * Loader replacement. Uses patch files.
     * Baked in: langemu (based on Luma)
     * Baked in: CPU speed change
   * ARM11 XN Clear
   * Alternate menu force (courtesy @Reisyukaku)

[1] I'm not just referring to speed; I'm referring to purpose-based optimization. For example;
    do you want something well documented? Would you choose a slower algorithm that can be more
    easily debugged at the expense of speed? Would you use preprocessor macros to use different implementations?

    CakesFW is Speed/Read. It's hit and miss on documentation, but of the bunch, cakes is the best documented.

    Luma uses very fast code to do its thing. The source, however, lacks documentation, and some things done
    are not self-evident without analysis. The amount of pointer math rather than structs is astounding.

    ReiNAND is cruft-free and well documented. It doesn't do a lot, but what it does do, it does well.
    Unix philosphy in a nutshell. The only complaint is memekey.

    My focus will always be readability over speed, unless choosing speed and adding additional documentation
    suffices. As an aside; if you see a single trigraph in my code, please report it. Trigraphs are by design a bug.

[2] CakesFW uses a patch format that has static offsets. They're loaded off SD and _in theory_ are impervious
    to updates. Memory patches in cakes are kind of a kludge, though.

    In practice, updating patches is a pain and better done by offset patches like Luma3DS/Rei.

    Either way, Cakes allows doing some things the other firmware authors would probably call idiotic; I call them
    smart since I get a choice. Cakes is lacking a few things from Luma3DS mainly because (in my opinion) the patch
    format is problematic and not really future proof or extensible.

[3] "Noob-proof" as github states very perfectly describes it for better or worse. It can do
    a lot, has a very good focus on what it wants to do and probably works good enough for 90% of users. It handles
    the vast majority of use cases well and sanely. You will never brick with Luma, but by the same
    token, a number of options aren't exposed to prevent users shooting themselves in the foot.

    As I've been told by @TuxSH - "Noob-proof" != Just for noobs. I agree. I do find that "Noob-proof" as a goal
    tends to somewhat limit the people who truly understand what they're doing. This isn't a bad thing.
    Simply a design choice.

    My only real complaint about Luma is the sheer amount of pointer math instead of reading headers correctly,
    and the more-than-slightly evangelical mindset of some users.

[4] Reisyukaku has fallen behind a bit; his firmware is still in use and rather slim. He does, however, do some fancy
    ass magic to load a re-encrypted copy (?) of the nintendo firmware using a key refered to as the 'memekey'. How
    encrypting data differently makes it any less illegal to rehost I have no clue.

[5] Yes, Corbenik currently lacks quite a few things from this graph. Quite. A. Few. This will change with time.

[6] Okay, first - I could rant about how NTR was harming us long run for hours. I won't rant for hours, but I will
    explain some of the rationale here:

    NTR is a secondary CFW; it's designed to be used with a primary one. It consists of a proprietary blob, ntr.bin
    which loads into memory with some offset patches done by BootNTR - then takes over the debug service(?) and
    subsequently the arm11 kernel and spawns a thread in HOME to keep itself resident[7].

    Normally, the functionality would be useful, but when you consider he's using techniques he hasn't bothered to doc
    on 3dbrew, you should by now know his goal is lock-in. From BootNTR and layeredFS it's also apparent he's a
    shitty coder[8].

    He also quit development because people kept pestering him for source. I understand his rationale, but do you
    really want to use software by someone who will throw up his arms and say 'fuck you' at a moment's notice
    due to a few bad eggs?

    The point is; NTR is not your friend. It's unmaintained, and nobody but him can continue it due to
    the source code issue. Stop using it.

[7] At least from what I can tell. See https://github.com/patois/NTRDisasm. This is an out of date version, as well.
    I have no idea how accurate I am here. I'd have to run NTR through a debugger.

[8] Tell me that giant switch statement in BootNTR couldn't be implemented as a dynamic patcher on the 3DS itself,
    considering all of the python offset extractor code could be rewritten in C. Dare you. He also can't be bothered
    to port to 10.4 NATIVE_FIRM even though there's literally no difference that should break NTR.

    And don't even get me started on layeredFS - again, python code that could be a dynamic patcher on console.
