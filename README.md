# omnet-vne
A VNE application for Omnet++ (INET)
It is based on the UdpBasicApp and was made for inetmanet-4.2.

With this framework one can simulate a virtual network of applications.
A virtual network consists of virtual nodes and virtual edges. A virtual node is an application. It can either be a source (it generates packets), or it simulates processing after it got a packet over every incoming edge.
After processing a node sends a packet on each outgoing virtual edge.

## How to use
Place the VirtualNodes as an application in the network nodes, and assign them a port. Every port must differ, no matter if they're placed in different nodes.

Set up the incomgin virtual edges by defining *addresses-in*, *ports-in* and *loops* of the corresponding virtual node. Those are strings where a space defines a new item. They must have the same number of items. The position of the items in these strings refer to the same virtual edge.

Set up the outgoing virtual edges by defining *addresses-out* and *ports-out* of the corresponding virtual node. Those are strings where a space defines a new item. They must have the same number of items. The position of the items in these strings refer to the same virtual edge.

Declare a virtual node as source (*is-source = true*) and it generates packets after each *packet-interal*.
