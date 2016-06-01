Next
-------------

 * Make config file for corbenik plaintext. Nobody likes binary configs. They suck. Massively. Especially when you fuck up a setting and need to change it on something that isn't a 3ds.
 * In place firmware loading/patching.
 * Weird hack central - Developer 11.4 leaked. Allow injecting debug module? Ehehehehe. No clue what the point of this would be, but hey, it'd be FUCKING cool, man.

Shortterm
-------------

 * Implement some kind of GUI menu functionality.
   * We also probably need UTF8 support. I want translations.
     * Dragging in freetype or a bitmap font tool is likely needed.
       * Also, VWF. Not like I haven't done it before...but ugh. It's still a pain.
   * Kconfig-based menus?
     * The logic can't be easily ported from linux, and would need to be reimplemented.

 * Implement program loading as...something else. The current linker is broken. There's multiple ways to go about this:
   * Figure out why it breaks. Fix it up. (Deemed impossible without static linking, which defeats the point.)
   * Scripting language / VM maybe?
     * Lua is the obvious choice, but there's a few negatives to this.
     * It isn't terribly hard to write an assembler and bytecode VM. Maybe I'll do that.
     * Has the advantages of code plus ARM9/ARM11 independence.

Longterm
-------------
 * Optimize the buffer logic out of printf. Render directly to the FB and keep track of dirty areas instead.
 * Attempt to create a replacement handler for Service 0x3D, AKA OutputDebugString(void const, int) to log to a file on SD.
   * Might be a pipe dream. It still would be cool to capture debug logs from games.
     * Pretty sure this goes over JTAG on PARTNER units and anything else >/dev/null.
 * Maybe replace svc 0xFF with something fancy.
 * Rewrite all hardcoded constants that are machine code as assembly.
   * Read: all the patches.
 * Change some stdlib functions to more closely imitate their userland counterparts
 * EmuNAND. By this, I mean any non-physical NAND, not just gateway style.
   * I'd like to implement a loop-mount-like NAND. The NAND would be a file off the SD card rather than a region before partition 1 in this setup, and on the upside near unlimited NANDs could be accomodated without moving partitions. On the downside, any user stupid enough to move the NAND bin while the system is running would be in for severe consequences.
