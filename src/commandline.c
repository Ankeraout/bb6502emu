#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commandline.h"

int parseCommandLine(
    struct ts_commandLineOptions *p_commandLineOptions,
    int p_argc,
    const char *p_argv[]
) {
    bool l_flagError = false;
    bool l_flagHelp = false;
    bool l_flagMachine = false;
    bool l_flagRom = false;
    bool l_flagSerial = false;

    memset(p_commandLineOptions, 0, sizeof(*p_commandLineOptions));

    for(int l_i = 1; l_i < p_argc; l_i++) {
        if(l_flagMachine) {
            p_commandLineOptions->m_machineName = p_argv[l_i];
            l_flagMachine = false;
        } else if(l_flagRom) {
            p_commandLineOptions->m_romFileName = p_argv[l_i];
            l_flagRom = false;
        } else if(l_flagSerial) {
            p_commandLineOptions->m_serialMode = p_argv[l_i];
            l_flagSerial = false;
        } else if(
            strcmp(p_argv[l_i], "-h") == 0
            || strcmp(p_argv[l_i], "--help") == 0
        ) {
            l_flagHelp = true;
            break;
        } else if(
            strcmp(p_argv[l_i], "-m") == 0
            || strcmp(p_argv[l_i], "--machine") == 0
        ) {
            if(p_commandLineOptions->m_machineName != NULL) {
                printf("Error: too many machine names specified.\n");
                l_flagError = true;
                break;
            } else {
                l_flagMachine = true;
            }
        } else if(strcmp(p_argv[l_i], "--rom") == 0) {
            if(p_commandLineOptions->m_romFileName != NULL) {
                printf("Error: too many ROM files specified.\n");
                l_flagError = true;
                break;
            } else {
                l_flagRom = true;
            }
        } else if(strcmp(p_argv[l_i], "--serial") == 0) {
            if(p_commandLineOptions->m_serialMode != NULL) {
                printf("Error: too many serial modes specified.\n");
                l_flagError = true;
                break;
            } else {
                l_flagSerial = true;
            }
        } else {
            printf("Error: invalid parameter \"%s\".\n", p_argv[l_i]);
            l_flagError = true;
            break;
        }
    }

    if(l_flagError) {
        return 1;
    } else if(l_flagHelp) {
        printf("Usage: %s [options]\n", p_argv[0]);
        printf("Options:\n");
        printf("  --help: Displays this help page.\n");
        printf("  --machine <name>: Selects the machine.\n");
        printf("  --rom <file>: Selects the ROM file.\n");
        printf("  --serial <mode>: Selects the serial mode.\n");
        printf("\n");
        printf("Shortcuts:\n");
        printf("  -h: --help\n");
        printf("  -m: --machine\n");
        exit(0);
    } else if(l_flagMachine) {
        printf("Error: required machine name after \"--machine\".\n");
        return 1;
    } else if(l_flagRom) {
        printf("Error: required file name after \"--rom\".\n");
        return 1;
    } else if(p_commandLineOptions->m_machineName == NULL) {
        printf("Error: no selected machine.\n");
        return 1;
    }

    return 0;
}
