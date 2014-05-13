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

mapper_monitor mon = 0;
mapper_db db = 0;

void create_monitor(void);

extern void print_help(void);
extern void print_version(void);
extern void list_devices(void);
extern void print_device(mapper_db_device dev, int details);
extern void print_link(mapper_db_link lnk, int details);
extern void print_connection(mapper_db_connection con, int details);
extern void print_signal(mapper_db_signal sig, int details);

extern void connect_signal(mapper_monitor mon,
      char * source_signal,
      char * dest_signal);
extern void disconnect_signal(mapper_monitor mon,
      char * source_signal,
      char * dest_signal);
extern void link_device(mapper_monitor mon,
      char * source_device,
      char * dest_device);
extern void unlink_device(mapper_monitor mon,
      char * source_device,
      char * dest_device);

static struct option long_options[] = {
   {"all",                    no_argument,         0, 'a'},
   {"help",                   no_argument,         0, 'h'},
   {"version",                no_argument,         0, 'v'},
   {"full",                   no_argument,         0, 'f'},

   {"device",                 no_argument,         0, 'd'},
   {"link",                   no_argument,         0, 'l'},
   {"connection",             no_argument,         0, 'c'},
   {"signal",                 no_argument,         0, 's'},

   {"connect_signal",         required_argument,   0, 'C'},
   {"disconnect_signal",      required_argument,   0, 'D'},

   {"link_device",            required_argument,   0, 'L'},
   {"unlink_device",          required_argument,   0, 'U'},

   {0, 0, 0, 0}
};

enum t_options{
   UMAPPER_PRINT,
   UMAPPER_CONNECT,
   UMAPPER_DISCONNECT,
   UMAPPER_LINK,
   UMAPPER_UNLINK
}options;

int option= -1;
int full_detail_arg = 0;
int devices_arg = 0;
int signals_arg = 0;
int connections_arg = 0;
int links_arg = 0;
char source_signal[256];
char dest_signal[256];
char source_device[256];
char dest_device[256];

int main(int argc, char * argv[])
{
   if(argc == 1){
      print_help();
      return 0;
   }

   create_monitor();
   mapper_monitor_request_devices(mon);

   int c;
   int option_index = 0;
   while((c = getopt_long(argc, argv,
               "hvfadlcsC:D:L:U:",
               long_options, &option_index)) != -1){
      switch (c){
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
            connections_arg = 1;
            links_arg = 1;
            option = UMAPPER_PRINT;
            break;
         case 'd':
            devices_arg = 1;
            option = UMAPPER_PRINT;
            break;
         case 'l':
            devices_arg = 1;
            links_arg = 1;
            option = UMAPPER_PRINT;
            break;
         case 'c':
            devices_arg = 1;
            links_arg = 1;
            signals_arg = 1;
            connections_arg = 1;
            option = UMAPPER_PRINT;
            break;
         case 's':
            devices_arg = 1;
            signals_arg = 1;
            option = UMAPPER_PRINT;
            break;
         case 'C':
            strcpy(source_signal, optarg);
            if(optind < argc){
               strcpy(dest_signal, argv[optind]);
               option = UMAPPER_CONNECT;
            }else{
               printf("Dest signal missing");
            }
            break;
         case 'D':
            strcpy(source_signal, optarg);
            if(optind < argc){
               strcpy(dest_signal, argv[optind]);
               option = UMAPPER_DISCONNECT;
            }else{
               printf("Dest signal missing");
            }
            break;
         case 'L':
            strcpy(source_device, optarg);
            if(optind < argc){
               strcpy(dest_device, argv[optind]);
               option = UMAPPER_LINK;
            }else{
               printf("Dest device missing");
            }
            break;
         case 'U':
            strcpy(source_device, optarg);
            if(optind < argc){
               strcpy(dest_device, argv[optind]);
               option = UMAPPER_UNLINK;
            }else{
               printf("Dest device missing");
            }
            break;
         case '?': printf("Missing argument\n"); break;
         default: print_help();
      }
   option_index = 0;
   }

   switch(option){
      case UMAPPER_PRINT:
         mapper_monitor_poll(mon, 1000);
         list_devices();
         break;
      case UMAPPER_LINK:
         link_device(mon, source_device, dest_device);
         mapper_monitor_poll(mon, 500);
         links_arg = 1;
         list_devices();
         break;
      case UMAPPER_UNLINK:
         unlink_device(mon, source_device, dest_device);
         mapper_monitor_poll(mon, 500);
         links_arg = 1;
         list_devices();
         break;
      case UMAPPER_CONNECT:
         connect_signal(mon, source_signal, dest_signal);
         mapper_monitor_poll(mon, 500);
         connections_arg = 1;
         list_devices();
         break;
      case UMAPPER_DISCONNECT:
         disconnect_signal(mon, source_signal, dest_signal);
         mapper_monitor_poll(mon, 500);
         connections_arg = 1;
         list_devices();
         break;
    }

    if (mon)
        mapper_monitor_free(mon);
    return 0;
}

void create_monitor(void) {
    mon = mapper_monitor_new(0, SUB_DEVICE_ALL);
    if (!mon)
        exit(0);
    db = mapper_monitor_get_db(mon);
}

void list_devices(void) {
    mapper_db_device *pdev = mapper_db_get_all_devices(db);
    if(pdev)
        printf("Device:\n");
    while (pdev) {
        mapper_db_device dev = *pdev;
        print_device(dev, full_detail_arg);

        if(links_arg) {
            mapper_db_link * link =
                mapper_db_get_links_by_device_name(db, dev->name);
            if(link)
                printf("\tLinks:\n");
            while(link) {
                print_link(*link, full_detail_arg);
                link = mapper_db_link_next(link);
            }
        }
        if(signals_arg) {
            mapper_db_signal *signal =
            mapper_db_get_inputs_by_device_name(db, dev->name);
            if(signal)
                printf("\tInput Signals:\n");
            while(signal) {
                print_signal(*signal, full_detail_arg);
                signal = mapper_db_signal_next(signal);
            }
            signal = mapper_db_get_outputs_by_device_name(db, dev->name);
            if(signal)
                printf("\tOutput Signals:\n");
            while(signal){
                print_signal(*signal, full_detail_arg);
                if(connections_arg){
                    mapper_db_connection *conn
                        = mapper_db_get_connections_by_src_signal_name(db, (*signal)->name);
                    if(conn)
                        printf("\t\tConnections:\n");
                    while(conn){
                        print_connection(*conn, full_detail_arg);
                        conn = mapper_db_connection_next(conn);
                    }
                }
                signal = mapper_db_signal_next(signal);
            }
        }
        printf("\n");
        pdev = mapper_db_device_next(pdev);
    }
}

