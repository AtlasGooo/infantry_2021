/*
 * @Author: your name
 * @Date: 2021-07-28 11:48:54
 * @LastEditTime: 2021-07-30 20:27:01
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Infantry_aimbot/src/Aimbot/probFilter3D.cc
 */



#include "Aimbot/probFilter3D.h"

//// public:

ProbFilter3D::ProbFilter3D(){
    this->setQueueSize();
    this->setDistancceThreash();
    this->setFuseThreash();
    this->setMapedScoreThreashAng();

    this->currentPos_ = cv::Point3f(0.0, 0.0, 5000.0);
    this->histArmors_ = std::vector<cv::Point3f>();
}


void ProbFilter3D::setQueueSize(int queueSize){
    size_ = queueSize;
}

void ProbFilter3D::setDistancceThreash(float distanceThreash){
    distThreash_ = distanceThreash;
}

void ProbFilter3D::setFuseThreash(float fuseThreash){
    fuseThreash_ = fuseThreash;
}

void ProbFilter3D::setMapedScoreThreashAng(float mapedScoreThreashAng){
    mapedScoreThreashAng_ = mapedScoreThreashAng;
}


void ProbFilter3D::setCurrentPos(float x, float y, float z){
    this->currentPos_ = cv::Point3f(x,y,z);
}

void ProbFilter3D::setYawPitch(float yaw_ang, float pitch_ang){
    this->currentYaw_ = yaw_ang;
    this->currentPitch_ = pitch_ang;
}

void ProbFilter3D::init(){
    for(int i=0; i<size_; i++){
        histArmors_.emplace_back(cv::Point3f(0.0, 0.0, 5000.0));
    }
}



float ProbFilter3D::calcProb(){

    scores_.clear();

    for(int i=0; i<size_; i++){
        scores_.emplace_back(calcSingleProb_(histArmors_[i]));
    }


    float beliefCoef = fuseAllScores_();
    this->belief_ = beliefCoef;

    if(is_debug_){
        cout << "beliefCoef: " << beliefCoef << endl;
    }

    return beliefCoef;
}


void ProbFilter3D::putArmorPos(){
    countNewPos_++;

    if(belief_ > fuseThreash_){
        /// belief is large
        if(countNewPos_> 3){
            auto front_it = histArmors_.begin();
            this->histArmors_.erase(front_it);
            this->histArmors_.emplace_back(this->currentPos_);        
            countNewPos_ = 0;
        }
        else{
            ;
        }        
    }
    else{
        /// belief is small
        if(countNewPos_> 3){

            /// (TODO)
            float randNum = (float)rand() / RAND_MAX;   /// 0~1

            float absYaw = abs(currentYaw_);
            float absPitch = abs(currentPitch_);
            float biasScore = absYaw + absPitch;

            float mapedScore;
            if(biasScore < mapedScoreThreashAng_){
                mapedScore = 1;
            }
            else{
                mapedScore = pow( (mapedScoreThreashAng_ / biasScore) , 2 );
            }

            /// (test)
            if(is_debug_){
                cout << "(absYaw,absPitch): " << "( " << absYaw << " , " << absPitch << ")" << endl;
                cout << "biasScore: " << biasScore << endl;
                cout << "mapedScore: " << mapedScore << endl;                
            }


            if(randNum < mapedScore){
                auto front_it = histArmors_.begin();
                this->histArmors_.erase(front_it);
                this->histArmors_.emplace_back(this->currentPos_);        
                countNewPos_ = 0;                
            }
                      
        }
        else{
            ;
        }

        

    }

} 

float ProbFilter3D::getfuseThreash(){
    return this->fuseThreash_;
}

void ProbFilter3D::if_debug(bool debugOrNot){
    is_debug_ = debugOrNot;
}










//// private:

    
float ProbFilter3D::calcSingleProb_(cv::Point3f histPos){

    float score;    
    float deltaX,deltaY,deltaZ;

    deltaX = currentPos_.x - histPos.x;
    deltaY = currentPos_.y - histPos.y;
    deltaZ = currentPos_.z - histPos.z;
    float distance = sqrt( pow(deltaX,2) + pow(deltaY,2) + pow(deltaZ,2) );

    if(distance <= distThreash_){
        score = 1.0;
    }
    else{
        score = pow(  1.0 / ( (distance - distThreash_ + 1000)/1000.0 ) , 2 );
    }

    return score;
}

float ProbFilter3D::fuseAllScores_(){
    
    float fusedScore = 0;
    for(int i=0; i<size_; i++){
        fusedScore += scores_[i];
    }
    fusedScore = fusedScore / (float)size_;
    fusedScore = pow(fusedScore,2);

    // if(fusedScore > fuseThreash_){
    //     fusedScore = 1;
    // }

    return fusedScore;
}


