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

#ifndef INET_APPLICATIONS_VNE_VIRTUALEDGE_H_
#define INET_APPLICATIONS_VNE_VIRTUALEDGE_H_

#include <string>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

class VirtualEdge {
public:
    VirtualEdge(L3Address address, int port, bool loop);
    VirtualEdge(L3Address address, int port);

    virtual ~VirtualEdge();

    L3Address address;
    int port;
    bool loop;
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_VNE_VIRTUALEDGE_H_ */
