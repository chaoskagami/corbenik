#include "common.h"

FILE conf_handle;

void regenerate_config() {
	f_mkdir(PATH_CFW);
    f_mkdir(PATH_FIRMWARES);
    f_mkdir(PATH_PATCHES);
    f_mkdir(PATH_LOCEMU);
    f_mkdir(PATH_TEMP);
    f_mkdir(PATH_KEYS);
    f_mkdir(PATH_EXEFS);

    cprintf(BOTTOM_SCREEN, "Created directory structure.\n");

    memset(&config, 0, sizeof(config));
    memcpy(&(config.magic), CONFIG_MAGIC, 4);
    config.config_ver = config_version;

    fopen(&conf_handle, PATH_CONFIG, "w");
    fwrite(&config, 1, sizeof(config), &conf_handle);
    fclose(&conf_handle);

    cprintf(BOTTOM_SCREEN, "Config file written.\n");
}

void load_config() {
    // Zero on success.
    if (fopen(&conf_handle, PATH_CONFIG, "r")) {
        cprintf(BOTTOM_SCREEN, "Config file is missing:\n"
                               "  %s\n"
                               "Will create it with defaults.\n",
                               PATH_CONFIG);
        regenerate_config();
    } else {
        fread(&config, 1, sizeof(config), &conf_handle);
        fclose(&conf_handle);

        if (memcmp(&(config.magic), CONFIG_MAGIC, 4)) {
            cprintf(BOTTOM_SCREEN, "Config file at:\n"
                                   "  %s\n"
                                   "has incorrect magic:\n"
                                   "  '%c%c%c%c'\n"
                                   "Regenerating with defaults.\n",
                                   PATH_CONFIG,
                                   config.magic[0], config.magic[1], config.magic[2], config.magic[3]);
            f_unlink(PATH_CONFIG);
            regenerate_config();
        }

        if (config.config_ver < config_version) {
            cprintf(BOTTOM_SCREEN, "Config file has outdated version:\n"
                                   "  %s\n"
                                   "Regenerating with defaults.\n",
                                   PATH_CONFIG);
            f_unlink(PATH_CONFIG);
            regenerate_config();
        }
    }

    cprintf(BOTTOM_SCREEN, "Config file loaded.\n");
}

