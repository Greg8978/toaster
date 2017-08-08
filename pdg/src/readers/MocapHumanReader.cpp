// A human reader is a class that will read data from human(s)


#include "pdg/readers/MocapHumanReader.h"

#include "geometry_msgs/PoseStamped.h"
#include <sys/time.h>
#include <math.h>
#include <ostream>

MocapHumanReader::MocapHumanReader(ros::NodeHandle& node, std::string topic) {
    std::cout << "Initializing MocapHumanReader" << std::endl;
    // ******************************************
    // Starts listening to the joint_states
    fullHuman_ = false;
    sub_ = node.subscribe(topic, 1, &MocapHumanReader::optitrackCallback, this);
    std::cout << "Done\n";
}

/*
  Gets data from a TrackedPersons msg in the human map. This msg contains a list of agens with
  their positions and orientations.
 */
void MocapHumanReader::optitrackCallback(const spencer_tracking_msgs::TrackedPersons::ConstPtr& msg) {
    tf::StampedTransform transform;
    ros::Time now = ros::Time::now();
    Human* curHuman;
    std::stringstream humId;

    try {
        std::string frame;
        frame = msg->header.frame_id;

        //transform from the mocap frame to map
        listener_.waitForTransform("/map", frame,
                msg->header.stamp, ros::Duration(3.0));
        listener_.lookupTransform("/map", frame,
                msg->header.stamp, transform);

        //for every agent present in the tracking message
        for (int i = 0; i < msg->tracks.size(); i++) {
            std::string humanName = "human";
            spencer_tracking_msgs::TrackedPerson person = msg->tracks[i];
            humId << "mocap_human" << person.track_id;
            //create a new human with the same id as the message
            if (lastConfig_.find(humId.str()) == lastConfig_.end()) {
                curHuman = new Human(humId.str());
                humanName.append(boost::to_string(humId.str()));
                curHuman->setName(humanName);
            } else {
                curHuman = lastConfig_[humId.str()];
            }

            //get the pose of the agent in the optitrack frame and transform it to the map frame
            geometry_msgs::PoseStamped optitrackPose, mapPose;
            optitrackPose.pose.position = person.pose.pose.position;
            optitrackPose.pose.orientation = person.pose.pose.orientation;
            optitrackPose.header.stamp = msg->header.stamp;
            optitrackPose.header.frame_id = frame;
            listener_.transformPose("/map", optitrackPose, mapPose);

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
