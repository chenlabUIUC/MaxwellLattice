/* -*- c++ -*- ----------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   https://www.lammps.org/, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

#ifdef PAIR_CLASS
// clang-format off
PairStyle(body/rounded/polygon/fvdw/fsh/nn,PairBodyRoundedPolygonFvdwFshNN);
// clang-format on
#else

#ifndef LMP_PAIR_BODY_ROUNDED_POLYGON_FVDW_FSH_NN_H
#define LMP_PAIR_BODY_ROUNDED_POLYGON_FVDW_FSH_NN_H

#include "pair.h"
//Shu:210802
// #include <map>
// #include <tuple>
#include <Eigen/Dense>
using namespace std;
using namespace Eigen;
//.
namespace LAMMPS_NS {

class PairBodyRoundedPolygonFvdwFshNN : public Pair {
 public:
  PairBodyRoundedPolygonFvdwFshNN(class LAMMPS *);
  ~PairBodyRoundedPolygonFvdwFshNN();
  void compute(int, int);
  void settings(int, char **);
  void coeff(int, char **);
  void init_style();
  double init_one(int, int);

  struct Contact {
    int ibody, jbody;     // body (i.e. atom) indices (not tags)
    int vertex;           // vertex of the first polygon
    int edge;             // edge of the second polygon
    double xv[3];         // coordinates of the vertex
    double xe[3];         // coordinates of the projection of the vertex on the edge
    double separation;    // separation at contact
  };

 protected:
  double **k_n;        // normal repulsion strength
  double **k_na;       // normal attraction strength
  double c_n;          // normal damping coefficient
  double c_t;          // tangential damping coefficient
  double mu;           // normal friction coefficient during gross sliding
  double delta_ua;     // contact line (area for 3D models) modification factor
  double cut_inner;    // cutoff for interaction between vertex-edge surfaces

  class AtomVecBody *avec;
  class BodyRoundedPolygon *bptr;

  double **discrete;    // list of all sub-particles for all bodies
  int ndiscrete;        // number of discretes in list
  int dmax;             // allocated size of discrete list
  int *dnum;            // number of discretes per line, 0 if uninit
  int *dfirst;          // index of first discrete per each line
  int nmax;             // allocated size of dnum,dfirst vectors

  double **edge;    // list of all edge for all bodies
  int nedge;        // number of edge in list
  int edmax;        // allocated size of edge list
  int *ednum;       // number of edges per line, 0 if uninit
  int *edfirst;     // index of first edge per each line
  int ednummax;     // allocated size of ednum,edfirst vectors

  double *enclosing_radius;    // enclosing radii for all bodies
  double *rounded_radius;      // rounded radii for all bodies
  double *maxerad;             // per-type maximum enclosing radius

  //Shu:210806
  char file_output[1024];
  char file_output_head[1024];
  int atom_tgt;
  int DT;
  int dump_time;
  double F_att,F_att_x,F_att_y,F_rep,F_rep_x,F_rep_y;
  //.

  //Shu:210802
  double cut_global;
  int flag_fvdw,flag_fsh; // 1-with fvdw, 0-without fvdw
  char paramFileName[1024];
  int nterm;
  MatrixXd param_fvdw_m_0, param_fvdw_m_2, param_fvdw_m_4, param_fvdw_m_6;
  MatrixXd param_fsh_m_0, param_fsh_m_2, param_fsh_m_4, param_fsh_m_6;
  VectorXd param_fvdw_v_0, param_fvdw_v_2, param_fvdw_v_4, param_fvdw_v_6;
  VectorXd param_fsh_v_0, param_fsh_v_2, param_fsh_v_4, param_fsh_v_6;

  // double **param_fvdw_0, **param_fvdw_2, **param_fvdw_4, **param_fvdw_6;
  // double **param_fsh_0, **param_fsh_2, **param_fsh_4, **param_fsh_6;
  // double delta_x, delta_y, delta_z;

  // void compute_fvdw_fsh(double * fvdw_j, double * ibody_coor, double * jbody_coor);
  // double cal_fvdw_byParam(double x, double y, double z, double *w);
  double coordinate_transfer(double *table_input,double *X1, double *X2, double theta1,double theta2);
  void global_to_local(double *xnew, double *x, double theta);
  void compute_fvdw_fsh(double * fvdw_j,double * fsh_j, double *xi, double *xj, double * ibody_coor, double * jbody_coor);
  // tuple <double, double, double, double> tuple4_000, tuple4_100, tuple4_010, tuple4_110, tuple4_001, tuple4_101, tuple4_011, tuple4_111;
  // std::map<tuple<double, double, double>, tuple<double, double, double, double>> fvdw_table;
  // std::map<tuple<double, double, double>, tuple<double, double, double>> fvdw_table;
  // void cal_fvdw_byTabel(double * output, double * input, double dx, double dy, double dz);
  void read_param(MatrixXd&, VectorXd&, char*, int, int);
  void cal_fvdw_nn(double * output, double * input);
  void cal_fsh_nn(double * output, double * input);
  //.

  void allocate();
  void body2space(int);

  // sphere-sphere interaction
  void sphere_against_sphere(int i, int j, double delx, double dely, double delz, double rsq,
                             double k_n, double k_na, double **x, double **v, double **f,
                             int evflag);
  // vertex-edge interaction
  int vertex_against_edge(int i, int j, double k_n, double k_na, double **x, double **f,
                          double **torque, tagint *tag, Contact *contact_list, int &num_contacts,
                          double &evdwl, double *facc);
  // compute distance between a point and an edge from another body
  int compute_distance_to_vertex(int ibody, int edge_index, double *xmi, double rounded_radius,
                                 double *x0, double x0_rounded_radius, double cut_inner, double &d,
                                 double hi[3], double &t, int &contact);
  // compute contact forces if contact points are detected
  void contact_forces(Contact &contact, double j_a, double **x, double **v, double **angmom,
                      double **f, double **torque, double &evdwl, double *facc);

  // compute the separation between two contacts
  double contact_separation(const Contact &c1, const Contact &c2);

  // accumulate torque to a body given a force at a given point
  double sum_torque(double *xm, double *x, double fx, double fy, double fz, double *torque);
  // helper functions
  int opposite_sides(double *x1, double *x2, double *a, double *b);
  void total_velocity(double *p, double *xcm, double *vcm, double *angmom, double *inertia,
                      double *quat, double *vi);
  inline void distance(const double *x2, const double *x1, double &r);
};

}    // namespace LAMMPS_NS

#endif
#endif

/* ERROR/WARNING messages:

E: Illegal ... command

Self-explanatory.  Check the input script syntax and compare to the
documentation for the command.  You can use -echo screen as a
command-line option when running LAMMPS to see the offending line.

E: Incorrect args for pair coefficients

Self-explanatory.  Check the input script or data file.

E: Pair body/rounded/polygon requires atom style body rounded/polygon

Self-explanatory.

E: Pair body requires body style rounded/polygon

This pair style is specific to the rounded/polygon body style.

*/
