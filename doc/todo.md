 * Attempt to create a replacement handler for Service 0x3D, AKA OutputDebugString(void const, int) to log to a file on SD.
   * Might be a pipe dream. It still would be cool to capture debug logs from games.
     * Pretty sure this goes over JTAG on PARTNER units and anything else >/dev/null.
 * Implement some kind of curses-like backend and replace terrible printf rewind on top screen.
   * Alternatively, implement a monochrome GUI.
   * We also need UTF8 support. I want translation support.
     * Dragging in freetype or a bitmap font tool.
       * Ugh, VWF. Not like I haven't done it before...
 * Config fragments for modules; and these need to be part of the modules, not corbenik's options menu.
   * Oppa Kconfig style.
     * Busybox may be helpful.
       * Probably need to reimplement anyways due to lack of userland.
 * Implement program loading as...something else. The current linker is broken. There's multiple ways to go about this:
   * Figure out why it breaks, and fix it up.
   * Embed a function table in corbenik itself, and rip this table out and generate a header post-compile which can be used by modules.
 * Allow modules to be internal AND external, and to build either way. Think of the whole kmod-versus-builtin deal.
   * All in all, this simplifies testing and allows multiple release types.
 * Rewrite all hardcoded constants that are machine code as assembly.
   * Read: all the patches
