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

#include "veins/modules/application/platooning/utilities/BasePositionHelper.h"
#include <vector>

Define_Module(BasePositionHelper);

void BasePositionHelper::initialize(int stage) {
	BaseApplLayer::initialize(stage);
	
	if (stage == 0) {
		platoonSize.push_back(par("platoonSize1").longValue());
		platoonSize.push_back(par("platoonSize2").longValue());
		platoonSize.push_back(par("platoonSize3").longValue());
		platoonSize.push_back(par("platoonSize4").longValue());
		platoonSize.push_back(par("platoonSize5").longValue());
		platoonSize.push_back(par("platoonSize6").longValue());		

		platoonSize.push_back(par("platoonSize7").longValue());
		platoonSize.push_back(par("platoonSize8").longValue());
		platoonSize.push_back(par("platoonSize9").longValue());
		platoonSize.push_back(par("platoonSize10").longValue());
		platoonSize.push_back(par("platoonSize11").longValue());
		platoonSize.push_back(par("platoonSize12").longValue());		

		platoonSize.push_back(par("platoonSize13").longValue());
		platoonSize.push_back(par("platoonSize14").longValue());
		platoonSize.push_back(par("platoonSize15").longValue());
		platoonSize.push_back(par("platoonSize16").longValue());

		platoonSize.push_back(par("platoonSize17").longValue());
		platoonSize.push_back(par("platoonSize18").longValue());		
		platoonSize.push_back(par("platoonSize19").longValue());
		platoonSize.push_back(par("platoonSize20").longValue());
		platoonSize.push_back(par("platoonSize21").longValue());
		platoonSize.push_back(par("platoonSize22").longValue());
		platoonSize.push_back(par("platoonSize23").longValue());
		platoonSize.push_back(par("platoonSize24").longValue());		
		platoonSize.push_back(par("platoonSize25").longValue());
		platoonSize.push_back(par("platoonSize26").longValue());		
		platoonSize.push_back(par("platoonSize27").longValue());
		platoonSize.push_back(par("platoonSize28").longValue());
		platoonSize.push_back(par("platoonSize29").longValue());
		platoonSize.push_back(par("platoonSize30").longValue());
		platoonSize.push_back(par("platoonSize31").longValue());
		platoonSize.push_back(par("platoonSize32").longValue());	

		platoonSize.push_back(par("platoonSize33").longValue());
		platoonSize.push_back(par("platoonSize34").longValue());		
		platoonSize.push_back(par("platoonSize35").longValue());
		platoonSize.push_back(par("platoonSize36").longValue());
		platoonSize.push_back(par("platoonSize37").longValue());
		platoonSize.push_back(par("platoonSize38").longValue());
		platoonSize.push_back(par("platoonSize39").longValue());
		platoonSize.push_back(par("platoonSize40").longValue());		
		platoonSize.push_back(par("platoonSize41").longValue());
		platoonSize.push_back(par("platoonSize42").longValue());		
		platoonSize.push_back(par("platoonSize43").longValue());
		platoonSize.push_back(par("platoonSize44").longValue());
		platoonSize.push_back(par("platoonSize45").longValue());
		platoonSize.push_back(par("platoonSize46").longValue());
		platoonSize.push_back(par("platoonSize47").longValue());
		platoonSize.push_back(par("platoonSize48").longValue());	
	




		for (int i=0; i<platoonSize.size(); i++) {
			platoonRoute.push_back(i+1);
		}


		nLanes = par("nLanes").longValue();	
		mobility = Veins::TraCIMobilityAccess().get(getParentModule());
		traci = mobility->getCommandInterface();
		traciVehicle = mobility->getVehicleCommandInterface();		
		highestId = nCars - 1;
		
		platoonNumber = initializePlatoonNumber();
		nCars = initializenCars();
		setPlatoonJoiner();

		for (int i=0; i<nCars; i++) {
			xLocationOut.push_back(0);
			yLocationOut.push_back(0);
			currentTimeOut.push_back(0);
			xLocationIn.push_back(0);
			yLocationIn.push_back(0);
			currentTimeIn.push_back(0); 

		}

	}
}

void BasePositionHelper::finish() {
	BaseApplLayer::finish();
}

std::string BasePositionHelper::getExternalId() {
	return mobility->getExternalId();
}

int BasePositionHelper::getId() {
	return myId;
}




int BasePositionHelper::getJoinerRoute() {
	return joinerRoute;
}

void BasePositionHelper::setJoinerRoute(int routeId) {
	joinerRoute = routeId;
}

int BasePositionHelper::initializenCars(){
	int nCars = 0;
	for (auto&& element : platoonSize)
		nCars = nCars + element;
	return nCars;
}

int BasePositionHelper::getnCars(){
	return nCars;
}


int BasePositionHelper::initializePlatoonNumber(){
	int PlatoonNumber = 0;
	for (auto&& element : platoonSize)
		if (element>1)
			PlatoonNumber = PlatoonNumber +1;
	return PlatoonNumber;
}


int BasePositionHelper::getHighestId() {
	return getnCars()-1;
}

int BasePositionHelper::getPosition() {
	return position;
}

int BasePositionHelper::getPlatoonNumber() {
	return platoonNumber;
}

// used for followers and leaders only
int BasePositionHelper::getLeaderId() {
	return leaderId;
}

int BasePositionHelper::getLeaderIdUnknown(int id) {
	for (int i=id; i>=0; i--){
		if (isLeaderUnknownId(i)){
			std::cout<<"leader id unknown is "<<i<<endl;
			return i;
		}
	}
	return -1;
}

int BasePositionHelper::getPlatoonJoiner(int platoonId) {
	return platoonJoiner[platoonId];
}

int BasePositionHelper::getLeaderIdAccPlatoon(int platoonId) {	
	int leaderId=0;
	for (int i=1; i<=platoonId; i++) {
		leaderId = leaderId + platoonSize[i-1];
	}
	return leaderId;
}

bool BasePositionHelper::isLeaderUnknownId(int id) {
	int i=0;
	if (id==0)
		return 1;
	else {
		for (int k=0; k<platoonSize.size()-1; k++) {
			i = i+platoonSize[k];
			//std::cout<<"i is "<<i<<" k is "<<k<<endl;
//			if (id==i && platoonSize[k+1]>1)	{
			if (id==i) 	{ // human vehicles are also considered leaders in this sense
				//std::cout<<"the id is "<<id<<" k is "<<k<<endl;
				return 1;
			}
			else if (i>id)
				return 0;
		}
		return 0;
	}
}

int BasePositionHelper::getPlatoonSizeInitial(int id) {
// returns platoon size acc. to leader id
	int i=0;
	if (id==0)
		return platoonSize[0];
	else {
		for (int k=0; k<platoonSize.size()-1; k++) {
			i = i+platoonSize[k];
			if (id==i)
				return platoonSize[k+1];
		}
		return 0;
	}
}



bool BasePositionHelper::isHumanUnknownId(int id) {
	int i=0;

	for (int k=0; k<nCars-1; k++) {
		i = i+platoonSize[k];
//			if (id==i && platoonSize[k+1]>1)	{
			if (id==i) 	{ // human vehicles are also considered leaders in this sense
			return 1;
		}
		else if (i>id)
			return 0;
	}
	return 0;
}


void BasePositionHelper::setRoundaboutLeader(int id){
	roundaboutLeader = id;
}

int  BasePositionHelper::getRoundaboutLeader(){
	return roundaboutLeader;
}


bool BasePositionHelper::isLeader() {
	return leader;
}

int BasePositionHelper::getFrontId() {
	return frontId;
}

int BasePositionHelper::getMemberId(int position) {
// will work on the specific platoon we concentrate on, in case not a joiner
	std::cout<<"in get memeber ID"<<endl;
	return getLeaderId()+position-1;
}

int BasePositionHelper::getMemberIdUnknown(int vehicleId, int position) {
// will work on the specific platoon we concentrate on, in case not a joiner
	std::cout<<"in get memeber ID unknown"<<endl;
	return getLeaderIdUnknown(vehicleId)+position-1;
}


int BasePositionHelper::getMemberPosition(int vehicleId) {
	// not a joiner
	std::cout<<"in get memeber position the vehicle id is "<<vehicleId<<endl;
	if (vehicleId != getnCars()){
		return vehicleId - getLeaderIdUnknown(vehicleId) + 1;
	}
	else return 1; //return 1 in case the position is of the joiner
}

int BasePositionHelper::getPlatoonId() {
	return platoonId;
}


int BasePositionHelper::getPlatoonLane() {
	return platoonLane;
}

bool BasePositionHelper::isInSamePlatoon(int vehicleId) {
	return platoonId == getPlatoonIdAccLeader(getLeaderIdUnknown(vehicleId));
}

int BasePositionHelper::getLanesCount() {
	return nLanes;
}

int BasePositionHelper::getPlatoonSize(int platoonId) {
	return platoonSize[platoonId];
}

int BasePositionHelper::getPlatoonRoute() {
	return platoonRoute[getPlatoonId()];
}

int BasePositionHelper::getPlatoonRouteStranger(int vehicleId) {
	return platoonRoute[getPlatoonIdAccLeader(getLeaderIdUnknown(vehicleId))];
}

int BasePositionHelper::getPlatoonIdAccLeader(int id){
	int i=0, k=0;
	if (id==i)
		return k;
	
	k=k+1;
	for (auto&& element : platoonSize) {
		i=i+element;
		if (id==i)
			return k;
		k=k+1;
	}
	return -1;
}


double BasePositionHelper::getYLocationOut(int vehicleId){
	return yLocationOut[vehicleId];
}

double BasePositionHelper::getXLocationOut(int vehicleId){
	return xLocationOut[vehicleId];
}

void BasePositionHelper::setYLocationOut(double yPoint, int vehicleId){
	yLocationOut[vehicleId] = yPoint;
}

void BasePositionHelper::setXLocationOut(double xPoint, int vehicleId){
	xLocationOut[vehicleId] = xPoint;
}

void BasePositionHelper::setTimeOut(double simTime1, int vehicleId){
	currentTimeOut[vehicleId] = simTime1;
}

double BasePositionHelper::getTimeOut(int vehicleId){
	return currentTimeOut[vehicleId];
}




double BasePositionHelper::getYLocationIn(int vehicleId){
	return yLocationIn[vehicleId];
}

double BasePositionHelper::getXLocationIn(int vehicleId){
	return xLocationIn[vehicleId];
}

void BasePositionHelper::setYLocationIn(double yPoint, int vehicleId){
	yLocationIn[vehicleId] = yPoint;
}

void BasePositionHelper::setXLocationIn(double xPoint, int vehicleId){
	xLocationIn[vehicleId] = xPoint;
}

void BasePositionHelper::setTimeIn(double simTime1, int vehicleId){
	currentTimeIn[vehicleId] = simTime1;
}

double BasePositionHelper::getTimeIn(int vehicleId){
	return currentTimeIn[vehicleId];
}




void BasePositionHelper::setId(int id) {
	myId = id;
}

void BasePositionHelper::setHighestId(int id) {
	highestId = id;
}

void BasePositionHelper::setPosition(int position) {
	this->position = position;
}

void BasePositionHelper::setPlatoonJoiner() {
	platoonJoiner.reserve(platoonSize.size());
	for (int i=0; i<platoonSize.size(); i++) {
		platoonJoiner.push_back(0);	
	}
}

void BasePositionHelper::updatePlatoonJoiner(int platoonNumber, int status) {
	platoonJoiner.at(platoonNumber) = status;
}

void BasePositionHelper::setLeaderId(int id) {
	leaderId = id;
}

void BasePositionHelper::setIsLeader(bool isLeader) {
	leader = isLeader;
}

void BasePositionHelper::setFrontId(int id) {
	frontId = id;
}

void BasePositionHelper::setPlatoonId(int id) {
	platoonId = id;
}

void BasePositionHelper::setPlatoonLane(int lane) {
	platoonLane = lane;
}

void BasePositionHelper::setLanesCount(int lanes) {
	nLanes = lanes;
}

void BasePositionHelper::setPlatoonSize(int platoonId, int size) {
	platoonSize[platoonId] = size;
}
