# Examples

To try and make the library accessible, many examples have been written.

## Example 1 - Join mesh

This sketch will simply join the mesh and periodically report how many members (excluding the current node) there are.

## Example 2 - Hello Mesh

This sketch will flood the mesh with messages saying "Hello Mesh", and print them if it receives them.

## Example 3 - Send random data types

This sketch will flood the mesh with messages containing a random number of fields of random types of data. It should be used in conjunction with another node running Example 4 and can be used generally to show that a mesh is functioning.

## Example 4 - Display received data

This sketch will show all the fields of data in any message received. It should be used as the companion sketch to Example 3 and several others but may also be useful in debugging your own code or showing the messages from any other example.

## Example 5 - Set node name

The mesh can optionally use node names to set destinations on messages, to avoid hardcoding MAC addresses etc. The sketch demonstrates setting and retrieving node names and MAC addresses.

## Example 6 - Use node name

The mesh can optionally use node names to set destinations on messages, to avoid hardcoding MAC addresses etc. The sketch demonstrates using a node name to set the destintation of a message.

## Example 7 - Send data to each node

This sketch demonstrates adding a destination so the data does not flood the whole mesh. It can be used along with Example 4 to demonstrate more targeted delivery of data.

## Example 8 - Synced Blink

The mesh has a shared time source, its 'uptime' that is synced across all nodes. This can be used to synchronise events. So long as at least one node has remained up, the uptime will continuously increase much like the Arduino 'millis()' function.

## Example 9 - RTC server

The mesh builds on the shared 'uptime' source by allowing nodes to become RTC servers and share a real-time-clock synchronised with NTP.

## Example 10 - RTC client

Demonstrates how the real-time-clock on every node synchronises with another mesh node acting as an RTC server.

## Example 11 - Event Callbacks

There is experimental support for an event driven style of programming, which this sketch demonstrates.

## Example 12 - Trace to random node

The library includes a traceroute-like method for tracing the route to a node and measuring the round trip time.

## Example 15 - m2mMesh Info

This is a 'network analyser' application that shows a great deal of information about what is going on across the mesh.
