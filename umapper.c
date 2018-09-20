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

mapper_graph graph = 0;

void create_graph(void);

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

static mapper_device find_dev_by_name(mapper_graph g, const char *name)
{
    if (!g)
        return 0;
    mapper_object *list = mapper_graph_get_objects(g, MAPPER_DEVICE);
    list = mapper_object_list_filter(list, MAPPER_PROP_NAME, NULL, 1,
                                     MAPPER_STRING, name, MAPPER_OP_EQUAL);
    mapper_device *dev = list ? *list : NULL;
    mapper_object_list_free(list);
    return dev;
}

static mapper_signal find_sig_by_name(mapper_device d, const char *name)
{
    if (!d)
        return 0;
    mapper_object *list = mapper_device_get_signals(d, MAPPER_DIR_ANY);
    list = mapper_object_list_filter(list, MAPPER_PROP_NAME, NULL, 1,
                                     MAPPER_STRING, name, MAPPER_OP_EQUAL);
    mapper_signal *sig = list ? *list : NULL;
    mapper_object_list_free(list);
    return sig;
}

int main(int argc, char * argv[])
{
    if (argc == 1){
        print_help();
        return 0;
    }

    create_graph();
    if (!graph)
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

    mapper_graph_poll(graph, 1000);

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
                mapper_device dev = find_dev_by_name(graph, src_name);
                if (dev)
                    src = find_sig_by_name(dev, slash+1);
            }
            if (!src) {
                printf("error(map): source signal '%s:%s' not found.\n",
                       src_name, slash+1);
                break;
            }
            slash = (char*)strchr(dst_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mapper_device dev = find_dev_by_name(graph, dst_name);
                if (dev)
                    dst = find_sig_by_name(dev, slash+1);
            }
            if (!dst) {
                printf("error(map): destination signal '%s:%s' not found.\n",
                       dst_name, slash+1);
                break;
            }
            mapper_map map = mapper_map_new(1, &src, 1, &dst);

            // check for properties
            while (++optind < argc) {
                printf("setting prop '%s' to '%s'\n",
                       argv[optind]+1, argv[optind+1]);
                char *propname = argv[optind];
                if (propname && propname[0] == '@')
                    mapper_object_set_prop(map, MAPPER_PROP_UNKNOWN, propname+1,
                                           1, 's', argv[optind+1], 1);
                ++optind;
            }

            mapper_object_print(map, 0);
            mapper_object_push(map);

            if (!mapper_map_ready(map)) {
                // wait another second
                mapper_graph_poll(graph, 5000);
            }
            if (mapper_map_ready(map)) {
                printf("mapped: "),
                print_map(map, full_detail_arg);
            }
        }
            break;
        case UMAPPER_UNMAP: {
            mapper_signal src = 0, dst = 0;
            char *slash = (char*)strchr(src_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mapper_device dev = find_dev_by_name(graph, src_name);
                if (dev)
                    src = find_sig_by_name(dev, slash+1);
            }
            if (!src) {
                printf("error(unmap): source signal '%s:%s' not found.\n",
                       src_name, slash+1);
                break;
            }
            slash = (char*)strchr(dst_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mapper_device dev = find_dev_by_name(graph, dst_name);
                if (dev)
                    dst = find_sig_by_name(dev, slash+1);
            }
            if (!dst) {
                printf("error(unmap): destination signal '%s:%s' not found.\n",
                       dst_name, slash+1);
                break;
            }
            mapper_map *out = mapper_signal_get_maps(src, MAPPER_DIR_OUT);
            if (!out) {
                printf("error(unmap): source signal has no outgoing maps.\n");
                break;
            }
            mapper_map *in = mapper_signal_get_maps(dst, MAPPER_DIR_IN);
            if (!in) {
                printf("error(unmap): destination signal has no incoming maps.\n");
                break;
            }
            mapper_map *both = mapper_object_list_intersection(out, in);
            if (!both) {
                printf("error(unmap): no maps found from '%s:%s' to '%s:%s'.\n",
                       src_name,
                       mapper_object_get_prop_str(src, MAPPER_PROP_NAME, NULL),
                       dst_name, slash+1);
            }
            while (both) {
                mapper_map_release(*both);
                both = mapper_object_list_next(both);
            }
        }
            break;
    }

    if (graph)
        mapper_graph_free(graph);
    return 0;
}

void create_graph(void) {
    graph = mapper_graph_new(MAPPER_OBJECT);
    if (!graph)
        exit(0);
    mapper_graph_request_devices(graph);
    mapper_graph_poll(graph, 100);
}

void list_devices(void) {
    mapper_device *devs = mapper_graph_get_objects(graph, MAPPER_DEVICE);
    if (devs)
        printf("Devices:\n");
    while (devs) {
        mapper_device dev = *devs;
        print_device(dev, full_detail_arg);

        if (signals_arg) {
            mapper_signal *sigs = mapper_device_get_signals(dev, MAPPER_DIR_IN);
            if (sigs)
                printf("        input signals:\n");
            while (sigs) {
                print_signal(*sigs, full_detail_arg);
                sigs = mapper_object_list_next(sigs);
            }
            sigs = mapper_device_get_signals(dev, MAPPER_DIR_OUT);
            if (sigs)
                printf("        output signals:\n");
            while (sigs){
                print_signal(*sigs, full_detail_arg);
                if (maps_arg){
                    mapper_map *maps = mapper_signal_get_maps(*sigs, MAPPER_DIR_OUT);
                    if (maps)
                        printf("                Maps:\n");
                    while (maps){
                        print_map(*maps, full_detail_arg);
                        maps = mapper_object_list_next(maps);
                    }
                }
                sigs = mapper_object_list_next(sigs);
            }
        }
        devs = mapper_object_list_next(devs);
    }
}

