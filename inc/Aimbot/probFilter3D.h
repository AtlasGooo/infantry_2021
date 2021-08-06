/*
 * @Author: Lin Zijun (lzj)
 * @Date: 2021-07-28 11:48:39
 * @LastEditTime: 2021-07-30 20:20:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Infantry_aimbot/inc/Aimbot/probFilter3D.h
 */
#ifndef _PROBFILTER3D_H_
#define _PROBFILTER3D_H_


#include "Aimbot/General.h"
#include <iostream>
#include <queue>

using namespace std;


class ProbFilter3D
{


public:

    ProbFilter3D();

    void setQueueSize(int queueSize = 20);
    void setDistancceThreash(float distanceThreash = 500.0);
    void setFuseThreash(float fuseThreash = 0.5);
    void setMapedScoreThreashAng(float mapedScoreThreashAng = 20.0);
    void init();

    /**
     * @description: 
     * @param {float} x - Current armor's x pos
     * @param {float} y - Current armor's y pos
     * @param {float} z - Current armor's z pos
     * @return {*}
     */
    void setCurrentPos(float x, float y, float z);


    void setYawPitch(float yaw_ang, float pitch_ang);

    /**
     * @brief Calculate a belief from the history queue, and put armor int the queue finally
     * @param {*}
     * @return {float} beliefCoef - The coefficient represent belief and will be used to scale the yaw/pitch next
     */
    float calcProb();

    void putArmorPos();
    
    float getfuseThreash();

    void if_debug(bool debugOrNot);

private:
 
       
    float calcSingleProb_(cv::Point3f histPos);    // include distance function

    float fuseAllScores_();

private:
    int size_;

    float distThreash_;
    float fuseThreash_;
    float mapedScoreThreashAng_;

    float belief_;  // havent use yet
    int countNewPos_;

    cv::Point3f currentPos_;
    float currentYaw_,currentPitch_;

    vector<cv::Point3f> histArmors_;
    vector<float> scores_;  /// scores of current armor to each hist armors

    bool is_debug_;

};




#endif