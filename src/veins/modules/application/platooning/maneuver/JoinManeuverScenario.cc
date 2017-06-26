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

#include <ctime>
#include <chrono>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "veins/modules/application/platooning/maneuver/JoinManeuverScenario.h"

Define_Module(JoinManeuverScenario);
std::clock_t begin = clock();

void JoinManeuverScenario::initialize(int stage) {
	//auto start = std::chrono::high_resolution_clock::now();

	std::cout<<"in initialize"<<endl;
	BaseScenario::initialize(stage);
	if (stage == 0) {

		//name the FSMs
		leaderFsm.setName("leaderFsm");
		followerFsm.setName("followerFsm");
		joinerFsm.setName("joinerFsm");
		leaverFsm.setName("leaverFsm");
		leaverFsm.setName("humanFsm");

	}

	if (stage == 1) {

		//lane where the platoon is driving
		// in the calculation of lanes a 4 lane configuration is taken into account. therefore when wishing to have 2 lanes, 
		// and driving on the most right lane, the number of lane will be 2 and not 0. 
		int platoonLane = 0;
		//std::cout<<"about to enter prepare maneuver"<<endl;
		prepareManeuverCars(platoonLane);
		//std::cout<<"ended prepare maneuver"<<endl;
		protocol = FindModule<BaseProtocol*>::findSubModule(getParentModule());
		//connect maneuver application to protocol
		protocol->registerApplication(MANEUVER_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"));
		//we are also interested in receiving beacons: the joiner must compute
		//its distance to the front vehicle while approaching it
		protocol->registerApplication(BaseProtocol::BEACON_TYPE, gate("lowerLayerIn"), gate("lowerLayerOut"));
	}

	//auto end = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	//std::cout << "JoinManeuverScenario::initialize " << duration << std::endl;
}


void JoinManeuverScenario::prepareManeuverCars(int platoonLane) {
	std::cout<<"*************in the prepare maneuver func"<<endl;
	std::vector<int> temp;
	int member_ID=0;
	int current_ID;
	current_ID = positionHelper->getId();
	double dis_from_begin =  traciVehicle->getDistanceFromRouteBegin();

	std::cout<<"current_ID = positionHelper->getId() "<<positionHelper->getId()<<endl;
	vehicleData.joinerId =  positionHelper->getnCars(); 
	std::cout<<"vehicleData.joinerId "<<vehicleData.joinerId<<endl;
	std::cout<<"positionHelper->getPlatoonSizeInitial(positionHelper->getId()) "<<positionHelper->getPlatoonSizeInitial(positionHelper->getId())<<endl;
	if (positionHelper->isLeaderUnknownId(positionHelper->getId()) && positionHelper->getPlatoonSizeInitial(positionHelper->getId()) >1 ) {

			std::cout<<"identified as a leader"<<endl;
			//this is the leader
			traciVehicle->setCruiseControlDesiredSpeed(50.0 / 3.6);
			traciVehicle->setActiveController(Plexe::ACC);
			traciVehicle->setFixedLane(platoonLane);		
			role = LEADER;


			positionHelper->setLeaderId(positionHelper->getId());
			positionHelper->setIsLeader(true);
			positionHelper->setPlatoonLane(platoonLane);
			positionHelper->setPlatoonId(positionHelper->getPlatoonIdAccLeader(positionHelper->getId()));
			std::cout<<"positionHelper->getPlatoonId()"<<positionHelper->getPlatoonId()<<endl;

			vehicleData.joinerId = -1;
			vehicleData.speed = 50/3.6;
			
			for (int i = 0; i < positionHelper->getPlatoonNumber(); i++) {
					vehicleData.formation.reserve(positionHelper->getPlatoonSize(i)+1);
					for (int j = 0; j< positionHelper->getPlatoonSize(i); j++) {
						temp.push_back(member_ID);	
						member_ID ++;
					}
					vehicleData.formation.push_back(temp);
					temp.clear();
			}
			//std::cout<<"************updated vehicleData.formation in prepare maneuver - leader ********************"<<endl;

		}
	else if (positionHelper->isLeaderUnknownId(positionHelper->getId()) && positionHelper->getPlatoonSizeInitial(positionHelper->getId()) ==1) {
			// this is a human vehicle because is a leader with 1 as platoon size
			Veins::TraCIColor color(240, 255, 255, 255);
			traciVehicle->setColor(color);
			std::cout<<"about to color in white"<<endl;

			traciVehicle->setCruiseControlDesiredSpeed(50.0 / 3.6);
			traciVehicle->setActiveController(Plexe::ACC);
			traciVehicle->setFixedLane(platoonLane);		
			role = HUMAN;

	}

	else if (current_ID < vehicleData.joinerId) {
			//these are the followers which are already in the platoon
			traciVehicle->setCruiseControlDesiredSpeed(50.0 / 3.6); 
			traciVehicle->setActiveController(Plexe::CACC);
			traciVehicle->setFixedLane(platoonLane);	
			role = FOLLOWER;
			
			positionHelper->setLeaderId(positionHelper->getLeaderIdUnknown(current_ID));
			positionHelper->setFrontId(positionHelper->getId() - 1);
			positionHelper->setIsLeader(false);
			positionHelper->setPlatoonLane(platoonLane);
			positionHelper->setPlatoonId(positionHelper->getPlatoonIdAccLeader(positionHelper->getLeaderIdUnknown(current_ID)));
			vehicleData.speed = 50/3.6;

			for (int i = 0; i < positionHelper->getPlatoonNumber(); i++) {
					vehicleData.formation.reserve(positionHelper->getPlatoonSize(i)+1);
					for (int j = 0; j< positionHelper->getPlatoonSize(i); j++) {
						temp.push_back(member_ID);	
						member_ID ++;
					}
					vehicleData.formation.push_back(temp);
					temp.clear();
			}
		}
	else if (current_ID == vehicleData.joinerId && (dis_from_begin < 100 || (dis_from_begin>790 && dis_from_begin<1000))) {
	
			std::cout<<"suppose to be a joiner, with the Id.... "<<current_ID<<endl;
			//std::cin.get();
			//this is the car which will join
			traciVehicle->setFixedLane(platoonLane); 
			role = JOINER;

			
			// Only temp definitions
			positionHelper->setLeaderId(0);
			positionHelper->setPlatoonId(positionHelper->getPlatoonIdAccLeader(positionHelper->getLeaderId()));
			positionHelper->setFrontId(-1);
			positionHelper->setIsLeader(false);
			positionHelper->setPlatoonLane(-1);

			vehicleData.speed = 50/3.6;
				
			startManeuver = new cMessage();
			std::cout<<"current time is "<<simTime()<<endl;
			if (dis_from_begin < 100)
   				scheduleAt(100, startManeuver);
			else 
				scheduleAt(simTime()+1, startManeuver);
			std::cout<<"scheduled msg for "<<simTime()+1<<endl;
	
	 }
	
	else if (current_ID == vehicleData.joinerId){

			//this is the car which will leave
			std::cout<<"suppose to be first leaver, with the Id.... "<<current_ID<<endl;
			traciVehicle->setCruiseControlDesiredSpeed(50/3.6);
			traciVehicle->setActiveController(Plexe::CACC);
			role = LEAVER;

			positionHelper->setIsLeader(false);

			startManeuver = new cMessage();
			//std::cout<<"current time is "<<simTime()<<endl;;
   			scheduleAt(simTime()+1, startManeuver);
			//std::cin.get();


	}

}


void JoinManeuverScenario::finish() {
	//std::cout<<"in finish"<<endl;
	if (startManeuver) {
		cancelAndDelete(startManeuver);
		startManeuver = 0;
	}

	BaseScenario::finish();
	endSimulation();

}

ManeuverMessage *JoinManeuverScenario::generateMessage() {

	std::cout<<"**** in generate msg"<<endl;
	ManeuverMessage *msg = new ManeuverMessage();
	std::cout<<"positionHelper->getId() "<<positionHelper->getId()<<endl;
	msg->setVehicleId(positionHelper->getId());
	std::cout<<"positionHelper->getPlatoonId()"<<positionHelper->getPlatoonId()<<endl;
	msg->setPlatoonId(positionHelper->getPlatoonId());
	std::cout<<"positionHelper->getPlatoonLane()"<<positionHelper->getPlatoonLane()<<endl;
	msg->setPlatoonLane(positionHelper->getPlatoonLane());
	std::cout<<"vehicleData.speed"<<vehicleData.speed<<endl;
	msg->setPlatoonSpeed(vehicleData.speed);
	return msg;
}

void JoinManeuverScenario::handleSelfMsg(cMessage *msg) {
	std::cout<<"**** in handle self msg"<<endl;
	//this takes car of feeding data into CACC and reschedule the self message
	BaseScenario::handleSelfMsg(msg);


	if (msg == startManeuver) {
		
		//handleJoinerMsg(msg);
		
		double dis = traciVehicle->getDistanceFromRouteBegin();

		if (dis < 400 || (dis>=790 && dis<1000)) {
			std::cout<<"about to enter joiner msg"<<endl;
			std::cout<<"msg is "<<msg;
			handleJoinerMsg(msg);
		}
		else
		{
				std::cout<<"about to enter leaver msg"<<endl;
				handleLeaverMsg(msg);
		}
	}
}

void JoinManeuverScenario::handleLowerMsg(cMessage *msg) {	
	std::cout<<"**** in handle lower msg the id is "<<positionHelper->getId()<<endl;


		switch (role) {
			case LEADER:
				handleLeaderMsg(msg);
				break;
			case FOLLOWER:
				    handleFollowerMsg(msg);
					break;
			case JOINER:
			    	handleJoinerMsg(msg);
					break;
			case LEAVER:
					handleLeaverMsg(msg);
					break;
			case HUMAN:
					handleHumanMsg(msg);
					break;

			default:
				ASSERT(false);
				break;	
		};
}
	
void JoinManeuverScenario::sendUnicast(cPacket *msg, int destination) {
	std::cout<<"********in send uni cast"<<endl;
	UnicastMessage *unicast = new UnicastMessage("", MANEUVER_TYPE);
	unicast->setDestination(destination);
	unicast->setChannel(Channels::CCH);
	unicast->encapsulate(msg);
	sendDown(unicast);
	
}

void JoinManeuverScenario::handleLeaderMsg(cMessage *msg) {
	  // my addition in order to keep stable leader speed

	std::cout<<"****************in the handle leader msg func"<<endl;

	std::cout<<"traciVehicle->getDistanceFromRouteBegin() "<<traciVehicle->getDistanceFromRouteBegin()<<endl;
	//this message can be a self message, or a unicast message
	//with an encapsulated beacon or maneuver message
	//traciVehicle->setSpeedMode(0);
	ManeuverMessage *maneuver = 0;
	PlatooningBeacon *beacon = 0;
	cPacket *encapsulated = 0;
	//maneuver message to be sent, if needed
	ManeuverMessage *toSend;
	bool did_not_enter_close = 0;
	bool did_not_enter_far = 0;

	int will_crash = 0;
	int centerPointType = 2; // 2 stands for the lower conflict intersection, 1 - stands for the upper conflict intersection
	//int will_crash_case2 = 0;

	int is_platoon = 1;

	//first check if this is a unicast message, and in case if it is a beacon or a maneuver
	UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
	if (unicast) {
		encapsulated = unicast->decapsulate();
		maneuver = dynamic_cast<ManeuverMessage *>(encapsulated);
		beacon = dynamic_cast<PlatooningBeacon *>(encapsulated);
	}
	

	//check current leader status

    std::cout<<"positionHelper->getId() "<<positionHelper->getId()<<endl;

	/*if (simTime()>234 &&  positionHelper->getId()==45)	{
		traciVehicle->setCruiseControlDesiredSpeed(0.0 / 3.6);
		std::cout<<"45 platoon is supposed to stop now"<<endl;
		std::cin.get();
	}*/




	/*if (beacon)	{
		std::cout<<"beacon->getVehicleId() "<<beacon->getVehicleId()<<endl;
		std::cout<<"positionHelper->getPlatoonSize(beacon->getVehicleId()) "<<positionHelper->getPlatoonSize(beacon->getVehicleId())<<endl;
		std::cin.get();
}*/
	if (beacon && (positionHelper->isLeaderUnknownId(beacon->getVehicleId()) == 1)){  //&& positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId()))>1 )) {
	
		std::cout<<"%*$%*$*%$* in leader indentified a beacon %$%$#%#%"<<endl;
		//std::cout<<"entered the if after identifying a beacon"<<endl;

		std::cout<<"the beacon is from "<<beacon->getVehicleId()<<endl;

		std::list<std::string> roundabout_platoon_ls = traciVehicle->getPlannedRoadIdsStranger(beacon->getVehicleId());
		std::list<std::string>::iterator i,j,dup1,dup3, dup4;

				//get my position
		Veins::TraCICoord traciPosition = mobility->getManager()->omnet2traci(mobility->getCurrentPosition());
		Coord position(traciPosition.x, traciPosition.y);
		//get front vehicle position
		Coord frontPosition(beacon->getPositionX(), beacon->getPositionY(), 0);

		double distance = position.distance(frontPosition);
		std::cout<<"distnace is "<<distance<<endl;


		// if approaching the roundabout but not in the roundabout
		std::string enteringRoad = traciVehicle->getRoadId();
		std::cout<<"entering road "<<enteringRoad<<endl;
//		if (((position.x >= 7284 && position.x < 7287 && position.y >= 600 && position.y < 784) || 
//			(position.x >= 7300 && position.x < 7450 && position.y >= 800 && position.y < 804)) && 

		if (((position.x >= 7284 && position.x < 7287 && position.y >= 684 && position.y < 784) || 
			(position.x >= 7300 && position.x < 7400 && position.y >= 800 && position.y < 804)) && 
			(enteringRoad.compare("gneE47") && enteringRoad.compare("gneE48") && enteringRoad.compare("gneE49") && enteringRoad.compare("gneE50") &&
			enteringRoad.compare(":gneJ54_2") && enteringRoad.compare(":gneJ54_1") &&  enteringRoad.compare(":gneJ55_0") &&  enteringRoad.compare(":gneJ56_0") && 
			enteringRoad.compare(":gneJ6_2") && enteringRoad.compare(":gneJ6_0") && enteringRoad.compare(":gneJ6_3") && enteringRoad.compare(":gneJ6_1") ))
		{

			
			if (distance < sqrt(116*116*2)) { // if are relatively close to one another: 100 m from the roundabout
			
			double inRBdistance=0, MyinRBdistance = 0, MyFrontVehicleDistance=0;
			std::string roadFirst = traciVehicle->getRoadIdStranger(beacon->getVehicleId());

			Coord centerPointLow(7285.33, 783.5, 0);  // the lower enterance
			Coord centerPointHigh(7302, 801.5, 0);   // the upper enterance

			double distanceToRoundabout = fmin(centerPointLow.distance(frontPosition),centerPointHigh.distance(frontPosition));
			//std::cout<<"in the if"<<endl;
			double entering_dis = fmin(centerPointLow.distance(position),centerPointHigh.distance(position));
			// if inside the roundabout or there is a vehicle which is closer to the roundabout than me
			std::cout<<"entering_dis "<<entering_dis<<endl;
			std::cout<<"distanceToRoundabout "<<distanceToRoundabout<<endl;
			 // the distance is small but not in the roundabout
				if (roadFirst.compare("gneE47") && roadFirst.compare("gneE48") && roadFirst.compare("gneE49") && roadFirst.compare("gneE50") &&
					roadFirst.compare(":gneJ54_2") && roadFirst.compare(":gneJ54_1") && roadFirst.compare(":gneJ54_0") &&  roadFirst.compare(":gneJ55_0") &&  roadFirst.compare(":gneJ56_0") && 
					roadFirst.compare(":gneJ6_2") && roadFirst.compare(":gneJ6_0") && roadFirst.compare(":gneJ6_3") && roadFirst.compare(":gneJ6_1") && roadFirst.compare("edge_0_8")) {

						std::cout<<"not in the roundabout"<<endl;
						int stopped_by_human_turning_right = 0;
						if ((positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId()))==1) && (!roadFirst.compare("gneE11"))) {// if stopped not by a platoon
							stopped_by_human_turning_right = 1;
						}
						if (stopped_by_human_turning_right==0){

							//&& roadFirst.compare("edge_0_8")
							if (entering_dis>distanceToRoundabout) {	// the addition of the distance is meant to prevent from platoons to stop too earlier 
								// infront of the roundabout

								Coord formerPosition(positionHelper->getXLocationOut(beacon->getVehicleId()), positionHelper->getYLocationOut(beacon->getVehicleId()),0);
								std::cout<<"former position "<<positionHelper->getXLocationOut(beacon->getVehicleId())<<" "<<positionHelper->getYLocationOut(beacon->getVehicleId())<<endl;
								std::cout<<"position "<<position.x<<" "<<position.y<<endl;
							
								double currentSpeed = position.distance(formerPosition) /(simTime().dbl() - positionHelper->getTimeOut(beacon->getVehicleId()));
								positionHelper->setXLocationOut(position.x,beacon->getVehicleId());
								positionHelper->setYLocationOut(position.y,beacon->getVehicleId());
								positionHelper->setTimeOut(simTime().dbl(), beacon->getVehicleId());
								if (position.x >= 7284 && position.x < 7287)  // the lower enterance
									centerPointType = 2;
								else centerPointType = 1;
							
								std::cout<<"currentSpeed "<<currentSpeed<<endl;
								int roundabout_route_outside= 2; //2 for the vertical route; 1 for the horizontal one
								
								////// to print center point type and roundabout route outside!!!!!!!!!!!!!
									if (!(roadFirst).compare("-edge_0_8")) {
										roundabout_route_outside= 1;
									}
								
								std::cout<<"centerPointType "<<centerPointType<<endl;
								std::cout<<"roundabout_route_outside "<<roundabout_route_outside<<endl;
								if (currentSpeed> 16/3.6 && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8") && ((centerPointType == 2 && roundabout_route_outside == 1) || (centerPointType == 1 && roundabout_route_outside == 2))) { // the comparison to gneE11 is for preventing exiting platoon to stop entering platoon, entering using the same exit
					
									will_crash = 1; // another platoon is about to enter the roundabout
									std::cout<<"will crash"<<endl;
								}


								else if (currentSpeed>1/3.6 && (((centerPointLow.distance(position) <16) && centerPointType == 2 && roundabout_route_outside == 1) || ((centerPointHigh.distance(position) <16) && centerPointType == 1 && roundabout_route_outside == 2)) && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8")) {
									will_crash = 1; // another platoon is about to enter the roundabout
									std::cout<<"will crash for slow vehicles"<<endl;
								}
								else if (currentSpeed<1/3.6 && (((centerPointLow.distance(position) <4.5) && centerPointType == 2 && roundabout_route_outside == 1) || ((centerPointHigh.distance(position) <4.5) && centerPointType == 1 && roundabout_route_outside == 2)) && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8")) {
									will_crash = 1; // another platoon is about to enter the roundabout
									std::cout<<"will crash for slow vehicles"<<endl;
								}


							}	

						}
				}
				else if (!roadFirst.compare("edge_0_8")) { // if just left the roundabout
					std::cout<<"in the edge 0 8 case"<<endl;
					double interDistance = 6;
					double roundabout_radius = 18;
					if (position.x >= 7284 && position.x < 7287) { // conflict point is at the lower enterance
						//the distance from conflict point is the distance in edge_0_8 plus a quarter cicrcle
												
						//double distance_from_conflict_point = centerPointHigh.distance(frontPosition) + (0.25)*roundabout_radius*M_PI; 
						double roundabout_platoonLength = positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId())) * 4 + 
						(positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId())) - 1) * interDistance;
						double crosses_conflict_point = roundabout_platoonLength - centerPointHigh.distance(frontPosition) - (0.25)*roundabout_radius*M_PI;
						if (crosses_conflict_point>0) {
							Coord formerPosition(positionHelper->getXLocationOut(beacon->getVehicleId()), positionHelper->getYLocationOut(beacon->getVehicleId()),0);
							std::cout<<"former position "<<positionHelper->getXLocationOut(beacon->getVehicleId())<<" "<<positionHelper->getYLocationOut(beacon->getVehicleId())<<endl;
							std::cout<<"position "<<position.x<<" "<<position.y<<endl;
							
							double currentSpeed = position.distance(formerPosition) /(simTime().dbl() - positionHelper->getTimeOut(beacon->getVehicleId()));

						  if (centerPointLow.distance(position)/currentSpeed < crosses_conflict_point/(30/3.6)) {

						//if (distance_from_conflict_point<roundabout_platoonLength+10){

							will_crash = 1;
						  }
						}
					}
				}


				else { // in the roundabout
					int roundabout_route = 2; //2 vertical route, 1 horizontal route
					if (positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId()))>1) {// if stopped by a platoon
						for(j=roundabout_platoon_ls.begin(); j!=roundabout_platoon_ls.end(); j++){
							if (!(*j).compare("edge_0_8")) {
								roundabout_route= 1;
								break;
							}
						}
					}
					else {   // the only case where a single vehicle endangers a platoon is when a single vehicle comes from upper conflict point and a platoon tries to enter from the lower one
							// therefore in all cases a single vehicle can always be considered to be of type 1 (horizontal route)
								
									roundabout_route= 1;
									is_platoon = 0;
						   }
							
					
				
						
					if (position.x >= 7284 && position.x < 7287) { // the lower enterance
						Coord centerPoint(7285.33, 783.5, 0);
						
						//entering_dis = position.distance(centerPoint);
					    inRBdistance = centerPoint.distance(frontPosition);

						if (roundabout_route == 1 && frontPosition.x < position.x )
							did_not_enter_close = 1;
						else if (roundabout_route == 1 && (!roadFirst.compare("gneE48") || !roadFirst.compare(":gneJ6_0")))
							did_not_enter_far = 1;

						// for the case where the upper enterance threatens the platoon entering from the lower one//
						Coord AnotherCenterPoint(7302, 801.5, 0);   // the upper enterance
						MyinRBdistance = AnotherCenterPoint.distance(position); // calculating entering vehicle position with respect to the platoon in the upper junction
						// this vehicle has not yet entered the upper enterance
						MyFrontVehicleDistance = AnotherCenterPoint.distance(frontPosition);
					}
					
					else { 
						// center point (7302, 801.5)
						Coord centerPoint(7302, 801.5, 0);   // the upper enterance
					    //entering_dis = position.distance(centerPoint);
						inRBdistance = centerPoint.distance(frontPosition);
						centerPointType = 1;
						if (roundabout_route == 2 && (!roadFirst.compare("gneE47") || !roadFirst.compare(":gneJ54_0")))
							did_not_enter_close = 1;
						}
				

						// the time it would take the entering platoon to reach the crash point
						// real inner radius is 16 m, but in the lane itself, I assumed it is 2 meters bigger 								
					double roundabout_radius = 18;
					double radian_angle;
					double arc_sin = (inRBdistance/2)/roundabout_radius;
					std::cout<<"arc_sin "<<arc_sin<<endl;

					if (arc_sin>1) arc_sin = 1;
					else if (arc_sin<-1) arc_sin = -1;
					
					
					// in order to calculate the right distance from conflict point
					std::cout<<"roundabout route is "<<roundabout_route<<endl;
					if ((roundabout_route == 2 && centerPointType==1 && (!roadFirst.compare("gneE50")  ||  !roadFirst.compare(":gneJ54_1"))) ||
					   ((roundabout_route == 2 && centerPointType==2) && (!roadFirst.compare("gneE50")|| !roadFirst.compare("gneE49") || !roadFirst.compare(":gneJ54_1"))) ||	
					   ((roundabout_route == 1 && centerPointType==1) && (!roadFirst.compare("gneE50") ||!roadFirst.compare("gneE47") || !roadFirst.compare(":gneJ54_2") || !roadFirst.compare(":gneJ6_2"))) ||	
					   (roundabout_route == 1 && centerPointType==2 && !roadFirst.compare("gneE48")))
					   					
						radian_angle = 2*M_PI-2*(asin(arc_sin)); // not the measured angle but its complementary
					
					else    radian_angle = 2*(asin(arc_sin)); 
					std::cout<<"radian_angle "<<radian_angle<<endl;
					
					double round_distance = (radian_angle/(2*M_PI))*roundabout_radius*2*M_PI;

								
					bool entering_platoon_with_joiner = 0;
					if (positionHelper->getPlatoonJoiner(positionHelper->getPlatoonIdAccLeader(positionHelper->getId()))==1)
						entering_platoon_with_joiner = 1;

					double interDistance = 6;
					double entering_platoonLength = positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(positionHelper->getId())) * 4 + 
						(positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(positionHelper->getId())) - 1) * interDistance + entering_platoon_with_joiner *16;
					// relating to the platoon once in the roundabout
					// I assumed that the joiner is more distance than "regular" followers in the platoon
					

					//double entering_leader_time = entering_dis/(50/3.6);
					//double entering_platoon_time = entering_platoonLength / (50/3.6);
				
					Coord formerPosition(positionHelper->getXLocationIn(beacon->getVehicleId()), positionHelper->getYLocationIn(beacon->getVehicleId()),0);
					
					double currentSpeed = position.distance(formerPosition) /(simTime().dbl() - positionHelper->getTimeIn(beacon->getVehicleId()));
					std::cout<<"former position "<<positionHelper->getXLocationIn(beacon->getVehicleId())<<" "<<positionHelper->getYLocationIn(beacon->getVehicleId())<<endl;
					std::cout<<"position "<<position.x<<" "<<position.y<<endl;
					positionHelper->setXLocationIn(position.x,beacon->getVehicleId());
					positionHelper->setYLocationIn(position.y,beacon->getVehicleId());
					positionHelper->setTimeIn(simTime().dbl(),beacon->getVehicleId());


					double entering_leader_time = (-(currentSpeed/3.6) + sqrt((currentSpeed/3.6)*(currentSpeed/3.6)+3*entering_dis))/1.5;
					double entering_platoon_time =  (-(currentSpeed/3.6) + sqrt((currentSpeed/3.6)*(currentSpeed/3.6)+3*entering_platoonLength))/1.5;

					double total_entering_time = entering_leader_time + entering_platoon_time;



					double roundabout_platoonLength = positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId())) * 4 + 
						(positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId())) - 1) * interDistance;
				

					double roundabout_leader_time = round_distance/(30/3.6);
					double roundabout_platoon_time = 0;
					if (did_not_enter_far == 1 || did_not_enter_close == 1) {
						std::cout<<"did not enter "<<endl;
						//assumption - the platoon drives at 50 kph but at the roundabout its speed is 30!!!
						roundabout_platoon_time = roundabout_platoonLength/(30/3.6);
					}
					else {
						// already in the roundabout
						std::cout<<"did enter "<<endl;
						roundabout_leader_time = 0;
						roundabout_platoon_time = (roundabout_platoonLength - round_distance) /(30/3.6);
					}

					//double total_roundabout_platoon_time = roundabout_leader_time + roundabout_platoon_time;

					// to prevent cases where entering platoons carsh into partly entered platoons
					int IdLast;
					//std::string roadLast;
					int roundabout_platoon_size = positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId()));
					//std::cout<<"rondabout platoon size"<<roundabout_platoon_size<<endl;
					if (positionHelper->getPlatoonJoiner(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId()))==0)
					// if there is no joiner in this platoon
					IdLast = positionHelper->getMemberIdUnknown(beacon->getVehicleId(), roundabout_platoon_size);
					else IdLast = roundabout_platoon_size +1;

					//roadLast = traciVehicle->getRoadIdStranger(IdLast);
					//std::cout<<"the road of the last vehicle "<<roadLast<<endl;

					std::cout<<"enteringRoad "<<enteringRoad<<endl;

					if (!(enteringRoad).compare("-edge_0_8") || !(enteringRoad).compare("gneE11")){


						//std::cout<<"currentSpeed "<<currentSpeed<<endl;
						int speed_or_location_stop = 0;
						std::cout<<"centerPointLow.distance(position) "<<centerPointLow.distance(position)<<endl;
						std::cout<<"centerPointHigh.distance(position) "<<centerPointHigh.distance(position)<<endl;
						std::cout<<"centerPointType "<<centerPointType<<endl;
						std::cout<<"currentSpeed "<<currentSpeed<<endl;

						if (currentSpeed> 16/3.6 && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8") && ((centerPointType == 2 && roundabout_route ==1 && is_platoon == 1) ||  (centerPointType == 1 && roundabout_route ==2)))
							speed_or_location_stop = 1;

						else if (currentSpeed>1/3.6 && (((centerPointLow.distance(position) <16) && centerPointType == 2 && roundabout_route ==1 && is_platoon == 1) || ((centerPointHigh.distance(position) <16) && centerPointType == 1 && roundabout_route ==2)) && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8")) 
							speed_or_location_stop = 1;

						else if (currentSpeed<1/3.6 && (((centerPointLow.distance(position) <4.5) && centerPointType == 2 && roundabout_route ==1 && is_platoon == 1) || ((centerPointHigh.distance(position) <4.5) && centerPointType == 1 && roundabout_route ==2)) && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8"))
							speed_or_location_stop = 1;



						if (speed_or_location_stop==1) {
							
							std::cout<<"did_not_enter_far "<<did_not_enter_far<<endl;
							std::cout<<"did_not_enter_close "<<did_not_enter_close<<endl;
							// if tries to enter while the platoon is still not in the intersection	
							 if ((did_not_enter_far == 1 || did_not_enter_close == 1) && (1+total_entering_time >roundabout_leader_time)) { 
								std::cout<<"will crash on arrival"<<endl;
								will_crash = 1;
							}
							// if tries to enter after the leader of the roundabout platoon has passed the intersetion
							else if (((did_not_enter_far == 0) && (did_not_enter_close == 0)) && (entering_leader_time < roundabout_platoon_time+1.5)) {
								std::cout<<"will crash while roundabout platoon is evacuating"<<endl;
								will_crash = 1;
							}

							else if (((did_not_enter_far == 1) && (did_not_enter_close == 1)) && (roundabout_leader_time < entering_platoon_time )) {
								std::cout<<"will crash while entering platoon is evacuating"<<endl;
								will_crash = 1;
							}
							else if (centerPointType == 2) { // for the case where might crash in the upper enterance although the conflict point is the lower one

								arc_sin = (MyFrontVehicleDistance/2)/roundabout_radius;
								if (arc_sin>1) arc_sin = 1;
								else if (arc_sin<-1) arc_sin = -1;
								if (roadFirst.compare("gneE50")) // if has not passed half a roundabout
									radian_angle = 2*(asin(arc_sin));
								else radian_angle = 2*M_PI-2*(asin(arc_sin));
									//std::cout<<"radian_angle "<<radian_angle<<endl;	
								// the time it would take the platoon in the roundabout to reach the crash point
								double round_distance = (radian_angle/(2*M_PI))*roundabout_radius*2*M_PI;
								double new_roundabout_platoon_time = (roundabout_platoonLength - round_distance) /(15/3.6);
								double new_entering_leader_time = entering_leader_time + 0.5*M_PI*
									(-(currentSpeed/3.6) + sqrt((currentSpeed/3.6)*(currentSpeed/3.6)+3*roundabout_radius))/1.5;

								std::cout<<"entering_leader_time "<<entering_leader_time<<endl;
								std::cout<<"new_roundabout_platoon_time "<<new_roundabout_platoon_time<<endl;
								std::cout<<"new_entering_leader_time "<<new_entering_leader_time<<endl;

							
							}
						}

					}

					std::cout<<"distance "<<distance<<endl;
					std::cout<<"roundabout_leader_time "<<roundabout_leader_time<<endl;
					std::cout<<"roundabout_platoon_time "<<roundabout_platoon_time<<endl;
					std::cout<<"round_distance "<<round_distance<<endl;
					std::cout<<"roundabout_platoonLength "<<roundabout_platoonLength<<endl;
					std::cout<<"inRBdistance "<<inRBdistance<<endl;
					std::cout<<"entering_dis "<<entering_dis<<endl;
					std::cout<<"entering_leader_time "<<entering_leader_time<<endl;
					std::cout<<"entering_leader_time "<<entering_leader_time<<endl;	
					std::cout<<"total_entering_time "<<total_entering_time<<endl;
					std::cout<<"positionHelper->getRoundaboutLeader() "<<positionHelper->getRoundaboutLeader()<<endl;
					std::cout<<"beacon->getPositionY() "<<beacon->getPositionY()<<endl;
					}
					if (will_crash) {
					
							std::cout<<"suppose to stop now"<<endl;
							positionHelper->setRoundaboutLeader(positionHelper->getLeaderIdUnknown(beacon->getVehicleId()));
													
							traciVehicle->setCruiseControlDesiredSpeed(0.0 / 3.6);

							/*if (simTime()>230 &&  positionHelper->getId()==45)
								std::cin.get();*/
							//toSend = generateMessage();
							//toSend->setMessageType(LM_STOPPED_AT_ROUNDABOUT);
							//sendUnicast(toSend, -1);				

							/*if (simTime()>230 && positionHelper->getId()==45){
								std::cout<<"in leader positionHelper->getLeaderId() "<<positionHelper->getLeaderId()<<endl;
								std::cin.get();
							}*/
					}
		
					else if (positionHelper->getRoundaboutLeader() == positionHelper->getLeaderIdUnknown(beacon->getVehicleId())) {
						traciVehicle->setCruiseControlDesiredSpeed(50.0 / 3.6);

						/*if (simTime()>230 &&  positionHelper->getId()==45) {
							std::cout<<"suppose to continue"<<endl;
								std::cin.get();
						}*/

							//toSend = generateMessage();
							//toSend->setMessageType(LM_WAITED_ENOUGH);
							//sendUnicast(toSend, -1);				

				}
			}

		}

	

		/*if ((simTime()>84 && positionHelper->getId()==12)){ //&&(beacon->getVehicleId()==27)) {
			std::cin.get();
		}*/

	}


					

	/*if (simTime()>7 && positionHelper->getId()==33) {// &&((beacon->getVehicleId()==56)) ){
			std::cin.get();
		}*/


	/*if (simTime()>46 && positionHelper->getId()==36 && ((beacon->getVehicleId()==28)))
	std::cin.get();*/
	/*if (simTime()>85 && positionHelper->getId()==42)
		std::cin.get();	*/
	
	FSM_Switch(leaderFsm) {
		case FSM_Exit(LS_INIT): {
			FSM_Goto(leaderFsm, LS_LEADING);
			break;
		}
		case FSM_Exit(LS_LEADING): {
			 std::cout<<"in leading"<<endl;
			//when getting a message, and being in the LEADING state, we need
			//to check if this is a join request. if not check if this is a leave request, if not just ignore it	


			 if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()){
				std::cout<<"got a maneuver msg"<<endl;
				if (maneuver->getMessageType() == JM_REQUEST_JOIN) {
					std::cout<<"*****got a join request****"<<endl;
					toSend = generateMessage();
					toSend->setMessageType(LM_MOVE_IN_POSITION);
					//this will be the front vehicle for the car which will join
					toSend->setFrontVehicleId(positionHelper->getLeaderId() + positionHelper->getPlatoonSize(positionHelper->getPlatoonId()));
					//save some data. who is joining?										
					vehicleData.joinerId = maneuver->getVehicleId();
					//send a positive ack to the joiner
					sendUnicast(toSend, vehicleData.joinerId);
					FSM_Goto(leaderFsm, LS_WAIT_JOINER_IN_POSITION);
				}

				else if (maneuver->getMessageType() == LVM_REQUEST_LEAVE) {
					std::cout<<"****got a leave request****"<<endl;
					
					toSend = generateMessage();
					toSend->setMessageType(LM_LEAVE_PLATOON);
					sendUnicast(toSend, vehicleData.joinerId);

					FSM_Goto(leaderFsm, LS_WAIT_LEAVE);
					
				}
			}
			break;
		}
		case FSM_Exit(LS_WAIT_LEAVE): {	
			std::cout<<"in LS wait to leave"<<endl;
			if (maneuver) {
				std::cout<<"maneuver->getPlatoonId() "<<maneuver->getPlatoonId()<<endl;
				std::cout<<"positionHelper->getPlatoonId() "<<positionHelper->getPlatoonId()<<endl;
			}

			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				std::cout<<"inside the if about to update formation of the leader"<<endl;
				if (maneuver->getMessageType() == LVM_LEFT_PLATOON) {									
					for (int i = 0; i < positionHelper->getPlatoonNumber(); i++) {
						for (int j = 0; j <vehicleData.formation[i].size(); j++) {
							if (vehicleData.formation[i][j] == vehicleData.leaverId)
								vehicleData.formation[i].erase(vehicleData.formation[i].begin()+j);
						}						
					}
					// still needs to update set platoon formation array size and platoon formation in maneuver message
					toSend = generateMessage();
					toSend->setMessageType(LM_UPDATE_FORMATION);
					toSend->setFrontVehicleId(-1);
					std::cout<<"was supposed to sent update msg to leaver and followers"<<endl;
					std::cout<<"the platoon id is "<<positionHelper->getPlatoonId()<<endl;
				
					positionHelper->updatePlatoonJoiner(positionHelper->getPlatoonId(), 0);		
					toSend->setPlatoonFormationArraySize(positionHelper->getnCars());
					int k=0;
					for (int i = 0; i <  positionHelper->getPlatoonNumber(); i++) {								
						for (unsigned int j = 0; j < positionHelper->getPlatoonSize(i); j++) {
							toSend->setPlatoonFormation(k, vehicleData.formation[i][j]);
							k++;
						}				
					}				
					//send to all vehicles
					sendUnicast(toSend, -1);				
					FSM_Goto(leaderFsm, LS_LEADING);
				}
				//std::cin.get();
			}
			break;
		}
		case FSM_Exit(LS_WAIT_JOINER_IN_POSITION): {
			std::cout<<"wait joiner in position"<<endl;
			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()){
				//the joiner is now in position and is ready to join
				if (maneuver->getMessageType() == JM_IN_POSITION) {

					//tell him to join the platoon
					toSend = generateMessage();
					toSend->setMessageType(LM_JOIN_PLATOON);
					sendUnicast(toSend, vehicleData.joinerId);

					FSM_Goto(leaderFsm, LS_WAIT_JOINER_TO_JOIN);
				}
			}
			break;
		}
		case FSM_Exit(LS_WAIT_JOINER_TO_JOIN): {
			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()){
				//the joiner has joined the platoon
				std::cout<<"positionHelper->getPlatoonId() "<<positionHelper->getPlatoonId()<<endl;
				if (maneuver->getMessageType() == JM_IN_PLATOON) {
					//add the joiner to the list of vehicles in the platoon ======>  I changed that to be more general
					std::cout<<"in wait joiner to join positionHelper->getId() "<<positionHelper->getId()<<endl;			
					positionHelper->updatePlatoonJoiner(positionHelper->getPlatoonId(), 1);
					vehicleData.formation[positionHelper->getPlatoonId()].push_back(vehicleData.joinerId);	
					toSend = generateMessage();
					toSend->setMessageType(LM_UPDATE_FORMATION);
					toSend->setPlatoonFormationArraySize(positionHelper->getnCars()+1);   
					//std::cout<<"vehicleData.formation[positionHelper->getPlatoonId()].size() "<<vehicleData.formation[positionHelper->getPlatoonId()].size()<<endl;
					//std::cout<<"send update formation request"<<endl;
					int k=0;
					for (unsigned int i = 0; i <positionHelper->getPlatoonNumber(); i++) {
						for (unsigned int j = 0; j <vehicleData.formation[i].size(); j++) {	
							toSend->setPlatoonFormation(k, vehicleData.formation[i][j]);
							k++;
						}						
					}
					//send to all vehicles
					sendUnicast(toSend, -1);

					FSM_Goto(leaderFsm, LS_LEADING);
				}
				//std::cin.get();
			}
			break;
		}
	}

	if (encapsulated) {
		delete encapsulated;
	}
	if (unicast) {
		delete unicast;
	} 

}


void JoinManeuverScenario::handleJoinerMsg(cMessage *msg) {
	double dis_from_begin = traciVehicle->getDistanceFromRouteBegin();
	std::cout<<"**************in the handle joiner msg func"<<endl;
	std::cout<<"traciVehicle->getDistanceFromRouteBegin() "<<dis_from_begin<<endl;
    std::cout<<"positionHelper->getId() "<<positionHelper->getId()<<endl;
	
	//this message can be a self message, or a unicast message
	//with an encapsulated beacon or maneuver message
	ManeuverMessage *maneuver = 0;
	PlatooningBeacon *beacon = 0;
	cPacket *encapsulated = 0;
	//maneuver message to be sent, if needed
	ManeuverMessage *toSend;

	//first check if this is a unicast message, and in case if it is a beacon or a maneuver
	UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
	if (unicast) {
		encapsulated = unicast->decapsulate();
		maneuver = dynamic_cast<ManeuverMessage *>(encapsulated);
		beacon = dynamic_cast<PlatooningBeacon *>(encapsulated);
	}

	std::cout<<"*msg "<<*msg<<endl;
	std::cout<<"another msg"<<endl;
	//check current joiner status
	FSM_Switch(joinerFsm) {
		std::cout<<"in the joiner case"<<endl;
		//init state, just move to the idle state
		case FSM_Exit(JS_INIT): {
			std::cout<<"in the joiner init state"<<endl;

			FSM_Goto(joinerFsm, JS_IDLE);
			break;
		}

		case FSM_Exit(JS_IDLE): {
			std::cout<<"in the joiner idle state"<<endl;
			traciVehicle->slowDown(0,100);

			//if this is a self message triggering the beginning of procedure, then ask for joining	
			//std::cout<<"msg is "<<msg<<endl;
			//std::cout<<"was supposed to stop now"<<endl;
			if (msg == startManeuver) {	
				//std::cout<<"going to wait to send request"<<endl;
				FSM_Goto(joinerFsm, JS_WAIT_TO_SEND_REQUEST);
			}  
			break;
		}
		
		case FSM_Exit(JS_WAIT_TO_SEND_REQUEST): {
				//std::cout<<"was supposed to stop now"<<endl;
				std::cout<<"in the wait to send request"<<endl;	
				traciVehicle->slowDown(0,100);

				if (beacon && positionHelper->isLeaderUnknownId(beacon->getVehicleId()) == 1 && positionHelper->getPlatoonSizeInitial(beacon->getVehicleId()) >1) {
					//std::cout<<"entered the if after identifying a beacon"<<endl;
					std::cout<<"the beacon is from "<<beacon->getVehicleId()<<endl;
					
					//get front vehicle position
					Coord behindPosition(beacon->getPositionX(), beacon->getPositionY(), 0);
					//get my position
					Veins::TraCICoord traciPosition = mobility->getManager()->omnet2traci(mobility->getCurrentPosition());
					Coord position(traciPosition.x, traciPosition.y);

					int beaconPlatoonSize = positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(positionHelper->getLeaderIdUnknown(beacon->getVehicleId())));
					double interDistance = 7;
					double platoonLength = beaconPlatoonSize * 4 + (positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(positionHelper->getLeaderIdUnknown(beacon->getVehicleId()))) - 1) * interDistance;
					//compute distance (-4 because of vehicle length)
					double distance = position.distance(behindPosition) - platoonLength;
					//if we are in position, tell the leader about that

					//std::cout<<"from wait to send request the distance is "<<distance<<endl;
					
					std::list<std::string> joiner_ls = traciVehicle->getPlannedRoadIds();
					std::list<std::string> leader_ls = traciVehicle->getPlannedRoadIdsStranger(beacon->getVehicleId());
					std::list<std::string>::iterator i,j,dup1,dup2,  dup4,dup5,dup6,dup7;
					std::string my_roadId = traciVehicle->getRoadId();
					std::string stranger_roadId = traciVehicle->getRoadIdStranger(beacon->getVehicleId());
					std::cout<<"before the will meet section"<<endl;

					if (((behindPosition.x >= 7284 && behindPosition.x < 7287 && behindPosition.y >= 55 + (beaconPlatoonSize - 3)*5  && behindPosition.y + (beaconPlatoonSize - 3)*5< 60) || 
						(behindPosition.x >= 7375+ (beaconPlatoonSize - 3)*5 && behindPosition.x < 7380+ (beaconPlatoonSize - 3)*5 && behindPosition.y >= 797 && behindPosition.y < 800)) && (distance < 100)) { 


				/*	int will_meet = 0;
					for(i=joiner_ls.begin(); i!=joiner_ls.end(); i++) {
						dup1=std::next(i,1);
						dup2=std::next(i,2);
						if (*i==my_roadId) {

							for(j=leader_ls.begin(); j!=std::prev(leader_ls.end(),4); j++){

								if (*j==stranger_roadId) {

									//if (j!=std::prev(leader_ls.end(),1) && j!=std::prev(leader_ls.end(),3)) {
										std::cout<<"*j is "<<*j<<endl;
										dup4=std::next(j,1);
										std::cout<<"*dup4 "<<*dup4<<endl;
										dup5=std::next(j,2);
										std::cout<<"*dup5 "<<*dup5<<endl;
										dup6=std::next(j,3);
										std::cout<<"*dup6 "<<*dup6<<endl;
										dup7=std::next(j,4);
										std::cout<<"*dup7 "<<*dup7<<endl;

										if (!(*i).compare(*j) || !(*dup1).compare(*j) || !(*dup2).compare(*j) || !(*dup2).compare(*dup4) || !(*dup2).compare(*dup5) || !(*dup2).compare(*dup6) || !(*dup2).compare(*dup7)){
											will_meet = 1;
											break;										
										}
									//}	
								}
							}
						}
						if (will_meet)
							break;
					}
					std::cout<<"will meet? "<<will_meet<<endl;
					if (distance < 100 && will_meet==1) {
						std::cout<<"in the if "<<endl;
					
					*/
					
						positionHelper->setLeaderId(beacon->getVehicleId());
						std::cout<<"beacon->getVehicleId() "<<beacon->getVehicleId()<<endl;
						//std::cout<<"positionHelper->getLeaderId() "<<positionHelper->getLeaderId()<<endl;
						positionHelper->setPlatoonId(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId()));
						std::cout<<"sent resquest join msg from beacon->getVehicleId()"<<beacon->getVehicleId()<<endl;
						toSend = generateMessage();
						toSend->setMessageType(JM_REQUEST_JOIN);
						sendUnicast(toSend, beacon->getVehicleId());
						FSM_Goto(joinerFsm, JS_WAIT_REPLY);
					}
				}
					std::cout<<"after the will meet section"<<endl;
					
					
					
					break;
		}

		case FSM_Exit(JS_WAIT_REPLY): {
			std::cout<<"in the joiner wait reply"<<endl;


			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()){
				std::cout<<"positionHelper->getPlatoonId() "<<positionHelper->getPlatoonId()<<endl;
				//if the leader told us to move in position, we can start approaching the platoon
				if (maneuver->getMessageType() == LM_MOVE_IN_POSITION) {
					//save some data about the platoon
					//std::cout<<"maneuver->getPlatoonId() "<<maneuver->getPlatoonId()<<endl;
					//std::cout<<"positionHelper->getPlatoonSize(maneuver->getPlatoonId()) "<<positionHelper->getPlatoonSize(maneuver->getPlatoonId())<<endl;
					//std::cout<<"the front id will be "<<positionHelper->getMemberId(positionHelper->getPlatoonSize(maneuver->getPlatoonId()))<<endl;
					positionHelper->setFrontId(positionHelper->getMemberId(positionHelper->getPlatoonSize(maneuver->getPlatoonId())));
					vehicleData.joinLane = maneuver->getPlatoonLane();
					//traciVehicle->slowDown(0,0);

					//check for correct lane. if not in correct lane, change it
					int currentLane = traciVehicle->getLaneIndex();
					if (currentLane != vehicleData.joinLane) {
						traciVehicle->setFixedLane(vehicleData.joinLane);
					}


					//activate faked CACC. this way we can approach the front car using data obtained through GPS
					traciVehicle->setCACCConstantSpacing(15);
					//we have no data so far, so for the moment just initialize with some fake data
					traciVehicle->setControllerFakeData(15, vehicleData.speed, 0, vehicleData.speed, 0);
					//set a CC speed higher than the platoon speed to approach it
					traciVehicle->setCruiseControlDesiredSpeed(vehicleData.speed + 100/3.6);
					traciVehicle->setActiveController(Plexe::FAKED_CACC);
					FSM_Goto(joinerFsm, JS_MOVE_IN_POSITION);
				}
			}
			break;
		}

		case FSM_Exit(JS_MOVE_IN_POSITION): {
			std::cout<<"in the joiner - move in position"<<endl;
			//if we get data, just feed the fake CACC
			
			if (beacon && beacon->getVehicleId() == positionHelper->getFrontId()) {
				std::cout<<"in beacon"<<endl;
				//get front vehicle position
				Coord frontPosition(beacon->getPositionX(), beacon->getPositionY(), 0);
				//get my position
				Veins::TraCICoord traciPosition = mobility->getManager()->omnet2traci(mobility->getCurrentPosition());
				Coord position(traciPosition.x, traciPosition.y);
				//compute distance (-4 because of vehicle length)
				double distance = position.distance(frontPosition) - 4;
				std::cout<<"distance is "<<distance<<endl;

				bool will_meet_next_section = 0;
				/*if (dis_from_begin<650){

					std::list<std::string> joiner_ls = traciVehicle->getPlannedRoadIds();
					std::list<std::string>::iterator i,dup1;
					std::string my_roadId = traciVehicle->getRoadId();
					std::string stranger_roadId = traciVehicle->getRoadIdStranger(beacon->getVehicleId());
					//	int will_meet = 0;
					for(i=joiner_ls.begin(); i!=joiner_ls.end(); i++) {
						std::cout<<*i<<" "<<endl;
						if (*i==my_roadId){ 
							dup1 = std::next(i,1);
							if (*dup1==stranger_roadId)
								will_meet_next_section=1;
							
						}
					}
				}*/
				//if we are in position, tell the leader about that
				if (distance < 16  && (traciVehicle->getRoadId()==traciVehicle->getRoadIdStranger(beacon->getVehicleId()) ||  will_meet_next_section )) {
					toSend = generateMessage();
					toSend->setMessageType(JM_IN_POSITION);
					sendUnicast(toSend, positionHelper->getLeaderIdUnknown(beacon->getVehicleId()));
					FSM_Goto(joinerFsm, JS_WAIT_JOIN);
				}
			}		
			break;
		}

		case FSM_Exit(JS_WAIT_JOIN): {
			
			std::cout<<"in the joiner wait join"<<endl;
			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				//if we get confirmation from the leader, switch from faked CACC to real CACC
				if (maneuver->getMessageType() == LM_JOIN_PLATOON) {
					traciVehicle->setActiveController(Plexe::CACC);
					//set spacing to 5 meters to get close to the platoon
					traciVehicle->setCACCConstantSpacing(5);
				}
				//tell the leader that we're now in the platoon
				toSend = generateMessage();
				toSend->setMessageType(JM_IN_PLATOON);
				sendUnicast(toSend, positionHelper->getLeaderId());
				FSM_Goto(joinerFsm, JS_FOLLOW);

			}
			break;
		}

		case FSM_Exit(JS_FOLLOW): {
			std::cout<<"in js follow "<<endl;
			//std::cout<<"positionHelper->getPlatoonJoiner(positionHelper->getPlatoonId()) "<<positionHelper->getPlatoonJoiner(positionHelper->getPlatoonId());
			int k=0;
			std::vector<int> temp;
			if (maneuver) {
				std::cout<<"a manvuever!in LVS idle"<<endl;
				std::cout<<"maneuver->getPlatoonId() "<<maneuver->getPlatoonId()<<endl;
				std::cout<<"positionHelper->getPlatoonId() "<<positionHelper->getPlatoonId()<<endl;
				//std::cin.get();
			}
			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				//std::cout<<"got a maneuver msg in js follow"<<endl;
				if (maneuver->getMessageType() == LM_UPDATE_FORMATION) {						
					vehicleData.formation.clear();										
					positionHelper->updatePlatoonJoiner(positionHelper->getPlatoonId(), 1);
					//std::cout<<"in update formaton from joiner"<<endl;
					for (int i = 0; i <positionHelper->getPlatoonNumber(); i++) {
						//std::cout<<"in the joiner follow inside the first loop"<<endl;
						for (int j = 0; j <positionHelper->getPlatoonSize(i)+ positionHelper->getPlatoonJoiner(i); j++) {
								temp.push_back(maneuver->getPlatoonFormation(k));
								k++;
						}
						vehicleData.formation.push_back(temp);
						temp.clear();
						//std::cout<<"vehicleData.formation[i].size() "<<vehicleData.formation[i].size()<<endl;
					}
				}
				
			}
			//once gets here will immidiately try to turn into a leaver
			//std::cout<<"positionHelper->getPlatoonJoiner(positionHelper->getPlatoonId()) "<<positionHelper->getPlatoonJoiner(positionHelper->getPlatoonId())<<endl;
			double dis_from_start = traciVehicle->getDistanceFromRouteBegin();
			if (positionHelper->getPlatoonJoiner(positionHelper->getPlatoonId())==1 && 
				((dis_from_start>=650 && dis_from_start<740) || 
				(dis_from_start>=1400 && dis_from_start<1650))){
				std::cout<<"entered the if in js follow about to become a leaver"<<endl;

				//std::cin.get();
				role = LEAVER;
				FSM_Goto(joinerFsm,JS_INIT);
				prepareManeuverCars(0);
			}
			break;
		}
	}

	if (encapsulated) {
		delete encapsulated;
	}
	if (unicast) {
		delete unicast;
	}

}

void JoinManeuverScenario::handleLeaverMsg(cMessage *msg) {

	std::cout<<"**********in the handle leaver msg func"<<endl;
	double dis_from_start = traciVehicle->getDistanceFromRouteBegin();

	std::cout<<"traciVehicle->getDistanceFromRouteBegin() "<<dis_from_start<<endl;
	//this message can be a self message, or a unicast message
	//with an encapsulated beacon or maneuver message
	ManeuverMessage *maneuver = 0;
	PlatooningBeacon *beacon = 0;
	cPacket *encapsulated = 0;
	//maneuver message to be sent, if needed
	ManeuverMessage *toSend;
	//int front_vehicle = 0;

	

	//first check if this is a unicast message, and in case if it is a beacon or a maneuver
	UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
	if (unicast) {
		encapsulated = unicast->decapsulate();
		maneuver = dynamic_cast<ManeuverMessage *>(encapsulated);
		beacon = dynamic_cast<PlatooningBeacon *>(encapsulated);
	}

	FSM_Switch(leaverFsm) {
		//init state, just move to the idle state
		case FSM_Exit(LVS_INIT): {
			//std::cout<<"leaver - move to follow state"<<endl;
			
			//leaver info
			
			
			FSM_Goto(leaverFsm, LVS_FOLLOW);
			break;
		}

		case FSM_Exit(LVS_FOLLOW): {
			//if this is a self message triggering the beginning of procedure, then ask for leaving
			std::cout<<"leaver - in follow state"<<endl;


			//std::cout<<"msg is "<<msg<<endl;
			if (msg == startManeuver) {
				FSM_Goto(leaverFsm, LVS_WAIT_TO_SEND_REQUEST);
			}
			break;
		}

		case FSM_Exit(LVS_WAIT_TO_SEND_REQUEST): {
			std::cout<<"leaver - in wait to send request"<<endl;

			if ((dis_from_start > 690 && dis_from_start <= 750) ||
			    (dis_from_start > 1300 && dis_from_start <= 1620)){
				//std::cout<<"start leaveing vehicleData.formation.size() "<<vehicleData.formation.size()<<endl;
				toSend = generateMessage();
				toSend->setMessageType(LVM_REQUEST_LEAVE);
				sendUnicast(toSend, positionHelper->getLeaderId());
				FSM_Goto(leaverFsm,LVS_WAIT_LEAVE); 
			}
			break;
		}

		case FSM_Exit(LVS_WAIT_LEAVE): {
			std::cout<<"leaver - in wait leave state"<<endl;

			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				//we're out of the platton. if we get an update of the formation, change it accordingly
				if (maneuver->getMessageType() == LM_LEAVE_PLATOON) {
					traciVehicle->setActiveController(Plexe::ACC);
					toSend = generateMessage();
					toSend->setMessageType(LVM_LEFT_PLATOON);
					sendUnicast(toSend, positionHelper->getLeaderId());
					FSM_Goto(leaverFsm,LVS_IDLE); 
				}
			}
			break;
	   }

		case FSM_Exit(LVS_IDLE): {

			std::cout<<"leaver - in idle state"<<endl;
			//we're out of the platton. if we get an update of the formation, change it accordingly
			//std::cout<<"the platoon id is "<<positionHelper->getPlatoonId()<<endl;

			int k=0;

			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				if (maneuver->getMessageType() == LM_UPDATE_FORMATION) {
					std::cout<<"leaver in update formation"<<endl;
					traciVehicle->setActiveController(Plexe::ACC);
					//std::cin.get();

					for (int i = 0; i <positionHelper->getPlatoonNumber(); i++){
						vehicleData.formation[i].clear();
					}
					positionHelper->updatePlatoonJoiner(positionHelper->getPlatoonId(), 0);
					std::cout<<"just updated platoon joiner"<<endl;
					for (unsigned int i = 0; i <positionHelper->getPlatoonNumber(); i++) {					
						for (unsigned int j = 0; j < positionHelper->getPlatoonSize(i); j++){
							vehicleData.formation[i].push_back(maneuver->getPlatoonFormation(k));
							k++;
						}
					}
				}
				//std::cin.get();
			}
			std::cout<<"positionHelper->getPlatoonJoiner(positionHelper->getPlatoonId()) "<<positionHelper->getPlatoonJoiner(positionHelper->getPlatoonId());
			std::cout<<"dis_from_start "<<dis_from_start<<endl;
				
			if (positionHelper->getPlatoonJoiner(positionHelper->getPlatoonId())==0 && dis_from_start >= 805 && dis_from_start < 1000) {
				
				traciVehicle->slowDown(0,100);
				role = JOINER;
				FSM_Goto(leaverFsm,LVS_INIT);
				prepareManeuverCars(0);
			}

			if (dis_from_start >= 1530)
				traciVehicle->slowDown(0,100);
			break;

		}
	}


	if (encapsulated) {
		delete encapsulated;
	}
	if (unicast) {
		delete unicast;
	}

}


void JoinManeuverScenario::handleFollowerMsg(cMessage *msg) {

	std::cout<<"*************in the handle follower msg func"<<endl;
    std::cout<<"positionHelper->getId() "<<positionHelper->getId()<<endl;
	if (simTime()>350)
		finish();
	int k=0;
	//this message can be a self message, or a unicast message
	//with an encapsulated beacon or maneuver message
	ManeuverMessage *maneuver = 0;
	cPacket *encapsulated = 0;

	//first check if this is a unicast message, and in case if it is a beacon or a maneuver
	UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
	if (unicast) {
		encapsulated = unicast->decapsulate();
		maneuver = dynamic_cast<ManeuverMessage *>(encapsulated);
	}

	//check current follower status
	FSM_Switch(followerFsm) {
		case FSM_Exit(FS_INIT): {
			FSM_Goto(followerFsm, FS_FOLLOW);
			break;
		}

		case FSM_Exit(FS_FOLLOW): {
			std::cout<<"in FS follow"<<endl;
			//std::cout<<"msg is "<<msg<<endl;

			std::cout<<"the platoon id is "<<positionHelper->getPlatoonId()<<endl;
			if (maneuver && maneuver->getPlatoonId() == positionHelper->getPlatoonId()) {
				std::cout<<"get a maneuver msg from the leader"<<endl;
				if (maneuver->getMessageType() == LM_UPDATE_FORMATION) {
					for (int i = 0; i <positionHelper->getPlatoonNumber(); i++){
						vehicleData.formation[i].clear();
					}
					positionHelper->updatePlatoonJoiner(positionHelper->getPlatoonId(), 0);
					for (unsigned int i = 0; i <positionHelper->getPlatoonNumber(); i++) {					
						for (unsigned int j = 0; j < positionHelper->getPlatoonSize(i); j++){
							vehicleData.formation[i].push_back(maneuver->getPlatoonFormation(k));
							k++;
						}
					}

				}
					
			}		
			/*if ((simTime()>48.7) && (positionHelper->getId()==46 ||positionHelper->getId()==47)) {
				std::cout<<"positionHelper->getPlatoonJoiner(positionHelper->getPlatoonId()) "<<positionHelper->getPlatoonJoiner(positionHelper->getPlatoonId())<<endl;
				std::cin.get();
			}*/

			break;
		}
	  }								  

	if (encapsulated) {
		delete encapsulated;
	}
	if (unicast) {
		delete unicast;
	}

}


void JoinManeuverScenario::handleHumanMsg(cMessage *msg) {
	//traciVehicle->setSpeedMode(0);
	
	PlatooningBeacon *beacon = 0;
	cPacket *encapsulated = 0;
	bool did_not_enter_close = 0;
	bool did_not_enter_far = 0;

	int will_crash = 0;
	int centerPointType = 2; // 2 stands for the lower conflict intersection, 1 - stands for the upper conflict intersection
	//int will_crash_case2 = 0;

	//first check if this is a unicast message, and in case if it is a beacon or a maneuver
	UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
	if (unicast) {
		encapsulated = unicast->decapsulate();
		beacon = dynamic_cast<PlatooningBeacon *>(encapsulated);
	}
	


		//check current leader status
	//************* only platoons should cause a stop, humans don't produce any conflict due to their route ****************//
    std::cout<<"positionHelper->getId() "<<positionHelper->getId()<<endl;

	if (beacon && (positionHelper->isLeaderUnknownId(beacon->getVehicleId()) == 1 && positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId()))>1 )) {


		std::cout<<"%*$%*$*%$* in human indentified a beacon %$%$#%#%"<<endl;
		//std::cout<<"entered the if after identifying a beacon"<<endl;
		std::cout<<"the beacon is from "<<beacon->getVehicleId()<<endl;

		std::list<std::string> roundabout_platoon_ls = traciVehicle->getPlannedRoadIdsStranger(beacon->getVehicleId());
		std::cout<<"after the roundabout platoon ls "<<endl;
		std::list<std::string>::iterator i,j,dup1,dup3, dup4;

				//get my position
		Veins::TraCICoord traciPosition = mobility->getManager()->omnet2traci(mobility->getCurrentPosition());
		Coord position(traciPosition.x, traciPosition.y);
		//get front vehicle position
		Coord frontPosition(beacon->getPositionX(), beacon->getPositionY(), 0);
		std::cout<<"after the definition of the front position "<<endl;
		double distance = position.distance(frontPosition);
		std::cout<<"distnace is "<<distance<<endl;


		// if approaching the roundabout but not in the roundabout
		std::string enteringRoad = traciVehicle->getRoadId();

		if (((position.x >= 7284 && position.x < 7287 && position.y >= 684 && position.y < 784) || 
			(position.x >= 7300 && position.x < 7400 && position.y >= 800 && position.y < 804)) && 
			(enteringRoad.compare("gneE47") && enteringRoad.compare("gneE48") && enteringRoad.compare("gneE49") && enteringRoad.compare("gneE50") &&
			enteringRoad.compare(":gneJ54_2") && enteringRoad.compare(":gneJ54_1") &&  enteringRoad.compare(":gneJ55_0") &&  enteringRoad.compare(":gneJ56_0") && 
			enteringRoad.compare(":gneJ6_2") && enteringRoad.compare(":gneJ6_0") && enteringRoad.compare(":gneJ6_3") && enteringRoad.compare(":gneJ6_1") ))
		{
			
			if (distance < sqrt(116*116*2)) { // if are relatively close to one another: 100 m from the roundabout
					
			double inRBdistance=0, MyinRBdistance = 0, MyFrontVehicleDistance=0;
			std::string roadFirst = traciVehicle->getRoadIdStranger(beacon->getVehicleId());

			Coord centerPointLow(7285.33, 783.5, 0);  // the lower enterance
			Coord centerPointHigh(7302, 801.5, 0);   // the upper enterance

			double distanceToRoundabout = fmin(centerPointLow.distance(frontPosition),centerPointHigh.distance(frontPosition));
			//std::cout<<"in the if"<<endl;
			double entering_dis = fmin(centerPointLow.distance(position),centerPointHigh.distance(position));
			// if inside the roundabout or there is a vehicle which is closer to the roundabout than me
			std::cout<<"entering_dis "<<entering_dis<<endl;
			std::cout<<"distanceToRoundabout "<<distanceToRoundabout<<endl;
			 // the distance is small but not in the roundabout
				if (roadFirst.compare("gneE47") && roadFirst.compare("gneE48") && roadFirst.compare("gneE49") && roadFirst.compare("gneE50") &&
					roadFirst.compare(":gneJ54_2") && roadFirst.compare(":gneJ54_1") &&  roadFirst.compare(":gneJ55_0") &&  roadFirst.compare(":gneJ56_0") && 
					roadFirst.compare(":gneJ6_2") && roadFirst.compare(":gneJ6_0") && roadFirst.compare(":gneJ6_3") && roadFirst.compare(":gneJ6_1") ) {

						std::cout<<"not in the roundabout"<<endl;


						//&& roadFirst.compare("edge_0_8")
						if (entering_dis>distanceToRoundabout) {	// the addition of the distance is meant to prevent from platoons to stop too earlier 
							// infront of the roundabout

							Coord formerPosition(positionHelper->getXLocationOut(beacon->getVehicleId()), positionHelper->getYLocationOut(beacon->getVehicleId()),0);
							std::cout<<"former position "<<positionHelper->getXLocationOut(beacon->getVehicleId())<<" "<<positionHelper->getYLocationOut(beacon->getVehicleId())<<endl;
							std::cout<<"position "<<position.x<<" "<<position.y<<endl;
							
							double currentSpeed = position.distance(formerPosition) /(simTime().dbl() - positionHelper->getTimeOut(beacon->getVehicleId()));
							positionHelper->setXLocationOut(position.x,beacon->getVehicleId());
							positionHelper->setYLocationOut(position.y,beacon->getVehicleId());
							positionHelper->setTimeOut(simTime().dbl(), beacon->getVehicleId());
							if (position.x >= 7284 && position.x < 7287)  // the lower enterance
								centerPointType = 2;
							else centerPointType = 1;
							
							std::cout<<"currentSpeed "<<currentSpeed<<endl;

								int roundabout_route_outside = 2; //2 vertical route, 1 horizontal route
								if (!(roadFirst).compare("-edge_0_8")) {
										roundabout_route_outside= 1;
								}
							

							std::cout<<"currentSpeed "<<currentSpeed<<endl;
							if (currentSpeed> 16/3.6  && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8") && ((centerPointType == 2 && roundabout_route_outside == 1) || (centerPointType == 1 && roundabout_route_outside == 2))) {
								will_crash = 1; // another platoon is about to enter the roundabout
								std::cout<<"will crash"<<endl;
							}


							else if (currentSpeed>1/3.6 && (((centerPointLow.distance(position) <16) && centerPointType == 2 && roundabout_route_outside == 1) || ((centerPointHigh.distance(position) <16) && centerPointType == 1 && roundabout_route_outside == 2)) && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8")) {
								will_crash = 1; // another platoon is about to enter the roundabout
								std::cout<<"will crash for slow vehicles"<<endl;
							}
							else if (currentSpeed<1/3.6 && (((centerPointLow.distance(position) <4.5) && centerPointType == 2 && roundabout_route_outside == 1) || ((centerPointHigh.distance(position) <4.5) && centerPointType == 1 && roundabout_route_outside == 2)) && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8")) {
								will_crash = 1; // another platoon is about to enter the roundabout
								std::cout<<"will crash for slow vehicles"<<endl;
							}


						}	

					}

				else if (!roadFirst.compare("edge_0_8")) { // if just left the roundabout
					std::cout<<"in the edge 0 8 case"<<endl;
					double interDistance = 6;
					double roundabout_radius = 18;
					if (position.x >= 7284 && position.x < 7287) { // conflict point is at the lower enterance
						//the distance from conflict point is the distance in edge_0_8 plus a quarter cicrcle
						double distance_from_conflict_point = centerPointHigh.distance(frontPosition) + (0.25)*roundabout_radius*M_PI; 
						double roundabout_platoonLength = positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId())) * 4 + 
						(positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId())) - 1) * interDistance;
						if (distance_from_conflict_point<roundabout_platoonLength+10){

							will_crash = 1;
						}
					}
				}


				else { // in the roundabout
					int roundabout_route = 2; //2 vertical route, 1 horizontal route

					for(j=roundabout_platoon_ls.begin(); j!=roundabout_platoon_ls.end(); j++){
						if (!(*j).compare("edge_0_8")) {
							roundabout_route= 1;
							break;
						}
					}
					if (position.x >= 7284 && position.x < 7287) { // the lower enterance
						Coord centerPoint(7285.33, 783.5, 0);
						
						//entering_dis = position.distance(centerPoint);
					    inRBdistance = centerPoint.distance(frontPosition);

						if (roundabout_route == 1 && frontPosition.x < position.x )
							did_not_enter_close = 1;
						else if (roundabout_route == 1 && (!roadFirst.compare("gneE48") || !roadFirst.compare(":gneJ6_0")))
							did_not_enter_far = 1;

						// for the case where the upper enterance threatens the platoon entering from the lower one//
						Coord AnotherCenterPoint(7302, 801.5, 0);   // the upper enterance
						MyinRBdistance = AnotherCenterPoint.distance(position); // calculating entering vehicle position with respect to the platoon in the upper junction
						// this vehicle has not yet entered the upper enterance
						MyFrontVehicleDistance = AnotherCenterPoint.distance(frontPosition);
					}
					
					else { 
						// center point (7302, 801.5)
						Coord centerPoint(7302, 801.5, 0);   // the upper enterance
					    //entering_dis = position.distance(centerPoint);
						inRBdistance = centerPoint.distance(frontPosition);
						centerPointType = 1;
						if (roundabout_route == 2 && (!roadFirst.compare("gneE47") || !roadFirst.compare(":gneJ54_0")))
							did_not_enter_close = 1;
						}
				

						// the time it would take the entering platoon to reach the crash point
						// real inner radius is 16 m, but in the lane itself, I assumed it is 2 meters bigger 								
					double roundabout_radius = 18;
					double radian_angle;
					double arc_sin = (inRBdistance/2)/roundabout_radius;
					std::cout<<"arc_sin "<<arc_sin<<endl;

					if (arc_sin>1) arc_sin = 1;
					else if (arc_sin<-1) arc_sin = -1;
					
					
					// in order to calculate the right distance from conflict point
					std::cout<<"roundabout route is "<<roundabout_route<<endl;
					if ((roundabout_route == 2 && centerPointType==1 && (!roadFirst.compare("gneE50")  ||  !roadFirst.compare(":gneJ54_1"))) ||
					   ((roundabout_route == 2 && centerPointType==2) && (!roadFirst.compare("gneE50")|| !roadFirst.compare("gneE49") || !roadFirst.compare(":gneJ54_1"))) ||	
					   ((roundabout_route == 1 && centerPointType==1) && (!roadFirst.compare("gneE50") ||!roadFirst.compare("gneE47") || !roadFirst.compare(":gneJ54_2") || !roadFirst.compare(":gneJ6_2"))) ||	
					   (roundabout_route == 1 && centerPointType==2 && !roadFirst.compare("gneE48")))
					   					
						radian_angle = 2*M_PI-2*(asin(arc_sin));
					
					else    radian_angle = 2*(asin(arc_sin)); // not the measured angle but its complementary
					std::cout<<"radian_angle "<<radian_angle<<endl;
					
					double round_distance = (radian_angle/(2*M_PI))*roundabout_radius*2*M_PI;

								
					bool entering_platoon_with_joiner = 0;
					if (positionHelper->getPlatoonJoiner(positionHelper->getPlatoonIdAccLeader(positionHelper->getId()))==1)
						entering_platoon_with_joiner = 1;

					double interDistance = 6;
					double entering_platoonLength = positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(positionHelper->getId())) * 4 + 
						(positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(positionHelper->getId())) - 1) * interDistance + entering_platoon_with_joiner *16;
					// relating to the platoon once in the roundabout
					// I assumed that the joiner is more distance than "regular" followers in the platoon
					

					//double entering_leader_time = entering_dis/(50/3.6);
					//double entering_platoon_time = entering_platoonLength / (50/3.6);
				
					Coord formerPosition(positionHelper->getXLocationIn(beacon->getVehicleId()), positionHelper->getYLocationIn(beacon->getVehicleId()),0);
					
					double currentSpeed = position.distance(formerPosition) /(simTime().dbl() - positionHelper->getTimeIn(beacon->getVehicleId()));
					std::cout<<"former position "<<positionHelper->getXLocationIn(beacon->getVehicleId())<<" "<<positionHelper->getYLocationIn(beacon->getVehicleId())<<endl;
					std::cout<<"position "<<position.x<<" "<<position.y<<endl;
					positionHelper->setXLocationIn(position.x,beacon->getVehicleId());
					positionHelper->setYLocationIn(position.y,beacon->getVehicleId());
					positionHelper->setTimeIn(simTime().dbl(),beacon->getVehicleId());


					double entering_leader_time = (-(currentSpeed/3.6) + sqrt((currentSpeed/3.6)*(currentSpeed/3.6)+3*entering_dis))/1.5;
					double entering_platoon_time =  (-(currentSpeed/3.6) + sqrt((currentSpeed/3.6)*(currentSpeed/3.6)+3*entering_platoonLength))/1.5;

					double total_entering_time = entering_leader_time + entering_platoon_time;



					double roundabout_platoonLength = positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId())) * 4 + 
						(positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId())) - 1) * interDistance;
				

					double roundabout_leader_time = round_distance/(30/3.6);
					double roundabout_platoon_time = 0;
					if (did_not_enter_far == 1 || did_not_enter_close == 1) {
						std::cout<<"did not enter "<<endl;
						//assumption - the platoon drives at 50 kph but at the roundabout its speed is 30!!!
						roundabout_platoon_time = roundabout_platoonLength/(30/3.6);
					}
					else {
						// already in the roundabout
						std::cout<<"did enter "<<endl;
						roundabout_leader_time = 0;
						roundabout_platoon_time = (roundabout_platoonLength - round_distance) /(30/3.6);
					}

					//double total_roundabout_platoon_time = roundabout_leader_time + roundabout_platoon_time;

					// to prevent cases where entering platoons carsh into partly entered platoons
					int IdLast;
					//std::string roadLast;
					int roundabout_platoon_size = positionHelper->getPlatoonSize(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId()));
					//std::cout<<"rondabout platoon size"<<roundabout_platoon_size<<endl;
					if (positionHelper->getPlatoonJoiner(positionHelper->getPlatoonIdAccLeader(beacon->getVehicleId()))==0)
					// if there is no joiner in this platoon
					IdLast = positionHelper->getMemberIdUnknown(beacon->getVehicleId(), roundabout_platoon_size);
					else IdLast = roundabout_platoon_size +1;

					//roadLast = traciVehicle->getRoadIdStranger(IdLast);
					//std::cout<<"the road of the last vehicle "<<roadLast<<endl;

					std::cout<<"enteringRoad "<<enteringRoad<<endl;

					if (!(enteringRoad).compare("-edge_0_8") || !(enteringRoad).compare("gneE11")){


						//std::cout<<"currentSpeed "<<currentSpeed<<endl;
						int speed_or_location_stop = 0;
						std::cout<<"centerPointLow.distance(position) "<<centerPointLow.distance(position)<<endl;
						std::cout<<"centerPointHigh.distance(position) "<<centerPointHigh.distance(position)<<endl;
						std::cout<<"centerPointType "<<centerPointType<<endl;
						std::cout<<"currentSpeed "<<currentSpeed<<endl;

						if (currentSpeed> 16/3.6 && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8") && ((centerPointType == 2 && roundabout_route ==1) ||  (centerPointType == 1 && roundabout_route ==2)))  
							speed_or_location_stop = 1;

						else if (currentSpeed>1/3.6 && (((centerPointLow.distance(position) <16) && centerPointType == 2 && roundabout_route ==1) || ((centerPointHigh.distance(position) <16) && centerPointType == 1 && roundabout_route ==2)) && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8"))
							speed_or_location_stop = 1;

						else if (currentSpeed<1/3.6 && (((centerPointLow.distance(position) <4.5) && centerPointType == 2 && roundabout_route ==1) || ((centerPointHigh.distance(position) <4.5) && centerPointType == 1 && roundabout_route ==2)) && roadFirst.compare("-gneE11") && roadFirst.compare("edge_0_8"))
							speed_or_location_stop = 1;



						if (speed_or_location_stop==1) {
							
							std::cout<<"did_not_enter_far "<<did_not_enter_far<<endl;
							std::cout<<"did_not_enter_close "<<did_not_enter_close<<endl;
							// if tries to enter while the platoon is still not in the intersection	
							 if ((did_not_enter_far == 1 || did_not_enter_close == 1) && (3.5+total_entering_time >roundabout_leader_time)) { 
								std::cout<<"will crash on arrival"<<endl;
								will_crash = 1;
							}
							// if tries to enter after the leader of the roundabout platoon has passed the intersetion
							else if (((did_not_enter_far == 0) && (did_not_enter_close == 0)) && (entering_leader_time < roundabout_platoon_time+1.5)) {
								std::cout<<"will crash while roundabout platoon is evacuating"<<endl;
								will_crash = 1;
							}

							else if (((did_not_enter_far == 1) && (did_not_enter_close == 1)) && (roundabout_leader_time < entering_platoon_time )) {
								std::cout<<"will crash while entering platoon is evacuating"<<endl;
								will_crash = 1;
							}
							else if (centerPointType == 2) { // for the case where might crash in the upper enterance although the conflict point is the lower one

								arc_sin = (MyFrontVehicleDistance/2)/roundabout_radius;
								if (arc_sin>1) arc_sin = 1;
								else if (arc_sin<-1) arc_sin = -1;
								if (roadFirst.compare("gneE50")) // if has not passed half a roundabout
									radian_angle = 2*(asin(arc_sin));
								else radian_angle = 2*M_PI-2*(asin(arc_sin));
									//std::cout<<"radian_angle "<<radian_angle<<endl;	
								// the time it would take the platoon in the roundabout to reach the crash point
								double round_distance = (radian_angle/(2*M_PI))*roundabout_radius*2*M_PI;
								double new_roundabout_platoon_time = (roundabout_platoonLength - round_distance) /(15/3.6);
								double new_entering_leader_time = entering_leader_time + 0.5*M_PI*
									(-(currentSpeed/3.6) + sqrt((currentSpeed/3.6)*(currentSpeed/3.6)+3*roundabout_radius))/1.5;

								std::cout<<"entering_leader_time "<<entering_leader_time<<endl;
								std::cout<<"new_roundabout_platoon_time "<<new_roundabout_platoon_time<<endl;
								std::cout<<"new_entering_leader_time "<<new_entering_leader_time<<endl;

								if (new_entering_leader_time < new_roundabout_platoon_time){
									will_crash = 1;
								}
							}
						}

					}

					std::cout<<"distance "<<distance<<endl;
					std::cout<<"roundabout_leader_time "<<roundabout_leader_time<<endl;
					std::cout<<"roundabout_platoon_time "<<roundabout_platoon_time<<endl;
					std::cout<<"round_distance "<<round_distance<<endl;
					std::cout<<"roundabout_platoonLength "<<roundabout_platoonLength<<endl;
					std::cout<<"inRBdistance "<<inRBdistance<<endl;
					std::cout<<"entering_dis "<<entering_dis<<endl;
					std::cout<<"entering_leader_time "<<entering_leader_time<<endl;
					std::cout<<"entering_leader_time "<<entering_leader_time<<endl;	
					std::cout<<"total_entering_time "<<total_entering_time<<endl;
					std::cout<<"positionHelper->getRoundaboutLeader() "<<positionHelper->getRoundaboutLeader()<<endl;
					std::cout<<"beacon->getPositionY() "<<beacon->getPositionY()<<endl;
					}
					if (will_crash) {
					
							std::cout<<"suppose to stop now"<<endl;
							positionHelper->setRoundaboutLeader(positionHelper->getLeaderIdUnknown(beacon->getVehicleId()));
							traciVehicle->setCruiseControlDesiredSpeed(0.0 / 3.6);
								
					}
		
					else if (positionHelper->getRoundaboutLeader() == positionHelper->getLeaderIdUnknown(beacon->getVehicleId())) {
						traciVehicle->setCruiseControlDesiredSpeed(50.0 / 3.6);

				}
			}

		}
		/*if (simTime()>190 && positionHelper->getId()==67){// && beacon->getVehicleId()==45) {
			std::cin.get();
		}*/
	}

	// in case gets a beacon from the joinng vehicle and the vehicle is about to join a platoon (located in the rest area and just started to move)


	/*std::string HumanRoad = traciVehicle->getRoadId();

	if (!HumanRoad.compare("edge_0_2") || !HumanRoad.compare("edge_0_3"))
		traciVehicle->setACCHeadwayTime(5); // setting the headway time to 5 seconds to allow for the joiner to join the new platoon
		*/








// to delete!!!!!!!!!!!!!
	/*else if (beacon && (beacon->getVehicleId()) == positionHelper->getnCars()) { 

		std::list<std::string> roundabout_platoon_ls = traciVehicle->getPlannedRoadIdsStranger(beacon->getVehicleId());
		std::cout<<"after the roundabout platoon ls "<<endl;
		std::list<std::string>::iterator i,j,dup1,dup3, dup4;

				//get my position
		//Veins::TraCICoord traciPosition = mobility->getManager()->omnet2traci(mobility->getCurrentPosition());
		//Coord position(traciPosition.x, traciPosition.y);
		//get front vehicle position
		Coord joinerPosition(beacon->getPositionX(), beacon->getPositionY(), 0);
		std::string HumanRoad = traciVehicle->getRoadId();

		if ((joinerPosition.x >= 7380 && joinerPosition.x < 7400) && !HumanRoad.compare("edge_0_8"))
			traciVehicle->setCruiseControlDesiredSpeed(0.0 / 3.6);
		else traciVehicle->setCruiseControlDesiredSpeed(50.0 / 3.6);

	}*/
		



	
	if (encapsulated) {
		delete encapsulated;
	}
	if (unicast) {
		delete unicast;
	}
}



void JoinManeuverScenario::handleLowerControl(cMessage *msg) {
	//lower control message
	UnicastProtocolControlMessage *ctrl = 0;

	ctrl = dynamic_cast<UnicastProtocolControlMessage *>(msg);
	//TODO: check for double free corruption
	if (ctrl) {
		delete ctrl;
	}
	else {
		delete msg;
	}

}

