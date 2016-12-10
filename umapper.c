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

void on_device(mapper_database db, mapper_device dev, mapper_record_action a,
               const void *user)
{
    if (a == MAPPER_ADDED) {
        printf("new device! '%s'\n", mapper_device_name(dev));
        mapper_device_print(dev);
        if (mapper_device_property(dev, "foo", 0, 0, 0)) {
            printf("device has NO prop 'foo'\n");
            const char *bar = "bar";
            mapper_device_set_property(dev, "foo", 1, 's', bar);
        }
        else {
            printf("device has prop 'foo'\n");
            mapper_device_remove_property(dev, "foo");
        }
        mapper_device_push(dev);
    }
}

void on_signal(mapper_database db, mapper_signal sig, mapper_record_action a,
               const void *user)
{
    if (a == MAPPER_ADDED) {
        printf("new signal! '%s'\n", mapper_signal_name(sig));
//        const char *bar = "bar";
//        mapper_signal_set_property(sig, "sigfoo5", 1, 's', bar);
//        mapper_signal_remove_property(sig, "sigfoo4");
//        mapper_signal_push(sig);
    }
}

void on_link(mapper_database db, mapper_link lnk, mapper_record_action a,
             const void *user)
{
    if (a == MAPPER_ADDED) {
        printf("new link!\n");
//        mapper_link_print(lnk);
        const char *bar = "bar";
        mapper_link_set_property(lnk, "linkfoo5", 1, 's', bar);
        mapper_link_remove_property(lnk, "linkfoo4");
        mapper_link_push(lnk);
    }
}

void on_map(mapper_database db, mapper_map map, mapper_record_action a,
            const void *user)
{
    if (a == MAPPER_ADDED) {
        printf("new map!\n");
//        mapper_map_print(map);
//        const char *bar = "bar";
//        mapper_map_set_property(map, "mapfoo1", 1, 's', bar);
//        mapper_map_remove_property(map, "mapfoo2");
//        mapper_map_push(map);
    }
}

int main(int argc, char * argv[])
{
    if (argc == 1){
        print_help();
        return 0;
    }

    create_db();

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

    switch(option) {
        case UMAPPER_PRINT: {
            mapper_database_poll(db, 1000);
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
                    src = mapper_device_signal_by_name(db, slash+1);
            }
            if (!src)
                break;
            slash = (char*)strchr(dst_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mapper_device dev = mapper_database_device_by_name(db, dst_name);
                if (dev)
                    dst = mapper_device_signal_by_name(db, slash+1);
            }
            if (src && dst) {
                mapper_map map = mapper_map_new(1, &src, dst);
                mapper_map_push(map);
            }
            mapper_database_poll(db, 500);
            maps_arg = 1;
            list_devices();
        }
            break;
        case UMAPPER_UNMAP: {
            mapper_signal src = 0, dst = 0;
            char *slash = (char*)strchr(src_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mapper_device dev = mapper_database_device_by_name(db, src_name);
                if (dev)
                    src = mapper_device_signal_by_name(db, slash+1);
            }
            if (!src)
                break;
            slash = (char*)strchr(dst_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mapper_device dev = mapper_database_device_by_name(db, dst_name);
                if (dev)
                    dst = mapper_device_signal_by_name(db, slash+1);
            }
            if (src && dst) {
                mapper_map *out = mapper_signal_maps(src, MAPPER_DIR_OUTGOING);
                mapper_map *in = mapper_signal_maps(dst, MAPPER_DIR_INCOMING);
                mapper_map *both = mapper_map_query_intersection(out, in);
                while (both) {
                    mapper_map_release(*both);
                    both = mapper_map_query_next(both);
                }
            }
            mapper_database_poll(db, 500);
            maps_arg = 1;
            list_devices();
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
    mapper_database_add_device_callback(db, on_device, 0);
    mapper_database_add_signal_callback(db, on_signal, 0);
    mapper_database_add_link_callback(db, on_link, 0);
    mapper_database_add_map_callback(db, on_map, 0);
    mapper_database_request_devices(db);
    mapper_database_poll(db, 100);
}

void list_devices(void) {
    printf("list_devices()\n");
    mapper_device *devs = mapper_database_devices(db);
    if (devs)
        printf("Device:\n");
    while (devs) {
        mapper_device dev = *devs;
        print_device(dev, full_detail_arg);

        if (signals_arg) {
            mapper_signal *sigs = mapper_device_signals(dev, MAPPER_DIR_INCOMING);
            if (sigs)
                printf("\tInput Signals:\n");
            while (sigs) {
                print_signal(*sigs, full_detail_arg);
                sigs = mapper_signal_query_next(sigs);
            }
            sigs = mapper_device_signals(dev, MAPPER_DIR_OUTGOING);
            if (sigs)
                printf("\tOutput Signals:\n");
            while (sigs){
                print_signal(*sigs, full_detail_arg);
                if (maps_arg){
                    mapper_map *maps = mapper_signal_maps(*sigs, MAPPER_DIR_OUTGOING);
                    if (maps)
                        printf("\t\tMaps:\n");
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

