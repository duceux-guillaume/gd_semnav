#include <ros/ros.h>
#include <sn_msgs/DetectionArray.h>
#include <eigen3/Eigen/Core>
#include <visualization_msgs/Marker.h>
#include <sn_msgs/TrackerArray.h>
#include <sn_geometry/sn_geometry.h>

struct Tracking{
    ros::Publisher mDebugPub;
    ros::Publisher mTrackerPub;
    ros::Publisher mTrackerPub2;
    ros::Subscriber mDetectSub;

    double mThreshold;
    double mTimeThreshold;
    sn_msgs::TrackerArray mTrackers;
    std::map<ros::Time, std::array<float, 3>> mColors;

    void detection_callback(const sn_msgs::DetectionArrayConstPtr &ptr);
    void track();
    sn_msgs::Tracker create_tracker(const sn_msgs::Detection& det);

};

sn_msgs::Tracker Tracking::create_tracker(const sn_msgs::Detection& det){
    sn_msgs::Tracker res;
    res.detections.push_back(det);
    res.ended = false;
    res.uid = ros::Time::now();
    res.header = det.header;
    return res;
}

void Tracking::detection_callback(const sn_msgs::DetectionArrayConstPtr &ptr){

    mTrackers.header = ptr->header;
    for(auto& tck: mTrackers.trackers)
        tck.ended = true;

    for(auto& det: ptr->detections){

        // find closest tracker
        double mindist = std::numeric_limits<double>::max();
        sn_msgs::Tracker* arg_min = NULL;
        bool found = false;

        for(auto& tck: mTrackers.trackers){
            auto& dk = tck.detections.back();
            sn::point_t center1 = dk.bbox.center;
            sn::point_t center2 = det.bbox.center;
            double distance = sn::l2_norm(center1-center2);
            if(distance < mindist && distance<mThreshold){
                arg_min=&tck;
                mindist = distance;
                found = true;
            }
        }

        if(!found){
            // new tracker
            mTrackers.trackers.push_back(create_tracker(det));
        }else{
            arg_min->detections.push_back(det);
            arg_min->ended = false;
            arg_min->header = ptr->header;
        }
    }

    mTrackerPub.publish(mTrackers);

    // erase those that haven´t been updated
    for(uint i=0; i<mTrackers.trackers.size(); ++i){
        ros::Duration duration = mTrackers.header.stamp - mTrackers.trackers[i].header.stamp;
        if(duration.toSec() > mTimeThreshold && mTrackers.trackers[i].ended){
            mTrackerPub2.publish(mTrackers.trackers[i]);
            mTrackers.trackers.erase(mTrackers.trackers.begin()+i);
            --i;
        }
    }

    if(mDebugPub.getNumSubscribers() == 0)
        return;

    for(auto& tck: mTrackers.trackers){
        auto& det = tck.detections.back();
        visualization_msgs::Marker marker;
        marker.type = visualization_msgs::Marker::CYLINDER;
        marker.action = visualization_msgs::Marker::ADD;
        auto center = det.bbox.center;
        marker.pose.position.x = center.x;
        marker.pose.position.y = center.y;
        marker.pose.position.z = center.z;
        marker.pose.orientation.x = 0.0;
        marker.pose.orientation.y = 0.0;
        marker.pose.orientation.z = 0.0;
        marker.pose.orientation.w = 1.0;
        auto label = tck.uid;
        if(mColors.count( label ) == 0){
            static std::default_random_engine generator;
            static std::uniform_real_distribution<float> distribution(0.f,1.f);
            static auto random = std::bind ( distribution, generator );
            mColors[label][0] = random();
            mColors[label][1] = random();
            mColors[label][2] = random();
        }
        marker.scale.x = mThreshold;
        marker.scale.y = mThreshold;
        marker.scale.z = 0.01;
        marker.color.r = mColors[label][0];
        marker.color.g = mColors[label][1];
        marker.color.b = mColors[label][2];
        marker.color.a = 1.0;
        marker.lifetime = ros::Duration(mTimeThreshold);
        marker.header.frame_id = ptr->header.frame_id;
        marker.ns = "cylinder";
        marker.id =  label.toNSec();
        mDebugPub.publish(marker);
    }
    for(auto& tck: mTrackers.trackers){
        auto& det = tck.detections.back();
        visualization_msgs::Marker marker;
        marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
        marker.action = visualization_msgs::Marker::ADD;
        auto center = det.bbox.center;
        marker.pose.position.x = center.x;
        marker.pose.position.y = center.y;
        marker.pose.position.z = center.z;
        auto label = tck.uid;
        marker.scale.z = 0.4;
        marker.color.r = 0;
        marker.color.g = 0;
        marker.color.b = 0;
        marker.color.a = 1.0;
        std::stringstream str;
        str << label;
        marker.text = str.str();
        marker.lifetime = ros::Duration(mTimeThreshold);
        marker.header.frame_id = ptr->header.frame_id;
        marker.ns = "text";
        marker.id = label.toNSec();
        mDebugPub.publish(marker);
    }

}

int main( int argc, char** argv )
{
    ros::init(argc, argv, "tracking");
    ros::NodeHandle handle(std::string("~"));

    Tracking tracking;
    std::string input;
    ros::param::param<std::string>("~input", input,
                                   "/detections");
    tracking.mDetectSub = handle.subscribe(input,
                                           1,
                                           &Tracking::detection_callback,
                                           &tracking
                                           );

    tracking.mTrackerPub = handle.advertise<sn_msgs::TrackerArray>("trackers", 1);
    tracking.mTrackerPub2 = handle.advertise<sn_msgs::Tracker>("ended_tracker", 5);
    tracking.mDebugPub = handle.advertise<visualization_msgs::Marker>("markers", 1);

    ROS_INFO("Listening to: %s", input.c_str());
    ros::param::param<double>("~threshold", tracking.mThreshold, 0.35);
    std::cout << "Tracking: " << tracking.mThreshold << std::endl;
    ros::param::param<double>("~time_threshold", tracking.mTimeThreshold, 2.0);
    std::cout << "Tracking: " << tracking.mTimeThreshold << std::endl;

    ros::spin();

    return 0;
}

