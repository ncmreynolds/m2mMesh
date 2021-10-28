# m2mMesh

This is an attempt to make a genuinely self-organising, reasonably reliable mesh network for communication between microcontrollers.

Developed initially on Espressif ESP8266/8285/32 microcontrollers using [ESP-Now](ESP-Now.md), but there is an aspiration to make it usable on others using other transports.

## Concepts

- Every member of the mesh is a 'node' and all nodes are equal.
- A message can be sent to either one other node or 'flooded' to all nodes.
- Nodes can referred to by their MAC address, or more simply, a 'name'.
- Messages may contain multiple types of data and is 'strongly typed', ie. you add an uint16_t and when it is received you need to extract it into an uint16_t.
- Most data types in a message can also be sent as arrays, for example uint8_t[12].
- C strings and Arduino Strings can be included in a message.
- Messages are ill-suited to verbose formats, eg. JSON but they can be used.
- Nodes share some 'health' information about themselves such as free memory, uptime and so on.
- Nodes automatically elect a 'sync server' which creates a shared time signal to allow synchronising of events. If the 'sync server' goes offline another will be elected.
- One or more nodes can optionally act as an 'RTC server' which shares a real-time clock and timezone to allow for scheduling of events. If one 'RTC server' goes offline another will be elected.