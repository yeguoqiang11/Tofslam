# MultiSensorSlam

# 1. Introduction
MultiSensorSlam is a project of integrating multi-sensors to generate localization and 3D map, it may use tof, imu, image and maybe laser as well. we start develop by using tof, then extend it to multi-sensor.
we hope it is a fusion library supportting a lot of sensors.

# 2. Prerequisites
we use at least C++14 and test it on ubuntu 18.04
## OpenCV
we use opencv to manipulate images and features. Download and install instructions can be found at: http://opencv.org. **Required at least 3.4. Tested with OpenCV 3.4.11.**
## Eigen3
we use eigen3 to manipulate matrix and vector. Download and install instructions can be found at http://eigen.tuxfamily.org. **Tested with eigen 3.2.10**
## GTSAM
use gtsam to optimize trajectory, map and so on. Download and install instructions can be found at https://github.com/borglab/gtsam.
## Pangolin
use Pangolin to show 3D map and trajectory. Download and install instructions can be found at https://github.com/stevenlovegrove/Pangolin.

# 3. Building Projectory
Clone the repository
```
git clone git@git.dreame.com:Multi-SensroFusion/MultiSensorSlam.git
mkdir build
cd build
cmake ..
make -j8
```

# 4. Run Dataset
## Tum Dataset
```
cd examples
./tum_run
```

## Dreame Dataset
```
cd examples
./DM_run
```