# m2mMesh

m2mMesh is an Arduino library that provides a self-organising machine-to-machine (m2m) mesh network between Espressif ESP8266/8285/32 microcontrollers, using ESP-Now as a transport.

By using ESP-Now there is no need for Wi-Fi access points and there can be multiple routes to a destination which are tolerant to changing conditions and unreliable transmission.

It is not any form of 'Internet access' for other devices, it is intended purely for machine-to-machine (m2m) communication, though one or more nodes can act as bridges into other messaging protocols such as MQTT.

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

m2mMesh abstracts away a lot of unpleasantness around mesh networking to try and make it easy to implement in a project. Member nodes can have 'names' and you then package and send data to the name (or MAC address) and the library keeps everything ticking over in the background.

The library was written to allow network between large numbers of cheap battery powered devices over a small-ish area that would nonetheless be hard to cover with conventional Wi-Fi. For long range communication another technology, eg. LORA is better suited.

In principle this appears to overlap with Espressif's ESP-Mesh library, but this is focused on IoT connectivity and expects 'controller' and 'gateway' nodes. m2mMesh is non-IP and completely self-organising and 'flat'.

The routing protocol behind the scenes is inspired by [Open-Mesh](https://www.open-mesh.org/projects/open-mesh) and the routing algorithm is a simplified hybrid of the B.A.T.M.A.N. III and IV protocols, but shares no code.

m2mMesh nodes can also speak conventional Wi-Fi and IP, connecting to, or acting as access points but care should be taken to ensure they do not become isolated on different channels to the other nodes.

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

## Building

Instructions for how to build your project

```
Examples should be included
```

## Running Tests

Describe how to run unit tests for your project.

```
Examples should be included
```

### Other Tests

If you have formatting checks, coding style checks, or static analysis tests that must pass before changes will be considered, add a section for those and provide instructions

## Installation

From the project page, select 'Code' -> 'Download ZIP' and save the file somewhere appropriate.

You can then install in the Arduino IDE by choosing 'Sketch' -> 'Include Library' -> 'Add .ZIP Library...'.

## Usage

[Documentation](documentation/README.md) for m2mMesh is included as markdown in the 'documentation' directory.

```
Examples should be included
```

If your project provides an API, either provide details for usage in this document or link to the appropriate API reference documents

**[Back to top](#table-of-contents)**

# Release Process

Talk about the release process. How are releases made? What cadence? How to get new releases?

## Versioning

This project uses [Semantic Versioning](http://semver.org/) to be compatible with the Arduino library standards.

For a list of available versions, see the [repository tag list](https://github.com/ncmreynolds/m2mMesh/tags).

### Version history

#### 0.1.0

First public release.

# How to Get Help

Drop me a message on m2mMesh@arcanium.london, I can't promise to be super-responsive but I'll try.

# Contributing

I'd love to get help with this work, either drop me a message here or on m2mMesh@arcanium.london. I'm new to using git for collaboration so please be patient.

We encourage public contributions! Please review [CONTRIBUTING.md](docs/CONTRIBUTING.md) for details on our code of conduct and development process.

**[Back to top](#table-of-contents)**

# Further Reading

The Espressif [ESP-Now resources](https://www.espressif.com/en/products/software/esp-now/resources) are helpful.

**[Back to top](#table-of-contents)**

# License

Copyright (c) 2021 Nick Reynolds

This project is licensed under the GNU Lesser General Public License - see [LICENSE.md](LICENSE.md) file for details.

**[Back to top](#table-of-contents)**

# Authors

* **[Nick Reynolds](https://github.com/ncmreynolds)** 

**[Back to top](#table-of-contents)**

# Acknowledgments

This library was inspired by the work done by others on [PainlessMesh](https://gitlab.com/painlessMesh/painlessMesh) and [Open-Mesh](https://www.open-mesh.org/projects/open-mesh). As mentioned in the documentation the routing algorithm is a simplified hybrid of the B.A.T.M.A.N. III and IV protocols, but shares no code.

**[Back to top](#table-of-contents)**