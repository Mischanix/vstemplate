#include "include.h"

char *make_guid() {
    char *buf = (char *)malloc(48);
    sprintf_s(buf, 48, "%08X-%04X-%04X-%04X-%08X%04X",
        stb_rand(),
        stb_rand() & 0xffff,
        stb_rand() & 0xffff,
        stb_rand() & 0xffff,
        stb_rand(),
        stb_rand() & 0xffff);
    return buf;
}

void generate(int argc, char **argv) {
    string_hash opts(true);
    opts.set("compiler", "v120");
    opts.set("root", "D:\\nix");
    opts.set("solution_name", argv[0]);
    opts.set("project_name", argv[0]);
    argc -= 1;
    argv += 1;
    {
#define with_opt(opt) if ((opt_len = (int) strlen(opt), 1) && !strncmp(arg, opt, opt_len) && (arg += opt_len, 1))
        for (int i = 0; i < argc; i++) {
            auto arg = argv[i];
            int opt_len;

            with_opt("-project=") {
                opts.set("project_name", arg);
            } else with_opt("-compiler=") {
                opts.set("compiler", arg);
            } else with_opt("-root=") {
                opts.set("root", arg);
            }

        }
#undef with_opt
    }
    char *guids[2];
    {
        stb_srand((unsigned long)time(0));
        guids[0] = make_guid();
        guids[1] = make_guid();
        opts.set("solution_guid", guids[0]);
        opts.set("project_guid", guids[1]);
    }
    {
        char buf[256];
        int num_lines;
        auto lines = stb_stringfile("templates/template.sln", &num_lines);
        sprintf_s(buf, 256, "%s.sln", opts.get("solution_name"));
        auto out = fopen(buf, "w");
        
        for (int i = 0; i < num_lines; i++) {
            auto line = lines[i];
            
            opts.foreach([&](char *key, void *value) {
                sprintf_s(buf, 256, "[[%s]]", key);
                auto new_line = stb_dupreplace(line, buf, (char *)value);
                if (line != lines[i]) free(line);
                line = new_line;
                return true;
            });

            fputs(line, out);
            fputs("\n", out);
        }

        free(lines);
    }
    {
        char buf[256];
        int num_lines;
        auto lines = stb_stringfile("templates/template.vcxproj", &num_lines);
        sprintf_s(buf, 256, "%s.vcxproj", opts.get("project_name"));
        auto out = fopen(buf, "w");

        for (int i = 0; i < num_lines; i++) {
            auto line = lines[i];

            opts.foreach([&](char *key, void *value) {
                sprintf_s(buf, 256, "[[%s]]", key);
                auto new_line = stb_dupreplace(line, buf, (char *)value);
                if (line != lines[i]) free(line);
                line = new_line;
                return true;
            });

            fputs(line, out);
            fputs("\n", out);
        }

        free(lines);
    }
    free(guids[0]);
    free(guids[1]);
}

int main(int argc, char **argv) {
    argc--; argv++;
    if (argc <= 1) {
        puts("Usage: vstemplate generate name [opts...]\n");
        return 0;
    }

    if (!strcmp(argv[0], "generate")) {
        generate(argc - 1, argv + 1);
    }

    return 0;
}
