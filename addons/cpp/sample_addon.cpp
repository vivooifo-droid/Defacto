extern "C" int defo_addon_init(void* api) {
    (void)api;
    return 0;
}

extern "C" const char* defo_addon_name() {
    return "sample.addon";
}
