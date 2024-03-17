#ifndef __INCLUDE_COMMANDLINE_H__
#define __INCLUDE_COMMANDLINE_H__

struct ts_commandLineOptions {
    const char *m_machineName;
    const char *m_romFileName;
    const char *m_serialMode;
};

int parseCommandLine(
    struct ts_commandLineOptions *p_commandLineOptions,
    int p_argc,
    const char *p_argv[]
);

#endif
