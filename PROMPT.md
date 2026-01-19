if (options->libDir == NULL) {
        options->libDir = makeString(strings, getenv("CXY_STDLIB_DIR"));
    }
    cstring cxyRoot = getenv("CXY_ROOT");
    if (options->libDir == NULL && cxyRoot != NULL) {
        options->libDir = makeStringConcat(strings, cxyRoot, "/lib/cxy/std");
    }

    if (options->pluginsDir == NULL) {
        if (options->buildDir) {
            options->pluginsDir =
                makeStringConcat(strings, options->buildDir, "/plugins");
        }
        else {
            options->pluginsDir = makeString(strings, "./plugins");
        }
    }
