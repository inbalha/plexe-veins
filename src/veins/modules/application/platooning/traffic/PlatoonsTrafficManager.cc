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

#include "PlatoonsTrafficManager.h"

Define_Module(PlatoonsTrafficManager);

void PlatoonsTrafficManager::initialize(int stage) {
	std::cout<<"platoons traffic manager - initializing"<<endl;
	TraCIBaseTrafficManager::initialize(stage);
	if (stage == 0) {

		nCars = par("nCars").longValue();	
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

		realPlatoons = par("realPlatoons").doubleValue();
		humanCars = platoonSize.size() - realPlatoons;

		platoonInsertTime = SimTime(par("platoonInsertTime").doubleValue());
		platoonInsertSpeed = par("platoonInsertSpeed").doubleValue();
		platoonInsertDistance = par("platoonInsertDistance").doubleValue();
		platoonInsertHeadway = par("platoonInsertHeadway").doubleValue();
		platoonLeaderHeadway = par("platoonLeaderHeadway").doubleValue();
		platooningVType = par("platooningVType").stdstringValue();
		insertPlatoonMessage = new cMessage("");
		scheduleAt(platoonInsertTime, insertPlatoonMessage);
	}
	std::cout<<"platoons traffic manager - ended initializing"<<endl;

}


void PlatoonsTrafficManager::scenarioLoaded() {
	automated.id = findVehicleTypeIndex(platooningVType);
	automated.lane = 0;
	automated.position = 0;
	automated.speed = platoonInsertSpeed/3.6;
}

void PlatoonsTrafficManager::handleSelfMsg(cMessage *msg) {
	TraCIBaseTrafficManager::handleSelfMsg(msg);

	if (msg == insertPlatoonMessage) {
		insertPlatoons();
		insertHumans();
	}
}

void PlatoonsTrafficManager::insertPlatoons() {

	std::cout<<"in insert platoon platoons traffic manager realplatoons are"<<realPlatoons<<endl;

	//compute intervehicle distance
	double distance = platoonInsertSpeed / 3.6 * platoonInsertHeadway + platoonInsertDistance;
	//total number of platoons per lane
	//int nPlatoons = nCars / platoonSize / nLanes;
	// always one platoon per type
	int nPlatoons = 1;

	int sectionLength = 100;

	int humansPerSection = humanCars / realPlatoons;
	double intraHumanDis;
	
	//inter-platoon distance
	double platoonDistance = platoonInsertSpeed / 3.6 * platoonLeaderHeadway;
	//length of 1 platoon
	std::vector<double> platoonLength;
	//total length for one lane
	std::vector<double> totalLength;
	for (int k = 0; k < platoonSize.size(); k++){
		platoonLength.push_back(platoonSize[k] * 4 + (platoonSize[k] - 1) * distance);
		totalLength.push_back(nPlatoons * platoonLength[k] + (nPlatoons - 1) * platoonDistance);
	}

	std::vector<double> currentPos;
	currentPos = totalLength;
	int currentCar = 0;
	// ****** for each platoon type **********
	
	for (int k = 0; k < realPlatoons; k++){
		for (int i = 0; i < platoonSize[k]; i++) {
			automated.position = currentPos[k];
			std::cout<<"i is "<<i<<endl;
			std::cout<<"the current pos is "<<currentPos[k]<<endl;
			//automated lane used to be l
			automated.lane = 0;
			std::cout<<"the automated lane is "<<automated.lane<<endl;
			std::cout<<"***********about to enter add vehicle to queue in platoon traffic manager with 0 for route"<<endl;
			// I want it to enter the platoon route
			std::cout<<"about to enter another vehicle to platoon"<<endl;
			addVehicleToQueue(k+1, automated);
			std::cout<<"k+1 is "<<k+1<<endl;
			currentCar++;
			if (currentCar == platoonSize[k]) {
				currentCar = 0;
				//add inter platoon gap
				currentPos[k] -= (platoonDistance + 4);
			}
			else {
				//add intra platoon gap
				currentPos[k] -= (4 + distance);
			}		
		}
	}
}

void PlatoonsTrafficManager::insertHumans() {
	int nPlatoons = 1;
	int sectionLength = 100;
	double distance = platoonInsertSpeed / 3.6 * platoonInsertHeadway + platoonInsertDistance;
	double platoonDistance = platoonInsertSpeed / 3.6 * platoonLeaderHeadway;
	std::vector<double> totalLength;
	std::vector<double> platoonLength;
	for (int k = 0; k < platoonSize.size(); k++){
		platoonLength.push_back(platoonSize[k] * 4 + (platoonSize[k] - 1) * distance);
		totalLength.push_back(nPlatoons * platoonLength[k] + (nPlatoons - 1) * platoonDistance);
	}

	int humansPerSection = humanCars / realPlatoons;
	double intraHumanDis;

	for (int k = 0; k < realPlatoons; k++){
		std::cout<<"k is "<<k<<endl;
		double humanCurrentPos = totalLength[k];
		std::cout<<"total length is "<<totalLength[k]<<endl;
		std::cout<<"humanCars is "<<humanCars<<endl;
		intraHumanDis = (sectionLength - totalLength[k])/(humanCars/realPlatoons+1);
		for (int j = 0; j < humansPerSection; j++) {
			std::cout<<"j is "<<j<<endl;
			automated.position = humanCurrentPos + intraHumanDis * (j+1);
			std::cout<<"the current human pos is "<<automated.position<<endl;
			std::cout<<"the human vehicle id "<<automated.id<<endl;
			std::cout<<"***********about to enter human vehicle to queue in platoon traffic manager with 0 for route"<<endl;
			addVehicleToQueue(realPlatoons + k+1, automated);
		}

	}
	

	std::cout<<"ended insert platoon platoons traffic manager"<<endl;
}

void PlatoonsTrafficManager::finish() {
	TraCIBaseTrafficManager::finish();
	if (insertPlatoonMessage) {
		cancelAndDelete(insertPlatoonMessage);
		insertPlatoonMessage = 0;
	}
}
