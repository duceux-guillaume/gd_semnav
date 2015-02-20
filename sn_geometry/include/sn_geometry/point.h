#ifndef POINT_H
#define POINT_H

#include <geometry_msgs/Point.h>
#include <geometry_msgs/Pose2D.h>

namespace sn {

typedef geometry_msgs::Point point_t;
typedef std::vector<point_t> vector_pts_t;
typedef geometry_msgs::Pose2D pose_t;

point_t create(double x, double y, double z);
pose_t create_pose(double x, double y, double z);

template<typename F>
point_t generic_operator(point_t const& a, point_t const& b, F func);

template<typename F>
point_t generic_operator(point_t const& a, double s, F func);

double l2_norm(const point_t& pt);

point_t ominus(pose_t const& pose, point_t const& pt);

}

//Arithmetic operators
sn::point_t operator -(sn::point_t const& a, sn::point_t const& b);
sn::point_t operator +(sn::point_t const& a, sn::point_t const& b);
sn::point_t operator /(sn::point_t const& a, double b);
sn::point_t operator *(sn::point_t const& a, double b);

sn::point_t operator -(sn::point_t const& a);

//Compound assignment operators
sn::point_t& operator +=(sn::point_t& a, sn::point_t const& b);
sn::point_t& operator /=(sn::point_t& a, double b);

#endif // POINT_H
