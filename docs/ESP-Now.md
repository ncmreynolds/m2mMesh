# Use of ESP-Now as a transport in m2mMesh

The default transport for m2mMesh is ESP-Now, a proprietary protocol from Espressif only supported on their ESP8266/8285/32 microcontrollers. This uses the usual 802.11b/g/n radio standards and embeds data in a 'vendor action frame' which is specifically to allow vendor equipment to share proprietary information directly between radios.

In the most basic terms, it allows you to send up 250 bytes of data to another ESP, using just its MAC address. It also has a practical minimum packet size of 64 bytes.

## Peering

For ESP-Now to work, you must establish a 'peer' relationship between two ESPs based on the destination MAC address, which can also be the broadcast MAC address.

All nodes in m2mMesh have the broadcast MAC address as a peer and share mesh information on this.

Any application messages flooded to the whole network with be sent to the broadcast MAC address.

Any messages sent to a specific node, rather than flooded, will be sent directly between specific nodes, where possible across the mesh. If a necessary peering does not exist between two nodes, the first packet will be broadcast and include a flag asking to create a peering for future traffic.

Idle peerings are periodically removed, as there is a low limit on the number of peers, 20 on ESP8266 if unencrypted.

## Encryption

ESP-Now offers encryption between peers, but not if they communicate on the broadcast MAC address. For this reason it is not used by m2mMesh and any future addition of encryption to the protocol is likely to be difficult given the limited resources on each ESP.