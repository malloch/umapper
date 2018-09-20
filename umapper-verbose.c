
#include <mapper/mapper.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define die_unless(a, ...) { if (!(a)) { printf("-- " __VA_ARGS__); assert(a); } }

void print_version(){
    printf("µMapper v 0.02\n");
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
    printf("    ");
    if (details)
        mapper_object_print(dev, 0);
    else {
        printf("%s", mapper_object_get_prop_str(dev, MAPPER_PROP_NAME, NULL));
        printf("\n");
    }
}

void print_map(mapper_map map, int details) {
    printf("                     ");
    if (details)
        mapper_object_print(map, 0);
    else {
        mapper_device dev;
        mapper_signal sig;
        int i, num_src = mapper_map_get_num_signals(map, MAPPER_LOC_SRC);
        if (num_src > 1)
            printf("[");
        for (i = 0; i < num_src; i++) {
            sig = mapper_map_get_signal(map, MAPPER_LOC_SRC, i);
            dev = mapper_signal_get_device(sig);
            printf("%s/%s, ",
                   mapper_object_get_prop_str(dev, MAPPER_PROP_NAME, NULL),
                   mapper_object_get_prop_str(sig, MAPPER_PROP_NAME, NULL));
        }
        sig = mapper_map_get_signal(map, MAPPER_LOC_DST, 0);
        dev = mapper_signal_get_device(sig);
        printf("\b\b%s -> %s/%s", num_src > 1 ? "]" : "",
               mapper_object_get_prop_str(dev, MAPPER_PROP_NAME, NULL),
               mapper_object_get_prop_str(sig, MAPPER_PROP_NAME, NULL));
    }
    printf("\n");
}

void print_signal(mapper_signal sig, int details) {
    printf("            ");
    if (details)
        mapper_object_print(sig, 0);
    else
        printf("%s ", mapper_object_get_prop_str(sig, MAPPER_PROP_NAME, NULL));
    printf("\n");
}

