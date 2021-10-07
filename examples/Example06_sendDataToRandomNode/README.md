# Send data to random nodes

This sketch sends a simple "Hello there" message to random nodes in the network, also with a chance of flooding it to the whole network.

It can be used alongside Example 4 to check the function of the mesh.

![](output.png)

If a message destination is not set the message will flood the whole mesh and every node will receive it.

While this is simple, in a large network it can cause congestion and unnecessary processing as every node broadcasts it to every other node it can reach. 

A message with a specific destination will take the most reliable path to the destination the underlying routing algorithm of the mesh can find. It will be processed by a fraction of the nodes that a flood would.

The destination of a message **must** be set before any data fields are added. It can be specified in three different ways.

- Numeric ID
- MAC address
- Name

### ID

The 'ID', is a temporary ID number the local node assigns to another one and cannot be relied on across restarts to mean the same node.

### MAC address

The 'MAC address' is the **base** MAC address of the ESP8266/8285/32. Note that the MAC address reported via various methods in the WiFi libraries is often not the **base** MAC address but a temporary one derived from it and can vary. m2mMesh always uses the base MAC address for destination addresses and its own internal tables.

### Name

The 'name' is one set when starting the mesh by each node. For ease of configuration it is recommended node names are used as destinations, but it can take up to five minutes for a node to learn the name of another. Once learned it is remembered. It is possible to change the name of a node after the mesh has started but it can take another five minutes for the name change to be learned.

See example 6 for how to set the name of a node.

## Methods for setting the message destination

```c++
bool m2mMesh.destination(<destination ID>)
bool m2mMesh.destination(uint8_t* <MAC address>)
bool m2mMesh.destination(<destination name>)
```

## Other methods

