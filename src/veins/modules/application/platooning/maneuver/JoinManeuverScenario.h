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

#ifndef JOINMANEUVERSCENARIO_H_
#define JOINMANEUVERSCENARIO_H_

#include "veins/modules/application/platooning/scenarios/BaseScenario.h"
#include "veins/modules/application/platooning/protocols/BaseProtocol.h"

#include "veins/modules/application/platooning/messages/ManeuverMessage_m.h"
// my addition:
#include "veins/modules/application/platooning/traffic/PlatoonsTrafficManager.h"

#include "veins/modules/mobility/traci/TraCIColor.h"

#include <deque>
class JoinManeuverScenario : public BaseScenario
{

	protected:
		//define the roles
		enum JOIN_ROLE {LEADER, FOLLOWER, JOINER, LEAVER, HUMAN};
		//define the possible maneuvers
		enum MANU_TYPE {JOIN, LEAVE};
		//data that each car needs to keep
		struct VEHICLE_DATA {
			double							speed;		//speed of the platoon
			int								joinLane;	//the lane chosen for joining the platoon
			int								joinerId;	//the id of the vehicle joining the platoon
			int								leaveLane; // the lane of the leaving vehicle, after leaving the platoon
			int								leaverId;   // the id of the vehicle leaving the platoon
			std::vector<std::vector<int>>	formation;	//list of vehicles in the platoon
			std::deque<int>				RAqueue; // a queue for managing the queue infront of the roundabout enterance
		};
		//define the states for each role
		typedef enum _LEADER_STATES {
			LS_INIT = 0,
			LS_LEADING = FSM_Steady(1),
			LS_WAIT_JOINER_IN_POSITION = FSM_Steady(2),
			LS_WAIT_JOINER_TO_JOIN = FSM_Steady(3),
			LS_WAIT_LEAVE = FSM_Steady(4),

		} LEADER_STATES;
		typedef enum _JOINER_STATES {
			JS_INIT = 0,
			JS_IDLE = FSM_Steady(1),
			JS_WAIT_TO_SEND_REQUEST = FSM_Steady(2),
			JS_WAIT_REPLY = FSM_Steady(3),
			JS_MOVE_IN_POSITION = FSM_Steady(4),
			JS_WAIT_JOIN = FSM_Steady(5),
			JS_JOIN_PLATOON = FSM_Steady(6),
			JS_FOLLOW = FSM_Steady(7)
		} JOINER_STATES;
		typedef enum _FOLLOWER_STATES {
			FS_INIT = 0,
			FS_FOLLOW = FSM_Steady(1),
		} FOLLOWER_STATES;
		typedef enum _LEAVER_STATES { // to change accordingly!!!!
			LVS_INIT = 0,
			LVS_FOLLOW = FSM_Steady(1),
			LVS_WAIT_TO_SEND_REQUEST = FSM_Steady(2),
			LVS_WAIT_LEAVE = FSM_Steady(3),
			LVS_IDLE = FSM_Steady(4)
		} LEAVER_STATES;
		//define the messages that can be sent by each role
		enum LEADER_MSGS {
			LM_MOVE_IN_POSITION = 0,
			LM_JOIN_PLATOON = 1,
			LM_UPDATE_FORMATION = 2,
			LM_LEAVE_PLATOON = 3,		
			LM_STOPPED_AT_ROUNDABOUT = 4,
			LM_WAITED_ENOUGH = 5
		};
		enum JOINER_MSGS {
			JM_REQUEST_JOIN = 5,
			JM_IN_POSITION = 6,
			JM_IN_PLATOON = 7
		};
		enum LEAVER_MSGS {
			LVM_REQUEST_LEAVE = 8,
			LVM_LEFT_PLATOON = 9,
			LVM_MOVES_LANE = 10
		};
		
		// the maneuver handler
		//MANU_TYPE Mtype;
		//the state machine handler
		cFSM leaderFsm, joinerFsm, followerFsm, leaverFsm;
		//the role of this vehicle
		JOIN_ROLE role;
		//the position of this vehicle in the platoon
		int position;
		//data known by the vehicle
		struct VEHICLE_DATA vehicleData;
		//message used to start the maneuver
		cMessage *startManeuver;
		//pointer to protocol
		BaseProtocol *protocol;

	public:

		static const int MANEUVER_TYPE = 12347;

		virtual void initialize(int stage);
		virtual void finish();

	protected:
		void sendUnicast(cPacket *msg, int destination);

	private:

	public:
		JoinManeuverScenario()	{
			startManeuver = 0;
		}

	protected:

		virtual void handleSelfMsg(cMessage *msg);
		//override this method of BaseApp. we want to handle it ourself
		virtual void handleLowerMsg(cMessage *msg);
		virtual void handleLowerControl(cMessage *msg);
		//virtual void handleMostUpperMsg(cMessage *msg);

		ManeuverMessage *generateMessage();

		void handleRoundabout(cMessage *msg);
		void handleLeaderMsg(cMessage *msg);
		void handleJoinerMsg(cMessage *msg);
		void handleFollowerMsg(cMessage *msg);
		void handleLeaverMsg(cMessage *msg);
		void handleHumanMsg(cMessage *msg);

		void prepareManeuverCars(int platoonLane);
		//void prepareLeaveManeuverCars(int platoonLane);

};

#endif
