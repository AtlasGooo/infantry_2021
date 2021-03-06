#include "Aimbot/Armor.h"

/**
 *@brief: calculate the cross point of four points in order bl(below left),tl(top left),tr(top right),br(below right)
 */
const Point2f crossPointof(const Point2f& bl,const Point2f& tl,const Point2f& tr, const Point2f& br){
	float a1 = tr.y - bl.y;
	float b1 = tr.x - bl.x;
	float c1 = bl.x*tr.y - tr.x*bl.y;

	float a2 = br.y - tl.y;
	float b2 = br.x - tl.x;
	float c2 = tl.x*br.y - br.x*tl.y;

	float d = a1 * b2 - a2 * b1;

	if (d == 0.0){
		return Point2f(FLT_MAX, FLT_MAX);
	}
	else{
		return cv::Point2f((b2*c1 - b1 * c2) / d, (c1*a2 - c2 * a1) / d);
	}
}

/**
 *@brief: using the lightRect of two lightBar to construct the armorVertices
 */
void setArmorVertices(const LightBar & l_light, const LightBar & r_light, ArmorBox & armor) {

	/// (lzj) modified method of extracting vertices
	//handle two lights
	// cv::Size exLSize(int(l_light.lightRect.size.width), int(l_light.lightRect.size.height * 2));
	// cv::Size exRSize(int(r_light.lightRect.size.width), int(r_light.lightRect.size.height * 2));

	cv::Size exLSize(int(l_light.lightRect.size.width), int(l_light.lightRect.size.height * 1));
	cv::Size exRSize(int(r_light.lightRect.size.width), int(r_light.lightRect.size.height * 1));

	cv::RotatedRect exLLight(l_light.center, exLSize, armor.armorAngle);
	cv::RotatedRect exRLight(r_light.center, exRSize, armor.armorAngle);




	/// (lzj) modified light corner point
	cv::Point2f pts_l[4];
	exLLight.points(pts_l);
	// cv::Point2f upper_l = pts_l[2];
	// cv::Point2f lower_l = pts_l[3];
	cv::Point2f upper_l,lower_l;
	upper_l.x = (pts_l[1].x + pts_l[2].x) / 2;
	upper_l.y = (pts_l[1].y + pts_l[2].y) / 2;
	lower_l.x = (pts_l[0].x + pts_l[3].x) / 2;
	lower_l.y = (pts_l[0].y + pts_l[3].y) / 2;


	cv::Point2f pts_r[4];
	exRLight.points(pts_r);
	// cv::Point2f upper_r = pts_r[1];
	// cv::Point2f lower_r = pts_r[0];
	cv::Point2f upper_r,lower_r;
	upper_r.x = (pts_r[1].x + pts_r[2].x) / 2;
	upper_r.y = (pts_r[1].y + pts_r[2].y) / 2;
	lower_r.x = (pts_r[0].x + pts_r[3].x) / 2;
	lower_r.y = (pts_r[0].y + pts_r[3].y) / 2;


	/// (lzj) modified order of armor vertices
	// armor.armorVertices[0] = upper_l;
	// armor.armorVertices[1] = upper_r;
	// armor.armorVertices[2] = lower_r;
	// armor.armorVertices[3] = lower_l;
	armor.armorVertices[0] = lower_l;
	armor.armorVertices[1] = upper_l;
	armor.armorVertices[2] = upper_r;
	armor.armorVertices[3] = lower_r;

}

ArmorBox::ArmorBox() {
	l_index = -1;
	r_index = -1;
	l_light = LightBar();
	r_light = LightBar();
	armorNum = 0;
	armorVertices.resize(4);
	type = SMALL_ARMOR;
	center = Point2f();
	armorRect = Rect();
	armorImg = Mat();
}

ArmorBox::ArmorBox(const LightBar & l_light, const LightBar & r_light) {
	this->l_light = l_light;
	this->r_light = r_light;
	
	armorNum = 0;
	armorAngle = (l_light.angle + r_light.angle) / 2;
	
	//set armorVertices bl->tl->tr->br     ?????? ?????? ?????? ??????
	armorVertices.resize(4);
	setArmorVertices(l_light, r_light, *this);    // '*this' means the reference of this ArmorBox

	//set armor center
	center = crossPointof(armorVertices[0], armorVertices[1], armorVertices[2], armorVertices[3]);

	//set armorRect using boundingRect for convenience
	armorRect = boundingRect(armorVertices);

	//set armor type according to its x ration
	if (this->getDislocationX() > 1.5)
		type = BIG_ARMOR;
}

ArmorBox::~ArmorBox() {}

// angle difference: the angle difference of left and right lights ??????????????????????????????
float ArmorBox::getAngleDiff() const {
	float angle_diff = abs(l_light.angle - r_light.angle); //get the abs of angle_diff ??????????????????
	return angle_diff;
}

// deviation angle : the horizon angle of the line of centers of lights ??????????????????(???????????????????????????????????????) 
float ArmorBox::getDeviationAngle() const {
	float delta_x = r_light.center.x - l_light.center.x; //??x
	float delta_y = r_light.center.y - l_light.center.y; //??y
	float deviationAngle = abs(atan(delta_y / delta_x)) * 180 / CV_PI; //tan??=??y/??x
	return deviationAngle;
}

// dislocation judge X: r-l light center distance ration on the X-axis ?????????????????? ???????????????x??????????????????
float ArmorBox::getDislocationX() const {

	float meanLen = (l_light.length + r_light.length) / 2;
	float xDiff = abs(l_light.center.x - r_light.center.x); //x distance ration y??????????????????????????????y????????????????????????????????????
	float xDiff_ratio = xDiff / meanLen;

	return xDiff_ratio;
}

// dislocation judge Y:  r-l light center distance ration on the Y-axis ?????????????????? ???????????????Y??????????????????
float ArmorBox::getDislocationY() const {

	// float meanLen = (l_light.length + r_light.length) / 2;
	// float yDiff = abs(l_light.center.y - r_light.center.y);  //y distance ration x??????????????????????????????x????????????????????????????????????
	// float yDiff_ratio = yDiff / meanLen;

	/// (lzj) match armor : use the smaller one's lengh instead of mean value of the two
	float minLen = min(l_light.length , r_light.length);
	float yDiff = abs(l_light.center.y - r_light.center.y);  //y distance ration x??????????????????????????????x????????????????????????????????????
	float yDiff_ratio = yDiff / minLen;

	return yDiff_ratio;
}

// length difference ration: the length difference ration r-l lights ???????????????????????????
float ArmorBox::getLengthRation() const {

	/// (lzj) match armor : use the smaller one's lengh instead of mean value of the two
	float length_diff = abs(l_light.length - r_light.length);
	float lengthDiffRation = length_diff / std::min(l_light.length, r_light.length);

	return lengthDiffRation;
}

// judge whether this armor is suitable or not  ?????????????????????????????????????????????
bool ArmorBox::isSuitableArmor() const
{
	return
		// (lzj) Cancel match light angle 
		this->getAngleDiff()	  < armorParam.max_angle_diff	    &&		// angle difference judge the angleDiff should be less than max_angle_diff ??????????????????????????????????????????????????????
		this->getDeviationAngle() < armorParam.max_deviation_angle  &&		// deviation angle judge: the horizon angle of the line of centers of lights ??????????????????(???????????????????????????????????????)?????? 
		this->getDislocationX()	  < armorParam.max_x_diff_ratio     &&		// dislocation judge: the x and y can not be too far ?????????????????? ???????????????x???y?????????????????????????????????????????????????????????
		this->getDislocationY()	  < armorParam.max_y_diff_ratio     &&		// dislocation judge: the x and y can not be too far ?????????????????? ???????????????x???y?????????????????????????????????????????????????????????
		this->getDislocationX()   > armorParam.min_x_diff_ratio     &&      // (lzj)
		this->getLengthRation()   < armorParam.max_lengthDiff_ratio;
}