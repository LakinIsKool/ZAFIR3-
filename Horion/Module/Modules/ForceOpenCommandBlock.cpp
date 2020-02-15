#include "ForceOpenCommandBlock.h"

ForceOpenCommandBlock::ForceOpenCommandBlock() : IModule(0x0, Category::EXPLOITS, "Open command blocks even if you are not operator") {
}

ForceOpenCommandBlock::~ForceOpenCommandBlock() {
}

const char* ForceOpenCommandBlock::getModuleName() {
	return ("ForceOpenCommandBlock");
}

void ForceOpenCommandBlock::onTick(C_GameMode* gm) {
	if (!GameData::canUseMoveKeys()) return;
	PointingStruct* pointingStruct = g_Data.getClientInstance()->getPointerStruct();
	C_Block* block = gm->player->region->getBlock(pointingStruct->block);
	int blockId = block->toLegacy()->blockId;
	if (GameData::isRightClickDown() && !clicked) {
		clicked = true;
		if (pointingStruct->entityPtr != nullptr && pointingStruct->entityPtr->getEntityTypeId() == 100) {
			__int64* id = pointingStruct->entityPtr->getUniqueId();
			g_Data.getLocalPlayer()->openCommandBlockMinecart(*id);
		} else if (block != nullptr && (blockId == 137 || blockId == 188 || blockId == 189)) {
			g_Data.getLocalPlayer()->openCommandBlock(pointingStruct->block);
		}
	} else if (!GameData::isRightClickDown()) {
		clicked = false;
	}
}
