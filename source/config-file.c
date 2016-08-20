#include <common.h>

FILE *conf_handle;

struct config_file *config;
extern uint8_t *enable_list;
char *config_file_path = NULL;
int changed_consoles = 0;
uint32_t cid[4];

void list_patches_build(char *name, int desc_is_fname);

void
regenerate_config()
{
    for(int i=0; i < 4; i++)
        config->magic[i] = CONFIG_MAGIC[i];

    config->config_ver = config_version;
    config->options[OPTION_ACCENT_COLOR] = 2;
    config->options[OPTION_BRIGHTNESS]   = 3;

    if (!(conf_handle = fopen(config_file_path, "w")))
        poweroff();

    fwrite(config, 1, sizeof(struct config_file) + FCRAM_SPACING / 2, conf_handle);
    fclose(conf_handle);
}

void
mk_structure()
{
    f_mkdir(PREFIX);
      f_mkdir(LIBEXECDIR);
      f_mkdir(LIBDIR);
        f_mkdir(PATH_EXEFS);
          f_mkdir(PATH_EXEFS_TEXT);
          f_mkdir(PATH_EXEFS_DATA);
          f_mkdir(PATH_EXEFS_RO);
        f_mkdir(PATH_FIRMWARES);
        f_mkdir(PATH_MODULES);
      f_mkdir(BINDIR);
#if defined(CHAINLOADER) && CHAINLOADER == 1
      f_mkdir(PATH_CHAINS);
#endif
      f_mkdir(SBINDIR);
      f_mkdir(SYSCONFDIR);
      f_mkdir(LOCALSTATEDIR);
        f_mkdir(PATH_TEMP);
          f_mkdir(PATH_LOADER_CACHE);
        f_mkdir(PATH_LOG);
      f_mkdir(SHAREDIR);
        f_mkdir(PATH_KEYS);
        f_mkdir(LOCALEDIR);
          f_mkdir(PATH_LOCEMU);
}

void
update_config()
{
    int updated = 0;

    if (config->options[OPTION_ACCENT_COLOR] == 0) {
        config->options[OPTION_ACCENT_COLOR] = 2;
        updated = 1;
    }

    if (updated) {
        save_config(); // Save the configuration.
    }
}

void get_cfg_path();

void
load_config()
{
    mk_structure(); // Make directory structure if needed.

    if (!config_file_path) {
        config_file_path = malloc(256); // MAX_PATH
        memset(config_file_path, 0, 256);

        sdmmc_get_cid(1, cid);

        FILE* f = fopen(SYSCONFDIR "/current-nand-cid", "r");
        if (!f) {
            // Nonexistent. Write it.
            f = fopen(SYSCONFDIR "/current-nand-cid", "w");
            fwrite(cid, 1, 4, f);
            fclose(f);
            f = fopen(SYSCONFDIR "/current-nand-cid", "r");
        }

        fread(&cid[1], 1, 4, f);

        // If our console's CID doesn't match what was read, we need to regenerate caches immediately when we can.
        if (cid[0] != cid[1]) {
            changed_consoles = 1;
        }

        strcpy(config_file_path, SYSCONFDIR "/config-");

        size_t len = strlen(config_file_path) + 7;
        uint32_t cid_cp = cid[0];
        while (cid_cp) {
            static const char hexDigits[] = "0123456789ABCDEF";
            config_file_path[len--] = hexDigits[(uint32_t)(cid_cp & 0xF)];
            cid_cp >>= 4;
        }

        config = (struct config_file*)malloc(sizeof(struct config_file) + FCRAM_SPACING / 2);
        memset(config, 0, sizeof(struct config_file) + FCRAM_SPACING / 2);
        enable_list = (uint8_t*)config + sizeof(struct config_file);
        fclose(f);
    }

    // Zero on success.
    if (!(conf_handle = fopen(config_file_path, "r"))) {
        regenerate_config();
    } else {
        fread(config, 1, sizeof(struct config_file) + FCRAM_SPACING / 2, conf_handle);

        fclose(conf_handle);

        if (memcmp(&(config->magic), CONFIG_MAGIC, 4)) {
            f_unlink(config_file_path);
            regenerate_config();
        }

        if (config->config_ver != config_version) {
            f_unlink(config_file_path);
            regenerate_config();
        }
    }

    list_patches_build(PATH_PATCHES, 0);

    update_config();
}

void
save_config()
{
    if (changed_consoles) {
        FILE* f = fopen(SYSCONFDIR "/current-nand-cid", "w");
        fwrite(cid, 1, 4, f);
        fclose(f);
    }

    f_unlink(config_file_path);

    if (!(conf_handle = fopen(config_file_path, "w")))
		while(1);

    fwrite(config, 1, sizeof(struct config_file) + FCRAM_SPACING / 2, conf_handle);

    fclose(conf_handle);
}
