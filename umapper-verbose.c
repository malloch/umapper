
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

void print_device(mpr_dev dev, int details) {
    printf("    ");
    if (details)
        mpr_obj_print(dev, 0);
    else {
        printf("%s", mpr_obj_get_prop_as_str(dev, MPR_PROP_NAME, NULL));
        printf("\n");
    }
}

void print_map(mpr_map map, int details) {
    printf("                     ");
    if (details)
        mpr_obj_print(map, 0);
    else {
        mpr_dev dev;
        mpr_sig sig;

        // sources
        mpr_list list = mpr_map_get_sigs(map, MPR_LOC_SRC);
        int size = mpr_list_get_size(list);
        if (size > 1)
            printf("[");
        while (list) {
            sig = (mpr_sig)*list;
            dev = mpr_sig_get_dev(sig);
            printf("%s/%s, ",
                   mpr_obj_get_prop_as_str(dev, MPR_PROP_NAME, NULL),
                   mpr_obj_get_prop_as_str(sig, MPR_PROP_NAME, NULL));
            list = mpr_list_get_next(list);
        }
        printf("\b\b%s -> ", size > 1 ? "]" : "");

        // destinations
        list = mpr_map_get_sigs(map, MPR_LOC_DST);
        size = mpr_list_get_size(list);
        if (size > 1)
            printf("[");
        while (list) {
            sig = (mpr_sig)*list;
            dev = mpr_sig_get_dev(sig);
            printf("%s/%s, ",
                   mpr_obj_get_prop_as_str(dev, MPR_PROP_NAME, NULL),
                   mpr_obj_get_prop_as_str(sig, MPR_PROP_NAME, NULL));
            list = mpr_list_get_next(list);
        }
        printf("\b\b%s", size > 1 ? "]" : "");
    }
    printf("\n");
}

void print_signal(mpr_sig sig, int details) {
    printf("            ");
    if (details)
        mpr_obj_print(sig, 0);
    else
        printf("%s ", mpr_obj_get_prop_as_str(sig, MPR_PROP_NAME, NULL));
    printf("\n");
}

