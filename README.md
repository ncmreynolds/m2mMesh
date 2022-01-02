# m2mMesh

m2mMesh is an Arduino library that provides a self-organising machine-to-machine (m2m) mesh network between Espressif ESP8266/8285/32 microcontrollers, using ESP-Now as a transport.

It was written to allow small, cheap microcontroller based projects to communicate with no need for 'infrastructure' or 'servers'.

**Beware it is still riddled with problems that need addressing and various features may change in both style and internal implementation, so it has not been submitted for inclusion in the Arduino Library Manager yet.**

## Table of Contents

1. [About the Project](#about-the-project)
1. [Project Status](#project-status)
1. [Getting Started](#getting-started)
	1. [Dependencies](#dependencies)
	1. [Building](#building)
	1. [Installation](#installation)
	1. [Usage](#usage)
1. [Release Process](#release-process)
	1. [Versioning](#versioning)
1. [How to Get Help](#how-to-get-help)
1. [Further Reading](#further-reading)
1. [Contributing](#contributing)
1. [License](#license)
1. [Authors](#authors)
1. [Acknowledgements](#acknowledgements)

# About the Project

m2mMesh abstracts away a lot of unpleasantness around mesh networking to try and make it easy to implement in an Arduino project.

The library was written to allow networking between large numbers of cheap battery powered devices over a small-ish outdoor area that would nonetheless be hard to cover with conventional Wi-Fi. For long range communication or long battery life another technology, eg. LORA is better suited.

Member nodes can have 'names' and you then package and send data to the name (or MAC address) and the library keeps everything ticking over in the background. The mesh also generates a shared 'time signal' and optionally propagates an RTC for syncing events across nodes.

By using ESP-Now there is no need for Wi-Fi access points and there can be multiple routes to a destination which are tolerant to changing conditions, moving nodes and unreliable transmission. There is a general aspiration to allow other transports on other microcontrollers but the low cost, high performance and ubiquity of the Espressif devices means it hasn't been tackled yet.

The resulting mesh network does not offer 'Internet access' for other devices, it is intended purely for machine-to-machine (m2m) communication, though one or more nodes can act as bridges into other messaging protocols such as MQTT.

In principle m2mMesh appears to overlap with Espressif's ESP-Mesh library, but this is focused on IoT connectivity and expects 'controller' and 'gateway' nodes. m2mMesh is non-IP, completely self-organising and 'flat'.

The routing protocol behind the scenes is inspired by [Open-Mesh](https://www.open-mesh.org/projects/open-mesh) and the routing algorithm is a simplified hybrid of the B.A.T.M.A.N. III and IV algorithms, but shares no code.

m2mMesh nodes can also speak conventional Wi-Fi and IP, connecting to, or acting as access points but care should be taken to ensure they do not become isolated on different channels from the other nodes. Presently all nodes in the mesh must be on the same Wi-Fi channel.

**[Back to top](#table-of-contents)**

# Project Status

This is the very first public release, which is working for the specific project I wrote it for. However I'm aware my naming of methods, approaches to passing arguments and so on are messy and inconsistent so some of this may change frequently. This is reflected in the version numbering, which is currently 0.1.0.

**[Back to top](#table-of-contents)**

# Getting Started

From the project page, select 'Code' -> 'Download ZIP' and save the file somewhere appropriate.

You can then install in the Arduino IDE by choosing 'Sketch' -> 'Include Library' -> 'Add .ZIP Library...'.

You are strongly recommended to try the various examples before starting work on your own application using it.

## Dependencies

The only dependency is on the ESP-Now library in the Espressif SDK, which should be present in the Arduino core. You will need version 2.5.2 or higher of the ESP8266 Arduino core, but this is now quite old.

## Getting the Source

This project is [hosted on GitHub](https://github.com/ncmreynolds/m2mMesh).

## Examples

In order to make this library accessible to 'casual' programmers who are likely to be using the Arduino IDE (I'm one myself) there are many examples included. You only need 2-3 Espressif devices to be able to try all the features of m2mMesh. All these examples show things happening in the Serial console.

1. Join Mesh - Joins a mesh and displays the number of nodes
2. Hello Mesh - Floods "Hello Mesh" to the whole mesh
3. Send random data - Floods data of many different random types and amounts to the mesh
4. Display received data - Displays any received data and its type
5. Set node name - Sets the name of a node
6. Send data to each node - Sends a message to each node on the mesh in turn
7. 
8. Synced blink - Uses the shared time signal to synchronise a blinking LED across the whole mesh
9. RTC server - Shares a real-time-clock across the whole mesh, based off the mesh time signal
10. RTC client - Shows the real-time-clock set by the RTC server
11. Event callbacks - Shows using callbacks for responding to events on the mesh. Callbacks for event based structure are still under development.
12. Trace to random node. Sends a trace packet (similar to IP traceroute) to random nodes on the mesh, displaying the route and round trip times.
13.  
14.  
15. Mesh info. A kind of 'protocol analyser' for the mesh which allows you to see what is happening by showing stats etc. over the UART of an ESP. You must use a fully featured terminal emulator (PuTTY, basically) as it uses control character to draw the screens. The Arduino IDE Serial monitor is not a proper terminal emulator and will not work.



**[Back to top](#table-of-contents)**

## Installation

From the project page, select 'Code' -> 'Download ZIP' and save the file somewhere appropriate.

You can then install in the Arduino IDE by choosing 'Sketch' -> 'Include Library' -> 'Add .ZIP Library...'.

Once the library API is stable it will be submitted to the Arduino Library Manager for inclusion.

**[Back to top](#table-of-contents)**

## Usage

[Documentation](docs/README.md) for m2mMesh is included as markdown in the 'docs' directory but also as README.md files included with every example which will give you a quicker start.

**[Back to top](#table-of-contents)**

# Release Process

## Versioning

This project uses [Semantic Versioning](http://semver.org/) to be compatible with the Arduino library standards.

For a list of available versions, see the [repository tag list](https://github.com/ncmreynolds/m2mMesh/tags). There is also the [changelog](docs/CHANGELOG.md).

### Version history

#### 0.1.0

First public release.

**[Back to top](#table-of-contents)**

# How to Get Help

Drop me a message on m2mMesh@arcanium.london, I can't promise to be super-responsive but I'll try.

**[Back to top](#table-of-contents)**

# Contributing

I'd love to get help with this work, either drop me a message here or on m2mMesh@arcanium.london. I'm new to using git for collaboration so please be patient.

We encourage public contributions! Please review [CONTRIBUTING.md](docs/CONTRIBUTING.md) for details on our code of conduct and development process.

**[Back to top](#table-of-contents)**

# Further Reading

The Espressif [ESP-Now resources](https://www.espressif.com/en/products/software/esp-now/resources) are helpful.

**[Back to top](#table-of-contents)**

# License

Copyright (c) 2021 Nick Reynolds

This project is licensed under the GNU Lesser General Public License - see [LICENCE.md](LICENCE.md) file for details.

**[Back to top](#table-of-contents)**

# Authors

* **[Nick Reynolds](https://github.com/ncmreynolds)** 

**[Back to top](#table-of-contents)**

# Acknowledgments

This library was inspired by the work done by others on [PainlessMesh](https://gitlab.com/painlessMesh/painlessMesh) and [Open-Mesh](https://www.open-mesh.org/projects/open-mesh). As mentioned in the documentation the routing algorithm is a simplified hybrid of the B.A.T.M.A.N. III and IV protocols, but shares no code.

**[Back to top](#table-of-contents)**