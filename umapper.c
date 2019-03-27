
#include <mpr/mpr.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <lo/lo.h>
#include <getopt.h> // For args

#include <unistd.h>
#include <signal.h>

mpr_graph graph = 0;

void create_graph(void);

extern void print_help(void);
extern void print_version(void);
extern void list_devices(void);
extern void print_device(mpr_dev dev, int details);
extern void print_map(mpr_map map, int details);
extern void print_signal(mpr_sig sig, int details);

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
    UMPR_PRINT,
    UMPR_MAP,
    UMPR_UNMAP
} options;

int option= -1;
int full_detail_arg = 0;
int devices_arg = 0;
int signals_arg = 0;
int maps_arg = 0;
char src_name[256];
char dst_name[256];

static mpr_dev find_dev_by_name(mpr_graph g, const char *name)
{
    if (!g)
        return 0;
    mpr_list list = mpr_graph_get_list(g, MPR_DEV);
    list = mpr_list_filter(list, MPR_PROP_NAME, NULL, 1, MPR_STR, name, MPR_OP_EQ);
    mpr_dev *dev = list ? *list : NULL;
    mpr_list_free(list);
    return dev;
}

static mpr_sig find_sig_by_name(mpr_dev d, const char *name)
{
    if (!d)
        return 0;
    mpr_list list = mpr_dev_get_sigs(d, MPR_DIR_ANY);
    list = mpr_list_filter(list, MPR_PROP_NAME, NULL, 1, MPR_STR, name, MPR_OP_EQ);
    mpr_sig *sig = list ? *list : NULL;
    mpr_list_free(list);
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
                option = UMPR_PRINT;
                break;
            case 'd':
                devices_arg = 1;
                option = UMPR_PRINT;
                break;
            case 'l':
                devices_arg = 1;
                option = UMPR_PRINT;
                break;
            case 'm':
                devices_arg = 1;
                signals_arg = 1;
                maps_arg = 1;
                option = UMPR_PRINT;
                break;
            case 's':
                devices_arg = 1;
                signals_arg = 1;
                option = UMPR_PRINT;
                break;
            case 'M':
                strcpy(src_name, optarg);
                if (optind < argc) {
                    strcpy(dst_name, argv[optind]);
                    option = UMPR_MAP;
                } else {
                    printf("Dest signal missing");
                }
                break;
            case 'U':
                strcpy(src_name, optarg);
                if (optind < argc) {
                    strcpy(dst_name, argv[optind]);
                    option = UMPR_UNMAP;
                } else {
                    printf("Dest signal missing");
                }
                break;
            case '?': printf("Missing argument\n"); break;
            default: print_help();
        }
        option_index = 0;
    }

    mpr_graph_poll(graph, 1000);

    switch(option) {
        case UMPR_PRINT: {
            list_devices();
        }
            break;
        case UMPR_MAP: {
            mpr_sig src = 0, dst = 0;
            char *slash = (char*)strchr(src_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mpr_dev dev = find_dev_by_name(graph, src_name);
                if (!dev) {
                    printf("error(map): source device '%s' not found.\n",
                           src_name);
                    break;
                }
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
                mpr_dev dev = find_dev_by_name(graph, dst_name);
                if (!dev) {
                    printf("error(map): destination device '%s' not found.\n",
                           src_name);
                    break;
                }
                dst = find_sig_by_name(dev, slash+1);
            }
            if (!dst) {
                printf("error(map): destination signal '%s:%s' not found.\n",
                       dst_name, slash+1);
                break;
            }
            mpr_map map = mpr_map_new(1, &src, 1, &dst);

            // check for properties
            while (++optind < argc) {
                printf("setting prop '%s' to '%s'\n",
                       argv[optind]+1, argv[optind+1]);
                char *propname = argv[optind];
                if (propname && propname[0] == '@')
                    mpr_obj_set_prop(map, MPR_PROP_UNKNOWN, propname+1, 1, 's',
                                     argv[optind+1], 1);
                ++optind;
            }

            mpr_obj_print(map, 0);
            mpr_obj_push(map);

            if (!mpr_map_ready(map)) {
                // wait another second
                mpr_graph_poll(graph, 5000);
            }
            if (mpr_map_ready(map)) {
                printf("mapped: "),
                print_map(map, full_detail_arg);
            }
        }
            break;
        case UMPR_UNMAP: {
            mpr_sig src = 0, dst = 0;
            char *slash = (char*)strchr(src_name+1, '/');
            if (slash) {
                slash[0] = '\0';
                mpr_dev dev = find_dev_by_name(graph, src_name);
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
                mpr_dev dev = find_dev_by_name(graph, dst_name);
                if (dev)
                    dst = find_sig_by_name(dev, slash+1);
            }
            if (!dst) {
                printf("error(unmap): destination signal '%s:%s' not found.\n",
                       dst_name, slash+1);
                break;
            }
            mpr_list out = mpr_sig_get_maps(src, MPR_DIR_OUT);
            if (!out) {
                printf("error(unmap): source signal has no outgoing maps.\n");
                break;
            }
            mpr_list in = mpr_sig_get_maps(dst, MPR_DIR_IN);
            if (!in) {
                printf("error(unmap): destination signal has no incoming maps.\n");
                break;
            }
            mpr_list both = mpr_list_isect(out, in);
            if (!both) {
                printf("error(unmap): no maps found from '%s:%s' to '%s:%s'.\n",
                       src_name, mpr_obj_get_prop_str(src, MPR_PROP_NAME, NULL),
                       dst_name, slash+1);
            }
            while (both) {
                mpr_map_release(*both);
                both = mpr_list_next(both);
            }
        }
            break;
    }

    if (graph)
        mpr_graph_free(graph);
    return 0;
}

void create_graph(void) {
    graph = mpr_graph_new(MPR_OBJ);
    if (!graph)
        exit(0);
//    mpr_graph_request_devices(graph);
    mpr_graph_poll(graph, 1000);
}

void list_devices(void) {
    mpr_list devs = mpr_graph_get_list(graph, MPR_DEV);
    if (devs)
        printf("Devices:\n");
    while (devs) {
        mpr_dev dev = *devs;
        print_device(dev, full_detail_arg);

        if (signals_arg) {
            mpr_list sigs = mpr_dev_get_sigs(dev, MPR_DIR_IN);
            if (sigs)
                printf("        input signals:\n");
            while (sigs) {
                print_signal(*sigs, full_detail_arg);
                sigs = mpr_list_next(sigs);
            }
            sigs = mpr_dev_get_sigs(dev, MPR_DIR_OUT);
            if (sigs)
                printf("        output signals:\n");
            while (sigs){
                print_signal(*sigs, full_detail_arg);
                if (maps_arg){
                    mpr_list maps = mpr_sig_get_maps(*sigs, MPR_DIR_OUT);
                    if (maps)
                        printf("                Maps:\n");
                    while (maps){
                        print_map(*maps, full_detail_arg);
                        maps = mpr_list_next(maps);
                    }
                }
                sigs = mpr_list_next(sigs);
            }
        }
        devs = mpr_list_next(devs);
    }
}

