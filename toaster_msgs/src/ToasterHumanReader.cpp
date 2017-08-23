/*
 * File:   ToasterObjectReader.cpp
 * Author: gmilliez
 *
 * Created on December 12, 2014, 2:23 PM
 */

#include "toaster_msgs/ToasterHumanReader.h"
#include "tf/transform_datatypes.h"

ToasterHumanReader::ToasterHumanReader(ros::NodeHandle& node, bool fullHuman, std::string topic) : EntityReader<Human>(fullHuman){
    std::cout << "[Toaster_msgs] Initializing ToasterHumanReader" << std::endl;
    // Starts listening to the topic
    sub_ = node.subscribe(topic, 1, &ToasterHumanReader::humanJointStateCallBack, this);
}

void ToasterHumanReader::humanJointStateCallBack(const toaster_msgs::HumanListStamped::ConstPtr& msg) {

    double roll, pitch, yaw;
    Human * curHuman;

    for (unsigned int i = 0; i < msg->humanList.size(); i++) {

        // If this human is not assigned we have to allocate data.
        if (lastConfig_.find(msg->humanList[i].meAgent.meEntity.id) == lastConfig_.end()) {
            curHuman = new Human(msg->humanList[i].meAgent.meEntity.id);
        } else
            curHuman = lastConfig_[msg->humanList[i].meAgent.meEntity.id];

        std::vector<double> humanOrientation;
        bg::model::point<double, 3, bg::cs::cartesian> humanPosition;

        Mobility curHumanMobility = FULL;
        curHuman->setId(msg->humanList[i].meAgent.meEntity.id);
        curHuman->setName(msg->humanList[i].meAgent.meEntity.name);

        curHuman->setMobility(curHumanMobility);
        curHuman->setTime(msg->humanList[i].meAgent.meEntity.time);
        curHuman->busyHands_ = msg->humanList[i].meAgent.busyHands;

        humanPosition.set<0>(msg->humanList[i].meAgent.meEntity.pose.position.x);
        humanPosition.set<1>(msg->humanList[i].meAgent.meEntity.pose.position.y);
        humanPosition.set<2>(msg->humanList[i].meAgent.meEntity.pose.position.z);
        curHuman->setPosition(humanPosition);

        tf::Quaternion q;

        tf::quaternionMsgToTF(msg->humanList[i].meAgent.meEntity.pose.orientation, q);
        tf::Matrix3x3(q).getRPY(roll, pitch, yaw);

        humanOrientation.push_back(roll);
        humanOrientation.push_back(pitch);
        humanOrientation.push_back(yaw);
        curHuman->setOrientation(humanOrientation);

        lastConfig_[curHuman->getId()] = curHuman;

        if (fullConfig_) {
            Joint * curJnt;
            for (unsigned int i_jnt = 0; i_jnt < msg->humanList[i].meAgent.skeletonJoint.size(); i_jnt++) {

                // If this joint is not assigned we have to allocate data.
                if (lastConfig_[curHuman->getId()]->skeleton_[msg->humanList[i].meAgent.skeletonJoint[i_jnt].meEntity.name ] == NULL) {
                    curJnt = new Joint(msg->humanList[i].meAgent.skeletonJoint[i_jnt].meEntity.id, msg->humanList[i].meAgent.meEntity.id);
                } else
                    curJnt = lastConfig_[curHuman->getId()]->skeleton_[msg->humanList[i].meAgent.skeletonJoint[i_jnt].meEntity.name ];

                std::vector<double> jointOrientation;
                bg::model::point<double, 3, bg::cs::cartesian> jointPosition;

                curJnt->setName(msg->humanList[i].meAgent.skeletonJoint[i_jnt].meEntity.name);
                curJnt->setAgentId(curHuman->getId());
                curJnt->setTime(msg->humanList[i].meAgent.skeletonJoint[i_jnt].meEntity.time);

                jointPosition.set<0>(msg->humanList[i].meAgent.skeletonJoint[i_jnt].meEntity.pose.position.x);
                jointPosition.set<1>(msg->humanList[i].meAgent.skeletonJoint[i_jnt].meEntity.pose.position.y);
                jointPosition.set<2>(msg->humanList[i].meAgent.skeletonJoint[i_jnt].meEntity.pose.position.z);
                curJnt->setPosition(jointPosition);

                tf::Quaternion q;

                tf::quaternionMsgToTF(msg->humanList[i].meAgent.skeletonJoint[i_jnt].meEntity.pose.orientation, q);
                tf::Matrix3x3(q).getRPY(roll, pitch, yaw);

                jointOrientation.push_back(roll);
                jointOrientation.push_back(pitch);
                jointOrientation.push_back(yaw);
                curJnt->setOrientation(jointOrientation);

                curJnt->position = msg->humanList[i].meAgent.skeletonJoint[i_jnt].position;

                lastConfig_[curHuman->getId()]->skeleton_[curJnt->getName()] = curJnt;
            }
        }
    }
}
