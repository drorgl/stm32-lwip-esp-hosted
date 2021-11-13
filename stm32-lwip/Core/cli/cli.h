#pragma once



void cli_register(const char * command, const char * description, void (*callback)(int argc, const char * const * argv, void (*print)(const char * message)));
void cli_initialize(void);

