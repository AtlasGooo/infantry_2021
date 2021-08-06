#include <csignal>
#include <thread>
#include <unistd.h>

#include <opencv2/opencv.hpp>

#include "PacketManager/HostPacket.hpp"
#include "PacketManager/Time.hpp"

#include "Aimbot/General.h"
#include "Aimbot/Armor.h"
#include "Aimbot/AngleSolver.h"

#include "GxCamera/GxCamera.hpp"

#include "ThreadSafe/ThreadSafeQueue.hpp"

#include "utils/Timer.hpp"

#include <unistd.h>

using GxCamera::Camera;
using GxCamera::CameraParam;

using RmAimbot::HostPacketManager;
using ThreadSafe::ThreadSafeQueue;

using namespace std;

#define RECORD_FRAMES

#define DEBUG_TIME_FRAMERATE
#define DEBUG_FILTER
// #define DEBUG_IMG



struct SerialData{
    float yaw_cmd = 0.0f;
    float pitch_cmd = 0.0f;
    uint8_t if_fire = 0;
    uint8_t is_found = 0;
    uint16_t seq = 0;
};


namespace{
    Color enemy_color = RED;
    int target_num = 2;
    bool stop_flag = false;

    float tmp_seq = 0.0f;

    // Gloable data
    SerialData serial_data;

    // ThreadSafeQueue<SerialData> data_queue;
}

void SerialSendThread();

void SigintHandler(int sig) {
    stop_flag = true;
    cout << "SIGINT received, exiting" << endl;
}

int main(int argc, char const *argv[]){
    // Set camera parameters
    CameraParam cam_param;
    // Serial number
    cam_param.if_open_by_serialnum = true;
    cam_param.serial_num = string("KE0200080464");
    // Software trigger
    cam_param.enable_software_trigger = false;
    // Auto exposure
    cam_param.enable_auto_exposure = true;
    cam_param.auto_exposure_max = 3000.0;
    cam_param.auto_exposure_min = 1000.0;
    cam_param.exposure_time_no_auto = 1000.0;
    // Auto gain
    cam_param.enable_auto_gain = false;
    cam_param.auto_gain_max = 16.0;
    cam_param.auto_gain_min = 5.0;
    cam_param.gain_no_auto = 12.0;
 


    /// (lzj) Keep trying until camera is connected, and also for re-connecting process
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

    // Config detection
    ArmorDetector detector;
    AngleSolver angleSolver;

    /// (lzj) Add ProbFilter3D in main()
    std::unique_ptr<ProbFilter3D> pFilter = \
        std::make_unique<ProbFilter3D>();
    pFilter.get()->setQueueSize(20);
    pFilter.get()->setDistancceThreash(300.0);
    pFilter.get()->setFuseThreash(0.5);
    pFilter.get()->setMapedScoreThreashAng(15.0);
    pFilter.get()->init();

    #ifdef DEBUG_FILTER
        pFilter.get()->if_debug(true);
    #else
        pFilter.get()->if_debug(false);
    #endif

    angleSolver.setFilter3D(pFilter.get());


    //Set armor detector prop
    detector.loadSVM("../config/123svm.xml");

    //Set angle solver prop
    angleSolver.setCameraParam("../config/camera_params.xml", 1);


    /// (lzj)TODO
    // angleSolver.setArmorSize(SMALL_ARMOR,135,125);
    // angleSolver.setArmorSize(BIG_ARMOR,230,127);
    angleSolver.setArmorSize(SMALL_ARMOR,130,70);
    angleSolver.setArmorSize(BIG_ARMOR,220,70);

    /// (lzj)TODO
    // angleSolver.setBulletSpeed(15000);
    angleSolver.setBulletSpeed(22000);

    // Prepare serial send thread
    std::thread serial_send_thread(SerialSendThread);

    double timer_start,timer_span;
    char key_pressed = 0;

    signal(SIGINT, SigintHandler);
    


    #ifdef DEBUG_TIME_FRAMERATE
        Timer tic;
        Timer allTic;
        allTic.Start();
    #endif


    int frameCounter = 0;   
    int record_flag = -1;   /// -1:not record   0:record


    /// (lzj) Record frame while the path was found
    #ifdef RECORD_FRAMES
        time_t timep;
        time(&timep);
        char tmp[64];
        strftime(tmp, sizeof(tmp),"%Y%m%d%H%M%S",localtime(&timep));
        string startTime = string(tmp);

        // string record_path = "/media/artinx-004/A594-F1E4/record/" + startTime; 
        // string record_path = "/home/artinx-004/LinZijuns/record/"  + startTime; 

        string record_path = "/media/artinx-004/81AA-9E3B";
        record_flag = access(record_path.c_str(),W_OK);

        if(record_flag == 0){
            record_path = record_path + "/record/" + startTime;        
            mkdir(record_path.c_str(),S_IRUSR|S_IWUSR|S_IXUSR|S_IRWXG|S_IRWXO);
            record_flag = access(record_path.c_str(),W_OK);
            cout << "record_path: " << record_path << endl;
        }   

        cout << "record_flag: " << record_flag << endl;
    #endif





    //// Start running loop !
    int countCamFailed = 0;

    do{

        #ifdef DEBUG_TIME_FRAMERATE
            tic.Start();
        #endif

        double yaw=0,pitch=0,distance=0;

        detector.setEnemyColor(enemy_color); //here set enemy color



        /// (lzj) While getting img, try to detect camera's failure and call restart process
        Mat img;
        if(cam.GetLatestColorImg(img) != GX_STATUS_SUCCESS){
            countCamFailed++;
            if(countCamFailed > 2){
                cout << "Camera failed !" << endl;
                system("cd /home/artinx-007/workspaces/Infantry_aimbot/shell && ./restart_sys_all_test.sh");
                break;      /// break running loop       
            }
            else{
                countCamFailed = 0;
            }
        }


        cv::cvtColor(img, img, cv::COLOR_RGB2BGR);

        detector.run(img);

        Point2f centerPoint;
        if(detector.isFoundArmor()){
            vector<Point2f> contourPoints;
            ArmorType type;
            detector.getTargetInfo(contourPoints, centerPoint, type);
            angleSolver.getAngle(contourPoints,centerPoint,type,yaw,pitch,distance);
        }

        serial_data.yaw_cmd = (float)yaw;
        serial_data.pitch_cmd = (float)pitch;
        serial_data.if_fire = 1;
        serial_data.is_found = 1;
        serial_data.seq = tmp_seq;


        // std::cout << "[DEBUG INFO]" << std::endl
        //           << "-- Time used(ms): " << time_span * 1000 << std::endl
        //           << "-- Frame rate: " << 1.0f / time_span << std::endl
        //           << "-- Image size: " << img.cols << "x" << img.rows << std::endl;

        // data_queue.Push(data);

        //串口在此获取信息 yaw pitch distance，同时设定目标装甲板数字
        // Serial(yaw,pitch,true,detector.isFoundArmor());
        //操作手用，实时设置目标装甲板数字
        // detector.setTargetNum(targetNum);





        // if(detector.isFoundArmor()){
        //     // printf("Found Target! Center(%d,%d)\n",centerPoint.x,centerPoint.y);
        //     // cout << "Yaw: " << yaw << "Pitch: " << pitch 
        //     //      << "Distance: " << distance <<endl;
        // }

        #ifdef DEBUG_IMG
            //********************** DEGUG **********************//
            //装甲板检测识别调试参数是否输出
            //param:
            //		1.showSrcImg_ON,		  是否展示原图
            //		2.bool showSrcBinary_ON,  是否展示二值图
            //		3.bool showLights_ON,	  是否展示灯条图
            //		4.bool showArmors_ON,	  是否展示装甲板图
            //		5.bool textLights_ON,	  是否输出灯条信息
            //		6.bool textArmors_ON,	  是否输出装甲板信息
            //		7.bool textScores_ON	  是否输出打击度信息
            //					   1  2  3  4  5  6  7
            detector.showDebugInfo(1, 1, 1, 1, 0, 0, 0);

            // if(detector.isFoundArmor())
            // {
            //     //角度解算调试参数是否输出
            //     //param:
            //     //		1.showCurrentResult,	  是否展示当前解算结果
            //     //		2.bool showTVec,          是否展示目标坐标
            //     //		3.bool showP4P,           是否展示P4P算法计算结果
            //     //		4.bool showPinHole,       是否展示PinHole算法计算结果
            //     //		5.bool showCompensation,  是否输出补偿结果
            //     //		6.bool showCameraParams	  是否输出相机参数
            //     //					      1  2  3  4  5  6
            //     angleSolver.showDebugInfo(1, 0, 0, 0, 0, 0);
            // }

            char chKey = waitKey(1);
            switch (chKey) {
            case '1':
                target_num = 1;
                break;
            case '2':
                target_num = 2;
                break;
            case '3':
                target_num = 3;
                break;
            case 'b':
            case 'B':
                enemy_color = BLUE;
                break;
            case 'r':
            case 'R':
                enemy_color = RED;
                break;
            case 'q':
            case 'Q':
            case 27:
                stop_flag = true;
                break;
            default:
                break;
            }

        #endif



        /// (test) test overall frequency / frame rate
        #ifdef DEBUG_TIME_FRAMERATE
            double time_span = tic.Stop();
            double all_time_span = allTic.Stop();

            cout << "OverallTime: " << all_time_span << " Time: " << time_span << " Frequency: " << 1/time_span << endl;
            cout << endl;
        #endif


        #ifdef RECORD_FRAMES
            if(record_flag == 0){
                frameCounter++;
                if(frameCounter % 10 == 0 && frameCounter<300000){

                    string framePathName = \
                        record_path + "/" + "F_" + to_string(frameCounter) +  "_T_" + to_string(all_time_span) + ".jpg";

                    cout << endl;
                    cout << "FramePathName: " << framePathName << endl;

                    if(!img.empty()){
                        cv::imwrite(framePathName,img);   
                    }
                }
            }
        #endif

    } while (!stop_flag);
    


    serial_send_thread.join();

    cam.CameraStreamOff();
    cam.CameraClose();

    return EXIT_SUCCESS;
}




void SerialSendThread(){
    // Init serial communication
    HostPacketManager& host_packet_manager = *HostPacketManager::Instance();
    host_packet_manager.Init("/dev/ttyUSB0", 921600);
    Time::Init(1);

    while (!stop_flag){
        // Update 
        host_packet_manager.Update();

        std::this_thread::sleep_for(1ms);

        // SerialData data;
        // data_queue.WaitAndPop(data);

        host_packet_manager.GetTestPacket().cmd_yaw = serial_data.yaw_cmd;
        host_packet_manager.GetTestPacket().cmd_pitch = serial_data.pitch_cmd;
        host_packet_manager.GetTestPacket().cmd_fire = serial_data.if_fire;
        host_packet_manager.GetTestPacket().is_found = serial_data.is_found;
        // host_packet_manager.GetTestPacket().seq = serial_data.seq;
        host_packet_manager.GetTestPacket().seq++;

        host_packet_manager.GetTestPacket().SendPacket();

        /// (lzj) comment debug info
        // unsigned int diff = host_packet_manager.GetTestPacket().seq - host_packet_manager.GetEchoPacket().seq;
        // std::cout << "[DEBUG INFO]" << std::endl
        //           << "-- Send seq: " << host_packet_manager.GetTestPacket().seq << std::endl
        //           << "-- Recv seq: " << host_packet_manager.GetEchoPacket().seq << std::endl
        //           << "-- DIFF: " << diff << std::endl
        //           << "-- YAW: " << serial_data.yaw_cmd << std::endl
        //           << "-- PITCH: " << serial_data.pitch_cmd << std::endl;
    }
    
}