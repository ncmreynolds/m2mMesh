# Packets sent across the mesh

The mesh relies on regular transmission of 'beacon' packets of various types to make things work. They are sent at two different frequencies, 'fast startup', which is used when the mesh is being established and 'default' once it is stable.

## ELP - Echo Location Protocol

This simple packet advertises this node's existence to others and includes a list of other nodes it can directly see. This is used to quickly build a partial mesh and then also to invalidate poor quality next hops, without relying OGM discovered routes to time out, which is slow. Typically ELP packets are not forwarded by nodes.

Default interval: 10s

Fast startup interval: 5s

## OGM - Originator message

This packet is used to discover routes across the mesh. It is forwarded from node to node across the whole mesh and the success or failure of delivery determines the 'transmission quality' for each hop, which is then used to construct a hopefully reliable route to every node.

Default interval: 60s

Fast startup interval: 30s

## NHS - Node health/state

This packet is used to advertise the general status of each node across the whole mesh and includes many stats about it.

Default interval: 300s

Fast startup interval: 60s

## USR - User data

This packet is packed with user data. It can be either flooded to the whole mesh or sent to a single node.

Default interval: N/A

Fast startup interval: N/A
