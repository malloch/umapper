
#include <mapper/mapper.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define die_unless(a, ...) { if (!(a)) { printf("-- " __VA_ARGS__); assert(a); } }

void print_version(){
    printf("Ugly Mapper v 0.01\n");
}

void print_help(){
    print_version();
    printf("An ugly front-end to libmapper mapping network.\n");
    printf("Usage:\n");
    printf("\t-h\t--help\t\t\t\t\tPrint this help\n");
    printf("\t-v\t--version\t\t\t\tPrint version\n");
    printf("\t-a\t--all\t\t\t\t\tList devices, signals, and maps\n");
    printf("\t-f\t--full\t\t\t\t\tList all details\n");
    printf("\n");
    printf("\t-d\t--device\t\t\t\tList only devices\n");
    printf("\t-s\t--signal\t\t\t\tList devices with signals\n");
    printf("\t-m\t--map\t\t\t\t\tList devices, signals and maps\n");
    printf("\n");
    printf("\t-M\t--map_signals source dest\t\tMap source signal to dest signal.\n");
    printf("\t-U\t--unmap_signals source dest\t\tUnmap source signal to dest signal.\n");
    printf("\n");
    printf("Example:\n");
    printf("\tumapper -l -f\n");
}

void print_device(mapper_device dev, int details) {
    printf("\t");
    if (details)
        mapper_device_print(dev);
    else
        printf("%s", mapper_device_name(dev));
    printf("\n");
}

void print_map(mapper_map map, int details) {
    printf("\t\t\t");
    if (details)
        mapper_map_print(map);
    else {
        mapper_device dev;
        mapper_signal sig;
        mapper_slot slot;
        int i, num_src = mapper_map_num_sources(map);
        if (num_src > 1)
            printf("[");
        for (i = 0; i < num_src; i++) {
            slot = mapper_map_slot(map, MAPPER_LOC_SOURCE, i);
            sig = mapper_slot_signal(slot);
            dev = mapper_signal_device(sig);
            printf("%s/%s, ", mapper_device_name(dev), mapper_signal_name(sig));
        }
        slot = mapper_map_slot(map, MAPPER_LOC_DESTINATION, 0);
        sig = mapper_slot_signal(slot);
        dev = mapper_signal_device(sig);
        printf("\b\b%s -> %s/%s", num_src > 1 ? "]" : "",
               mapper_device_name(dev), mapper_signal_name(sig));
    }
    printf("\n");
}

void print_signal(mapper_signal sig, int details) {
    printf("\t\t");
    if (details)
        mapper_signal_print(sig, 1);
    else
        printf("%s ", mapper_signal_name(sig));
    printf("\n");
}

