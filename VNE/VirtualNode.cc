//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
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

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "VirtualNode.h"

namespace inet {

Define_Module(VirtualNode);

VirtualNode::~VirtualNode(){
    cancelAndDelete(selfMsg);
}

void VirtualNode::initialize(int stage){

    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        //some statistics
        numSent = 0;
        numReceived = 0;
        numProcessed = 0;
        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numProcessed);


        local_port = par("local_port");
        job_name = par("job_name");

        //If vNode is source and how fast it is generating packets
        is_source = par("is_source");
        packet_interval = par("packet_interval");

        //the size of the packets sended out
        packet_size = par("packet_size");


        //UDP and simulation stuff
        startTime = par("startTime");
        stopTime = par("stopTime");

        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("sendTimer");
        selfMsg->setKind(START);
    }
}

void VirtualNode::finish(){
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    recordScalar("jobs processed", numProcessed);
    ApplicationBase::finish();
}

void VirtualNode::parseEdges(){
    //Getting the incoming virtual edges
    const char *addresses_in_str = par("addresses_in");
    const char *ports_in_str  = par("ports_in");
    const char *loops_str  = par("loops");

    std::vector<std::string> addresses_in = cStringTokenizer(addresses_in_str).asVector();
    std::vector<int> ports_in = cStringTokenizer(ports_in_str).asIntVector();
    std::vector<int> loops = cStringTokenizer(loops_str).asIntVector();

    if(addresses_in.size() != ports_in.size() || loops.size() != addresses_in.size()){
        throw cRuntimeError("Array of incoming addresses, ports and loops must have the  same length");
    }

    for(int i = 0; i < addresses_in.size(); i++){
        edges_in.push_back(new VirtualEdge(L3AddressResolver().resolve(addresses_in[i].c_str(), L3AddressResolver::ADDR_IPv4), ports_in[i], loops[i]));
        recv_msg_counter.insert({ports_in[i], loops[i]});
    }

    //Getting the outcoming virtual edges
    const char *addresses_out_str = par("addresses_out");
    const char *ports_out_str = par("ports_out");


    std::vector<std::string> addresses_out = cStringTokenizer(addresses_out_str).asVector();
    std::vector<int> ports_out = cStringTokenizer(ports_out_str).asIntVector();

    if(addresses_out.size() != ports_out.size()){
        throw cRuntimeError("Array of outcoming addresses and ports must have the same length");
    }

    for(int i = 0; i < addresses_out.size(); i++){
       edges_out.push_back(new VirtualEdge(L3AddressResolver().resolve(addresses_out[i].c_str(), L3AddressResolver::ADDR_IPv4), ports_out[i]));
    }

}

void VirtualNode::setSocketOptions(){

    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int dscp = par("dscp");
    if (dscp != -1)
        socket.setDscp(dscp);

    int tos = par("tos");
    if (tos != -1)
        socket.setTos(tos);

//    const char *multicastInterface = par("multicastInterface");
//    if (multicastInterface[0]) {
//        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
//        InterfaceEntry *ie = ift->findInterfaceByName(multicastInterface);
//        if (!ie)
//            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
//        socket.setMulticastOutputInterface(ie->getInterfaceId());
//    }
//
//    bool receiveBroadcast = par("receiveBroadcast");
//    if (receiveBroadcast)
//        socket.setBroadcast(true);
//
//    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
//    if (joinLocalMulticastGroups) {
//        MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
//        socket.joinLocalMulticastGroups(mgl);
//    }
    socket.setCallback(this);
}


void VirtualNode::sendPacket(){
    numProcessed++;
    for(VirtualEdge *edge : edges_out){
            std::ostringstream str;
            str << "VNE-" << job_name << "-" << numProcessed;
            Packet *packet = new Packet(str.str().c_str());
            packet->addTag<FragmentationReq>()->setDontFragment(true);

            const auto& payload = makeShared<ApplicationPacket>();
            payload->setChunkLength(B(packet_size));
            payload->setSequenceNumber(numSent);
            payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
            packet->insertAtBack(payload);

            emit(packetSentSignal, packet);

            socket.sendTo(packet, edge->address, edge->port);
            numSent++;
    }
}

void VirtualNode::processStart(){

    local_address = L3AddressResolver().resolve(par("local_address"));
    parseEdges();
    socket.setOutputGate(gate("socketOut"));
    socket.bind(local_address, local_port);
    setSocketOptions();

    if (is_source) {
        selfMsg->setKind(SEND);
        processSend();
    }
    else {
        if (stopTime >= SIMTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }
}

void VirtualNode::processSend(){

    sendPacket();

    simtime_t d = simTime() + packet_interval;

    if (stopTime < SIMTIME_ZERO || d < stopTime) {
        selfMsg->setKind(SEND);
        scheduleAt(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}

void VirtualNode::processStop(){
    socket.close();
}

void VirtualNode::processJob(){
    sendPacket();
    idle = true;
    doJob();
}

void VirtualNode::doJob(){
    //über jeden incoming edge packet erhalten?
    bool start_processing = true;
    for(auto it = recv_msg_counter.begin(); it != recv_msg_counter.end(); ++it){
        if(it->second < 1){
            start_processing = false;
        }
    }

    //simuliere Processing
    if(start_processing && idle){
        for(auto it = recv_msg_counter.begin(); it != recv_msg_counter.end(); ++it){
            it->second--;
        }
        idle = false;
        double delay = par("processing_delay"); //TODO add random delay
        selfMsg->setKind(JOB);
        scheduleAt(simTime() + delay, selfMsg);
    }

}


void VirtualNode::handleMessageWhenUp(cMessage *msg){
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
                processSend();
                break;

            case STOP:
                processStop();
                break;

            case JOB:
                   processJob();
                   break;
            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else
        socket.processMessage(msg);
}

void VirtualNode::socketDataArrived(UdpSocket *socket, Packet *packet){
    // process incoming packet
    processPacket(packet);
}

void VirtualNode::socketErrorArrived(UdpSocket *socket, Indication *indication){
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void VirtualNode::socketClosed(UdpSocket *socket){
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void VirtualNode::refreshDisplay() const{
    ApplicationBase::refreshDisplay();

    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void VirtualNode::processPacket(Packet *pk){
    emit(packetReceivedSignal, pk);
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    numReceived++;
    int key = pk->getTag<L4PortInd>()->getSrcPort();
    recv_msg_counter[key]++;

    doJob();

    delete pk;
}

void VirtualNode::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void VirtualNode::handleStopOperation(LifecycleOperation *operation){
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void VirtualNode::handleCrashOperation(LifecycleOperation *operation){
    cancelEvent(selfMsg);
    socket.destroy();         //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

} // namespace inet

