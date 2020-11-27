# umapper

A command line tool for interacting with distributed signal networks using [libmapper](https://github.com/libmapper/libmapper).

## Example

umapper offers a simple method for mapping signals between devices at **runtime**. Suppose for this example that you have two libmapper devices, a producer and a consumer that are defined in **Python 3** as follows:

```
import mapper

producer = mapper.device("Producer")
consumer = mapper.device("Consumer")
```

Each of the devices have one signal that we will map with umapper.

```
producer.add_signal(mapper.DIR_OUT, "Producer-Signal", 1, mapper.INT32)

consumer.add_signal(mapper.DIR_IN, "Consumer-Signal", 1, mapper.INT32)
```

After polling each of the devices, they will enter the ready state and be visible to umapper. To view all libmapper devices currently ready, issue the command:

```
umapper -a
```

Which in this example will output:

```
Devices:
    Producer.1
        output signals:
            Producer-Signal
    Consumer.1
        input signals:
            Consumer-Signal

```

To map the producer's output signal to the consumer's input signal, issue the command:

```
umapper -M Producer.1/Producer-Signal Consumer.1/Consumer-Signal
```

Which if successful will output the following:

```
MAP: 'Producer.1:Producer-Signal' -> 'Consumer.1:Consumer-Signal', id=0, is_local=F, muted=F, num_sigs_in=1, process_loc=unknown, protocol=osc.udp, scope=NULL, status=2, use_inst=F, version=0
mapped:                      Producer.1/Producer-Signal -> Consumer.1/Consumer-Signal,

```

You have successfully mapped the signals! To verify this with umapper, you can run `umapper -a` again which will output the following:

```
Devices:
    Consumer.1
        input signals:
            Consumer-Signal
    Producer.1
        output signals:
            Producer-Signal
                Maps:
                     Producer.1/Producer-Signal -> Consumer.1/Consumer-Signal,

```
