#pragma once

#include <Windows.h>
#include <stdio.h>

#include <nix.h>

#define RAPIDXML_NO_EXCEPTIONS
#define RAPIDXML_NO_STREAMS
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
void rapidxml::parse_error_handler(const char *what, void *where) {
    printf("xml error: %s\n", what);
    ExitProcess(1);
}
