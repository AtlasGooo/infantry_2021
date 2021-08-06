/*
 * @Author: your name
 * @Date: 2021-07-25 16:23:54
 * @LastEditTime: 2021-07-30 12:56:24
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Infantry_aimbot/inc/utils/Timer.hpp
 */
  
/**
 * File: Timer.hpp
 * Author: Jing Yonglin
 * Description: Class definition for a simple timer
 */

#ifndef LIDAR_ALGORITHM_TIMER
#define LIDAR_ALGORITHM_TIMER

#include <chrono>

class Timer{
public:
    Timer() = default;

    void Start();
    double Stop();
private:
    std::chrono::steady_clock::time_point start_pt_;
};

#endif