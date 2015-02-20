#ifndef LASER_DESCRIPTOR_H
#define LASER_DESCRIPTOR_H

#include <sn_geometry/sn_geometry.h>
#include <sn_msgs/Detection.h>

namespace sn {

void up_sampling(vector_pts_t const& input, vector_pts_t& output, double resolution);

void down_sampling_nb(vector_pts_t const& input, vector_pts_t& output, int nb);

void down_sampling_step(vector_pts_t const& input, vector_pts_t& output, int step);

void pair_of_points(vector_pts_t const& input, vector_pts_t& output);

double compute_angle_ref(vector_pts_t const& input);

void compute_centered_oriented(vector_pts_t const& input, vector_pts_t& output);

void smooth(vector_pts_t const& input, vector_pts_t& output, int neigh);

void triangle_points(vector_pts_t const& input, vector_pts_t& output);

}

#endif // LASER_DESCRIPTOR_H