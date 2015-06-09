#include "include.h"

#define ROOT "D:\\nix"

char *make_guid() {
	char *buf = (char *)malloc(48);
	sprintf_s(buf, 48, "%08X-%04X-%04X-%04X-%08X%04X",
		stb_rand(),
		stb_rand() & 0xffff,
		0x4000 | (stb_rand() & 0x0fff),
		0x8000 | (stb_rand() & 0x3fff),
		stb_rand(),
		stb_rand() & 0xffff);
	return buf;
}

void generate(int argc, char **argv) {
	string_hash opts(true);
	opts.set("compiler", "v120");
	opts.set("root", ROOT);
	opts.set("solution_name", argv[0]);
	opts.set("project_name", argv[0]);
	argc--;
	argv++;
	{
#define with_opt(opt)                                                          \
	if ((opt_len = (int)strlen(opt), 1) && !strncmp(arg, opt, opt_len) &&      \
		(arg += opt_len, 1))

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
	char buf[256];
	{
		stb_srand((unsigned long)time(0));
		guids[0] = make_guid();
		guids[1] = make_guid();
		opts.set("solution_guid", guids[0]);
		opts.set("project_guid", guids[1]);
	}
	{
		int num_lines = 0;
		auto lines = stb_stringfile(ROOT "/templates/template.sln", &num_lines);
		sprintf_s(buf, 256, "%s.sln", opts.get("solution_name"));
		auto out = fopen(buf, "w");

		for (int i = 0; i < num_lines; i++) {
			auto line = lines[i];

			opts.foreach ([&](char *key, void *value) {
				sprintf_s(buf, 256, "[[%s]]", key);
				auto new_line = stb_dupreplace(line, buf, (char *)value);
				if (line != lines[i])
					free(line);
				line = new_line;
				return true;
			});

			fputs(line, out);
			fputs("\n", out);
		}

		fclose(out);
		free(lines);
	}
	{
		int num_lines = 0;
		auto lines =
			stb_stringfile(ROOT "/templates/template.vcxproj", &num_lines);
		sprintf_s(buf, 256, "%s.vcxproj", opts.get("project_name"));
		auto out = fopen(buf, "w");

		for (int i = 0; i < num_lines; i++) {
			auto line = lines[i];

			opts.foreach ([&](char *key, void *value) {
				sprintf_s(buf, 256, "[[%s]]", key);
				auto new_line = stb_dupreplace(line, buf, (char *)value);
				if (line != lines[i])
					free(line);
				line = new_line;
				return true;
			});

			fputs(line, out);
			fputs("\n", out);
		}

		fclose(out);
		free(lines);
	}
	stb_copyfile(ROOT "/templates/.editorconfig", ".editorconfig");
	stb_copyfile(ROOT "/templates/_gitignore", ".gitignore");
	free(guids[0]);
	free(guids[1]);

	auto project_name = (char *)opts.get("project_name");
	{
		auto out = fopen("build.cpp", "w");
		sprintf_s(buf, 256, "#include \"include.h\"\n\n#include \"%s.cpp\"\n",
				  project_name);
		fputs(buf, out);
		fclose(out);

		out = fopen("include.h", "w");
		fputs("#pragma once\n\n#include <nix.h>\n", out);
		fclose(out);

		sprintf_s(buf, 256, "%s.cpp", project_name);
		out = fopen(buf, "w");
		fputs("#include \"include.h\"\n", out);
		fclose(out);
	}
}

void add_file(int argc, char **argv) {
	char proj[256];
	sprintf_s(proj, 256, "%s.vcxproj", argv[0]);
	auto name = argv[1];
	auto name_len = strlen(name);
	// .h or .hpp:
	auto is_header = (name[name_len - 2] == '.' && name[name_len - 1] == 'h') ||
					 (name[name_len - 4] == '.' && name[name_len - 3] == 'h');
	// .c or .cpp:
	auto is_source = (name[name_len - 2] == '.' && name[name_len - 1] == 'c') ||
					 (name[name_len - 4] == '.' && name[name_len - 3] == 'c');

	if (!is_header && !is_source) {
		printf("error: invalid file extension\n");
		return;
	}

	size_t length = 0;
	auto proj_buf = (char *)stb_file(proj, &length);
	if (!proj_buf || !length) {
		printf("error: couldn't open project file\n");
		return;
	}

	rapidxml::xml_document<char> doc{};
	doc.parse<0>(proj_buf);
	auto proj_root = doc.first_node();
	auto group_node = proj_root->first_node("ItemGroup");
	while (group_node) {
		if (is_header) {
			auto include_node = group_node->last_node("ClInclude");
			if (include_node) {
				break;
			}
		} else {
			auto compile_node = group_node->last_node("ClCompile");
			if (compile_node) {
				break;
			}
		}

		group_node = group_node->next_sibling("ItemGroup");
	}
	if (group_node) {
		if (is_header) {
			auto header_node = doc.allocate_node(
				rapidxml::node_type::node_element, "ClInclude");
			auto header_attr = doc.allocate_attribute("Include", name);
			header_node->append_attribute(header_attr);
			group_node->append_node(header_node);
		} else {
			auto source_node = doc.allocate_node(
				rapidxml::node_type::node_element, "ClCompile");
			auto source_attr = doc.allocate_attribute("Include", name);
			auto source_exclude = doc.allocate_node(
				rapidxml::node_type::node_element, "ExcludedFromBuild", "true");
			source_node->append_attribute(source_attr);
			source_node->append_node(source_exclude);
			group_node->append_node(source_node);
		}
	}

	char *xml = (char *)malloc(length * 2);
	auto xml_end = rapidxml::print<char *, char>(xml, doc);
	*xml_end = 0;

	auto out = fopen(proj, "w");
	fputs(xml, out);
	fclose(out);

	out = fopen(name, "w");
	fputs("#include \"include.h\"\n", out);
	fclose(out);
	free(proj_buf);
	free(xml);
}

int main(int argc, char **argv) {
	argc--;
	argv++;

    if (argc >= 2 && !strcmp(argv[0], "generate")) {
		generate(argc - 1, argv + 1);
		return 0;
	}

	if (argc >= 3 && !strcmp(argv[0], "add")) {
		add_file(argc - 1, argv + 1);
		return 0;
	}

	puts("Usage: vstemplate generate {name} [opts...]\n"
		 "              ... add {project} {file}\n");
	return 0;
}
