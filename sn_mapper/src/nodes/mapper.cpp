#include "mapper.h"

namespace sn{

MapUpdaterNode::MapUpdaterNode(const ros::NodeHandle& node):
  node_(node),
  mMapReceived(false),
  mGoodScore(false)
{

  mapSub_ = node_.subscribe("/scan", 1, &MapUpdaterNode::scanCallback, this);
  scoreSub_ = node_.subscribe("/score", 1, &MapUpdaterNode::scoreCallback, this);

  mapPub_ = node_.advertise<nav_msgs::OccupancyGrid>("/map", 2, true);

  ros::param::param<bool>("~new_map", mNewMap, true);
  ros::param::param<bool>("~score_disp", mScoreDisp, false);

}

void MapUpdaterNode::scanCallback(LaserPtr scan){
  if(!mNewMap && !mMapReceived)return;

  static bool first_scan = true;
  if(first_scan){
    sn::vector_pts_t l;
    sn::pose_t robot = create_pose(0,0,0);
    sn::rosToLaser(*scan, l);
    updateMap(l, robot, 255, 255);

    first_scan = false;
  }

  tf::StampedTransform transform;

  try{
    listener_.waitForTransform("map", "laser", scan->header.stamp, ros::Duration(0.5));
    listener_.lookupTransform("map", "laser", scan->header.stamp, transform);
  }
  catch (tf::TransformException ex){
    //        ROS_ERROR("%s",ex.what());
    OGrid og;
    sn::mapToRos(mMap, og);
    og.header.frame_id = "/map";
    mapPub_.publish(og);
    return;
  }


  sn::vector_pts_t l;
  sn::pose_t robot;
  sn::rosToProjectedLaser(*scan, transform, l);
  sn::tfToPose(transform, robot);

  //    double mean, std;
  //    scoreScan(l, mean, std);
  //    std::cout << "position " << robot << std::endl;
  //    std::cout << "score " << mean << " " << std << std::endl;

  if(mScoreDisp && !mGoodScore)return;

  updateMap(l, robot, 10, 1);

  static int count = 0;
  if(count%5 == 0){
    OGrid og;
    sn::mapToRos(mMap, og);
    og.header.frame_id = "/map";
    mapPub_.publish(og);
    count = 0;
  }else
    ++count;

}

void MapUpdaterNode::scoreCallback(std_msgs::Float64::ConstPtr score){
  if(score->data < 0.5 && !mGoodScore){
    mGoodScore = true;
    std::cout << "Starting to update map \n";
  }else if(score->data > 1.0 && mGoodScore){
    mGoodScore = false;
    std::cout << "Robot is lost \n";
  }
}

void MapUpdaterNode::mapCallback(OGridPtr map){
  ROS_INFO("Map received");

  sn::rosToMap(map, mMap);
  mMapReceived = true;

  OGrid og;
  sn::mapToRos(mMap, og);
  mapPub_.publish(og);

}

void MapUpdaterNode::updateMap(const sn::vector_pts_t &points, sn::pose_t &pose, int hit_s, int miss_s){
  mMap.computeDistDiff();

  cv::Point robot =  mMap.toMap(pose.x, pose.y);
  bool flag = false;
#pragma omp parallel for
  for(unsigned int i = 0; i<points.size(); ++i){
    cv::Point p = mMap.toMap(points[i]);
    cv::LineIterator it(mMap.grid, robot, p, 4);
    for(int i=0;  i < it.count && it.pos() != p; ++i, ++it){
      if(mMap.isValid(it.pos()))
        mMap.at(it.pos()) = std::min(255, mMap.at(it.pos())+miss_s);
      if(mMap.isValid(it.pos()) && mMap.at(it.pos()) < 128)
        break;
    }
    if(mMap.isValid(p) /*&& (mMap.at(p) < 255 || mMap.dist(p) < 2)*/)
      mMap.at(p) = std::max(0, mMap.at(p)-hit_s);
    if(!mMap.isValid(p))
      flag = true;
  }

  // make a bigger map
  if(flag)
  {
    mMap.rezise(10, 10, 10, 10);
  }

}

void MapUpdaterNode::scoreScan(const sn::vector_pts_t& points, double& mean, double &std){
  // score the whole scan
  double score = 0;
  double count = 0;
  double square = 0;
  for(unsigned int i = 0; i<points.size(); i++){
    cv::Point p = mMap.toMap(points[i]);
    if(mMap.isValid(p)){
      double dist = mMap.dist(points[i]);
      score += dist;
      square += dist*dist;
      ++count;
    }
  }
  score /= count;
  square /= count;
  std = std::sqrt(square - score*score);
  mean = score;
}

}

int main( int argc, char** argv )
{
  ros::init(argc, argv, "mapper");
  ros::NodeHandle n(std::string("~"));

  sn::MapUpdaterNode mynode(n);

  ros::spin();
  return 0;
}
