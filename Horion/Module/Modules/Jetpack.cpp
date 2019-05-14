#include "Jetpack.h"



Jetpack::Jetpack() : IModule('F')
{
	registerFloatSetting("speed", &this->speedMod, 1);
}


Jetpack::~Jetpack()
{
}

bool Jetpack::isFlashMode() {
	return true;
}

std::string Jetpack::getModuleName()
{
	return std::string("Jetpack");
}


void Jetpack::onTick(C_GameMode * gm)
{
	float calcYaw = (gm->player->yaw + 90) *  (PI / 180);
	float calcPitch = (gm->player->pitch)  * -(PI / 180);

	vec3_t moveVec;
	moveVec.x = cos(calcYaw) * cos(calcPitch) * speedMod;
	moveVec.y = sin(calcPitch)				  * speedMod;
	moveVec.z = sin(calcYaw) * cos(calcPitch) * speedMod;
	gm->player->setVelocity(moveVec);
}