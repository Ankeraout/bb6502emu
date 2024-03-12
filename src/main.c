#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commandline.h"
#include "machine/bb6502.h"

static struct ts_machine *init(int p_argc, const char *p_argv[]);

int main(int p_argc, const char *p_argv[]) {
    struct ts_machine *l_machine = init(p_argc, p_argv);

    if(l_machine == NULL) {
        printf("Failed to initialize machine.\n");
        return 1;
    }

    printf("Machine initialized successfully.\n");

    while(true) {
        l_machine->m_step(l_machine);
    }

    return 0;
}

static struct ts_machine *init(int p_argc, const char *p_argv[]) {
    struct ts_commandLineOptions l_commandLineOptions;

    if(parseCommandLine(&l_commandLineOptions, p_argc, p_argv) != 0) {
        return NULL;
    }

    if(strcmp(l_commandLineOptions.m_machineName, "bb6502") == 0) {
        return &bb6502_init(&l_commandLineOptions)->m_machine;
    } else {
        fprintf(
            stderr,
            "Error: unknown machine \"%s\".\n",
            l_commandLineOptions.m_machineName
        );
        return NULL;
    }
}
