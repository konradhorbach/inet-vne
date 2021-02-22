//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package vne.simulations;

// numOfHosts: 10
// parametric: true
// static:     false

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.INetworkNode;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import inet.physicallayer.apskradio.packetlevel.ApskScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

network general
{
    parameters:
        string hostType = default("ManetRouter");
        int numHosts = default(7);



        @display("bgb=1000,1000;bgg=100,1,grey95");
        @figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);

        @figure[generated](type=indicatorText; pos=380,20; anchor=w; font=,18; textFormat="generated: %g"; initialValue=0);
        //statistic[packet_received](source=node[0].app[0].numSent; record=figure(count); targetFigure=generated);

    submodules:

        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            @display("p=900,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=900,150");
        }
       radioMedium: <default("UnitDiskRadioMedium")> like IRadioMedium {
            @display("p=580,275");
        }

        node[numHosts]: <hostType> like INetworkNode {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070");
        }


}
