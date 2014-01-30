# @HEADER
# ************************************************************************
#
#            Trilinos: An Object-Oriented Solver Framework
#                 Copyright (2001) Sandia Corporation
#
#
# Copyright (2001) Sandia Corporation. Under the terms of Contract
# DE-AC04-94AL85000, there is a non-exclusive license for use of this
# work by or on behalf of the U.S. Government.  Export of this program
# may require a license from the United States Government.
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the Corporation nor the names of the
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# NOTICE:  The United States Government is granted for itself and others
# acting on its behalf a paid-up, nonexclusive, irrevocable worldwide
# license in this data to reproduce, prepare derivative works, and
# perform publicly and display publicly.  Beginning five (5) years from
# July 25, 2001, the United States Government is granted for itself and
# others acting on its behalf a paid-up, nonexclusive, irrevocable
# worldwide license in this data to reproduce, prepare derivative works,
# distribute copies to the public, perform publicly and display
# publicly, and to permit others to do so.
#
# NEITHER THE UNITED STATES GOVERNMENT, NOR THE UNITED STATES DEPARTMENT
# OF ENERGY, NOR SANDIA CORPORATION, NOR ANY OF THEIR EMPLOYEES, MAKES
# ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR
# RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY
# INFORMATION, APPARATUS, PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS
# THAT ITS USE WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS.
#
# ************************************************************************
# @HEADER

  
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/../../TrilinosCTestDriverCore.cmake")

#
# Platform/compiler specific options for typhon using gcc
#

MACRO(TRILINOS_SYSTEM_SPECIFIC_CTEST_DRIVER)

  # Base of Trilinos/cmake/ctest then BUILD_DIR_NAME

  SET( CTEST_DASHBOARD_ROOT "${TRILINOS_CMAKE_DIR}/../../${BUILD_DIR_NAME}" )

  SET( CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}" )
  
  SET( CTEST_BUILD_FLAGS "-j32 -i" )

  SET_DEFAULT( CTEST_PARALLEL_LEVEL "4" )

  SET_DEFAULT( Trilinos_ENABLE_SECONDARY_STABLE_CODE ON)

  # Only turn on PyTrilinos for shared libraries
  SET_DEFAULT(Trilinos_EXCLUDE_PACKAGES ${EXTRA_EXCLUDE_PACKAGES} TriKota Optika)
  
  CUDA_NVCC_FLAGS="-gencode;arch=compute_35,code=sm_35;-I/home/crtrott/lib/mpi/include"
  CUDA_NVCC_FLAGS="${CUDA_NVCC_FLAGS};-Xcompiler;-Wall,-ansi,-fopenmp"
  CUDA_NVCC_FLAGS="${CUDA_NVCC_FLAGS};-O3;-DKOKKOS_USE_UVM;"

  SET( EXTRA_SYSTEM_CONFIGURE_OPTIONS
    "-DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}"
    "-DTrilinos_ENABLE_DEPENDENCY_UNIT_TESTS:BOOL=OFF"
    "-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"

    "-DTPL_ENABLE_SuperLU:BOOL=ON"
    "-DSuperLU_INCLUDE_DIRS=/usr/local/superlu/SuperLU_4.3/include"
    "-DSuperLU_LIBRARY_DIRS=/usr/local/superlu/SuperLU_4.3/lib"
    "-DSuperLU_LIBRARY_NAMES=superlu_4.3"

    "-DTPL_ENABLE_BLAS:BOOL=ON"
    "-DBLAS_LIBRARY_DIRS=/usr/local/lapack/rhel6"
    "-DBLAS_LIBRARY_NAMES=blas"
    "-DTPL_ENABLE_LAPACK:BOOL=ON"
    "-DLAPACK_LIBRARY_DIRS=/usr/local/lapack/rhel6"
    "-DLAPACK_LIBRARY_NAMES=lapack"
 
    "-DTpetra_ENABLE_Kokkos_Refactor:BOOL=ON"
    "-DKokkosClassic_DefaultNode:STRING=Kokkos::Compat::KokkosOpenMPWrapperNode"
    "-DTPL_ENABLE_HWLOC:STRING=ON"

    "-DTPL_ENABLE_CUDA:STRING=ON"
    "-DCUDA_TOOLKIT_ROOT_DIR=/opt/nvidia/cuda/6.0.9"
    "-DTPL_ENABLE_CUSPARSE:STRING=OFF"
    "-DTpetra_ENABLE_Thrust:BOOL=OFF"
    "-DKokkosClassic_ENABLE_Thrust=OFF"
    "-DCUDA_NVCC_FLAGS:STRING=${CUDA_NVCC_FLAGS}"
    )

  SET_DEFAULT(COMPILER_VERSION "GCC-4.4.6")

  #Ensuring that MPI is on for all parallel builds that might be run.
  IF(COMM_TYPE STREQUAL MPI)
    SET( EXTRA_SYSTEM_CONFIGURE_OPTIONS
         ${EXTRA_SYSTEM_CONFIGURE_OPTIONS}
         "-DTPL_ENABLE_MPI:BOOL=ON"
         "-DMPI_BASE_DIR:PATH=/opt/mpi/openmpi-1.7/gcc-4.4.6/cuda609"
       )
  ENDIF()

  TRILINOS_CTEST_DRIVER()

ENDMACRO()