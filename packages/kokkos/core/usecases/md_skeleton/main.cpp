

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "system.h"

int create_system(System &system, int nx, int ny, int nz, double rho);
int neigh_setup(System &system);
int neigh_build(System &system);
double2 force(System &system,int evflag);

/* simple MD Skeleton which
 *   - constructs a simple FCC lattice,
 *   - computes a neighborlist
 *   - compute LJ-Force kernel a number of times
 */

int main(int argc, char** argv) {

  printf("Running MD Skeleton\n");
  /* Thread numbers for Host */

  int num_threads = 1;
  int teams = 1;

  /* Default value for number of force calculations */

  int iter = 100;

  /* Default value for system size (4*nx*ny*nz atoms)
   * nx, ny and nz are set to system_size if not specififed on commandline */

  int system_size = 20;
  int nx = -1;
  int ny = -1;
  int nz = -1;

  int neighbor_size = 1.5; // Default bin size for neighbor list construction

  double rho = 0.8442; // Number density of the system
  double delta = 0; // Scaling factor for random offsets of atom positions

  int device = 0; // Default device for GPU runs

  /* read in command-line arguments */

  for(int i = 0; i < argc; i++) {
    if((strcmp(argv[i], "-t") == 0) || (strcmp(argv[i], "--num_threads") == 0)) {
      num_threads = atoi(argv[++i]);
      continue;
    }

    if((strcmp(argv[i], "--teams") == 0)) {
      teams = atoi(argv[++i]);
      continue;
    }

    if((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--device") == 0))  {
      device = atoi(argv[++i]);
      continue;
    }

    if((strcmp(argv[i], "--delta") == 0)) {
      delta = atof(argv[++i]);
      continue;
    }

    if((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--iter") == 0))  {
      iter = atoi(argv[++i]);
      continue;
    }

    if((strcmp(argv[i], "-rho") == 0)) {
      rho = atoi(argv[++i]);
      continue;
    }

    if((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--size") == 0)) {
      system_size = atoi(argv[++i]);
      continue;
    }

    if((strcmp(argv[i], "-nx") == 0)) {
      nx = atoi(argv[++i]);
      continue;
    }

    if((strcmp(argv[i], "-ny") == 0)) {
      ny = atoi(argv[++i]);
      continue;
    }

    if((strcmp(argv[i], "-nz") == 0)) {
      nz = atoi(argv[++i]);
      continue;
    }

    if((strcmp(argv[i], "-b") == 0) || (strcmp(argv[i], "--neigh_bins") == 0))  {
      neighbor_size = atoi(argv[++i]);
      continue;
    }
  }

  if( nx < 0 ) nx = system_size;
  if( ny < 0 ) ny = system_size;
  if( nz < 0 ) nz = system_size;

  printf("-> Init Device\n");

#ifdef KOKKOS_HAVE_CUDA
  Kokkos::Cuda::SelectDevice select_device(device);
  Kokkos::Cuda::initialize(select_device);
#else
  device_type::initialize(std::pair<unsigned int,unsigned int>(teams,num_threads));
#endif

  System system;
  system.neigh_cut = 2.8;
  system.force_cut = 2.5;
  system.force_cutsq = system.force_cut*system.force_cut;
  system.delta = delta;

  printf("-> Build system\n");
  create_system(system,nx,ny,nz,rho);

  printf("-> Created %i atoms and %i ghost atoms\n",system.nlocal,system.nghost);

  system.nbinx = system.box.xprd/neighbor_size+1;
  system.nbiny = system.box.yprd/neighbor_size+1;
  system.nbinz = system.box.zprd/neighbor_size+1;


  printf("-> Building Neighborlist\n");

  neigh_setup(system);
  neigh_build(system);

  double2 ev = force(system,1);

  printf("-> Calculate Energy: %lf Virial: %lf\n",ev.x,ev.y);

  printf("-> Running %i force calculations\n",iter);

  Kokkos::Impl::Timer timer;

  for(int i=0;i<iter;i++) {
    force(system,0);
  }


  double time = timer.seconds();
  printf("Time: %e s for %i iterations with %i atoms\n",time,iter,system.nlocal);

  device_type::finalize();
}