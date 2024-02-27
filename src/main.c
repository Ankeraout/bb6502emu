#include <stdio.h>
#include <pthread.h>

#include "machine/bb6502.h"

static int init(struct ts_bb6502 *p_machine);
static int initSerialThreads(struct ts_bb6502 *p_machine);
static void *threadConsoleToSerial(void *p_arg);
static void *threadSerialToConsole(void *p_arg);

int main(void) {
    struct ts_bb6502 l_machine;

    if(init(&l_machine) != 0) {
        printf("Failed to initialize machine.\n");
        return 1;
    }

    if(initSerialThreads(&l_machine) != 0) {
        printf("Failed to initialize serial console threads.\n");
        return 1;
    }

    while(true) {
        l_machine.m_machine.m_step(&l_machine.m_machine);
    }

    return 0;
}

static int init(struct ts_bb6502 *p_machine) {
    uint8_t l_romData[32768];

    // Load ROM file
    FILE *l_file = fopen("samples/fibonacci/bin/fibonacci.bin", "rb");

    if(l_file == NULL) {
        printf("Failed to open file.\n");
        return 1;
    }

    if(fread(l_romData, 1, 32768, l_file) != 32768) {
        printf("Failed to read file.\n");
        return 1;
    }

    fclose(l_file);

    bb6502_init(p_machine, l_romData, 32768);

    return 0;
}

static int initSerialThreads(struct ts_bb6502 *p_machine) {
    pthread_t l_threadConsoleToSerial;
    pthread_t l_threadSerialToConsole;

    int l_returnValue = pthread_create(
        &l_threadConsoleToSerial,
        NULL,
        threadConsoleToSerial,
        p_machine
    );

    if(l_returnValue != 0) {
        perror("Failed to create console to serial thread");
        return 1;
    }

    l_returnValue = pthread_create(
        &l_threadSerialToConsole,
        NULL,
        threadSerialToConsole,
        p_machine
    );

    if(l_returnValue != 0) {
        perror("Failed to create serial to console thread");
        return 1;
    }

    return 0;
}

static void *threadConsoleToSerial(void *p_arg) {
    struct ts_bb6502 *l_machine = (struct ts_bb6502 *)p_arg;

    while(true) {
        int l_input = getchar();

        if(l_input == EOF) {
            break;
        }

        l_machine->m_serial.m_serial.m_write(
            &l_machine->m_serial.m_serial,
            (const uint8_t *)&l_input,
            1
        );
    }

    return NULL;
}

static void *threadSerialToConsole(void *p_arg) {
    struct ts_bb6502 *l_machine = (struct ts_bb6502 *)p_arg;

    while(true) {
        uint8_t l_buffer;

        l_machine->m_serial.m_serial.m_read(
            &l_machine->m_serial.m_serial,
            &l_buffer,
            1
        );

        putchar(l_buffer);
    }

    return NULL;
}
