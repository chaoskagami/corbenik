#include "common.h"

FILE *conf_handle;

struct config_file config;
extern uint8_t *enable_list;
void list_patches_build(char *name, int desc_is_fname);

void
regenerate_config()
{
    memset(&config, 0, sizeof(config));
    memcpy(&(config.magic), CONFIG_MAGIC, 4);
    config.config_ver = config_version;

    if (!(conf_handle = fopen(PATH_CONFIG, "w")))
        abort("Failed to open config for write?\n");

    fwrite(&config, 1, sizeof(config), conf_handle);
    fclose(conf_handle);

    fprintf(BOTTOM_SCREEN, "Config file written.\n");
}

void
mk_structure()
{
    f_mkdir(PATH_CFW);
    f_mkdir(PATH_CONFIG_DIR);
    f_mkdir(PATH_PATCHES);
    f_mkdir(PATH_FIRMWARES);
    f_mkdir(PATH_MODULES);
    f_mkdir(PATH_KEYS);
    f_mkdir(PATH_EXEFS);
    f_mkdir(PATH_EXEFS_TEXT);
    f_mkdir(PATH_EXEFS_DATA);
    f_mkdir(PATH_EXEFS_RO);
    f_mkdir(PATH_TEMP);
    f_mkdir(PATH_LOADER_CACHE);
}

void
load_config()
{
    mk_structure(); // Make directory structure if needed.

    // Zero on success.
    if (!(conf_handle = fopen(PATH_CONFIG, "r"))) {
        fprintf(BOTTOM_SCREEN, "Config file is missing:\n"
                               "  %s\n"
                               "Will create it with defaults.\n",
                PATH_CONFIG);
        regenerate_config();
    } else {
        fread(&config, 1, sizeof(config), conf_handle);
        fclose(conf_handle);

        if (memcmp(&(config.magic), CONFIG_MAGIC, 4)) {
            fprintf(BOTTOM_SCREEN, "Config file at:\n"
                                   "  %s\n"
                                   "has incorrect magic:\n"
                                   "  '%c%c%c%c'\n"
                                   "Regenerating with defaults.\n",
                    PATH_CONFIG, config.magic[0], config.magic[1], config.magic[2], config.magic[3]);
            f_unlink(PATH_CONFIG);
            regenerate_config();
        }

        if (config.config_ver < config_version) {
            fprintf(BOTTOM_SCREEN, "Config file has outdated version:\n"
                                   "  %s\n"
                                   "Regenerating with defaults...\n",
                    PATH_CONFIG);
            f_unlink(PATH_CONFIG);
            regenerate_config();
        }
    }

    list_patches_build(PATH_PATCHES, 0);

    if (!config.options[OPTION_SILENCE])
        fprintf(BOTTOM_SCREEN, "Config file loaded.\n");
}

void
save_config()
{
    fprintf(stderr, "Saving config...\n");

    write_file(enable_list, PATH_TEMP "/PATCHENABLE", FCRAM_SPACING / 2);

    f_unlink(PATH_CONFIG);

    if (!(conf_handle = fopen(PATH_CONFIG, "w")))
        abort("Failed to open config for write?\n");

	config.options[OPTION_RECONFIGURED] = 0; // This should not persist to disk.

    fwrite(&config, 1, sizeof(config), conf_handle);
    fclose(conf_handle);

	config.options[OPTION_RECONFIGURED] = 1; // Save caches on boot.
}
