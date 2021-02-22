//
// Copyright (C) 2021 Konrad Horbach
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.applications.VNE;

import inet.applications.contract.IApp;

// This INET application is an extension to UDPBasicApp of INET
//
// A VirtualNode represents an application in a topology of applications, that can be distributed over multiple nodes.
// It implements a UDP socket
// addresses_in and ports_in describe incoming edges, i.e. the VirtualNode has to get a packet from VirtualNode applications of each incoming edge, simulates processing and then sends a packet on each outcoming edge
// Every port of each VirtualNode of a topology has to differ.
// Declare a VirtualNode as source (is_source = true), and it is generating packets without the need of incoming edges. A source can theoretically have incoming edges, but is has no effect.
// Sink nodes are just virtual nodes without outgoing edges
// This application allows loops in the topology. In order to do this, set the loops-list in the first VirtualNode of the loop, so that the corresponding incoming edge has the entrty 1. Without loops set every entry to 0
//


simple VirtualNode like IApp
{
    parameters:
        
        int local_port = default(-1);
        string local_address = default("");
       	string job_name = default("job");
        
        string addresses_in = default(""); //list of addresses of incoming edges, addresses seperated by spaces
        string addresses_out =default(""); //list of addresses of outgoing edges, addresses seperated by spaces
        string ports_in = default(""); //list of ports of incoming edges, ports seperated by spaces, position in list has to be the same as the corresponding address in addresses_in
        string ports_out = default(""); //list of ports of  outgoing edges, ports seperated by spaces, position in list has to be the same as the corresponding address in addresses_out
        string loops = default(""); // list of loops of incoming edges, i.e. item=1 means that this incoming edge has already one packet in its buffer. Must have the same length as addresses_in/ports_in. Set default to "0 0 ... 0"
        
        bool is_source = default(false); // sources generate packets
        double packet_interval @unit(s) = default(0.01s);
        double processing_delay @unit(s) = default(0.001s);
        int packet_size @unit(B) = default(1B);
        


        string interfaceTableModule;
        double startTime @unit(s) = default(this.packet_interval); // application start time (start of the first packet)
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, -1s means forever
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        bool dontFragment = default(false); // if true, asks IP to not fragment the message during routing
        int dscp = default(-1); // if not -1, set the DSCP field (on IPv4/IPv6) of sent packets to this value
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        string multicastInterface = default("");  // if not empty, set the multicast output interface option on the socket (interface name expected)
        bool receiveBroadcast = default(false); // if true, makes the socket receive broadcast packets
        bool joinLocalMulticastGroups = default(false); // if true, makes the socket receive packets from all multicast groups set on local interfaces
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[throughput](title="throughput"; unit=bps; source="throughput(packetReceived)"; record=vector);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkLifetime](title="received packet lifetime"; source="dataAge(packetReceived)"; unit=s; record=stats,vector; interpolationmode=none);
        @statistic[rcvdPkSeqNo](title="received packet sequence number"; source="appPkSeqNo(packetReceived)"; record=vector; interpolationmode=none);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

