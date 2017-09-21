#include "LookingFact.h"

#include "toaster-lib/MathFunctions.h"
#include "toaster-lib/Joint.h"
#include "toaster-lib/Agent.h"

#include "toaster_msgs/Fact.h"

using namespace std;

map<string, double> LookingFact::compute(map<string, TRBuffer < Entity* > > mapEnts,
                                        string agentMonitored, double deltaDist,
                                        double angularAperture)
{
    Map_t returnMap;
    Pair_t pair;
    Entity * currentEntity;
    Entity * monitoredAgentHead;
    float halfAperture = angularAperture / 2.f;
    Vec_t agentHeadPosition(3);
    Vec_t agentHeadOrientation(3);
    Vec_t entityPosition(3);
    Vec_t agentToEntity(3);
    Vec_t coneAxis(3);
    Vec_t coneBase(3);
    float entitytoAxisAngle;
    Mat_t rotX(3);
    Mat_t rotY(3);
    Mat_t rotZ(3);
    bool isInInfiniteCone = false;
    double angle;

    //Get the monitored agent head entity
    //TODO add a rosparam for robot's head joint name
    string jointName = "";
    if (agentMonitored == "pr2")
      jointName = "head_tilt_link";
    else
      jointName = "head";

    map<string, Joint*> skelMap = ((Agent*) mapEnts[agentMonitored].back())->skeleton_;
    if (skelMap.find(jointName) != skelMap.end())
      monitoredAgentHead = skelMap[jointName];
    else
      return returnMap;

    //Get 3d position from agent head
    agentHeadPosition[0] = bg::get<0>(monitoredAgentHead->getPosition());
    agentHeadPosition[1] = bg::get<1>(monitoredAgentHead->getPosition());
    agentHeadPosition[2] = bg::get<2>(monitoredAgentHead->getPosition());
    //Get 3d orientation (roll pitch yaw) from agent head
    agentHeadOrientation = (Vec_t) monitoredAgentHead->getOrientation();
    //Compute rotation matricies from agent head orientation
    rotX = MathFunctions::matrixfromAngle(0, agentHeadOrientation[0]);
    rotY = MathFunctions::matrixfromAngle(1, agentHeadOrientation[1]);
    rotZ = MathFunctions::matrixfromAngle(2, agentHeadOrientation[2]);

    for (map<string, TRBuffer < Entity*> >::iterator it = mapEnts.begin(); it != mapEnts.end(); ++it) {
      if (it->first != agentMonitored)
      {
        //Get the current entity
        string jointName = "";
        if (it->first == "pr2")
          jointName = "head_tilt_link";
        else if(it->first == "HERAKLES_HUMAN1" || it->first == "HERAKLES_HUMAN2")
          jointName = "head";

        if(jointName != "") // robot or human
        {
          map<string, Joint*> skelMap = ((Agent*) it->second.back())->skeleton_;
          if (skelMap.find(jointName) != skelMap.end())
            currentEntity = skelMap[jointName];
          else
            break;
        }
        else //objects
          currentEntity = it->second.back();

        //Get the postion from current entity
        entityPosition[0] = bg::get<0>(currentEntity->getPosition());
        entityPosition[1] = bg::get<1>(currentEntity->getPosition());
        entityPosition[2] = bg::get<2>(currentEntity->getPosition());
        //Compute cone base coordinates
        coneBase[0] = agentHeadPosition[0] + deltaDist;
        coneBase[1] = agentHeadPosition[1];
        coneBase[2] = agentHeadPosition[2];
        //Compute cone axis
        coneAxis = MathFunctions::diffVec(agentHeadPosition, coneBase);
        //Apply rotation matrix from agent head orientation to cone base
        coneAxis = MathFunctions::multiplyMatVec(rotY, coneAxis);
        coneAxis = MathFunctions::multiplyMatVec(rotZ, coneAxis);
        //Compute the 3d vector from agent head to current entity
        agentToEntity = MathFunctions::diffVec(agentHeadPosition, entityPosition);
        //Compute angle
        angle = (MathFunctions::dotProd(agentToEntity, coneAxis) / MathFunctions::magn(agentToEntity) / MathFunctions::magn(coneAxis));
        //Test
        if (angle > cos(halfAperture))
          if (MathFunctions::dotProd(agentToEntity, coneAxis) / MathFunctions::magn(coneAxis) < MathFunctions::magn(coneAxis))
              returnMap.insert(std::pair<string, double>(it->first, angle));
      }
    }
    return returnMap;
}

void LookingFact::createTowardFact(map<string, double> mapIdValue, toaster_msgs::FactList &factList_msg,
                                  double angle, string subjectId, Entity* entity)
{
  createFact(mapIdValue, factList_msg, angle, subjectId,entity, "IsLookingToward");
}

void LookingFact::createAtFact(map<string, double> mapIdValue, toaster_msgs::FactList &factList_msg,
                                  double angle, string subjectId, Entity* entity)
{
  createFact(mapIdValue, factList_msg, angle, subjectId,entity, "IsLookingAt");
}

void LookingFact::createFact(map<string, double> mapIdValue, toaster_msgs::FactList &factList_msg,
                            double angle, string subjectId, Entity* entity, string property)
{
  if (!mapIdValue.empty())
  {
    for (std::map<std::string, double>::iterator it = mapIdValue.begin(); it != mapIdValue.end(); ++it)
    {
      toaster_msgs::Fact fact_msg;
      fact_msg.property = property;
      fact_msg.propertyType = "attention";
      fact_msg.subProperty = "agent";
      fact_msg.stringValue = "";
      fact_msg.subjectId = subjectId;
      fact_msg.targetId = it->first;
      fact_msg.confidence = (angle - it->second) / angle;
      fact_msg.doubleValue = it->second;
      fact_msg.time = entity->getTime();
      fact_msg.subjectOwnerId = "";
      fact_msg.targetOwnerId = "";
      fact_msg.valueType = 1;

      factList_msg.factList.push_back(fact_msg);
    }
  }
}
