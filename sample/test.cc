/*
 * @Author: your name
 * @Date: 2021-07-30 12:07:42
 * @LastEditTime: 2021-07-30 12:25:30
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Infantry_aimbot/sample/test.cc
 */
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace std;

int main(){

    cv::Mat mat1 = cv::Mat::zeros(100,100,CV_32FC3);
    cout << "hello " << endl;

    cv::imshow("test",mat1);
    cv::waitKey(0);

    cv::imwrite("/home/artinx-004/a.jpg",mat1);



}