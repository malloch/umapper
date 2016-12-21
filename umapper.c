//#include "mapper_internal.h"
#include <mapper/mapper.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <lo/lo.h>
#include <getopt.h> // For args

#include <unistd.h>
#include <signal.h>

mapper_database db = 0;

void create_db(void);

extern void print_help(void);
extern void print_version(void);
extern void list_devices(void);
extern void print_device(mapper_device dev, int details);
extern void print_map(mapper_map map, int details);
extern void print_signal(mapper_signal sig, int details);

static struct option long_options[] = {
    {"all",                    no_argument,         0, 'a'},
    {"help",                   no_argument,         0, 'h'},
    {"version",                no_argument,         0, 'v'},
    {"full",                   no_argument,         0, 'f'},

    {"device",                 no_argument,         0, 'd'},
    {"map",                    no_argument,         0, 'm'},
    {"signal",                 no_argument,         0, 's'},

    {"map_signals",            required_argument,   0, 'M'},
    {"unmap_signals",          required_argument,   0, 'U'},

    {0, 0, 0, 0}
};

enum t_options {
    UMAPPER_PRINT,
    UMAPPER_MAP,
    UMAPPER_UNMAP
} options;

int option= -1;
int full_detail_arg = 0;
int devices_arg = 0;
int signals_arg = 0;
int maps_arg = 0;
char src_name[256];
char dst_name[256];

int main(int argc, char * argv[])
{
    if (argc == 1){
        print_help();
        return 0;
    }

    create_db();
    if (!db)
        return 1;

    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv,
                            "hvfadlcsM:U:",
                            long_options, &option_index)) != -1) {
        switch (c) {
            case 0: printf("Missing argument\n"); break;
            case 'h':
                print_help();
                break;
            case 'v':
                print_version();
                break;
            case 'f':
                full_detail_arg = 1;
                break;
            case 'a':
                devices_arg = 1;
                signals_arg = 1;
                maps_arg = 1;
                option = UMAPPER_PRINT;
                break;
            case 'd':
                devices_arg = 1;
                option = UMAPPER_PRINT;
                break;
            case 'l':
                devices_arg = 1;
                option = UMAPPER_PRINT;
                break;
            case 'm':
                devices_arg = 1;
                signals_arg = 1;
                maps_arg = 1;
                option = UMAPPER_PRINT;
                break;
            case 's':
                devices_arg = 1;
                signals_arg = 1;
                option = UMAPPER_PRINT;
                break;
            case 'M':
                strcpy(src_name, optarg);
                if (optind < argc) {
                    strcpy(dst_name, argv[optind]);
                    option = UMAPPER_MAP;
                } else {
                    printf("Dest signal missing");
                }
                break;
            case 'U':
                strcpy(src_name, optarg);
                if (optind < argc) {
                    strcpy(dst_name, argv[optind]);
                    option = UMAPPER_UNMAP;
                } else {
                    printf("Dest signal missing");
                }
                break;
            case '?': printf("Missing argument\n"); break;
            default: print_help();
        }
        option_index = 0;
    }

    mapper_database_poll(db, 1000);

    switch(option) {
        case UMAPPER_PRINT: {
            list_devices();
        }
            break;
        case UMAPPER_MAP: {
            mapper_signal src = 0, dst = 0;
            char *slash = (char*)strchr(src_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mapper_device dev = mapper_database_device_by_name(db, src_name);
                if (dev)
                    src = mapper_device_signal_by_name(dev, slash+1);
            }
            if (!src) {
                printf("error(map): source signal '%s:%s' not found.\n",
                       src_name, slash+1);
                break;
            }
            slash = (char*)strchr(dst_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mapper_device dev = mapper_database_device_by_name(db, dst_name);
                if (dev)
                    dst = mapper_device_signal_by_name(dev, slash+1);
            }
            if (!dst) {
                printf("error(map): destination signal '%s:%s' not found.\n",
                       dst_name, slash+1);
                break;
            }
            mapper_map map = mapper_map_new(1, &src, 1, &dst);
            mapper_map_push(map);
        }
            break;
        case UMAPPER_UNMAP: {
            mapper_signal src = 0, dst = 0;
            char *slash = (char*)strchr(src_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mapper_device dev = mapper_database_device_by_name(db, src_name);
                if (dev)
                    src = mapper_device_signal_by_name(dev, slash+1);
            }
            if (!src) {
                printf("error(unmap): source signal '%s:%s' not found.\n",
                       src_name, slash+1);
                break;
            }
            slash = (char*)strchr(dst_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mapper_device dev = mapper_database_device_by_name(db, dst_name);
                if (dev)
                    dst = mapper_device_signal_by_name(dev, slash+1);
            }
            if (!dst) {
                printf("error(unmap): destination signal '%s:%s' not found.\n",
                       dst_name, slash+1);
                break;
            }
            mapper_map *out = mapper_signal_maps(src, MAPPER_DIR_OUTGOING);
            if (!out) {
                printf("error(unmap): source signal has no outgoing maps.\n");
                break;
            }
            mapper_map *in = mapper_signal_maps(dst, MAPPER_DIR_INCOMING);
            if (!in) {
                printf("error(unmap): destination signal has no incoming maps.\n");
                break;
            }
            mapper_map *both = mapper_map_query_intersection(out, in);
            if (!both) {
                printf("error(unmap): no maps found from '%s:%s' to '%s:%s'.\n",
                       src_name, mapper_signal_name(src), dst_name, slash+1);
            }
            while (both) {
                mapper_map_release(*both);
                both = mapper_map_query_next(both);
            }
        }
            break;
    }

    if (db)
        mapper_database_free(db);
    return 0;
}

void create_db(void) {
    db = mapper_database_new(0, MAPPER_OBJ_ALL);
    if (!db)
        exit(0);
    mapper_database_request_devices(db);
    mapper_database_poll(db, 100);
}

void list_devices(void) {
    mapper_device *devs = mapper_database_devices(db);
    if (devs)
        printf("Devices:\n");
    while (devs) {
        mapper_device dev = *devs;
        print_device(dev, full_detail_arg);

        if (signals_arg) {
            mapper_signal *sigs = mapper_device_signals(dev, MAPPER_DIR_INCOMING);
            if (sigs)
                printf("        input signals:\n");
            while (sigs) {
                print_signal(*sigs, full_detail_arg);
                sigs = mapper_signal_query_next(sigs);
            }
            sigs = mapper_device_signals(dev, MAPPER_DIR_OUTGOING);
            if (sigs)
                printf("        output signals:\n");
            while (sigs){
                print_signal(*sigs, full_detail_arg);
                if (maps_arg){
                    mapper_map *maps = mapper_signal_maps(*sigs, MAPPER_DIR_OUTGOING);
                    if (maps)
                        printf("                Maps:\n");
                    while (maps){
                        print_map(*maps, full_detail_arg);
                        maps = mapper_map_query_next(maps);
                    }
                }
                sigs = mapper_signal_query_next(sigs);
            }
        }
        printf("\n");
        devs = mapper_device_query_next(devs);
    }
}

