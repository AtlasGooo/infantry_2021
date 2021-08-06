#include <csignal>
#include <cstdlib>
#include <iostream>

#include <opencv2/opencv.hpp>

#include "GxCamera/GxCamera.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

using cv::Mat;
using cv::imshow;
using cv::waitKey;
using cv::cvtColor;

using GxCamera::Camera;
using GxCamera::CameraParam;

namespace {
    bool stop_flag = false;
}

void SigintHandler(int sig) {
    stop_flag = true;
    cout << "SIGINT received, exiting" << endl;
}

int main(){

    // Set camera parameters
    CameraParam cam_param;
    // Serial number
    cam_param.if_open_by_serialnum = true;
    cam_param.serial_num = string("KE0200080465");
    // Software trigger
    cam_param.enable_software_trigger = false;
    // Auto exposure
    cam_param.enable_auto_exposure = true;
    cam_param.auto_exposure_max = 3000.0;
    cam_param.auto_exposure_min = 1000.0;
    cam_param.exposure_time_no_auto = 2000.0;
    // Auto gain
    cam_param.enable_auto_gain = false;
    cam_param.auto_gain_max = 16.0;
    cam_param.auto_gain_min = 5.0;
    cam_param.gain_no_auto = 12.0;


    bool success_init_camera = false;
    GX_STATUS status;
    Camera cam;    
    while (!success_init_camera)
    {
        status = cam.CameraInit(cam_param);
        if(status != GX_STATUS_SUCCESS){
            cerr << "CameraInit fail " << endl;
            cv::waitKey(1000);
            continue;
        }

        status = cam.CameraStreamOn();
        if(status != GX_STATUS_SUCCESS){
            cerr << "Turn on camera stream fail" << endl;
            cv::waitKey(1000);
            continue;
        }    

        success_init_camera = true;    
    }
    

    cout << "Current expusore time: " << cam.GetCurrentExpTime() << endl;
    cout << "Current acquisition frame rate: " << cam.GetCurrentFrameRate() << endl;


    signal(SIGINT, SigintHandler);
    Mat img;

    /// (lzj) try detect camera's failure 
    int countFailed = 0;
    while (!stop_flag){

        if(cam.GetLatestColorImg(img) != 0){
            countFailed++;
            if(countFailed > 3){
                cout << "Camera failed !" << endl;
                system("cd /home/artinx-002/Workspaces/Infantry_aimbot/shell && ./restart.sh");
                break;
            }
        }
        else{
            countFailed = 0;
        }


        cvtColor(img, img, cv::COLOR_RGB2BGR);

        imshow("test", img);
        waitKey(1);
    }

    cam.CameraStreamOff();
    cam.CameraClose();
    
    return EXIT_SUCCESS;
}
