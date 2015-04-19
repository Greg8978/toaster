// This main will compute the facts for the requested agent.

#include "area_manager/PDGHumanReader.h"
#include "area_manager/PDGRobotReader.h"
#include "area_manager/PDGObjectReader.h"
#include "agent_monitor/AddAgent.h"
#include "agent_monitor/AddJointToAgent.h"
#include "agent_monitor/RemoveAgent.h"
#include "agent_monitor/RemoveJointToAgent.h"
#include "agent_monitor/RemoveAgents.h"
#include "agent_monitor/RemoveJointsToAgent.h"
#include "agent_monitor/PrintAgents.h"
#include "pdg/FactList.h"
#include "pdg/Fact.h"
#include "toaster-lib/TRBuffer.h"
#include "toaster-lib/MathFunctions.h"

std::vector<unsigned int> agentsMonitored_;
std::map<unsigned int, std::vector<std::string> > mapJointMonitoredName_;
std::map<std::string, unsigned int> mapNameToAgentId_;

// Move this to a library?
// create a fact

/*pdg::Fact createFact(std::string property, std::string propertyType, std::string subProperty, ) {
    pdg::Fact fact_msg;
    fact_msg.property = property;
    fact_msg.subProperty = subProperty;
    fact_msg.subjectId = subjectId;
    fact_msg.subjectName = subjectName;
    fact_msg.targetId = targetId;
    fact_msg.targetName = targetName;
    fact_msg.confidence = confidence;
    fact_msg.time = time;
}*/


bool computeMotion2D(TRBuffer< Entity* > confBuffer, unsigned long timelapse, double distanceThreshold) {
    int index;
    double dist = 0.0;
    long actualTimelapse = 0;
    long timeNew = confBuffer.getTimeFromIndex(confBuffer.size() - 1);
    long timeOld = timeNew - timelapse;
    Entity* entNew = confBuffer.getDataFromIndex(confBuffer.size() - 1);

    index = confBuffer.getIndexAfter(timeOld);
    // In case we don't have the index, we will just put isMoving to false
    if (index == -1)
        return false;
    actualTimelapse = timeNew - confBuffer.getTimeFromIndex(index); // Actual timelapse
    Entity* entOld = confBuffer.getDataFromIndex(index);

    dist = bg::distance(MathFunctions::convert3dTo2d(entNew->getPosition()), MathFunctions::convert3dTo2d(entOld->getPosition()));

    /*std::cout << "Distance is " << dist << std::endl;
    std::cout << "ds*actualTimeLapse / timelapse " << distanceThreshold * actualTimelapse / timelapse << std::endl;
    std::cout << "ds " << distanceThreshold << std::endl;
    std::cout << "timelapse " << timelapse << std::endl;
    std::cout << "actual timelapse " << actualTimelapse << std::endl;*/
    if (dist < distanceThreshold * actualTimelapse / timelapse) {
        return false;
    } else {
        return true;
    }
}

bool computeJointMotion2D(TRBuffer< Entity* > confBuffer, std::string jointName, unsigned long timelapse, double distanceThreshold) {
    int index;
    double dist = 0.0;
    long actualTimelapse = 0;
    long timeNew = confBuffer.getTimeFromIndex(confBuffer.size() - 1);
    long timeOld = timeNew - timelapse;
    Entity* entNew = ( (Agent*) confBuffer.getDataFromIndex(confBuffer.size() - 1) )->skeleton_[jointName];

    index = confBuffer.getIndexAfter(timeOld);
    // In case we don't have the index, we will just put isMoving to false
    if (index == -1)
        return false;
    actualTimelapse = timeNew - confBuffer.getTimeFromIndex(index); // Actual timelapse
    Entity* entOld = ((Agent*) confBuffer.getDataFromIndex(index))->skeleton_[jointName];

    dist = bg::distance(MathFunctions::convert3dTo2d(entNew->getPosition()), MathFunctions::convert3dTo2d(entOld->getPosition()));

    /*std::cout << "Distance is " << dist << std::endl;
    std::cout << "ds*actualTimeLapse / timelapse " << distanceThreshold * actualTimelapse / timelapse << std::endl;
    std::cout << "ds " << distanceThreshold << std::endl;
    std::cout << "timelapse " << timelapse << std::endl;
    std::cout << "actual timelapse " << actualTimelapse << std::endl;*/
    if (dist < distanceThreshold * actualTimelapse / timelapse) {
        return false;
    } else {
        return true;
    }
}


double computeMotion2DDirection(TRBuffer< Entity* > confBuffer, unsigned long timelapse) {
    double towardAngle;
    int index;
    //long actualTimelapse = 0;
    long timeNew = confBuffer.getTimeFromIndex(confBuffer.size() - 1);
    long timeOld = timeNew - timelapse;
    Entity* entNew = confBuffer.getDataFromIndex(confBuffer.size() - 1);

    index = confBuffer.getIndexAfter(timeOld);
    //actualTimelapse = timeNew - confBuffer.getTimeFromIndex(index);   // Actual timelapse
    Entity* entOld = confBuffer.getDataFromIndex(index);
    towardAngle = acos(fabs(entOld->getPosition().get<0>() - entNew->getPosition().get<0>())
            / bg::distance(MathFunctions::convert3dTo2d(entOld->getPosition()), MathFunctions::convert3dTo2d(entNew->getPosition())));

    // Trigonometric adjustment
    if (entNew->getPosition().get<0>() < entOld->getPosition().get<0>())
        towardAngle = 3.1416 - towardAngle;

    if (entNew->getPosition().get<1>() < entOld->getPosition().get<1>())
        towardAngle = -towardAngle;

    return towardAngle;
}

double computeJointMotion2DDirection(TRBuffer< Entity* > confBuffer, std::string jointName, unsigned long timelapse) {
    double towardAngle;
    int index;
    //long actualTimelapse = 0;
    long timeNew = confBuffer.getTimeFromIndex(confBuffer.size() - 1);
    long timeOld = timeNew - timelapse;
    Entity* entNew = ((Agent*) confBuffer.getDataFromIndex(confBuffer.size() - 1))->skeleton_[jointName];

    index = confBuffer.getIndexAfter(timeOld);
    //actualTimelapse = timeNew - confBuffer.getTimeFromIndex(index);   // Actual timelapse
    Entity* entOld = ((Agent*) confBuffer.getDataFromIndex(index))->skeleton_[jointName];
    towardAngle = acos(fabs(entOld->getPosition().get<0>() - entNew->getPosition().get<0>())
            / bg::distance(MathFunctions::convert3dTo2d(entOld->getPosition()), MathFunctions::convert3dTo2d(entNew->getPosition())));

    // Trigonometric adjustment
    if (entNew->getPosition().get<0>() < entOld->getPosition().get<0>())
        towardAngle = 3.1416 - towardAngle;

    if (entNew->getPosition().get<1>() < entOld->getPosition().get<1>())
        towardAngle = -towardAngle;

    return towardAngle;
}

std::map<unsigned int, double> computeMotion2DToward(std::map<unsigned int, TRBuffer < Entity* > > mapEnts,
        unsigned int agentMonitored, double towardAngle, double angleThreshold) {

    std::map<unsigned int, double> towardConfidence;

    //For each entities in the same room
    for (std::map<unsigned int, TRBuffer < Entity*> >::iterator it = mapEnts.begin(); it != mapEnts.end(); ++it) {
        if (it->first != agentMonitored)
            towardConfidence[it->first] = MathFunctions::isInAngle(mapEnts[agentMonitored].back(), it->second.back(), towardAngle, angleThreshold);
    }
    return towardConfidence;
}

std::map<unsigned int, double> computeJointMotion2DToward(std::map<unsigned int, TRBuffer < Entity* > > mapEnts,
        unsigned int agentMonitored, std::string jointName, double towardAngle, double angleThreshold) {

    std::map<unsigned int, double> towardConfidence;

    //For each entities in the same room
    for (std::map<unsigned int, TRBuffer < Entity*> >::iterator it = mapEnts.begin(); it != mapEnts.end(); ++it) {
        if (it->first != agentMonitored)
            towardConfidence[it->first] = MathFunctions::isInAngle(((Agent*) mapEnts[agentMonitored].back())->skeleton_[jointName], it->second.back(), towardAngle, angleThreshold);
    }
    return towardConfidence;
}


std::map<unsigned int, double> computeDeltaDist(std::map<unsigned int, TRBuffer < Entity* > > mapEnts, unsigned int agentMonitored, unsigned long timelapse) {
    std::map<unsigned int, double> deltaDistMap;
    double curDist = 0.0;
    double prevDist = 0.0;
    double deltaDist = 0.0;
    unsigned long timeCur = 0;
    unsigned long timePrev = 0;
    Entity * entCur(0);
    Entity * entMonitoredCur(0);
    Entity * entMonitoredPrev(0);

    //For each entities in the same room
    for (std::map<unsigned int, TRBuffer < Entity*> >::iterator it = mapEnts.begin(); it != mapEnts.end(); ++it) {
        if (it->first != agentMonitored) {
            // We compute the current distance
            entCur = it->second.back();
            entMonitoredCur = mapEnts[agentMonitored].back();

            //Put this in a function
            curDist = bg::distance(MathFunctions::convert3dTo2d(entCur->getPosition()), MathFunctions::convert3dTo2d(entMonitoredCur->getPosition()));

            // We compute the distance at now - timelapse
            timeCur = entMonitoredCur->getTime();
            timePrev = timeCur - timelapse;

            entMonitoredPrev = mapEnts[agentMonitored].getDataFromIndex(mapEnts[agentMonitored].getIndexAfter(timePrev));

            prevDist = bg::distance(MathFunctions::convert3dTo2d(entCur->getPosition()), MathFunctions::convert3dTo2d(entMonitoredPrev->getPosition()));


            //We compute Deltadist
            deltaDist = curDist - prevDist;

            // We fill towardConfidence
            deltaDistMap[it->first] = deltaDist;
        }
    }
    return deltaDistMap;
}

std::map<unsigned int, double> computeJointDeltaDist(std::map<unsigned int, TRBuffer < Entity* > > mapEnts, unsigned int agentMonitored, std::string jointName, unsigned long timelapse) {
    std::map<unsigned int, double> deltaDistMap;
    double curDist = 0.0;
    double prevDist = 0.0;
    double deltaDist = 0.0;
    unsigned long timeCur = 0;
    unsigned long timePrev = 0;
    Entity * entCur(0);
    Entity * entMonitoredCur(0);
    Entity * entMonitoredPrev(0);

    //For each entities in the same room
    for (std::map<unsigned int, TRBuffer < Entity*> >::iterator it = mapEnts.begin(); it != mapEnts.end(); ++it) {
        if (it->first != agentMonitored) {
            // We compute the current distance
            entCur = it->second.back();
            entMonitoredCur = ((Agent*) mapEnts[agentMonitored].back())->skeleton_[jointName];

            //Put this in a function
            curDist = bg::distance(MathFunctions::convert3dTo2d(entCur->getPosition()), MathFunctions::convert3dTo2d(entMonitoredCur->getPosition()));

            // We compute the distance at now - timelapse
            timeCur = entMonitoredCur->getTime();
            timePrev = timeCur - timelapse;

            entMonitoredPrev = ((Agent*) mapEnts[agentMonitored].getDataFromIndex(mapEnts[agentMonitored].getIndexAfter(timePrev)))->skeleton_[jointName];

            prevDist = bg::distance(MathFunctions::convert3dTo2d(entCur->getPosition()), MathFunctions::convert3dTo2d(entMonitoredPrev->getPosition()));


            //We compute Deltadist
            deltaDist = curDist - prevDist;

            // We fill towardConfidence
            deltaDistMap[it->first] = deltaDist;
        }
    }
    return deltaDistMap;
}

/*void initTRBuffer(unsigned int agentMonitored, TRBuffer<Entity*>& TRBEntity, unsigned int historyLength) {
    //We need to initiate the ringbuffer... or not

    TRBuffer<Entity*> mybuffer(historyLength);
    TRBEntity = mybuffer;
}*/


unsigned int getAgentId(std::string name) {
    if (mapNameToAgentId_.find(name) != mapNameToAgentId_.end())
        return mapNameToAgentId_[name];
    else
        return 0;
}

bool addMonitoredAgent(unsigned int id, std::string name = "") {
    if (std::find(agentsMonitored_.begin(), agentsMonitored_.end(), id) == agentsMonitored_.end()) {
        ROS_INFO("[agent_monitor][INFO] Agent %s with id %d now monitored", name.c_str(), id);
        agentsMonitored_.push_back(id);
    } else {
        ROS_INFO("[agent_monitor][INFO] Agent %s with id %d is already monitored", name.c_str(), id);
    }
}

bool removeMonitoredAgent(unsigned int id, std::string name = "") {
    if (std::find(agentsMonitored_.begin(), agentsMonitored_.end(), id) == agentsMonitored_.end()) {
        ROS_INFO("[agent_monitor][INFO] Agent %s with id %d no more monitored", name.c_str(), id);
        agentsMonitored_.erase(std::remove(agentsMonitored_.begin(), agentsMonitored_.end(), id), agentsMonitored_.end());
    } else {
        ROS_INFO("[agent_monitor][INFO] Agent %s with id %d is already not monitored", name.c_str(), id);
    }
}


///////////////////////////
//   Service functions   //
///////////////////////////

bool addAgent(agent_monitor::AddAgent::Request &req,
        agent_monitor::AddAgent::Response & res) {
    if (req.id != 0)
        res.answer = addMonitoredAgent(req.id);
    else if (req.name != "") {
        unsigned int id = getAgentId(req.name);
        // If we found the agent and his id
        if (id != 0)
            res.answer = addMonitoredAgent(id, req.name);

            // If id == 0, agent is not present
        else {
            ROS_INFO("[agent_monitor][WARNING] Agent named %s not found, sending back response: false", req.name.c_str());
            res.answer = false;
        }
    } else {
        ROS_INFO("[agent_monitor][WARNING] request to add agent with no id and no name specified, sending back response: false");
        res.answer = false;
    }
    return true;
}

bool removeAgent(agent_monitor::RemoveAgent::Request &req,
        agent_monitor::RemoveAgent::Response & res) {
    if (req.id != 0)
        res.answer = removeMonitoredAgent(req.id);
    else if (req.name != "") {
        unsigned int id = getAgentId(req.name);
        // If we found the agent and his id
        if (id != 0)
            res.answer = removeMonitoredAgent(id, req.name);
            // If id == 0, agent is not present
        else {
            ROS_INFO("[agent_monitor][WARNING] Agent named %s not found, sending back response: false", req.name.c_str());
            res.answer = false;
        }
    } else {
        ROS_INFO("[agent_monitor][WARNING] request to remove agent with no id and no name specified, sending back response: false");
        res.answer = false;
    }
    return true;
}

bool removeAgents(agent_monitor::RemoveAgents::Request &req,
        agent_monitor::RemoveAgents::Response & res) {

    agentsMonitored_.clear();
    ROS_INFO("[agent_monitor][WARNING] request to remove all agents");
    return true;
}

bool addJointToAgent(agent_monitor::AddJointToAgent::Request &req,
        agent_monitor::AddJointToAgent::Response & res) {

    if (req.agentId != 0)
        if (std::find(agentsMonitored_.begin(), agentsMonitored_.end(), req.agentId) != agentsMonitored_.end())
            if (req.jointName != "")
                if (mapJointMonitoredName_.find(req.agentId) != mapJointMonitoredName_.end())
                    // Joint was not monitored
                    if (std::find(mapJointMonitoredName_[req.agentId].begin(), mapJointMonitoredName_[req.agentId].end(), req.jointName) == mapJointMonitoredName_[req.agentId].end()) {
                        ROS_INFO("[agent_monitor][INFO] Now monitoring joint %s of agent with id %d , sending back response: true", req.jointName.c_str(), req.agentId);
                        res.answer = true;
                        mapJointMonitoredName_[req.agentId].push_back(req.jointName);
                        // Joint is already monitored
                    } else {
                        ROS_INFO("[agent_monitor][INFO] Joint %s of agent with id %d already monitored, sending back response: false", req.jointName.c_str(), req.agentId);
                        res.answer = false;
                    } else {
                    std::vector<std::string> tmpVec(1, req.jointName);
                    mapJointMonitoredName_[req.agentId] = tmpVec;
                }// No jointName specified
            else {
                ROS_INFO("[agent_monitor][WARNING] request to add joint to agent with id %d with no name specified, sending back response: false", req.agentId);
                res.answer = false;
            } else {
            ROS_INFO("[agent_monitor][WARNING] Agent with id %d is not monitored, can't monitor his joint. Sending back response: false", req.agentId);
            res.answer = false;
        } else if (req.agentName != "") {
        unsigned int id = getAgentId(req.agentName);
        // If we found the agent and his id
        if (id != 0) {
            if (req.jointName != "") {
                if (mapJointMonitoredName_.find(id) != mapJointMonitoredName_.end())
                    // Joint was not monitored
                    if (std::find(mapJointMonitoredName_[id].begin(), mapJointMonitoredName_[id].end(), req.jointName) == mapJointMonitoredName_[id].end()) {
                        ROS_INFO("[agent_monitor][INFO] Now monitoring joint %s of agent with id %d , sending back response: true", req.jointName.c_str(), id);
                        res.answer = true;
                        mapJointMonitoredName_[id].push_back(req.jointName);
                        // Joint is already monitored
                    } else {
                        ROS_INFO("[agent_monitor][INFO] Joint %s of agent with id %d already monitored, sending back response: false", req.jointName.c_str(), id);
                        res.answer = false;
                    } else {
                    std::vector<std::string> tmpVec(1, req.jointName);
                    mapJointMonitoredName_[id] = tmpVec;
                }
                // No jointName specified
            } else {
                ROS_INFO("[agent_monitor][WARNING] request to add joint to agent with id %d with no name specified, sending back response: false", id);
                res.answer = false;
            }
            // If id == 0, agent is not present
        } else {
            ROS_INFO("[agent_monitor][WARNING] Agent named %s not found, sending back response: false", req.agentName.c_str());
            res.answer = false;
        }
    } else {
        ROS_INFO("[agent_monitor][WARNING] request to remove agent with no id and no name specified, sending back response: false");
        res.answer = false;
    }
    return true;
}

bool removeJointToAgent(agent_monitor::RemoveJointToAgent::Request &req,
        agent_monitor::RemoveJointToAgent::Response & res) {

    if (req.agentId != 0)
        if (std::find(agentsMonitored_.begin(), agentsMonitored_.end(), req.agentId) != agentsMonitored_.end())
            if (req.jointName != "")
                // Joint was monitored
                if (std::find(mapJointMonitoredName_[req.agentId].begin(), mapJointMonitoredName_[req.agentId].end(), req.jointName) != mapJointMonitoredName_[req.agentId].end()) {
                    ROS_INFO("[agent_monitor][INFO] Joint %s of agent with id %d not monitored anymore, sending back response: true", req.jointName.c_str(), req.agentId);
                    res.answer = true;
                    mapJointMonitoredName_[req.agentId].erase(std::remove(mapJointMonitoredName_[req.agentId].begin(),
                            mapJointMonitoredName_[req.agentId].end(), req.jointName), mapJointMonitoredName_[req.agentId].end());
                    // Joint is not monitored
                } else {
                    ROS_INFO("[agent_monitor][INFO] Joint %s of agent with id %d already not monitored, sending back response: false", req.jointName.c_str(), req.agentId);
                    res.answer = false;
                }// No jointName specified
            else {
                ROS_INFO("[agent_monitor][WARNING] request to remove joint to agent with id %d with no name specified, sending back response: false", req.agentId);
                res.answer = false;
            } else {
            ROS_INFO("[agent_monitor][WARNING] Agent with id %d is not monitored, can't unmonitor his joint. Sending back response: false", req.agentId);
            res.answer = false;
        } else if (req.agentName != "") {
        unsigned int id = getAgentId(req.agentName);
        // If we found the agent and his id
        if (id != 0) {
            if (req.jointName != "") {
                // Joint was not monitored
                if (std::find(mapJointMonitoredName_[id].begin(), mapJointMonitoredName_[id].end(), req.jointName) != mapJointMonitoredName_[id].end()) {
                    ROS_INFO("[agent_monitor][INFO] Joint %s of agent with id %d and named %s no more monitored, sending back response: true", req.jointName.c_str(), id, req.agentName.c_str());
                    res.answer = true;
                    mapJointMonitoredName_[id].erase(std::remove(mapJointMonitoredName_[id].begin(),
                            mapJointMonitoredName_[id].end(), req.jointName), mapJointMonitoredName_[id].end());
                    // Joint is already monitored
                } else {
                    ROS_INFO("[agent_monitor][INFO] Joint %s of agent with id %d and named %s already monitored, sending back response: false", req.jointName.c_str(), id, req.agentName.c_str());
                    res.answer = false;
                }
                // No jointName specified
            } else {
                ROS_INFO("[agent_monitor][WARNING] request to add joint to agent with id %d with no name specified, sending back response: false", id);
                res.answer = false;
            }
            // If id == 0, agent is not present
        } else {
            ROS_INFO("[agent_monitor][WARNING] Agent named %s not found, sending back response: false", req.agentName.c_str());
            res.answer = false;
        }
    } else {
        ROS_INFO("[agent_monitor][WARNING] request to remove agent with no id and no name specified, sending back response: false");
        res.answer = false;
    }
    return true;
}

bool removeJointsToAgent(agent_monitor::RemoveJointsToAgent::Request &req,
        agent_monitor::RemoveJointsToAgent::Response & res) {

    if (req.agentId != 0)
        if (std::find(agentsMonitored_.begin(), agentsMonitored_.end(), req.agentId) != agentsMonitored_.end()) {
            ROS_INFO("[agent_monitor][INFO] Agent with id %d joint's not monitored anymore, sending back response: true", req.agentId);
            mapJointMonitoredName_[req.agentId].clear();
            res.answer = true;
        } else {
            ROS_INFO("[agent_monitor][INFO] Agent with id %d is already not monitored, sending back response: false", req.agentId);
            res.answer = false;
        } else if (req.agentName != "") {
        unsigned int id = getAgentId(req.agentName);
        // If we found the agent and his id
        if (id != 0)
            if (std::find(agentsMonitored_.begin(), agentsMonitored_.end(), id) != agentsMonitored_.end()) {
                ROS_INFO("[agent_monitor][INFO] Agent with id %d joints' not monitored anymore, sending back response: true", id);
                mapJointMonitoredName_[req.agentId].clear();
                res.answer = true;
            } else {
                ROS_INFO("[agent_monitor][INFO] Agent with id %d, named %s is already not monitored, sending back response: false", id, req.agentName.c_str());
                res.answer = false;
            }// If id == 0, agent is not present
        else {
            ROS_INFO("[agent_monitor][WARNING] Agent named %s not found, sending back response: false", req.agentName.c_str());
            res.answer = false;
        }
    } else {
        ROS_INFO("[agent_monitor][WARNING] request to remove agent joints' with no id and no name specified, sending back response: false");
        res.answer = false;
    }
    return true;
}

bool printAgents(agent_monitor::PrintAgents::Request &req,
        agent_monitor::PrintAgents::Response & res) {

    std::string name;
    ROS_INFO("[agent_monitor][PRINT] ****** Agents Monitored *******");
    for (std::vector<unsigned int>::iterator it = agentsMonitored_.begin(); it != agentsMonitored_.end(); ++it) {
        name = "";
        //Finding the name
        for (std::map<std::string, unsigned int>::iterator itMap = mapNameToAgentId_.begin(); itMap != mapNameToAgentId_.end(); ++itMap)
            if (itMap->second == *it) {
                name = itMap->first;
                break;
            }
        ROS_INFO("[agent_monitor][PRINT] Agent id: %d, name: %s", *it, name.c_str());
        for (std::vector<std::string>::iterator itJoint = mapJointMonitoredName_[*it].begin(); itJoint != mapJointMonitoredName_[*it].end(); ++itJoint) {
            ROS_INFO("[agent_monitor][PRINT] Joint Monitored name: %s", (*itJoint).c_str());
        }
    }
    return true;
}

int main(int argc, char** argv) {
    // Set this in a ros service
    const bool HUMAN_FULL_CONFIG = true; //If false we will use only position and orientation
    const bool ROBOT_FULL_CONFIG = false;

    // Make this a vector? Not really relevant if several monitored agents...
    //unsigned int roomOfInterest = 0;

    // Set this in a ros service
    // TODO: Make them vectors?
    //unsigned int agentMonitored = 101;
    //std::vector < std::string > jointsMonitoredName(1, "rWristX");
    //std::vector < unsigned int > jointsMonitoredId(1, 0);
    //bool humanMonitored = agentMonitored - 100;

    // Map of Timed Ring Buffer Entities
    static std::map<unsigned int, TRBuffer < Entity* > > mapTRBEntity;
    std::map<unsigned int, TRBuffer < Entity* > >::iterator itTRB;

    ros::init(argc, argv, "agent_monitor");
    ros::NodeHandle node;

    // TODO: add area_manager data reading to get the room of entities.
    //Data reading
    PDGHumanReader humanRd(node, HUMAN_FULL_CONFIG);
    PDGRobotReader robotRd(node, ROBOT_FULL_CONFIG);
    PDGObjectReader objectRd(node);



    //Services
    ros::ServiceServer serviceAdd = node.advertiseService("agent_monitor/add_agent", addAgent);
    ROS_INFO("Ready to add agent to monitor.");

    ros::ServiceServer serviceRemove = node.advertiseService("agent_monitor/remove_agent", removeAgent);
    ROS_INFO("Ready to remove monitored agent.");

    ros::ServiceServer serviceRemoves = node.advertiseService("agent_monitor/remove_agents", removeAgents);
    ROS_INFO("Ready to remove monitored agents.");

    ros::ServiceServer serviceAddJoint = node.advertiseService("agent_monitor/add_joint_to_agent", addJointToAgent);
    ROS_INFO("Ready to add joint to monitor for agent.");

    ros::ServiceServer serviceRemoveJt = node.advertiseService("agent_monitor/remove_joint_to_agent", removeJointToAgent);
    ROS_INFO("Ready to remove monitored joint to agent.");

    ros::ServiceServer serviceRemoveJts = node.advertiseService("agent_monitor/remove_joints_to_agent", removeJointsToAgent);
    ROS_INFO("Ready to remove monitored joints to agent.");

    ros::ServiceServer servicePrint = node.advertiseService("agent_monitor/print_agents", printAgents);
    ROS_INFO("Ready to print monitored agents.");



    ros::Publisher fact_pub = node.advertise<pdg::FactList>("agent_monitor/factList", 1000);

    pdg::FactList factList_msg;
    pdg::Fact fact_msg;


    // Set this in a ros service?
    ros::Rate loop_rate(30);


    /************************/
    /* Start of the Ros loop*/
    /************************/

    while (node.ok()) {
        // We received agentMonitored

        Agent* agntCur;

        //////////////////////////////////////
        //           Updating data          //
        //////////////////////////////////////


        for (std::vector<unsigned int>::iterator itAgnt = agentsMonitored_.begin(); itAgnt != agentsMonitored_.end(); ++itAgnt) {
            Agent* agentMonitored;

            bool isHuman = true;
            // Let's find back the monitored agent:
            if (robotRd.lastConfig_[(*itAgnt)] != NULL) {
                agentMonitored = robotRd.lastConfig_[(*itAgnt)];

                //Update mapNameToAgentId_ if needed:
                if (mapNameToAgentId_.find(robotRd.lastConfig_[(*itAgnt)]->getName()) == mapNameToAgentId_.end())
                    mapNameToAgentId_[ robotRd.lastConfig_[(*itAgnt)]->getName() ] = (*itAgnt);

                isHuman = false;
            } else if (humanRd.lastConfig_[(*itAgnt)] != NULL) {

                //Update mapNameToAgentId_ if needed:
                if (mapNameToAgentId_.find(humanRd.lastConfig_[(*itAgnt)]->getName()) == mapNameToAgentId_.end())
                    mapNameToAgentId_[ humanRd.lastConfig_[(*itAgnt)]->getName() ] = (*itAgnt);

                agentMonitored = humanRd.lastConfig_[(*itAgnt)];
                isHuman = true;
            }


            // We verify if the buffer is already there...
            itTRB = mapTRBEntity.find((*itAgnt));
            if (itTRB == mapTRBEntity.end()) {

                //1st time, we initialize variables
                TRBuffer<Entity*> buffAgnt, buffJnt;

                //Swap data
                if (isHuman) {
                    agntCur = humanRd.lastConfig_[(*itAgnt)];
                    humanRd.lastConfig_[(*itAgnt)] = new Human((*itAgnt));
                    //memcpy(agntCur, humanRd.lastConfig_[(*itAgnt)], sizeof (Human));
                } else {
                    agntCur = robotRd.lastConfig_[(*itAgnt)];
                    robotRd.lastConfig_[(*itAgnt)] = new Robot((*itAgnt));
                    //memcpy(agntCur, robotRd.lastConfig_[(*itAgnt)], sizeof (Robot));
                }



                buffAgnt.push_back(agntCur->getTime(), agntCur);
                mapTRBEntity[agntCur->getId()] = buffAgnt;
                //printf("adding human named: reader %s, tmp %s, in buffer: %s\n", humanRd.lastConfig_[agentMonitored]->getName().c_str(), agntCur->getName().c_str(), mapTRBEntity[agntCur->getId()].back()->getName().c_str());



                // We will use directly the joint from the agent buffer
                /*// adding monitored joints to the entity.
                for (std::vector<std::string>::iterator itJnt = mapJointMonitoredName_[ agntCur->getId() ].begin(); itJnt != mapJointMonitoredName_[ agntCur->getId() ].end(); ++itJnt) {
                    jntCur = new Joint(agntCur->skeleton_[(*itJnt)]->getId(), agntCur->getId());
                    memcpy(jntCur, humanRd.lastConfig_[agentMonitored]->skeleton_[jointsMonitoredName[i]], sizeof (Joint));

                    buffJnt.push_back(jntCur->getTime(), jntCur);

                    mapTRBEntity[jntCur->getId()] = buffJnt;
                    // printf("adding joint named: reader %d %s, tmp %s, in buffer: %s\n", agntCur->skeleton_[jointMonitoredName]->getId(), agntCur->skeleton_[jointMonitoredName]->getName().c_str(), jntCur->getName().c_str(), mapTRBEntity[jntCur->getId()].back()->getName().c_str());
                    if (jointsMonitoredId.size() < i + 1)
                        jointsMonitoredId.push_back(jntCur->getId());

                }

                for (unsigned int i = 0; i < jointsMonitoredName.size(); i++) {
                    jntCur = new Joint(agntCur->skeleton_[jointsMonitoredName[i]]->getId(), agentMonitored);
                    memcpy(jntCur, humanRd.lastConfig_[agentMonitored]->skeleton_[jointsMonitoredName[i]], sizeof (Joint));

                    buffJnt.push_back(jntCur->getTime(), jntCur);

                    mapTRBEntity[jntCur->getId()] = buffJnt;
                    // printf("adding joint named: reader %d %s, tmp %s, in buffer: %s\n", agntCur->skeleton_[jointMonitoredName]->getId(), agntCur->skeleton_[jointMonitoredName]->getName().c_str(), jntCur->getName().c_str(), mapTRBEntity[jntCur->getId()].back()->getName().c_str());
                    if (jointsMonitoredId.size() < i + 1)
                        jointsMonitoredId.push_back(jntCur->getId());

                }*/


                // This module is made for temporal reasoning.
                // We need more data to make computation, so we will end the loop here.

                // This will be done at the end of the for loop
                //ros::spinOnce();
                //loop_rate.sleep();
                continue;


            } else { // Agent is present in mapTRBEntity
                // If this is a new data we add it to the buffer
                bool newData = false;

                if (isHuman)
                    newData = (mapTRBEntity[(*itAgnt)].back()->getTime() < humanRd.lastConfig_[(*itAgnt)]->getTime());
                else
                    newData = (mapTRBEntity[(*itAgnt)].back()->getTime() < robotRd.lastConfig_[(*itAgnt)]->getTime());

                if (newData) {

                    //Swap data
                    if (isHuman) {
                        agntCur = humanRd.lastConfig_[(*itAgnt)];
                        humanRd.lastConfig_[(*itAgnt)] = new Human((*itAgnt));
                        //memcpy(agntCur, humanRd.lastConfig_[(*itAgnt)], sizeof (Human));
                    } else {
                        agntCur = robotRd.lastConfig_[(*itAgnt)];
                        robotRd.lastConfig_[(*itAgnt)] = new Robot((*itAgnt));
                        //memcpy(agntCur, robotRd.lastConfig_[(*itAgnt)], sizeof (Robot));
                    }

                    mapTRBEntity[agntCur->getId()].push_back(agntCur->getTime(), agntCur);
                    //printf("adding human named: reader %s, tmp %s, in buffer: %s\n", humanRd.lastConfig_[agentMonitored]->getName().c_str(), humCur->getName().c_str(), mapTRBEntity[humCur->getId()].back()->getName().c_str());


                    /*
                    // adding monitored joint to the entities.
                    for (unsigned int i = 0; i < jointsMonitoredName.size(); i++) {
                        jntCur = new Joint(humCur->skeleton_[jointsMonitoredName[i]]->getId(), agentMonitored);
                        memcpy(jntCur, humanRd.lastConfig_[agentMonitored]->skeleton_[jointsMonitoredName[i]], sizeof (Joint));

                        mapTRBEntity[jntCur->getId()].push_back(jntCur->getTime(), jntCur);
                        //printf("adding joint named: reader %d %s, tmp %s, in buffer: %s\n", humCur->skeleton_[jointMonitoredName]->getId(), humCur->skeleton_[jointMonitoredName]->getName().c_str(), jntCur->getName().c_str(), mapTRBEntity[jntCur->getId()].back()->getName().c_str());
                    }
                     */

                    // If we don't have new data
                    // Do we need this? Or should we update other entities?
                } else {
                    //printf("agent received without greater time: current is %lu < previous is %lu\n", humanRd.lastConfig_[agentMonitored]->getTime(), mapTRBEntity[agentMonitored].back()->getTime());

                    // This will be done at the end of the for loop
                    //ros::spinOnce();
                    //loop_rate.sleep();
                    continue;
                }
            }







            /////////////////////////////////////
            // Update TRBuffer for each entity //
            /////////////////////////////////////



            // for each entity
            //Put the following in a function?


            // For humans
            for (std::map<unsigned int, Human*>::iterator it = humanRd.lastConfig_.begin(); it != humanRd.lastConfig_.end(); ++it) {

                //Update mapNameToAgentId_ if needed:
                if (mapNameToAgentId_.find(it->second->getName()) == mapNameToAgentId_.end())
                    mapNameToAgentId_[ it->second->getName() ] = it->first;

                // if not monitored agent
                if (it->first != (*itAgnt)) {
                    itTRB = mapTRBEntity.find(it->first);

                    // If 1st data
                    if (itTRB == mapTRBEntity.end()) {
                        TRBuffer<Entity*> buffHum;

                        // Data Swap

                        Human * hum = humanRd.lastConfig_[it->first];
                        humanRd.lastConfig_[it->first] = new Human(it->first);
                        //Human * hum = new Human(it->first);
                        //memcpy(hum, humanRd.lastConfig_[it->first], sizeof (Human));

                        buffHum.push_back(hum->getTime(), hum);
                        mapTRBEntity[it->first] = buffHum;
                        //printf("adding human name: reader %s, tmp %s, in buffer: %s\n", humanRd.lastConfig_[it->first]->getName().c_str(), hum->getName().c_str(), mapTRBEntity[it->first].back()->getName().c_str());

                        // If this is a new data we add it
                    } else if (mapTRBEntity[it->first].back()->getTime() < it->second->getTime()) {
                        Human * hum = humanRd.lastConfig_[it->first];
                        humanRd.lastConfig_[it->first] = new Human(it->first);
                        //Human * hum = new Human(it->first);
                        //memcpy(hum, humanRd.lastConfig_[it->first], sizeof (Human));
                        mapTRBEntity[it->first].push_back(hum->getTime(), hum);
                    }
                }

            }

            // For robots
            for (std::map<unsigned int, Robot*>::iterator it = robotRd.lastConfig_.begin(); it != robotRd.lastConfig_.end(); ++it) {

                //Update mapNameToAgentId_ if needed:
                if (mapNameToAgentId_.find(it->second->getName()) == mapNameToAgentId_.end())
                    mapNameToAgentId_[ it->second->getName() ] = it->first;

                // if not monitored agent
                if (it->first != (*itAgnt)) {
                    itTRB = mapTRBEntity.find(it->first);
                    if (itTRB == mapTRBEntity.end()) {
                        TRBuffer<Entity*> buffRob;

                        Robot* rob = robotRd.lastConfig_[it->first];
                        robotRd.lastConfig_[it->first] = new Robot(it->first);
                        //Robot* rob = new Robot(it->first);
                        //memcpy(rob, robotRd.lastConfig_[it->first], sizeof (Robot));

                        buffRob.push_back(rob->getTime(), rob);
                        mapTRBEntity[it->first] = buffRob;
                        //printf("adding robot name: reader %s, tmp %s, in buffer: %s\n", robotRd.lastConfig_[it->first]->getName().c_str(), rob->getName().c_str(), mapTRBEntity[it->first].back()->getName().c_str());

                        // If this is a new data we add it
                    } else if (mapTRBEntity[it->first].back()->getTime() < it->second->getTime()) {
                        Robot* rob = robotRd.lastConfig_[it->first];
                        robotRd.lastConfig_[it->first] = new Robot(it->first);
                        //Robot* rob = new Robot(it->first);
                        //memcpy(rob, robotRd.lastConfig_[it->first], sizeof (Robot));
                        mapTRBEntity[it->first].push_back(rob->getTime(), rob);
                        //printf("adding robot name: reader %s, tmp %s, in buffer: %s\n", robotRd.lastConfig_[it->first]->getName().c_str(), rob->getName().c_str(), mapTRBEntity[it->first].back()->getName().c_str());
                    }
                }

            }

            //  For Objects
            for (std::map<unsigned int, Object*>::iterator it = objectRd.lastConfig_.begin(); it != objectRd.lastConfig_.end(); ++it) {
                // if in same room as monitored agent and not monitored agent
                //if (roomOfInterest == it->second->getRoomId()) {
                    itTRB = mapTRBEntity.find(it->first);
                    if (itTRB == mapTRBEntity.end()) {
                        TRBuffer<Entity*> buffObj;

                        Object* obj = objectRd.lastConfig_[it->first];
                        objectRd.lastConfig_[it->first] = new Object(it->first);
                        //Object* obj = new Object(it->first);
                        //memcpy(obj, objectRd.lastConfig_[it->first], sizeof (Object));

                        buffObj.push_back(obj->getTime(), obj);

                        mapTRBEntity[it->first] = buffObj;
                        //printf("adding object name: reader %s, tmp %s, in buffer: %s\n", objectRd.lastConfig_[it->first]->getName().c_str(), obj->getName().c_str(), mapTRBEntity[it->first].back()->getName().c_str());

                        // If this is a new data we add it
                    } else if (mapTRBEntity[it->first].back()->getTime() < it->second->getTime()) {
                        Object* obj = objectRd.lastConfig_[it->first];
                        objectRd.lastConfig_[it->first] = new Object(it->first);
                        //Object* obj = new Object(it->first);
                        //memcpy(obj, objectRd.lastConfig_[it->first], sizeof (Object));
                        //printf("adding object name: reader %s, tmp %s, in buffer: %s\n", objectRd.lastConfig_[it->first]->getName().c_str(), obj->getName().c_str(), mapTRBEntity[it->first].back()->getName().c_str());
                    }
                //} // TODO: else remove

            }




            //////////////////////////////////////////////
            // Compute facts concerning monitored agent //
            //////////////////////////////////////////////



            // Compute motion:
            unsigned long oneSecond = pow(10, 9);

            //if (!monitoredBufferInit) {
            //   printf("[AGENT_MONITOR][WARNING] agent monitored not found\n");
            //}else{
            //printf("[AGENT_MONITOR][DEBUG] agent from buffer %s is null? %d \n [AGENT_MONITOR][DEBUG] agent from reader %s is null? %d \n", mapTRBEntity[(*itAgnt)].back()->getName().c_str(),  mapTRBEntity[(*itAgnt)].back() == NULL, humanRd.lastConfig_[(*itAgnt)]->getName().c_str(), humanRd.lastConfig_[(*itAgnt)] == NULL);  
            //printf("[AGENT_MONITOR][WARNING] agent monitored buffer size %d, max_size %d, full %d, back is null? %d\n", mapTRBEntity[(*itAgnt)].size(), mapTRBEntity[(*itAgnt)].max_size(), mapTRBEntity[(*itAgnt)].full(), mapTRBEntity[(*itAgnt)].back() == NULL);

            // If the agent is moving
            if (computeMotion2D(mapTRBEntity[(*itAgnt)], oneSecond / 4, 0.03)) {
                printf("[AGENT_MONITOR][DEBUG] %s is moving %lu\n", mapTRBEntity[(*itAgnt)].back()->getName().c_str(), mapTRBEntity[(*itAgnt)].back()->getTime());

                //Fact moving
                fact_msg.property = "IsMoving";
                fact_msg.propertyType = "motion";
                fact_msg.subProperty = "agent";
                fact_msg.subjectId = (*itAgnt);
                fact_msg.subjectName = mapTRBEntity[(*itAgnt)].back()->getName().c_str();
                fact_msg.stringValue = "true";
                fact_msg.confidence = 0.90;
                fact_msg.time = mapTRBEntity[(*itAgnt)].back()->getTime();

                factList_msg.factList.push_back(fact_msg);


                double angleDirection = 0.0;
                std::map<unsigned int, double> mapIdValue;

                // We compute the direction toward fact:
                angleDirection = computeMotion2DDirection(mapTRBEntity[(*itAgnt)], oneSecond);
                mapIdValue = computeMotion2DToward(mapTRBEntity, (*itAgnt), angleDirection, 0.5);
                for (std::map<unsigned int, double>::iterator it = mapIdValue.begin(); it != mapIdValue.end(); ++it) {
                    if (it->second > 0.0)
                        printf("[AGENT_MONITOR][DEBUG] %s is moving toward %s with a confidence of %f\n",
                            mapTRBEntity[(*itAgnt)].back()->getName().c_str(), mapTRBEntity[it->first].back()->getName().c_str(), it->second);

                    //Fact moving toward
                    fact_msg.property = "IsMovingToward";
                    fact_msg.propertyType = "motion";
                    fact_msg.subProperty = "direction";
                    fact_msg.subjectId = (*itAgnt);
                    fact_msg.subjectName = mapTRBEntity[(*itAgnt)].back()->getName().c_str();
                    fact_msg.targetId = it->first;
                    fact_msg.targetName = mapTRBEntity[it->first].back()->getName().c_str();
                    fact_msg.confidence = it->second;
                    fact_msg.time = mapTRBEntity[(*itAgnt)].back()->getTime();

                    factList_msg.factList.push_back(fact_msg);
                }

                // Then we compute /_\distance
                mapIdValue = computeDeltaDist(mapTRBEntity, (*itAgnt), oneSecond / 4);
                for (std::map<unsigned int, double>::iterator it = mapIdValue.begin(); it != mapIdValue.end(); ++it) {
                    printf("[AGENT_MONITOR][DEBUG] agent %s has a deltadist toward  %s of %f\n",
                            mapTRBEntity[(*itAgnt)].back()->getName().c_str(), mapTRBEntity[it->first].back()->getName().c_str(), it->second);

                    //Fact moving toward
                    fact_msg.property = "IsMovingToward";
                    fact_msg.propertyType = "motion";
                    fact_msg.subProperty = "distance";
                    fact_msg.subjectId = (*itAgnt);
                    fact_msg.subjectName = mapTRBEntity[(*itAgnt)].back()->getName().c_str();
                    fact_msg.targetId = it->first;
                    fact_msg.targetName = mapTRBEntity[it->first].back()->getName().c_str();
                    fact_msg.confidence = it->second;
                    fact_msg.time = mapTRBEntity[(*itAgnt)].back()->getTime();

                }

                // If agent is not moving, we compute his joint motion
                // TODO: do this in 3D!
            } else {

                std::map<unsigned int, double> mapIdValue;
                double dist3D;
                std::string dist3DString;

                // What is the distance between joints and objects?
                for (std::vector<std::string>::iterator itJnt = mapJointMonitoredName_[(*itAgnt)].begin(); itJnt != mapJointMonitoredName_[(*itAgnt)].end(); ++itJnt) {
                    Joint* curMonitoredJnt = ((Agent*) mapTRBEntity[(*itAgnt)].back())->skeleton_[(*itJnt)];

                    for (std::map<unsigned int, TRBuffer < Entity*> >::iterator itEnt = mapTRBEntity.begin(); itEnt != mapTRBEntity.end(); ++itEnt) {
                        // if in same room as monitored agent and not monitored joint
                        //if ((roomOfInterest == it->second.back()->getRoomId()) && (it->first != jointsMonitoredId[i])) {
                        dist3D = bg::distance(curMonitoredJnt->getPosition(), itEnt->second.back()->getPosition());

                        if (dist3D < 0.05)
                            dist3DString = "reach";
                        else if (dist3D < 0.2)
                            dist3DString = "close";
                        else if (dist3D < 1.5)
                            dist3DString = "medium";
                        else if (dist3D < 8)
                            dist3DString = "far";
                        else
                            dist3DString = "out";

                        //Fact distance
                        fact_msg.property = "Distance";
                        fact_msg.propertyType = "position";
                        fact_msg.subProperty = "3D";
                        fact_msg.subjectId = curMonitoredJnt->getId();
                        fact_msg.subjectName = (*itJnt).c_str();
                        fact_msg.targetId = itEnt->first;
                        fact_msg.targetName = mapTRBEntity[itEnt->first].back()->getName().c_str();
                        fact_msg.valueType = 0;
                        fact_msg.stringValue = dist3DString;
                        fact_msg.doubleValue = dist3D;
                        fact_msg.confidence = 0.90;
                        fact_msg.time = curMonitoredJnt->getTime();

                        factList_msg.factList.push_back(fact_msg);

                        //}
                    }
                    // Is the joint moving?
                    if (computeJointMotion2D(mapTRBEntity[(*itAgnt)], (*itJnt), oneSecond / 4, 0.03)) {
                        printf("[AGENT_MONITOR][DEBUG] %s of agent %s is moving %lu\n", (*itJnt).c_str(), mapTRBEntity[(*itAgnt)].back()->getName().c_str(), mapTRBEntity[(*itAgnt)].back()->getTime());


                        //Fact moving
                        fact_msg.property = "IsMoving";
                        fact_msg.propertyType = "motion";
                        fact_msg.subProperty = "joint";
                        fact_msg.subjectId = curMonitoredJnt->getId();
                        fact_msg.subjectName = (*itJnt).c_str();
                        fact_msg.valueType = 0;
                        fact_msg.stringValue = "true";
                        fact_msg.confidence = 0.90;
                        fact_msg.time = curMonitoredJnt->getTime();

                        factList_msg.factList.push_back(fact_msg);


                        double angleDirection = 0.0;

                        // We compute the direction toward fact:
                        angleDirection = computeJointMotion2DDirection(mapTRBEntity[(*itAgnt)], (*itJnt), oneSecond);
                        mapIdValue = computeJointMotion2DToward(mapTRBEntity, (*itAgnt), (*itJnt), angleDirection, 0.5);
                        for (std::map<unsigned int, double>::iterator it = mapIdValue.begin(); it != mapIdValue.end(); ++it) {
                            if (it->second > 0.0)
                                printf("[AGENT_MONITOR][DEBUG] %s of agent %s is moving toward %s with a confidence of %f\n", (*itJnt).c_str(),
                                    mapTRBEntity[(*itAgnt)].back()->getName().c_str(), mapTRBEntity[it->first].back()->getName().c_str(), it->second);

                            //Fact moving toward
                            fact_msg.property = "IsMovingToward";
                            fact_msg.propertyType = "motion";
                            fact_msg.subProperty = "direction";
                            fact_msg.subjectId = curMonitoredJnt->getId();
                            fact_msg.subjectName = (*itJnt).c_str();
                            fact_msg.targetId = it->first;
                            fact_msg.targetName = mapTRBEntity[it->first].back()->getName().c_str();
                            fact_msg.confidence = it->second;
                            fact_msg.time = curMonitoredJnt->getTime();

                            factList_msg.factList.push_back(fact_msg);
                        }

                        // Then we compute /_\distance
                        mapIdValue = computeJointDeltaDist(mapTRBEntity, (*itAgnt), (*itJnt), oneSecond / 4);
                        for (std::map<unsigned int, double>::iterator it = mapIdValue.begin(); it != mapIdValue.end(); ++it) {
                            printf("[AGENT_MONITOR][DEBUG] joint %s of agent %s has a deltadist toward  %s of %f\n", (*itJnt).c_str(),
                                    mapTRBEntity[(*itAgnt)].back()->getName().c_str(), mapTRBEntity[it->first].back()->getName().c_str(), it->second);

                            //Fact moving toward
                            fact_msg.property = "IsMovingToward";
                            fact_msg.propertyType = "motion";
                            fact_msg.subProperty = "distance";
                            fact_msg.subjectId = curMonitoredJnt->getId();
                            fact_msg.subjectName = (*itJnt).c_str();
                            fact_msg.targetId = it->first;
                            fact_msg.targetName = mapTRBEntity[it->first].back()->getName().c_str();
                            fact_msg.confidence = it->second;
                            fact_msg.time = curMonitoredJnt->getTime();
                        }
                    } // Joint moving
                } // All monitored joints
            } // Joints or full agent?
        } // Is monitored agent here?

        fact_pub.publish(factList_msg);

        ros::spinOnce();

        factList_msg.factList.clear();

        loop_rate.sleep();

    }
    return 0;
}