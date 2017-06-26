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

moduleinterface BasePositionHelper
{
    parameters:
        //number of cars in the scenario
        int nCars;
        //number of cars in the platoon
        int platoonSize1;
        int platoonSize2;
        int platoonSize3;
        int platoonSize4;
        int platoonSize5;
        int platoonSize6;

        int platoonSize7;
        int platoonSize8;
        int platoonSize9;
        int platoonSize10;
        int platoonSize11;
        int platoonSize12;

        int platoonSize13;
        int platoonSize14;
        int platoonSize15;
        int platoonSize16;

        int platoonSize17;
        int platoonSize18;
        int platoonSize19;
        int platoonSize20;
        int platoonSize21;
        int platoonSize22;
        int platoonSize23;
        int platoonSize24;

        int platoonSize25;
        int platoonSize26;
        int platoonSize27;
        int platoonSize28;
        int platoonSize29;
        int platoonSize30;
        int platoonSize31;
        int platoonSize32;

        int platoonSize33;
        int platoonSize34;
        int platoonSize35;
        int platoonSize36;
        int platoonSize37;
        int platoonSize38;
        int platoonSize39;
        int platoonSize40;
        int platoonSize41;
        int platoonSize42;
        int platoonSize43;
        int platoonSize44;
        int platoonSize45;
        int platoonSize46;
        int platoonSize47;
        int platoonSize48;


        //number of lanes in the scenario
        int nLanes;


        @display("i=block/app2");
        @class(BasePositionHelper);

}
