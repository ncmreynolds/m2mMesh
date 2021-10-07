# Examples

## Example 1 - Join mesh

This sketch will simply join the mesh and periodically report how many members (excluding the current node) there are.

## Example 2 - Hello Mesh

This sketch will flood the mesh with messages saying "Hello Mesh".

## Example 3 - Send random data types

This sketch will flood the mesh with messages containing a random number of fields of random types of data. It should be used in conjunction with another node running Example 4 and can be used generally to show that a mesh is functioning.

## Example 4 - Display received data

This sketch will show all the fields of data in any message received. It should be used as the companion sketch to Example 3 and several others but may also be useful in debugging your own code.

## Example 5 - Set node name

The mesh can optionally use node names to set destinations on messages, to avoid hardcoding MAC addresses etc. The sketch demonstrates setting and retrieving node names and MAC addresses.

## Example 6 - Send data to random node

This sketch demonstrates adding a destination so the data does not flood the whole mesh. It can be used along with Example 4 to demonstrate more targeted delivery of data.
