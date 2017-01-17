// This is a do-nothing implementation of the configuration backend.
// It's solely to show what the access API for config should be.

// Recreates the configuration from scratch.
void regenerate_config(void) {}

// Loads the configuration.
void load_config(void) {}

// Saves the configuration.
void save_config(void) {}

// Toggles an option.
void toggle_opt(void* val) {}

// Gets an option as a string suitable for printing.
char* get_opt(void* val) {
    return "?";
}

// Gets an option as a uint32_t.
uint32_t get_opt_u32(uint32_t val) {
    return 0;
}

// Sets an option as a uint32_t.
int set_opt_u32(uint32_t key, uint32_t val) {
    return 0;
}

// Sets an option as a string.
int set_opt_str(uint32_t key, const char* val) {
    return 0;
}
