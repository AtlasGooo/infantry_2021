/*
 * @Author: your name
 * @Date: 2021-07-25 16:23:54
 * @LastEditTime: 2021-08-01 01:31:11
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /Infantry_aimbot/src/Aimbot/LightBar.cc
 */
#include "Aimbot/Armor.h"


LightBar::LightBar(){
	lightRect = RotatedRect();
	length = 0;
	center = Point2f();
	angle = 0;
}

LightBar::LightBar(const RotatedRect &lightRect){
	this->lightRect = lightRect;
	length = MAX(lightRect.size.height, lightRect.size.width);
	// length = lightRect.size.height;	
	center = lightRect.center;
	if( (lightRect.size.width/lightRect.size.height) < 0.6 ){
		if (lightRect.angle > 90)
			angle = lightRect.angle - 180;
		else
			angle = lightRect.angle;
	}
	else {
		angle = 0;
	}

}

LightBar::~LightBar(){}
