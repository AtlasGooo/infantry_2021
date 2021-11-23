# Infantry_2021

**AUTHOR:**  Zijun Lin  
**EMAIL1:** 11810710@mail.sustech.edu.cn  
**EMAIL2:** 1774525013@qq.com

Codes and documents are modified from [aimbot-artinx2021](https://github.com/NERanger/aimbot-artinx2021), which implements a basic code structure with serial communication module.


## Prerequisites
*The contents in this section are originate from [aimbot-artinx2021](https://github.com/NERanger/aimbot-artinx2021).*  

* [Daheng Galaxy USB3.0 driver](./docs/cam_driver_install.md)

* [OpenCV (Tested with 3.4.14)](https://docs.opencv.org/3.4.14/d7/d9f/tutorial_linux_install.html)

* Boost

```shell
sudo apt install libboost-all-dev
```

## Build the project

*The contents in this section are originate from [aimbot-artinx2021](https://github.com/NERanger/aimbot-artinx2021).*  

```shell
mkdir build
cd build
cmake ..
make
```

After the building process is done, the generated executables will be put in `bin/` and static libraries will be put in `lib/`.


## Get Started

*The contents in this section are originate from [aimbot-artinx2021](https://github.com/NERanger/aimbot-artinx2021).*  

* [Acquire camera image](./docs/acquire_cam_image.md)
* [Camera calibration](./docs/camera_calibration.md)
* [Run detection algorithm](./docs/run_detection_algorithm.md)
* [Communication with STM32 board via serial](./docs/serial_communication.md)


## Framework

* File tree:

```text
infantry_2021
|   CMakeLists.txt
|   README.md
|
+---config
|       123svm.xml
|       camera_params.xml
|
+---docs
|       acquire_cam_image.md
|       camera_calibration.md
|       cam_driver_install.md
|       run_detection_algorithm.md
|       serial_communication.md
|
+---inc
|   +---Aimbot
|   |       AngleSolver.h
|   |       Armor.h
|   |       General.h
|   |       probFilter3D.h
|   |
|   +---AsyncSerial
|   |       AsyncSerial.h
|   |       BufferedAsyncSerial.h
|   |
|   +---GxCamera
|   |       DxImageProc.h
|   |       GxCamera.hpp
|   |       GxIAPI.h
|   |
|   +---PacketManager
|   |       Crc.hpp
|   |       HashTable.hpp
|   |       HostPacket.hpp
|   |       Math.hpp
|   |       math_first_order_filter.h
|   |       MemoryStream.hpp
|   |       Observer.hpp
|   |       Packet.hpp
|   |       SerialPacketManager.hpp
|   |       Stream.hpp
|   |       Time.hpp
|   |
|   +---ThreadSafe
|   |       ThreadSafeQueue.hpp
|   |
|   \---utils
|           Timer.hpp
|
+---sample
|       CMakeLists.txt
|       SerialTest.cc
|       ShowCaptureImage.cc
|       SysAllTest.cc
|       test.cc
|       TestDetection.cc
|
+---shell
|       restart_sys_all_test.sh
|       start_sys_all_test.sh
|
\---src
    |   CMakeLists.txt
    |
    +---Aimbot
    |       AngleSolver.cc
    |       ArmorBox.cc
    |       ArmorDetector.cc
    |       ArmorNumClassifier.cc
    |       CMakeLists.txt
    |       findLights.cc
    |       LightBar.cc
    |       matchArmors.cc
    |       probFilter3D.cc
    |
    +---AsyncSerial
    |       AsyncSerial.cc
    |       BufferedAsyncSerial.cc
    |       CMakeLists.txt
    |
    +---GxCamera
    |       CMakeLists.txt
    |       GxCamera.cc
    |
    +---PacketManager
    |       CMakeLists.txt
    |       Crc.cpp
    |       HostPacket.cpp
    |       Math.cpp
    |       MemoryStream.cpp
    |       Packet.cpp
    |       SerialPacketManager.cpp
    |       Time.cpp
    |
    \---utils
            CMakeLists.txt
            Timer.cc
```

* Design concept:

  Every directory under `src/` will compile as a static library that provides certain function. When writing application code (such as those in `sample/`), you just link your code to the library according to the function you want. All project is organized with CMake, you may want to refer to [《CMake Cookbook》](https://github.com/xiaoweiChen/CMake-Cookbook/releases/download/0.2/CMake-CookBook.pdf) for CMake learning.


## Hint


* Remember to modified the sequence number and the file path in the sample codes and re-compile before you run the code.
* Comment/Uncomment the debug-macros in sample codes to show or hide debug information.
* Search "(lzj)"  (which is the collection of the letters for my Chinese name) in your code editor to view my comments and marks for the difference between this code and [aimbot-artinx2021](https://github.com/NERanger/aimbot-artinx2021).


## Some technical details
* View [this page](https://atlasgooo.wordpress.com/2021/08/08/some-details-in-rm-infantry-2021/) .




