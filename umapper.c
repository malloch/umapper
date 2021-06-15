
#include <mapper/mapper.h>
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

int option = -1;
int full_detail_arg = 0;
int devices_arg = 0;
int signals_arg = 0;
int maps_arg = 0;
int num_sigs = 0;
char *sig_names[10];

static mpr_dev find_dev_by_name(mpr_graph g, const char *name)
{
    if (!g)
        return 0;
    mpr_list list = mpr_graph_get_objs(g, MPR_DEV);
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
    int i;
    if (argc == 1){
        print_help();
        return 0;
    }

    create_graph();
    if (!graph)
        return 1;

    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "hvfadlmsM:U:", long_options, &option_index)) != -1) {
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
            case 'U':
                num_sigs = 0;
                sig_names[num_sigs++] = strdup(optarg);
                while (optind < argc) {
                    if (argv[optind][0] == '-' || argv[optind][0] == '@') {
                        --optind;
                        break;
                    }
                    sig_names[num_sigs++] = strdup(argv[optind]);
                    ++optind;
                }
                if (num_sigs < 2) {
                    printf("Need 2 or more signal arguments for Map commands.");
                    break;
                }
                option = (c == 'M') ? UMPR_MAP : UMPR_UNMAP;
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
            mpr_sig srcs[10], dst = 0;
            char *slash;
            for (i = 0; i < num_sigs - 1; i++) {
                slash = (char*)strchr(sig_names[i] + 1, '/');
                if (slash) {
                    slash[0] = '\0';
                    mpr_dev dev = find_dev_by_name(graph, sig_names[i]);
                    if (!dev) {
                        printf("error(map): source device '%s' not found.\n", sig_names[i]);
                        goto done;
                    }
                    srcs[i] = find_sig_by_name(dev, slash + 1);
                }
                if (!srcs[i]) {
                    printf("error(map): source signal '%s:%s' not found.\n",
                           sig_names[i], slash + 1);
                    goto done;
                }
            }
            slash = (char*)strchr(sig_names[i] + 1, '/');
            if (slash) {
                slash[0] = '\0';
                mpr_dev dev = find_dev_by_name(graph, sig_names[i]);
                if (!dev) {
                    printf("error(map): destination device '%s' not found.\n", sig_names[i]);
                    goto done;
                }
                dst = find_sig_by_name(dev, slash + 1);
            }
            if (!dst) {
                printf("error(map): destination signal '%s:%s' not found.\n",
                       sig_names[i], slash + 1);
                goto done;
            }
            mpr_map map = mpr_map_new(num_sigs - 1, srcs, 1, &dst);
            // check for properties
            while (++optind < argc) {
                printf("setting prop '%s' to '%s'\n", argv[optind] + 1, argv[optind + 1]);
                char *propname = argv[optind];
                if (propname && propname[0] == '@')
                    mpr_obj_set_prop(map, MPR_PROP_UNKNOWN, propname + 1, 1, 's',
                                     argv[optind + 1], 1);
                ++optind;
            }

            mpr_obj_print(map, 0);
            mpr_obj_push(map);

            do {
                mpr_graph_poll(graph, 1000);
            }
            while (!mpr_map_get_is_ready(map));

            if (mpr_map_get_is_ready(map)) {
                printf("mapped: "),
                print_map(map, full_detail_arg);
            }
        }
            break;
        case UMPR_UNMAP: {
            mpr_sig srcs[10], dst = 0;
            char *slash;
            for (i = 0; i < num_sigs - 1; i++) {
                slash = (char*)strchr(sig_names[i] + 1, '/');
                if (slash) {
                    slash[0] = '\0';
                    mpr_dev dev = find_dev_by_name(graph, sig_names[i]);
                    if (!dev) {
                        printf("error(map): source device '%s' not found.\n", sig_names[i]);
                        goto done;
                    }
                    srcs[i] = find_sig_by_name(dev, slash + 1);
                }
                if (!srcs[i]) {
                    printf("error(unmap): source signal '%s:%s' not found.\n",
                           sig_names[i], slash + 1);
                    break;
                }
            }
            slash = (char*)strchr(sig_names[i] + 1, '/');
            if (slash) {
                slash[0] = '\0';
                mpr_dev dev = find_dev_by_name(graph, sig_names[i]);
                if (!dev) {
                    printf("error(map): destination device '%s' not found.\n", sig_names[i]);
                    goto done;
                }
                dst = find_sig_by_name(dev, slash + 1);
            }
            if (!dst) {
                printf("error(unmap): destination signal '%s:%s' not found.\n",
                       sig_names[i], slash + 1);
                goto done;
            }
            mpr_list out = mpr_sig_get_maps(srcs[0], MPR_DIR_OUT);
            for (i = 1; i < num_sigs - 1; i++) {
                out = mpr_list_get_isect(out, mpr_sig_get_maps(srcs[i], MPR_DIR_OUT));
                if (!out) {
                    printf("error(unmap): no maps found with specified source signals.\n");
                    break;
                }
            }
            mpr_list in = mpr_sig_get_maps(dst, MPR_DIR_IN);
            if (!in) {
                printf("error(unmap): destination signal has no incoming maps.\n");
                break;
            }
            mpr_list both = mpr_list_get_isect(out, in);
            if (!both) {
                printf("error(unmap): no maps found between specified signals.\n");
            }
            while (both) {
                mpr_map_release(*both);
                both = mpr_list_get_next(both);
            }
        }
            break;
    }
done:
    for (i = 0; i < num_sigs; i++)
        free(sig_names[i]);
    if (graph)
        mpr_graph_free(graph);
    return 0;
}

void create_graph(void) {
    graph = mpr_graph_new(MPR_OBJ);
    if (!graph)
        exit(0);
    mpr_graph_poll(graph, 1000);
}

void list_devices(void) {
    mpr_list devs = mpr_graph_get_objs(graph, MPR_DEV);
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
                sigs = mpr_list_get_next(sigs);
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
                        maps = mpr_list_get_next(maps);
                    }
                }
                sigs = mpr_list_get_next(sigs);
            }
        }
        devs = mpr_list_get_next(devs);
    }
}

