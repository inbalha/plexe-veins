//
// Copyright (C) 2014-2016 Michele Segata <segata@ccs-labs.org>
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

#include "JoinTrafficManager.h"

Define_Module(JoinTrafficManager);

void JoinTrafficManager::initialize(int stage) {
	std::cout<<"in join traffic manager - initialize"<<endl;
	PlatoonsTrafficManager::initialize(stage);

	if (stage == 0) {

		insertJoinerMessage = new cMessage("");
		//the platoon is inserted after 6 seconds
		//the joiner does not enter the system as a result of the followoing line
		scheduleAt(platoonInsertTime + SimTime(0.1), insertJoinerMessage);

	}
}

void JoinTrafficManager::handleSelfMsg(cMessage *msg) {
	PlatoonsTrafficManager::handleSelfMsg(msg);

	if (msg == insertJoinerMessage) {
		insertJoiner();
	}

}

void JoinTrafficManager::insertJoiner() {
	automated.position = 0;
	automated.lane = 0;
	automated.speed = 0;
	std::cout<<"**********about to enter add vehicle to queue in platoon traffic manager with 0 for route"<<endl;
	addVehicleToQueue(0, automated);
}

void JoinTrafficManager::finish() {
	PlatoonsTrafficManager::finish();
	cancelAndDelete(insertJoinerMessage);
	insertJoinerMessage = 0;
}
