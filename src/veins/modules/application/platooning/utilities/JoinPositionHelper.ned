//
// Copyright (C) 2012-2016 Michele Segata <segata@ccs-labs.org>
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

package org.car2x.veins.modules.application.platooning.utilities;

import org.car2x.veins.modules.application.platooning.utilities.BasePositionHelper;

simple JoinPositionHelper like BasePositionHelper
{
    parameters:
        //number of cars in the scenario
        int nCars = default(5);
        //number of lanes in the scenario
        int nLanes = default(2);
        //number of cars in the platoon
        int platoonSize1 = default(5);
        int platoonSize2 = default(5);
        int platoonSize3 = default(5);
        int platoonSize4 = default(5);
        int platoonSize5 = default(5);
        int platoonSize6 = default(5);
        int platoonSize7 = default(5);
        int platoonSize8 = default(5);

        int platoonSize9 = default(5);
        int platoonSize10 = default(5);
        int platoonSize11 = default(5);
        int platoonSize12 = default(5);

        int platoonSize13 = default(5);
        int platoonSize14 = default(5);
        int platoonSize15 = default(5);
        int platoonSize16 = default(5);

        int platoonSize17 = default(5);
        int platoonSize18 = default(5);
        int platoonSize19 = default(5);
        int platoonSize20 = default(5);
        int platoonSize21 = default(5);
        int platoonSize22 = default(5);
        int platoonSize23 = default(5);
        int platoonSize24 = default(5);
        int platoonSize25 = default(5);
        int platoonSize26 = default(5);
        int platoonSize27 = default(5);
        int platoonSize28 = default(5);
        int platoonSize29 = default(5);
        int platoonSize30 = default(5);
        int platoonSize31 = default(5);
        int platoonSize32 = default(5);

        int platoonSize33 = default(5);
        int platoonSize34 = default(5);
        int platoonSize35 = default(5);
        int platoonSize36 = default(5);
        int platoonSize37 = default(5);
        int platoonSize38 = default(5);
        int platoonSize39 = default(5);
        int platoonSize40 = default(5);
        int platoonSize41 = default(5);
        int platoonSize42 = default(5);
        int platoonSize43 = default(5);
        int platoonSize44 = default(5);
        int platoonSize45 = default(5);
        int platoonSize46 = default(5);
        int platoonSize47 = default(5);
        int platoonSize48 = default(5);



        int headerLength @unit("bit") = default(0bit);

        @display("i=block/app2");
        @class(JoinPositionHelper);

    gates:
        input lowerLayerIn; // from network layer
        output lowerLayerOut; // to network layer
        input lowerControlIn; // control from network layer
        output lowerControlOut; // control to network layer

}
