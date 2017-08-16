/*
 * File:   GroupHumanReader.cpp
 * Author: gmilliez
 *
 * Created on April 21, 2015, 12:51 AM
 */

#include "pdg/readers/GroupHumanReader.h"

#include "geometry_msgs/PoseStamped.h"
#include <sys/time.h>
#include <math.h>
#include <ostream>

GroupHumanReader::GroupHumanReader(bool fullHuman) : HumanReader()
{
  fullHuman_ = fullHuman;
  listener_ = nullptr;
}

GroupHumanReader::~GroupHumanReader(){
  if(listener_ != nullptr)
    delete listener_;

  for(std::map<std::string, Human*>::iterator it = lastConfig_.begin() ; it != lastConfig_.end(); ++it){
      delete it->second;
  }
}

void GroupHumanReader::init(ros::NodeHandle* node, std::string topic, std::string param)
{
  std::cout << "[PDG] Initializing GroupHumanReader" << std::endl;
  Reader<Human>::init(node, param);
  // ******************************************
  // Starts listening to the joint_states
  sub_ = node_->subscribe(topic, 1, &GroupHumanReader::groupTrackCallback, this);
  listener_ = new tf::TransformListener;
}

/*
  Gets data from a TrackedPersons msg in the human map. This msg contains a list of agens with
  their positions and orientations.
 */
void GroupHumanReader::groupTrackCallback(const spencer_tracking_msgs::TrackedGroups::ConstPtr& msg) {
    tf::StampedTransform transform;
    ros::Time now = ros::Time::now();
    std::stringstream humId;

    try {
        std::string frame;
        frame = msg->header.frame_id;

        //transform from the groupTrack frame to map
        listener_->waitForTransform("/map", frame,
                msg->header.stamp, ros::Duration(3.0));
        listener_->lookupTransform("/map", frame,
                msg->header.stamp, transform);

        //for every group present in the tracking message
        for (int i = 0; i < msg->groups.size(); i++) {
            spencer_tracking_msgs::TrackedGroup group = msg->groups[i];
            humId << " group" << group.group_id;
            //create a new human with the same id as the message
            Human* curHuman = new Human(humId.str());

            //get the pose of the agent in the groupTrack frame and transform it to the map frame
            geometry_msgs::PoseStamped groupTrackPose, mapPose;
            //geometry_msgs::PoseStamped optitrackPose, mapPose;

            groupTrackPose.pose.position = group.centerOfGravity.pose.position;
            groupTrackPose.pose.orientation = group.centerOfGravity.pose.orientation;
            groupTrackPose.header.stamp = msg->header.stamp;
            groupTrackPose.header.frame_id = frame;
            listener_->transformPose("/map", groupTrackPose, mapPose);

            //set human position
            bg::model::point<double, 3, bg::cs::cartesian> humanPosition;
            humanPosition.set<0>(mapPose.pose.position.x);
            humanPosition.set<1>(mapPose.pose.position.y);
            humanPosition.set<2>(mapPose.pose.position.z);

            //set the human orientation
            std::vector<double> humanOrientation;

            //transform the pose message
            humanOrientation.push_back(0.0);
            humanOrientation.push_back(0.0);
            humanOrientation.push_back(tf::getYaw(mapPose.pose.orientation));

            //put the data in the human
            curHuman->setOrientation(humanOrientation);
            curHuman->setPosition(humanPosition);
            curHuman->setTime(now.toNSec());

            lastConfig_[humId.str()] = curHuman;
        }
    } catch (tf::TransformException ex) {
        ROS_ERROR("%s", ex.what());


    }
}
