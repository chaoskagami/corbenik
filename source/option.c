#include <common.h>

FILE *conf_handle;

struct config_file config;

void
regenerate_config()
{
    memset(&config, 0, sizeof(config));
    memcpy(&(config.magic), CONFIG_MAGIC, 4);
    config.config_ver = config_version;
    config.options[OPTION_ACCENT_COLOR] = 2;
    config.options[OPTION_BRIGHTNESS]   = 3;

    if (!(conf_handle = fopen(PATH_CONFIG, "w")))
        abort("Failed to open config for write?\n");

    fwrite(&config, 1, sizeof(config), conf_handle);
    fclose(conf_handle);

    fprintf(BOTTOM_SCREEN, "Config file written.\n");
}

void
mk_structure()
{
    f_mkdir(ROOT);
    f_mkdir(DATA);
      f_mkdir(LIBEXECDIR);
      f_mkdir(LIBDIR);
      f_mkdir(PATH_CHAINS);
      f_mkdir(SYSCONFDIR);
}

void
update_config()
{
    int updated = 0;

    if (config.options[OPTION_ACCENT_COLOR] == 0) {
        fprintf(stderr, "Config update: accent color\n");
        config.options[OPTION_ACCENT_COLOR] = 2;
        updated = 1;
    }

    if (updated) {
        save_config(); // Save the configuration.
    }
}

void
load_config()
{
    mk_structure(); // Make directory structure if needed.

    // Zero on success.
    if (!(conf_handle = fopen(PATH_CONFIG, "r"))) {
        fprintf(BOTTOM_SCREEN, "Config file is missing:\n"
                               "  %s\n"
                               "Regenerating with defaults.\n",
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
                                   "Regenerating with defaults.\n",
                    PATH_CONFIG);
            f_unlink(PATH_CONFIG);
            regenerate_config();
        }
    }

    if (!config.options[OPTION_SILENCE])
        fprintf(BOTTOM_SCREEN, "Config file loaded.\n");

    update_config();
}

void
save_config()
{
    f_unlink(PATH_CONFIG);

    if (!(conf_handle = fopen(PATH_CONFIG, "w")))
        abort("Failed to open config for write?\n");

    config.options[OPTION_RECONFIGURED] = 0; // This should not persist to disk.

    fwrite(&config, 1, sizeof(config), conf_handle);
    fclose(conf_handle);

    config.options[OPTION_RECONFIGURED] = 1; // Save caches on boot.

    fprintf(stderr, "Saved config successfully.\n");
}
