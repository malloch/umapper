#include <mapper/mapper.h>
#include <stdio.h>
#include <string.h>


/*void connection_modify(){*/
/*void mapper_monitor_connection_modify(mapper_monitor mon,*/
/*                                      mapper_db_connection_t *props,*/
/*                                      unsigned int props_flags)*/
/*}*/

void connect_signal(mapper_monitor mon,
      char * source_signal,
      char * dest_signal){
   mapper_monitor_connect(mon, source_signal, dest_signal, NULL, 0);
}

void disconnect_signal(mapper_monitor mon,
      char * source_signal,
      char * dest_signal){
   mapper_monitor_disconnect(mon,
                          source_signal,
                          dest_signal);
}

void link_device(mapper_monitor mon,
      char * source_device,
      char * dest_device){
   printf("Sendig request to link %s to %s\n", source_device, dest_device);
   mapper_monitor_link(mon,
                         source_device,
                         dest_device,
                         NULL,
                         0);
}

void unlink_device(mapper_monitor mon,
      char * source_device,
      char * dest_device){
   printf("Sendig request to unlink %s to %s\n", source_device, dest_device);
   mapper_monitor_unlink(mon,
                           source_device,
                           dest_device);
}
