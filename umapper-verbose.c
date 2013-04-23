#include "mapper_internal.h"
#include <mapper/mapper.h>
#include <stdio.h>
#include <string.h>

void print_version(){
   printf("Ugly Mapper v 0.01\n");
}

void print_help(){
   print_version();
   printf("An ugly front-end to libmapper mapping.\n");
   printf("Usage:\n");
   printf("\t-h\t--help\t\t\t\t\tPrint this help\n");
   printf("\t-v\t--version\t\t\t\tPrint version\n");
   printf("\t-a\t--all\t\t\t\t\tList devices, links, connections and signals\n");
   printf("\t-f\t--full\t\t\t\t\tList all details\n");
   printf("\n");
   printf("\t-d\t--device\t\t\t\tList only devices\n");
   printf("\t-s\t--signal\t\t\t\tList devices with signals\n");
   printf("\t-l\t--link\t\t\t\t\tList devices and links\n");
   printf("\t-c\t--connection\t\t\t\tList devices, links, signals and connections\n");
   printf("\n");
   printf("\t-C\t--connect_signal source dest\t\tConnect source signal to dest signal.\n");
   printf("\t-D\t--disconnect_signal source dest\t\tDisconnect source signal to dest signal.\n");
   printf("\n");
   printf("\t-L\t--link_device source dest\t\tLink source device to dest device.\n");
   printf("\t-U\t--unlink_device source dest\t\tUnink source device to dest device.\n");
   printf("\n");
   printf("Example:\n");
   printf("\tumapper -l -f\n");
}

void print_device(mapper_db_device dev, int details){
   printf("\t%s", dev->name);
   if(!details){
      printf("\n");
      return;
   }
   int i=0;
   const char *key;
   lo_type type;
   const lo_arg *val;
   while (!mapper_db_device_property_index(dev, i++, &key, &type, &val)){
      printf(", %s=", key);
      lo_arg_pp(type, (lo_arg*)val);
   }
   printf("\n");
}

void print_connection(mapper_db_connection con, int details){
   printf("\t\t\t%s -> %s ", con->src_name, con->dest_name);
   if(!details){
      printf("\n");
      return;
   }
    int i=0;
    const char *key;
    lo_type type;
    const lo_arg *val;
    while(!mapper_db_connection_property_index(con, i++, &key, &type, &val)){
        die_unless(val!=0, "returned zero value\n");
        printf(", %s=", key);
        lo_arg_pp(type, (lo_arg*)val);
    }
    printf("\n");

}

void print_signal(mapper_db_signal sig, int details){
   printf("\t\t%s%s ", sig->device_name, sig->name);
   if(!details){
      printf("\n");
      return;
   }
    int i=0;
    const char *key;
    lo_type type;
    const lo_arg *val;
    while(!mapper_db_signal_property_index(
              sig, i++, &key, &type, &val))
    {
        die_unless(val!=0, "returned zero value\n");

        // already printed these
        if (strcmp(key, "device_name")==0
            || strcmp(key, "name")==0
            || strcmp(key, "direction")==0)
            continue;

        printf(", %s=", key);
        lo_arg_pp(type, (lo_arg*)val);
    }
    printf("\n");

}

void print_link(mapper_db_link lnk, int details){
   printf("\t\t%s -> %s ", lnk->src_name, lnk->dest_name);
   if(!details){
      printf("\n");
      return;
   }
   int i=0;
   const char *key;
   lo_type type;
   const lo_arg *val;
   while (!mapper_db_link_property_index(lnk, i++, &key, &type, &val)){
      printf(", %s=", key);
      lo_arg_pp(type, (lo_arg*)val);
   }
   printf("\n");

}

