//
// Copyright (c) 2012-2016 Michele Segata <segata@ccs-labs.org>
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

#include "veins/modules/application/platooning/utilities/PositionHelper.h"

Define_Module(PositionHelper);

void PositionHelper::initialize(int stage) {

	BasePositionHelper::initialize(stage);

	if (stage == 0) {
		
		// does not relevant anymore - each platoon type appears one time only
		//nCars = getMyNcars(&platoonSize);
		
		myId = getIdFromExternalId(getExternalId());
		//leaderId = getLeader(&platoonSize);
		//leader = IsMyleader(myId,&leaderId);
		//frontId = getFrontVehicle(myId, nLanes, platoonSize);
		//position = getPositionInPlatoon(myId, nLanes, platoonSize);
		//platoonId = getPlatoonId(&leaderId, &platoonSize);
		//platoonLane = getPlatoonLane(myId, nLanes);
		

	}

}

void PositionHelper::finish() {
	BasePositionHelper::finish();
}

// my addition


/*int PositionHelper::getLeader(std::vector<int>* platoonSize) {
	int k=0;
	for (int i=0; i<platoonSize.size(); i++){
		leaderId.push_back(k);
		k = k + PlatoonSize[i];
	}
	return leaderId
}*/

/*bool PositionHelper::IsMyleader(int myId,std::vector<int>* leaderId) {
	for (auto&& element : platoonSize) {
		if (element == myId)
			return 1;
	}
	return 0;
}*/

/*int PositionHelper::getPlatoonId(std::vector<int>* leaderId, std::vector<int>* platoonSize) {
	int index=0;
	for (auto&& element : leaderId) {
		if (element == getLeaderId(&platoonSize)) {
			index = std::distance(leaderId.begin(), element);
			return index;
		}
	}
}*/

int PositionHelper::getIdFromExternalId(std::string externalId) {
	int dotIndex = externalId.find_last_of('.');
	std::string strId = externalId.substr(dotIndex + 1);
	return strtol(strId.c_str(), 0, 10);
}

/*
int PositionHelper::getPosition() {
	return position;
}

int PositionHelper::getMemberId(int position) {
	return leaderId + position * nLanes;
}

int PositionHelper::getMemberPosition(int vehicleId) {
	return (vehicleId - leaderId) / nLanes;
}

int PositionHelper::getLeaderId() { // pay attention - incorrect!!!
	std::cout<<"the leader Id from position Helper is "<<leaderId<<endl;
	
	return leaderId;
}

bool PositionHelper::isLeader() {
	return leader;
}

int PositionHelper::getFrontId() {
	return frontId;
}

int PositionHelper::getPlatoonId() {
	return platoonId;
}

int PositionHelper::getPlatoonLane() {
	std::cout<<"entered get platoon lane with no variables"<<endl;
	std::cout<<"the platoon lane from get platoon lane with no variables is....."<<platoonLane<<endl;
	return platoonLane;
}




bool PositionHelper::isLeader(int vehicleId, int nLanes, int platoonSize) {
	return (vehicleId / nLanes) % platoonSize == 0;
}
int PositionHelper::getPlatoonNumber(int vehicleId, int nLanes, int platoonSize) {
	std::cout<<"entered get platoon number"<<endl;
	return vehicleId/platoonSize;
	//return getPlatoonColumn(vehicleId, nLanes, platoonSize) * nLanes + getPlatoonLane(vehicleId, nLanes);
}
int PositionHelper::getPlatoonLane(int vehicleId, int nLanes) { // incorrect!!!
	std::cout<<"entered get platoon lane with two variables"<<endl;
	std::cout<<"the platoon lane from get platoon lane with two variables is....."<<vehicleId % nLanes<<endl;
	return vehicleId % nLanes;
}
int PositionHelper::getPlatoonColumn(int vehicleId, int nLanes, int platoonSize) {
		std::cout<<"entered get platoon columns"<<endl;
	return vehicleId / (nLanes * platoonSize);
}
int PositionHelper::getPlatoonLeader(int vehicleId, int platoonSize) { // this function does not work properly, therefore I wrote another one in BasePositionHelper: getLeader, which does the same
	std::cout<<"***************get platoon leader **************** "<<(vehicleId - vehicleId % platoonSize)<<endl;
	return (vehicleId - vehicleId % platoonSize); 
	//return getPlatoonColumn(vehicleId, nLanes, platoonSize) * nLanes * platoonSize + getPlatoonLane(getPlatoonNumber(vehicleId, nLanes, platoonSize), nLanes);
}
int PositionHelper::getFrontVehicle(int vehicleId, int nLanes, int platoonSize) {
	if (getPlatoonLeader(vehicleId, platoonSize) == vehicleId)
		return -1;
	else
		return vehicleId - nLanes;
}

bool PositionHelper::isFrontVehicle(int vehicleId, int myId, int nLanes, int platoonSize) {
	return getFrontVehicle(myId, nLanes, platoonSize) == vehicleId;
}
int PositionHelper::getPositionInPlatoon(int vehicleId, int nLanes, int platoonSize) { // pay attention - incorrect!!!!
		std::cout<<"entered get position platoon with 3 variables"<<endl;
	return vehicleId - nLanes;
}

*/