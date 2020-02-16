#include "Hooks.h"

Hooks g_Hooks;
bool isTicked = false;
//#define TEST_DEBUG

void Hooks::Init() {
	logF("Setting up Hooks...");
	// clang-format off
	// Vtables
	{
		// GameMode::vtable
		{
			uintptr_t sigOffset = FindSignature("48 8D 05 ?? ?? ?? ?? 48 89 01 33 D2 48 C7 41 ??");
			int offset = *reinterpret_cast<int*>(sigOffset + 3);
			uintptr_t** gameModeVtable = reinterpret_cast<uintptr_t**>(sigOffset + offset + /*length of instruction*/ 7);
			if (gameModeVtable == 0x0 || sigOffset == 0x0)
				logF("C_GameMode signature not working!!!");
			else {
				g_Hooks.GameMode_tickHook = std::make_unique<FuncHook>(gameModeVtable[9], Hooks::GameMode_tick);

				g_Hooks.GameMode_startDestroyBlockHook = std::make_unique<FuncHook>(gameModeVtable[1], Hooks::GameMode_startDestroyBlock);

				g_Hooks.GameMode_getPickRangeHook = std::make_unique<FuncHook>(gameModeVtable[10], Hooks::GameMode_getPickRange);
			}
		}

		// BlockLegacy::vtable
		{
			uintptr_t sigOffset = FindSignature("48 8D ?? ?? ?? ?? ?? 48 89 ?? 4C 39");  // BlockLegacy constructor
			int offset = *reinterpret_cast<int*>(sigOffset + 3);
			uintptr_t** blockLegacyVtable = reinterpret_cast<uintptr_t**>(sigOffset + offset + /*length of instruction*/ 7);
			if (blockLegacyVtable == 0x0 || sigOffset == 0x0)
				logF("C_BlockLegacy signature not working!!!");
			else {
				g_Hooks.BlockLegacy_getRenderLayerHook = std::make_unique<FuncHook>(blockLegacyVtable[118], Hooks::BlockLegacy_getRenderLayer);

				g_Hooks.BlockLegacy_getLightEmissionHook = std::make_unique<FuncHook>(blockLegacyVtable[15], Hooks::BlockLegacy_getLightEmission);
			}
		}

		// LocalPlayer::vtable
		{
			uintptr_t sigOffset = FindSignature("48 8D 05 ?? ?? ?? ?? 49 89 ?? ?? 49 8D ?? ?? ?? ?? ?? ?? 4D 8B");
			int offset = *reinterpret_cast<int*>(sigOffset + 3);
			uintptr_t** localPlayerVtable = reinterpret_cast<uintptr_t**>(sigOffset + offset + /*length of instruction*/ 7);
			if (localPlayerVtable == 0x0 || sigOffset == 0x0)
				logF("C_LocalPlayer signature not working!!!");
			else {
				g_Hooks.Actor_isInWaterHook = std::make_unique<FuncHook>(localPlayerVtable[61], Hooks::Actor_isInWater);

				g_Hooks.Actor_startSwimmingHook = std::make_unique<FuncHook>(localPlayerVtable[181], Hooks::Actor_startSwimming);

				g_Hooks.Actor_ladderUpHook = std::make_unique<FuncHook>(localPlayerVtable[321], Hooks::Actor_ladderUp);
			}
		}

		// MoveInputHandler::vtable
		{
			uintptr_t sigOffset = FindSignature("48 8D 05 ?? ?? ?? ?? 48 89 03 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 48 8B 42 ?? 48 85 C0 74 04");
			int offset = *reinterpret_cast<int*>(sigOffset + 3);
			uintptr_t** moveInputVtable = reinterpret_cast<uintptr_t**>(sigOffset + offset + /*length of instruction*/ 7);
			if (moveInputVtable == 0x0 || sigOffset == 0x0)
				logF("C_GameMode signature not working!!!");
			else {
				g_Hooks.MoveInputHandler_tickHook = std::make_unique<FuncHook>(moveInputVtable[1], Hooks::MoveInputHandler_tick);
			}
		}
	}

	// Signatures
	{
		void* surv_tick = reinterpret_cast<void*>(FindSignature("48 8B C4 55 57 41 56 48 8D 68 A1 48 ?? ?? ?? ?? ?? ?? 48 ?? ?? ?? ?? ?? ?? ?? 48 89 58 ?? 48 89 70 ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 45 ?? 48 8B F9 8B 41"));
		g_Hooks.SurvivalMode_tickHook = std::make_unique<FuncHook>(surv_tick, Hooks::SurvivalMode_tick);

		void* _sendChatMessage = reinterpret_cast<void*>(FindSignature("40 57 48 83 EC ?? 48 C7 44 24 ?? FE FF FF FF 48 89 9C 24 ?? ?? 00 00 48 8B D9 48 83 B9"));
		g_Hooks.ChatScreenController_sendChatMessageHook = std::make_unique<FuncHook>(_sendChatMessage, Hooks::ChatScreenController_sendChatMessage);

		void* _renderText = reinterpret_cast<void*>(FindSignature("48 8B C4 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 C7 45 ?? FE FF FF FF 48 89 58 ?? 0F 29 70 ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 ?? ?? ?? ?? 48 89 54 24"));
		g_Hooks.RenderTextHook = std::make_unique<FuncHook>(_renderText, Hooks::RenderText);

		void* setupRender = reinterpret_cast<void*>(FindSignature("40 57 48 ?? ?? ?? ?? ?? ?? 48 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 ?? ?? ?? ?? ?? ?? ?? ?? 48 8B DA 48 8B F9 33 D2 ?? ?? ?? ?? ?? ?? 48 8D 4C 24 30 E8 ?? ?? ?? ?? 4C 8B CF 4C 8B C3 48 8B 57 ?? 48 8D 4C 24 ??"));
		g_Hooks.UIScene_setupAndRenderHook = std::make_unique<FuncHook>(setupRender, Hooks::UIScene_setupAndRender);

		void* render = reinterpret_cast<void*>(FindSignature("40 56 57 41 56 48 ?? ?? ?? ?? ?? ?? 48 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 ?? ?? ?? ?? ?? ?? ?? ?? 48 8B FA 48 8B D9 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 48 8B 30 41 8B 04 36 39 05 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 33 C0"));
		g_Hooks.UIScene_renderHook = std::make_unique<FuncHook>(render, Hooks::UIScene_render);

		void* fogColorFunc = reinterpret_cast<void*>(FindSignature("0F 28 C2 C7 42 0C 00 00 80 3F F3"));
		g_Hooks.Dimension_getFogColorHook = std::make_unique<FuncHook>(fogColorFunc, Hooks::Dimension_getFogColor);

		void* timeOfDay = reinterpret_cast<void*>(FindSignature("44 8B C2 B8 F1 19 76 05"));
		g_Hooks.Dimension_getTimeOfDayHook = std::make_unique<FuncHook>(timeOfDay, Hooks::Dimension_getTimeOfDay);

		//void* sunLightIntensity = reinterpret_cast<void*>(FindSignature("48 89 5C 24 ?? 57 48 83 EC ?? 48 8B B9 ?? ?? ?? ?? 49 8B D8 0F"));

		void* ChestTick = reinterpret_cast<void*>(FindSignature("40 53 57 48 83 EC ?? 48 8B 41 ?? 48 8B FA 48 89 6C 24 ?? 48 8B D9 4C 89 74 24 ?? 48 85 C0 75 10 48 8D 51 ?? 48 8B CF E8 ?? ?? ?? ?? 48 89 43 ?? FF 43 ?? 48 85 C0"));
		g_Hooks.ChestBlockActor_tickHook = std::make_unique<FuncHook>(ChestTick, Hooks::ChestBlockActor_tick);

		void* lerpFunc = reinterpret_cast<void*>(FindSignature("8B 02 89 81 ?? 04 ?? ?? 8B 42 04 89 81 ?? ?? ?? ?? 8B 42 08 89 81 ?? ?? ?? ?? C3"));
		g_Hooks.Actor_lerpMotionHook = std::make_unique<FuncHook>(lerpFunc, Hooks::Actor_lerpMotion);

		void* getGameEdition = reinterpret_cast<void*>(FindSignature("8B 91 ?? ?? ?? ?? 85 D2 74 1C 83 EA 01"));
		g_Hooks.AppPlatform_getGameEditionHook = std::make_unique<FuncHook>(getGameEdition, Hooks::AppPlatform_getGameEdition);

		void* autoComplete = reinterpret_cast<void*>(FindSignature("48 8B C4 55 57 41 56 48 8D 68 ?? 48 81 EC ?? ?? ?? ?? 48 C7 45 ?? FE FF FF FF 48 89 58 ?? 48 89 70 ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 45 ?? 41 8B F9"));
		g_Hooks.PleaseAutoCompleteHook = std::make_unique<FuncHook>(autoComplete, Hooks::PleaseAutoComplete);

		void* sendtoServer = reinterpret_cast<void*>(FindSignature("48 89 5C 24 08 57 48 ?? ?? ?? ?? ?? ?? 0F B6 41 ?? 48 8B FA 88 42 ?? 48 8D 54 24 ?? 48 8B 59 ?? 48 8B CB E8 ?? ?? ?? ?? 45 33 C9"));
		g_Hooks.LoopbackPacketSender_sendToServerHook = std::make_unique<FuncHook>(sendtoServer, Hooks::LoopbackPacketSender_sendToServer);

		void* getFov = reinterpret_cast<void*>(FindSignature("40 53 48 83 EC ?? 0F 29 74 24 ?? 0F 29 7C 24 ?? 44 0F 29 44 24 ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 44 24 ??"));
		g_Hooks.LevelRendererPlayer_getFovHook = std::make_unique<FuncHook>(getFov, Hooks::LevelRendererPlayer_getFov);

		void* tick_entityList = reinterpret_cast<void*>(FindSignature("48 89 ?? ?? ?? 57 48 83 EC ?? 48 8B ?? E8 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 48 8B D8 ?? ?? ?? ?? ?? ?? 48 99"));
		g_Hooks.MultiLevelPlayer_tickHook = std::make_unique<FuncHook>(tick_entityList, Hooks::MultiLevelPlayer_tick);

		void* keyMouseFunc = reinterpret_cast<void*>(FindSignature("40 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 ?? ?? ?? ?? ?? ?? 48 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 0F 29 74 24 70 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 45 E0 49 8B F8 48 8B F1 45 33 ED 41 8B DD 89 5D DC 49 8B C8 E8"));
		g_Hooks.HIDController_keyMouseHook = std::make_unique<FuncHook>(keyMouseFunc, Hooks::HIDController_keyMouse);

		void* renderLevel = reinterpret_cast<void*>(FindSignature("40 53 56 57 48 81 EC ?? ?? ?? ?? 48 C7 44 24 ?? FE FF FF FF 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 84 24 ?? ?? ?? ?? 49 8B D8 48 8B FA 48 8B F1 33 D2"));
		g_Hooks.LevelRenderer_renderLevelHook = std::make_unique<FuncHook>(renderLevel, Hooks::LevelRenderer_renderLevel);

		void* clickHook = reinterpret_cast<void*>(FindSignature("48 8B C4 48 89 58 ?? 48 89 68 ?? 48 89 70 ?? 57 41 54 41 55 41 56 41 57 48 83 EC 60 44 ?? ?? ?? ?? ?? ?? ?? ?? 33 F6"));
		g_Hooks.ClickFuncHook = std::make_unique<FuncHook>(clickHook, Hooks::ClickFunc);

		void* chestScreenControllerTick = reinterpret_cast<void*>(FindSignature("48 89 5C 24 08 57 48 83 EC 20 48 8B F9 E8 ?? ?? ?? ?? 48 8B 17 48 8B CF 8B D8 FF 92 ?? ?? ?? ?? 84 C0 74 31"));
		g_Hooks.ChestScreenController_tickHook = std::make_unique<FuncHook>(chestScreenControllerTick, Hooks::ChestScreenController_tick);

		void* fullbright = reinterpret_cast<void*>(FindSignature("40 57 48 83 EC 40 48 ?? ?? ?? ?? ?? ?? ?? ?? 48 89 5C 24 ?? 48 89 74 24 ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 44 24 ?? 33 C0 48 89 44 24 ?? 48 89 44 24 ?? 48 8B 01 48 8D 54 24 ??"));
		g_Hooks.GetGammaHook = std::make_unique<FuncHook>(fullbright, Hooks::GetGamma);

		void* jump = reinterpret_cast<void*>(FindSignature("40 57 48 83 EC ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 44 24 ?? 48 8B 01 48 8B F9 0F 29 74 24"));
		g_Hooks.JumpPowerHook = std::make_unique<FuncHook>(jump, Hooks::JumpPower);

		void* onAppSuspended = reinterpret_cast<void*>(FindSignature("48 8B C4 55 48 8B EC 48 83 EC ?? 48 ?? ?? ?? ?? ?? ?? ?? 48 89 58 ?? 48 89 70 ?? 48 89 78 ?? 48 8B F1 ?? ?? ?? ?? ?? E8"));
		g_Hooks.MinecraftGame_onAppSuspendedHook = std::make_unique<FuncHook>(onAppSuspended, Hooks::MinecraftGame_onAppSuspended);

		void* RakNetInstance__tick = reinterpret_cast<void*>(FindSignature("48 8B C4 55 41 56 41 57 ?? ?? ?? ?? ?? ?? ?? 48 ?? ?? ?? ?? ?? ?? 48 ?? ?? ?? ?? ?? ?? ?? 48 89 58 ?? 48 89 70 ?? 48 89 78 ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 ?? ?? ?? ?? ?? ?? ?? 48 8B F1 80 B9 ?? ?? ?? ?? ?? 74 51 C6 81 ?? ?? ?? ?? ?? 66 ?? ?? ?? ?? ?? ?? ?? 74 40 48 8D 45"));
		g_Hooks.RakNetInstance_tickHook = std::make_unique<FuncHook>(RakNetInstance__tick, Hooks::RakNetInstance_tick);

		void* ConnectionRequest__create = reinterpret_cast<void*>(FindSignature("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D ?? ?? ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 C7 ?? ?? FE FF FF FF 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 45 ?? 49 8B D9 4C"));
		g_Hooks.ConnectionRequest_createHook = std::make_unique<FuncHook>(ConnectionRequest__create, Hooks::ConnectionRequest_create);

	#ifdef TEST_DEBUG
		void* addAction = reinterpret_cast<void*>(FindSignature("40 55 56 57 41 56 41 57 48 83 EC 30 48 ?? ?? ?? ?? ?? ?? ?? ?? 48 89 5C 24 ?? 48 8B EA 4C 8B F1 4C 8B C2 48 8B 51 ?? 48 8B 49 ?? E8"));
		g_Hooks.InventoryTransactionManager__addActionHook = std::make_unique<FuncHook>(addAction, Hooks::InventoryTransactionManager__addAction);
	#endif
	}
	MH_EnableHook(MH_ALL_HOOKS);
// clang-format on
}

void Hooks::Restore() {
	g_Hooks.GameMode_tickHook->Restore();
	g_Hooks.ChatScreenController_sendChatMessageHook->Restore();
	g_Hooks.RenderTextHook->Restore();
	g_Hooks.AppPlatform_getGameEditionHook->Restore();
	g_Hooks.PleaseAutoCompleteHook->Restore();
	g_Hooks.LevelRendererPlayer_getFovHook->Restore();
	g_Hooks.ChestBlockActor_tickHook->Restore();
	g_Hooks.LoopbackPacketSender_sendToServerHook->Restore();
	g_Hooks.MultiLevelPlayer_tickHook->Restore();
	g_Hooks.GameMode_startDestroyBlockHook->Restore();
	g_Hooks.HIDController_keyMouseHook->Restore();
	g_Hooks.BlockLegacy_getRenderLayerHook->Restore();
	g_Hooks.LevelRenderer_renderLevelHook->Restore();
	g_Hooks.BlockLegacy_getLightEmissionHook->Restore();
	g_Hooks.ClickFuncHook->Restore();
	g_Hooks.MoveInputHandler_tickHook->Restore();
	g_Hooks.ChestScreenController_tickHook->Restore();
	g_Hooks.GetGammaHook->Restore();
	g_Hooks.Actor_isInWaterHook->Restore();
	g_Hooks.JumpPowerHook->Restore();
	g_Hooks.MinecraftGame_onAppSuspendedHook->Restore();
	g_Hooks.Actor_ladderUpHook->Restore();
	g_Hooks.RakNetInstance_tickHook->Restore();
	g_Hooks.GameMode_getPickRangeHook->Restore();
	//g_Hooks.InventoryTransactionManager_addActionHook->Restore();
	MH_DisableHook(MH_ALL_HOOKS);
	Sleep(10);
}

void Hooks::GameMode_tick(C_GameMode* _this) {
	static auto oTick = g_Hooks.GameMode_tickHook->GetFastcall<void, C_GameMode*>();
	oTick(_this);

	GameData::updateGameData(_this);
	if (_this->player == g_Data.getLocalPlayer() && _this->player != nullptr) {
		moduleMgr->onTick(_this);
	}
}

void Hooks::SurvivalMode_tick(C_GameMode* _this) {
	static auto oTick = g_Hooks.SurvivalMode_tickHook->GetFastcall<void, C_GameMode*>();
	oTick(_this);
	GameData::updateGameData(_this);
	if (_this->player == g_Data.getLocalPlayer() && _this->player != nullptr) {
		moduleMgr->onTick(_this);
	}
}

void Hooks::ChatScreenController_sendChatMessage(uint8_t* _this) {
	static auto oSendMessage = g_Hooks.ChatScreenController_sendChatMessageHook->GetFastcall<void, void*>();

	using addCommandToChatHistory_t = void(__fastcall*)(__int64*, char*);
	static addCommandToChatHistory_t addCommandToChatHistory = reinterpret_cast<addCommandToChatHistory_t>(FindSignature("48 89 5C 24 ?? 57 48 83 EC ?? 48 83 79 ?? ?? 48 8B FA 48 8B D9 76 46 48 8B 41 ?? 48 89 74 24 ?? 33 F6"));

	uintptr_t* textLength = reinterpret_cast<uintptr_t*>(_this + 0x710);
	if (*textLength) {
		char* message = reinterpret_cast<char*>(_this + 0x700);
		if (*reinterpret_cast<__int64*>(_this + 0x718) >= 0x10)
			message = *reinterpret_cast<char**>(message);

		if (*message == cmdMgr->prefix) {
			cmdMgr->execute(message);

			__int64* a1 = (__int64*)(*(__int64(__cdecl**)(__int64))(**(__int64**)(*(__int64*)(_this + 0x668) + 0x20i64) + 0x960i64))(*(__int64*)(*(__int64*)(_this + 0x668) + 0x20i64));
			addCommandToChatHistory(a1, (char*)(_this + 0x700));  // This will put the command in the chat history (Arrow up/down)

			__int64 v17 = 0;
			__int64* v15 = *(__int64**)(*(__int64*)(_this + 0x668) + 0x20i64);
			__int64 v16 = *v15;

			if (*(BYTE*)(_this + 0x72A))
				v17 = (*(__int64(__cdecl**)(__int64*))(v16 + 0x968))(v15);
			else
				v17 = (*(__int64(__cdecl**)(__int64*))(v16 + 0x960))(v15);
			*(DWORD*)(_this + 0x724) = *(DWORD*)(v17 + 0x20);

			*reinterpret_cast<__int64*>(_this + 0x710) = 0i64;
			*message = 0x0;     // Remove command in textbox
			*textLength = 0x0;  // text length
			return;
		}
	}
	oSendMessage(_this);
}

__int64 Hooks::UIScene_setupAndRender(C_UIScene* uiscene, __int64 screencontext) {
	static auto oSetup = g_Hooks.UIScene_setupAndRenderHook->GetFastcall<__int64, C_UIScene*, __int64>();

	g_Hooks.shouldRender = uiscene->isPlayScreen();

	return oSetup(uiscene, screencontext);
}

__int64 Hooks::UIScene_render(C_UIScene* uiscene, __int64 screencontext) {
	static auto oRender = g_Hooks.UIScene_renderHook->GetFastcall<__int64, C_UIScene*, __int64>();

	g_Hooks.shouldRender = uiscene->isPlayScreen();
	if (!g_Hooks.shouldRender) {
		std::unique_ptr<TextHolder> alloc(new TextHolder());

		uiscene->getScreenName(alloc.get());

		if (strcmp(alloc->getText(), "hud_screen") == 0 || strcmp(alloc->getText(), "start_screen") == 0 || (alloc->getTextLength() >= 11 && strncmp(alloc->getText(), "play_screen", 11)) == 0)
			g_Hooks.shouldRender = true;
	}

	return oRender(uiscene, screencontext);
}

__int64 Hooks::RenderText(__int64 a1, C_MinecraftUIRenderContext* renderCtx) {
	static auto oText = g_Hooks.RenderTextHook->GetFastcall<__int64, __int64, C_MinecraftUIRenderContext*>();
	C_GuiData* dat = g_Data.getClientInstance()->getGuiData();
	DrawUtils::setCtx(renderCtx, dat);
	if (GameData::shouldHide())
		return oText(a1, renderCtx);

	{
		static bool wasConnectedBefore = false;
		static LARGE_INTEGER start;
		static LARGE_INTEGER frequency;
		if (frequency.QuadPart == 0) {
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&start);
		}
		if (!g_Data.isInjectorConnectionActive()) {
			__int64 retval = oText(a1, renderCtx);

			LARGE_INTEGER end, elapsed;
			QueryPerformanceCounter(&end);
			elapsed.QuadPart = end.QuadPart - start.QuadPart;
			elapsed.QuadPart /= frequency.QuadPart;
			if (elapsed.QuadPart > 1) {
				vec2_t windowSize = dat->windowSize;

				DrawUtils::fillRectangle(vec4_t(0, 0, windowSize.x, windowSize.y), MC_Color(0.2f, 0.2f, 0.2f, 1.f), 0.8f);

				std::string text = "Download the new injector at http://horionbeta.club/";
				if (!wasConnectedBefore)
					DrawUtils::drawText(vec2_t(windowSize.x / 2 - DrawUtils::getTextWidth(&text, 1.5f) / 2, windowSize.y * 0.4f), &text, MC_Color(), 1.5f);
				text = "Remember to keep the injector open while playing";
				DrawUtils::drawText(vec2_t(windowSize.x / 2 - DrawUtils::getTextWidth(&text, wasConnectedBefore ? 1.5f : 0.7f) / 2, windowSize.y * (wasConnectedBefore ? 0.5f : 0.7f)), &text, MC_Color(), wasConnectedBefore ? 1.5f : 0.7f);
				text = "Uninject by holding down CTRL + L";
				DrawUtils::drawText(vec2_t(windowSize.x / 2 - DrawUtils::getTextWidth(&text, 0.7f) / 2, windowSize.y * 0.8f), &text, MC_Color(), 0.7f);

				DrawUtils::flush();
			}

			return retval;
		} else
			wasConnectedBefore = true;
	}

	// Call PreRender() functions
	moduleMgr->onPreRender(renderCtx);
	DrawUtils::flush();

	__int64 retval = oText(a1, renderCtx);

#ifdef PERFORMANCE_TEST
	std::chrono::steady_clock::time_point beginPostRender = std::chrono::steady_clock::now();
#endif

	bool shouldRenderArrayList = true;
	bool shouldRenderTabGui = true;
	bool shouldRenderCoords = false;
	bool shouldRenderWatermark = true;

	// Call PostRender() functions
	{
		moduleMgr->onPostRender(renderCtx);
		static HudModule* hud = moduleMgr->getModule<HudModule>();
		if (hud == nullptr)
			hud = moduleMgr->getModule<HudModule>();
		else {
			shouldRenderTabGui = hud->tabgui && hud->isEnabled();
			shouldRenderArrayList = hud->arraylist && hud->isEnabled();
			shouldRenderCoords = hud->coordinates && hud->isEnabled();
			shouldRenderWatermark = hud->watermark && hud->isEnabled();
		}

		static IModule* ClickGuiModule = moduleMgr->getModule<ClickGuiMod>();
		if (ClickGuiModule == nullptr)
			ClickGuiModule = moduleMgr->getModule<ClickGuiMod>();
		else if (ClickGuiModule->isEnabled()) {
			ClickGui::render();
			shouldRenderArrayList = false;
			shouldRenderCoords = false;
			shouldRenderTabGui = false;
			shouldRenderWatermark = false;
		}

		if (shouldRenderTabGui) TabGui::render();
	}

	{
		// Display ArrayList on the Right?
		static constexpr bool isOnRightSide = true;
		static float rcolors[4];          // Rainbow color array RGBA
		static float disabledRcolors[4];  // Rainbow Colors, but for disabled modules
		static float currColor[4];        // ArrayList collors

		float yOffset = 0;  // Offset of next Text
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		vec2_t windowSizeReal = g_Data.getClientInstance()->getGuiData()->windowSizeReal;

		vec2_t mousePos = *g_Data.getClientInstance()->getMousePos();
		mousePos.div(windowSizeReal);
		mousePos.mul(windowSize);

		// Rainbow color updates
		{
			DrawUtils::rainbow(rcolors);  // Increase Hue of rainbow color array
			disabledRcolors[0] = min(1, rcolors[0] * 0.4f + 0.2f);
			disabledRcolors[1] = min(1, rcolors[1] * 0.4f + 0.2f);
			disabledRcolors[2] = min(1, rcolors[2] * 0.4f + 0.2f);
			disabledRcolors[3] = 1;
		}

		// Draw Horion logo
		static HudModule* hud = moduleMgr->getModule<HudModule>();
		if (hud == nullptr)
			hud = moduleMgr->getModule<HudModule>();
		else if (shouldRenderWatermark) {
			constexpr float nameTextSize = 1.5f;
			constexpr float versionTextSize = 0.7f;
			static const float textHeight = (nameTextSize + versionTextSize * 0.7f /* We don't quite want the version string in its own line, just a bit below the name */) * DrawUtils::getFont(Fonts::SMOOTH)->getLineHeight();
			constexpr float borderPadding = 1;
			constexpr float margin = 5;

			static std::string name = "Horion";
#ifdef _DEBUG
			static std::string version = "dev";
#elif defined _BETA
			static std::string version = "beta";
#else
			static std::string version = "public";
#endif

			float nameLength = DrawUtils::getTextWidth(&name, nameTextSize);
			float fullTextLength = nameLength + DrawUtils::getTextWidth(&version, versionTextSize);
			vec4_t rect = vec4_t(
				windowSize.x - margin - fullTextLength - borderPadding * 2,
				windowSize.y - margin - textHeight,
				windowSize.x - margin + borderPadding,
				windowSize.y - margin);

			DrawUtils::drawRectangle(rect, MC_Color(13, 29, 48, 1), 1.f, 2.f);
			DrawUtils::fillRectangle(rect, MC_Color(rcolors), 1.f);
			DrawUtils::drawText(vec2_t(rect.x + borderPadding, rect.y), &name, MC_Color(6, 15, 24, 1), nameTextSize);
			DrawUtils::drawText(vec2_t(rect.x + borderPadding + nameLength, rect.w - 7), &version, MC_Color(0, 0, 0, 0), versionTextSize);
		}

		// Draw ArrayList
		if (moduleMgr->isInitialized() && shouldRenderArrayList) {
			struct IModuleContainer {
				// Struct used to Sort IModules in a std::set
				IModule* backingModule;
				std::string moduleName;
				bool enabled;
				int keybind;
				float textWidth;
				vec2_t* pos;
				bool shouldRender = true;

				IModuleContainer(IModule* mod) {
					const char* moduleNameChr = mod->getModuleName();
					this->enabled = mod->isEnabled();
					this->keybind = mod->getKeybind();
					this->backingModule = mod;
					this->pos = mod->getPos();

					if (keybind == 0x0)
						moduleName = moduleNameChr;
					else {
						char text[50];
						sprintf_s(text, 50, "%s%s", moduleNameChr, hud->keybinds ? std::string(" [" + std::string(Utils::getKeybindName(keybind)) + "]").c_str() : "");
						moduleName = text;
					}

					if (!this->enabled && *this->pos == vec2_t(0.f, 0.f))
						this->shouldRender = false;
					this->textWidth = DrawUtils::getTextWidth(&moduleName);
				}

				bool operator<(const IModuleContainer& other) const {
					/*if (enabled) {
						if (!other.enabled)  // We are enabled
							return true;
					} else if (other.enabled)  // They are enabled
						return false;*/

					if (this->textWidth == other.textWidth)
						return moduleName < other.moduleName;
					return this->textWidth > other.textWidth;
				}
			};

			// Parameters
			static constexpr float textPadding = 1.0f;
			static constexpr float textSize = 1.0f;
			static constexpr float textHeight = textSize * 10.0f;
			static constexpr float smoothNess = 0.95f;

			// Mouse click detector
			static bool wasLeftMouseDown = GameData::isLeftClickDown();  // Last isDown value
			bool leftMouseDown = GameData::isLeftClickDown();            // current isDown value

			bool executeClick = leftMouseDown && leftMouseDown != wasLeftMouseDown;  // isDown == true AND (current state IS NOT last state)
			wasLeftMouseDown = leftMouseDown;                                        // Set last isDown value

			std::set<IModuleContainer> modContainerList;
			// Fill modContainerList with Modules
			{
				std::vector<IModule*>* moduleList = moduleMgr->getModuleList();

				for (auto it : *moduleList) {
					//if (it->isEnabled())
					{
						HudModule* hud = moduleMgr->getModule<HudModule>();
						if (it != hud) modContainerList.emplace(IModuleContainer(it));
					}
				}
			}

			int a = 0;
			int b = 0;
			int c = 0;

			// Loop through mods to display Labels
			for (std::set<IModuleContainer>::iterator it = modContainerList.begin(); it != modContainerList.end(); ++it) {
				if (!it->shouldRender)
					continue;

				std::string textStr = it->moduleName;
				float textWidth = it->textWidth;

				float xOffsetOri = windowSize.x - textWidth - (textPadding * 2);

				float xOffset = windowSize.x - it->pos->x;

				it->pos->x += smoothNess;

				if (xOffset < xOffsetOri) {
					xOffset = xOffsetOri;
				}
				if (!it->enabled) {
					xOffset += it->pos->y;
					it->pos->y += smoothNess;
				}
				if (xOffset >= windowSize.x && !it->enabled) {
					it->pos->x = 0.f;
					it->pos->y = 0.f;
				}

				vec2_t textPos = vec2_t(
					xOffset + textPadding,
					yOffset + textPadding);
				vec4_t rectPos = vec4_t(
					xOffset - 2,
					yOffset,
					isOnRightSide ? windowSize.x : textWidth + (textPadding * 2),
					yOffset + textPadding * 2 + textHeight);
				vec4_t leftRect = vec4_t(
					xOffset - 2,
					yOffset,
					xOffset - 1,
					yOffset + textPadding * 2 + textHeight);
				c++;
				b++;
				if (b < 20)
					a = moduleMgr->getEnabledModuleCount();
				else
					b = 0;
				currColor[3] = rcolors[3];
				Utils::ColorConvertRGBtoHSV(rcolors[0], rcolors[1], rcolors[2], currColor[0], currColor[1], currColor[2]);
				currColor[0] += 1.f / a * c;
				Utils::ColorConvertHSVtoRGB(currColor[0], currColor[1], currColor[2], currColor[0], currColor[1], currColor[2]);

				DrawUtils::fillRectangle(rectPos, MC_Color(13, 29, 48, 1), 1.f);
				DrawUtils::fillRectangle(leftRect, MC_Color(currColor), 1.f);
				if (!GameData::canUseMoveKeys() && rectPos.contains(&mousePos) && hud->clickToggle) {
					vec4_t selectedRect = rectPos;
					selectedRect.x = leftRect.z;
					if (leftMouseDown) {
						DrawUtils::fillRectangle(selectedRect, MC_Color(0.8f, 0.8f, 0.8f, 0.1f), 0.8f);
						if (executeClick)
							it->backingModule->toggle();
					} else
						DrawUtils::fillRectangle(selectedRect, MC_Color(0.8f, 0.8f, 0.8f, 0.8f), 0.3f);
				}
				DrawUtils::drawText(textPos, &textStr, MC_Color(currColor), textSize);

				yOffset += textHeight + (textPadding * 2);
			}
			c = 0;
			modContainerList.clear();
		}

		// Draw coordinates
		if (moduleMgr->isInitialized() && shouldRenderCoords && g_Data.getLocalPlayer() != nullptr) {
			vec3_t* pos = g_Data.getLocalPlayer()->getPos();
			std::string coords = "XYZ: " + std::to_string((int)pos->x) + " / " + std::to_string((int)pos->y) + " / " + std::to_string((int)pos->z);
			DrawUtils::drawText(vec2_t(5.f, shouldRenderTabGui ? windowSize.y - 12.f : 2.f), &coords, MC_Color(), 1.f);
		}
	}

	DrawUtils::flush();

#ifdef PERFORMANCE_TEST
	std::chrono::steady_clock::time_point endRender = std::chrono::steady_clock::now();
	//logF("PreRender: %.1f", std::chrono::duration_cast<std::chrono::microseconds>(endPreRender - beginPreRender).count() / 1000.f);
	logF("Render: %.2fms", std::chrono::duration_cast<std::chrono::microseconds>(endRender - beginPostRender).count() / 1000.f);
#endif

	return retval;
}

float* Hooks::Dimension_getFogColor(__int64 _this, float* color, float brightness) {
	static auto oGetFogColor = g_Hooks.Dimension_getFogColorHook->GetFastcall<float*, __int64, float*, float>();

	static float rcolors[4];

	static IModule* nightMod = moduleMgr->getModule<NightMode>();
	if (nightMod == nullptr)
		nightMod = moduleMgr->getModule<NightMode>();
	else if (nightMod->isEnabled()) {
		color[0] = 0.f;
		color[1] = 0.f;
		color[2] = 0.2f;
		color[3] = 1;
		return color;
	}

	static IModule* mod = moduleMgr->getModule<RainbowSky>();
	if (mod == nullptr)
		mod = moduleMgr->getModule<RainbowSky>();
	else if (mod->isEnabled()) {
		if (rcolors[3] < 1) {
			rcolors[0] = 1;
			rcolors[1] = 0.2f;
			rcolors[2] = 0.2f;
			rcolors[3] = 1;
		}

		Utils::ColorConvertRGBtoHSV(rcolors[0], rcolors[1], rcolors[2], rcolors[0], rcolors[1], rcolors[2]);  // perfect code, dont question this

		rcolors[0] += 0.001f;
		if (rcolors[0] >= 1)
			rcolors[0] = 0;

		Utils::ColorConvertHSVtoRGB(rcolors[0], rcolors[1], rcolors[2], rcolors[0], rcolors[1], rcolors[2]);

		return rcolors;
	}
	return oGetFogColor(_this, color, brightness);
}

float Hooks::Dimension_getTimeOfDay(__int64 _this, int a2, float a3) {
	static auto oGetTimeOfDay = g_Hooks.Dimension_getTimeOfDayHook->GetFastcall<float, __int64, int, float>();

	static IModule* nightMod = moduleMgr->getModule<NightMode>();
	if (nightMod == nullptr)
		nightMod = moduleMgr->getModule<NightMode>();
	else if (nightMod->isEnabled()) {
		return 0.5f;
	}

	return oGetTimeOfDay(_this, a2, a3);
}

float Hooks::Dimension_getSunIntensity(__int64 a1, float a2, vec3_t* a3, float a4) {
	static auto oGetSunIntensity = g_Hooks.Dimension_getSunIntensityHook->GetFastcall<float, __int64, float, vec3_t*, float>();

	static IModule* nightMod = moduleMgr->getModule<NightMode>();
	if (nightMod == nullptr)
		nightMod = moduleMgr->getModule<NightMode>();
	else if (nightMod->isEnabled()) {
		return -0.5f;
	}

	return oGetSunIntensity(a1, a2, a3, a4);
}

void Hooks::ChestBlockActor_tick(C_ChestBlockActor* _this, void* a) {
	static auto oTick = g_Hooks.ChestBlockActor_tickHook->GetFastcall<void, C_ChestBlockActor*, void*>();
	oTick(_this, a);
	GameData::addChestToList(_this);
}

void Hooks::Actor_lerpMotion(C_Entity* _this, vec3_t motVec) {
	static auto oLerp = g_Hooks.Actor_lerpMotionHook->GetFastcall<void, C_Entity*, vec3_t>();

	if (g_Data.getLocalPlayer() != _this)
		return oLerp(_this, motVec);

	static NoKnockBack* mod = moduleMgr->getModule<NoKnockBack>();
	if (mod == nullptr)
		mod = moduleMgr->getModule<NoKnockBack>();
	else if (mod->isEnabled()) {
		static void* networkSender = reinterpret_cast<void*>(FindSignature("41 80 BF ?? ?? ?? ?? 00 0F 85 ?? ?? ?? ?? FF"));
		if (networkSender == 0x0)
			logF("Network Sender not Found!!!");
		if (networkSender == _ReturnAddress()) {
			if (mod->xModifier == 0 && mod->yModifier == 0)
				return;
			else {
				motVec.x *= mod->xModifier;
				motVec.y *= mod->yModifier;
				motVec.z *= mod->xModifier;
			}
		}
	}
	oLerp(_this, motVec);
}

int Hooks::AppPlatform_getGameEdition(__int64 _this) {
	static auto oGetEditon = g_Hooks.AppPlatform_getGameEditionHook->GetFastcall<signed int, __int64>();

	static EditionFaker* mod = moduleMgr->getModule<EditionFaker>();
	if (mod == nullptr)
		mod = moduleMgr->getModule<EditionFaker>();
	else if (mod->isEnabled()) {
		return mod->getFakedEditon();
	}

	return oGetEditon(_this);
}

void Hooks::PleaseAutoComplete(__int64 a1, __int64 a2, TextHolder* text, int a4) {
	static auto oAutoComplete = g_Hooks.PleaseAutoCompleteHook->GetFastcall<void, __int64, __int64, TextHolder*, int>();
	char* tx = text->getText();
	if (tx != nullptr && text->getTextLength() >= 1 && tx[0] == '.') {
		std::string search = tx + 1;                                              // Dont include the '.'
		std::transform(search.begin(), search.end(), search.begin(), ::tolower);  // make the search text lowercase

		struct LilPlump {
			std::string cmdAlias;
			IMCCommand* command = 0;
			bool shouldReplace = true;  // Should replace the current text in the box (autocomplete)

			bool operator<(const LilPlump& o) const {
				return cmdAlias < o.cmdAlias;
			}
		};  // This is needed so the std::set sorts it alphabetically

		std::set<LilPlump> searchResults;

		std::vector<IMCCommand*>* commandList = cmdMgr->getCommandList();
		for (std::vector<IMCCommand*>::iterator it = commandList->begin(); it != commandList->end(); ++it) {  // Loop through commands
			IMCCommand* c = *it;
			auto* aliasList = c->getAliasList();
			for (std::vector<std::string>::iterator it = aliasList->begin(); it != aliasList->end(); ++it) {  // Loop through aliases
				std::string cmd = *it;
				LilPlump plump;

				for (size_t i = 0; i < search.size(); i++) {  // Loop through search string
					char car = search.at(i);
					if (car == ' ' && i == cmd.size()) {
						plump.shouldReplace = false;
						break;
					} else if (i >= cmd.size())
						goto outerContinue;

					if (car != cmd.at(i))  // and compare
						goto outerContinue;
				}
				// Not at outerContinue? Then we got a good result!
				{
					cmd.insert(0, 1, '.');  // Prepend the '.'

					plump.cmdAlias = cmd;
					plump.command = c;
					searchResults.emplace(plump);
				}

			outerContinue:
				continue;
			}
		}

		if (searchResults.size() > 0) {
			LilPlump firstResult = *searchResults.begin();

			size_t maxReplaceLength = firstResult.cmdAlias.size();
			if (searchResults.size() > 1) {
				for (auto it = searchResults.begin()++; it != searchResults.end(); it++) {
					auto alias = it->cmdAlias;
					maxReplaceLength = min(maxReplaceLength, alias.size());

					for (int i = 0; i < maxReplaceLength; i++) {
						if (alias[i] != firstResult.cmdAlias[i]) {
							maxReplaceLength = i;
							break;
						}
					}
				}
			} else
				maxReplaceLength = firstResult.cmdAlias.size();

			if (searchResults.size() > 1) {
				g_Data.getGuiData()->displayClientMessageF("==========");
				for (auto it = searchResults.begin(); it != searchResults.end(); ++it) {
					LilPlump plump = *it;
					g_Data.getGuiData()->displayClientMessageF("%s%s - %s%s", plump.cmdAlias.c_str(), GRAY, ITALIC, plump.command->getDescription());
				}
			} else {
				g_Data.getGuiData()->displayClientMessageF("==========");
				if (firstResult.command->getUsage()[0] == 0x0)
					g_Data.getGuiData()->displayClientMessageF("%s%s %s- %s", WHITE, firstResult.cmdAlias.c_str(), GRAY, firstResult.command->getDescription());
				else
					g_Data.getGuiData()->displayClientMessageF("%s%s %s %s- %s", WHITE, firstResult.cmdAlias.c_str(), firstResult.command->getUsage(), GRAY, firstResult.command->getDescription());
			}

			if (firstResult.shouldReplace) {
				if (search.size() == firstResult.cmdAlias.size() - 1 && searchResults.size() == 1) {
					maxReplaceLength++;
					firstResult.cmdAlias.append(" ");
				}
				
				text->setText(firstResult.cmdAlias.substr(0, maxReplaceLength));  // Set text
				// now sync with the UI thread
				using syncShit_t = void(__fastcall*)(__int64*, TextHolder*);
				static syncShit_t syncShit = nullptr;
				static __int64* winrt_ptr;
				if (syncShit == nullptr) {
					uintptr_t sigOffset = FindSignature("48 8B 0D ?? ?? ?? ?? 48 8B 01 49 8B D6 FF 90 ?? 04");  // The 04 at the end might get invalid in the future
					int offset = *reinterpret_cast<int*>(sigOffset + 3);
					winrt_ptr = *reinterpret_cast<__int64**>(sigOffset + offset + 7);
					int vtOffset = *reinterpret_cast<int*>(sigOffset + 15);
					syncShit = reinterpret_cast<syncShit_t>(*reinterpret_cast<__int64*>(*winrt_ptr + vtOffset));
				}

				syncShit(winrt_ptr, text);
			}
		}

		return;
	}
	oAutoComplete(a1, a2, text, a4);
}

void Hooks::LoopbackPacketSender_sendToServer(C_LoopbackPacketSender* a, C_Packet* packet) {
	static auto oFunc = g_Hooks.LoopbackPacketSender_sendToServerHook->GetFastcall<void, C_LoopbackPacketSender*, C_Packet*>();

	static Freecam* freecamMod = moduleMgr->getModule<Freecam>();
	static Blink* blinkMod = moduleMgr->getModule<Blink>();
	static NoPacket* noPacketMod = moduleMgr->getModule<NoPacket>();

	if (freecamMod == nullptr || blinkMod == nullptr || noPacketMod == nullptr) {
		freecamMod = moduleMgr->getModule<Freecam>();
		blinkMod = moduleMgr->getModule<Blink>();
		noPacketMod = moduleMgr->getModule<NoPacket>();
	} else if (noPacketMod->isEnabled()) {
		return;
	} else if (freecamMod->isEnabled() || blinkMod->isEnabled()) {
		if (packet->isInstanceOf<C_MovePlayerPacket>() || packet->isInstanceOf<PlayerAuthInputPacket>()) {
			if (blinkMod->isEnabled()) {
				if (packet->isInstanceOf<C_MovePlayerPacket>()) {
					C_MovePlayerPacket* meme = reinterpret_cast<C_MovePlayerPacket*>(packet);
					meme->onGround = true;                                                            //Don't take Fall Damages when turned off
					blinkMod->getMovePlayerPacketHolder()->push_back(new C_MovePlayerPacket(*meme));  // Saving the packets
				} else {
					blinkMod->getPlayerAuthInputPacketHolder()->push_back(new PlayerAuthInputPacket(*reinterpret_cast<PlayerAuthInputPacket*>(packet)));
				}
			}
			return;  // Dont call LoopbackPacketSender_sendToServer
		}
	} else if (!blinkMod->isEnabled()) {
		if (blinkMod->getMovePlayerPacketHolder()->size() > 0) {
			for (auto it : *blinkMod->getMovePlayerPacketHolder()) {
				oFunc(a, (it));
				delete it;
				it = nullptr;
			}
			blinkMod->getMovePlayerPacketHolder()->clear();
			return;
		}
		if (blinkMod->getPlayerAuthInputPacketHolder()->size() > 0) {
			for (auto it : *blinkMod->getPlayerAuthInputPacketHolder()) {
				oFunc(a, (it));
				delete it;
				it = nullptr;
			}
			blinkMod->getPlayerAuthInputPacketHolder()->clear();
			return;
		}
	}
	moduleMgr->onSendPacket(packet);
	oFunc(a, packet);
}

float Hooks::LevelRendererPlayer_getFov(__int64 _this, float a2, bool a3) {
	static auto oGetFov = g_Hooks.LevelRendererPlayer_getFovHook->GetFastcall<float, __int64, float, bool>();
	static void* renderItemInHand = reinterpret_cast<void*>(FindSignature("0F 28 F0 F3 44 0F 10 3D ?? ?? ?? ?? F3 41 0F 59 F7"));
	static void* setupCamera = reinterpret_cast<void*>(FindSignature("44 0F 28 D8 F3 44 0F 59 1D ?? ?? ?? ?? 41 0F B6 4E ??"));

	if (_ReturnAddress() == renderItemInHand) {
		return oGetFov(_this, a2, a3);
	}
	if (_ReturnAddress() == setupCamera) {
		return oGetFov(_this, a2, a3);
	}
#ifdef _DEBUG
	logF("LevelRendererPlayer_getFov Return Addres: %llX", _ReturnAddress());
	__debugbreak();  // IF we reach here, a sig is broken
#endif
	return oGetFov(_this, a2, a3);
}

void Hooks::MultiLevelPlayer_tick(C_EntityList* _this) {
	static auto oTick = g_Hooks.MultiLevelPlayer_tickHook->GetFastcall<void, C_EntityList*>();
	oTick(_this);
	GameData::EntityList_tick(_this);
}

void Hooks::GameMode_startDestroyBlock(C_GameMode* _this, vec3_ti* a2, uint8_t face, void* a4, void* a5) {
	static auto oFunc = g_Hooks.GameMode_startDestroyBlockHook->GetFastcall<void, C_GameMode*, vec3_ti*, uint8_t, void*, void*>();

	static Nuker* nukerModule = moduleMgr->getModule<Nuker>();
	static IModule* instaBreakModule = moduleMgr->getModule<InstaBreak>();
	if (nukerModule == nullptr || instaBreakModule == nullptr) {
		nukerModule = moduleMgr->getModule<Nuker>();
		instaBreakModule = moduleMgr->getModule<InstaBreak>();
	} else {
		if (nukerModule->isEnabled()) {
			vec3_ti tempPos;

			const int range = nukerModule->getNukerRadius();
			const bool isVeinMiner = nukerModule->isVeinMiner();

			C_BlockSource* region = g_Data.getLocalPlayer()->region;
			int selectedBlockId = (*(region->getBlock(*a2)->blockLegacy))->blockId;
			uint8_t selectedBlockData = region->getBlock(*a2)->data;

			for (int x = -range; x < range; x++) {
				for (int y = -range; y < range; y++) {
					for (int z = -range; z < range; z++) {
						tempPos.x = a2->x + x;
						tempPos.y = a2->y + y;
						tempPos.z = a2->z + z;
						if (tempPos.y > 0) {
							C_Block* blok = region->getBlock(tempPos);
							uint8_t data = blok->data;
							int id = (*(blok->blockLegacy))->blockId;
							if (id != 0 && (!isVeinMiner || (id == selectedBlockId && data == selectedBlockData)))
								_this->destroyBlock(&tempPos, face);
						}
					}
				}
			}
			return;
		}
		if (instaBreakModule->isEnabled()) {
			_this->destroyBlock(a2, face);
			return;
		}
	}

	oFunc(_this, a2, face, a4, a5);
}

void Hooks::HIDController_keyMouse(C_HIDController* _this, void* a2, void* a3) {
	static auto oFunc = g_Hooks.HIDController_keyMouseHook->GetFastcall<void, C_HIDController*, void*, void*>();
	GameData::setHIDController(_this);
	isTicked = true;
	oFunc(_this, a2, a3);
	return;
}

int Hooks::BlockLegacy_getRenderLayer(C_BlockLegacy* a1) {
	static auto oFunc = g_Hooks.BlockLegacy_getRenderLayerHook->GetFastcall<int, C_BlockLegacy*>();

	static IModule* XrayModule = moduleMgr->getModule<Xray>();
	if (XrayModule == nullptr)
		XrayModule = moduleMgr->getModule<Xray>();
	else if (XrayModule->isEnabled()) {
		char* text = a1->name.getText();
		if (strstr(text, "ore") == NULL)
			if (strcmp(text, "lava") != NULL)
				if (strcmp(text, "water") != NULL)
					return 10;
	}
	return oFunc(a1);
}

__int8* Hooks::BlockLegacy_getLightEmission(C_BlockLegacy* a1, __int8* a2) {
	static auto oFunc = g_Hooks.BlockLegacy_getLightEmissionHook->GetFastcall<__int8*, C_BlockLegacy*, __int8*>();

	static IModule* XrayModule = moduleMgr->getModule<Xray>();
	if (XrayModule == nullptr)
		XrayModule = moduleMgr->getModule<Xray>();
	else if (XrayModule->isEnabled()) {
		*a2 = 15;
		return a2;
	}
	return oFunc(a1, a2);
}

__int64 Hooks::LevelRenderer_renderLevel(__int64 _this, __int64 a2, __int64 a3) {
	static auto oFunc = g_Hooks.LevelRenderer_renderLevelHook->GetFastcall<__int64, __int64, __int64, __int64>();

	using reloadShit_t = void(__fastcall*)(__int64);
	static reloadShit_t reloadChunk = reinterpret_cast<reloadShit_t>(FindSignature("48 8B C4 56 57 41 54 41 56 41 57 48 83 EC ?? 48 C7 40 ?? FE FF FF FF 48 89 58 ?? 48 89 68 ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 44 24 ?? 48 8B F9 4C"));

	static IModule* xray = moduleMgr->getModule<Xray>();
	if (xray == nullptr) {
		xray = moduleMgr->getModule<Xray>();
	} else {
		static bool lastState = false;
		if (lastState != xray->isEnabled()) {
			lastState = xray->isEnabled();
			unsigned long long* v5;  // rdi
			unsigned long long* i;   // rbx

			v5 = *(unsigned long long**)(_this + 32);
			for (i = (unsigned long long*)*v5; i != v5; i = (unsigned long long*)*i)
				reloadChunk(i[3]);
		}
	}

	return oFunc(_this, a2, a3);
}

void Hooks::ClickFunc(__int64 a1, char mouseButton, bool isDown, __int16 mouseX, __int16 mouseY, __int16 a6, __int16 a7, char a8) {
	static auto oFunc = g_Hooks.ClickFuncHook->GetFastcall<void, __int64, char, bool, __int16, __int16, __int16, __int16, char>();
	static IModule* clickGuiModule = moduleMgr->getModule<ClickGuiMod>();

	if (clickGuiModule == nullptr)
		clickGuiModule = moduleMgr->getModule<ClickGuiMod>();
	else if (clickGuiModule->isEnabled()) {
		if (mouseButton != 0)  // Mouse click event
			return;
	}
	oFunc(a1, mouseButton, isDown, mouseX, mouseY, a6, a7, a8);
}

__int64 Hooks::MoveInputHandler_tick(C_MoveInputHandler* a1, C_Entity* a2) {
	static auto oTick = g_Hooks.MoveInputHandler_tickHook->GetFastcall<__int64, C_MoveInputHandler*, C_Entity*>();
	return oTick(a1, a2);
}

__int64 Hooks::ChestScreenController_tick(C_ChestScreenController* a1) {
	static auto oFunc = g_Hooks.ChestScreenController_tickHook->GetFastcall<__int64, C_ChestScreenController*>();

	static ChestStealer* ChestStealerMod = moduleMgr->getModule<ChestStealer>();
	if (ChestStealerMod == nullptr)
		ChestStealerMod = moduleMgr->getModule<ChestStealer>();
	else {
		ChestStealerMod->chestScreenController = a1;
	}

	return oFunc(a1);
}

__int64 Hooks::GetGamma(__int64 a1) {
	static auto oFunc = g_Hooks.GetGammaHook->GetFastcall<__int64, __int64>();

	static FullBright* fullBrightModule = moduleMgr->getModule<FullBright>();
	if (fullBrightModule == nullptr)
		fullBrightModule = moduleMgr->getModule<FullBright>();

	static __int64 v7 = 0;
	if (v7 == 0) {
		__int64 v6 = oFunc(a1);  // Calls to ClientInstance, returns options ptr
		if (*(bool*)(v6 + 0x1001))
			v7 = *(__int64*)(v6 + 0x7D8);
		else
			v7 = *(__int64*)(v6 + 0x138);
	} else {
		if (fullBrightModule != nullptr) {
			fullBrightModule->gammaPtr = reinterpret_cast<float*>(v7 + 0xF0);
		}
	}

	return oFunc(a1);
}

bool Hooks::Actor_isInWater(C_Entity* _this) {
	static auto oFunc = g_Hooks.Actor_isInWaterHook->GetFastcall<bool, C_Entity*>();

	if (g_Data.getLocalPlayer() != _this)
		return oFunc(_this);

	static AirSwim* AirSwimModule = moduleMgr->getModule<AirSwim>();
	if (AirSwimModule == nullptr)
		AirSwimModule = moduleMgr->getModule<AirSwim>();
	else if (AirSwimModule->isEnabled())
		return true;

	return oFunc(_this);
}

void Hooks::JumpPower(C_Entity* a1, float a2) {
	static auto oFunc = g_Hooks.JumpPowerHook->GetFastcall<void, C_Entity*, float>();
	static HighJump* HighJumpMod = moduleMgr->getModule<HighJump>();
	if (HighJumpMod == nullptr)
		HighJumpMod = moduleMgr->getModule<HighJump>();
	else if (HighJumpMod->isEnabled() && g_Data.getLocalPlayer() == a1) {
		a1->velocity.y = HighJumpMod->jumpPower;
		return;
	}
	oFunc(a1, a2);
}

__int64 Hooks::MinecraftGame_onAppSuspended(__int64 _this) {
	static auto oFunc = g_Hooks.MinecraftGame_onAppSuspendedHook->GetFastcall<__int64, __int64>();
	configMgr->saveConfig();
	return oFunc(_this);
}

void Hooks::Actor_ladderUp(C_Entity* _this) {
	static auto oFunc = g_Hooks.Actor_ladderUpHook->GetFastcall<void, C_Entity*>();

	static IModule* FastLadderModule = moduleMgr->getModule<FastLadder>();
	if (FastLadderModule == nullptr)
		FastLadderModule = moduleMgr->getModule<FastLadder>();
	else if (FastLadderModule->isEnabled() && g_Data.getLocalPlayer() == _this) {
		_this->velocity.y = 0.6f;
		return;
	}
	return oFunc(_this);
}

void Hooks::Actor_startSwimming(C_Entity* _this) {
	static auto oFunc = g_Hooks.Actor_startSwimmingHook->GetFastcall<void, C_Entity*>();

	static IModule* JesusModule = moduleMgr->getModule<Jesus>();
	if (JesusModule == nullptr)
		JesusModule = moduleMgr->getModule<Jesus>();
	else if (JesusModule->isEnabled() && g_Data.getLocalPlayer() == _this) {
		return;
	}
	oFunc(_this);
}

void Hooks::RakNetInstance_tick(C_RakNetInstance* _this, __int64 a2, __int64 a3) {
	static auto oTick = g_Hooks.RakNetInstance_tickHook->GetFastcall<void, C_RakNetInstance*, __int64, __int64>();
	GameData::setRakNetInstance(_this);
	oTick(_this, a2, a3);
}

float Hooks::GameMode_getPickRange(C_GameMode* _this, __int64 a2, char a3) {
	static auto oFunc = g_Hooks.GameMode_getPickRangeHook->GetFastcall<float, C_GameMode*, __int64, char>();

	static ForceOpenCommandBlock* forceOpenCmdBlock = moduleMgr->getModule<ForceOpenCommandBlock>();
	if (forceOpenCmdBlock->isEnabled() && forceOpenCmdBlock->isInCommandBlock) return forceOpenCmdBlock->distance;

	static InfiniteBlockReach* InfiniteBlockReachModule = moduleMgr->getModule<InfiniteBlockReach>();
	if (InfiniteBlockReachModule->isEnabled()) return InfiniteBlockReachModule->getBlockReach();

	static ClickTP* clickTP = moduleMgr->getModule<ClickTP>();
	if (clickTP->isEnabled()) return 255;

	return oFunc(_this, a2, a3);
}

struct face {
	struct facePart {
		int vertIndex = -1, normalIndex = -1, uvIndex = -1;
	} indices[8];
	int facesPresent = 4;
};

void to_json(json& j, const face& f) {
	std::vector<std::array<int, 3>> partArray;

	for (int i = 0; i < f.facesPresent; i++) {
		auto ind = f.indices[i];
		partArray.push_back({ind.vertIndex, ind.normalIndex, ind.uvIndex});
	}
	j = partArray;
}

__int64 Hooks::ConnectionRequest_create(__int64 _this, __int64 privateKeyManager, void* a3, TextHolder* selfSignedId, TextHolder* serverAddress, __int64 clientRandomId, TextHolder* skinId, SkinData* skinData, __int64 capeData, __int64 animatedImageDataArr, TextHolder* skinResourcePatch, TextHolder* skinGeometryData, TextHolder* skinAnimationData, bool isPremiumSkin, bool isPersonaSkin, TextHolder* deviceId, int inputMode, int uiProfile, int guiScale, TextHolder* languageCode, bool sendEduModeParams, TextHolder* tenantId, __int64 unused, TextHolder* platformUserId, TextHolder* thirdPartyName, bool thirdPartyNameOnly, TextHolder* platformOnlineId, TextHolder* platformOfflineId, bool isCapeOnClassicSkin, TextHolder* capeId) {
	static auto oFunc = g_Hooks.ConnectionRequest_createHook->GetFastcall<__int64, __int64, __int64, void*, TextHolder*, TextHolder*, __int64, TextHolder*, SkinData*, __int64, __int64, TextHolder*, TextHolder*, TextHolder*, bool, bool, TextHolder*, int, int, int, TextHolder*, bool, TextHolder*, __int64, TextHolder*, TextHolder*, bool, TextHolder*, TextHolder*, bool, TextHolder*>();

	if (g_Data.allowWIPFeatures()) {
		logF("Connection Request: InputMode: %i UiProfile: %i GuiScale: %i", inputMode, uiProfile, guiScale);
		logF("Geometry size: %d", skinGeometryData->getTextLength());

		//Logger::WriteBigLogFileF(skinGeometryData->getTextLength() + 20, "Geometry: %s", skinGeometryData->getText());
		auto hResourceGeometry = FindResourceA(g_Data.getDllModule(), MAKEINTRESOURCEA(IDR_TEXT1), "TEXT");
		auto hMemoryGeometry = LoadResource(g_Data.getDllModule(), hResourceGeometry);

		auto sizeGeometry = SizeofResource(g_Data.getDllModule(), hResourceGeometry);
		auto ptrGeometry = LockResource(hMemoryGeometry);

		auto hResourceSteve = FindResourceA(g_Data.getDllModule(), MAKEINTRESOURCEA(IDR_STEVE), (char*)RT_RCDATA);
		auto hMemorySteve = LoadResource(g_Data.getDllModule(), hResourceSteve);

		auto sizeSteve = SizeofResource(g_Data.getDllModule(), hResourceSteve);
		auto ptrSteve = LockResource(hMemorySteve);

		//std::unique_ptr<TextHolder> newGeometryData(new TextHolder(ptrGeometry, sizeGeometry));
		TextHolder* newGeometryData;
		{
			auto hResourceObj = FindResourceA(g_Data.getDllModule(), MAKEINTRESOURCEA(IDR_OBJ), "TEXT");
			auto hMemoryObj = LoadResource(g_Data.getDllModule(), hResourceObj);

			auto sizeObj = SizeofResource(g_Data.getDllModule(), hResourceObj);
			auto ptrObj = LockResource(hMemoryObj);

			char* str = new char[sizeObj + 1];
			memset(str, 0, sizeObj + 1);
			memcpy(str, ptrObj, sizeObj);

			std::istringstream f(reinterpret_cast<char*>(str));
			std::string line;

			std::vector<std::array<float, 3>> vertices;
			std::vector<std::array<float, 3>> normals;
			std::vector<std::array<float, 2>> uvs;
			std::vector<face> faces;

			while (std::getline(f, line)) {
				// Remove trailing whitespace
				// left out for performance reasons
				/*{
				size_t startpos = line.find_first_not_of(" \t");
				if (std::string::npos != startpos) {
					line = line.substr(startpos);
				}
			}*/

				if (line[0] != 'f' && line[0] != 'v')
					continue;

				auto firstWhiteSpace = line.find(" ");
				if (firstWhiteSpace == std::string::npos)  // comment
					continue;

				std::vector<std::string> args;
				size_t pos = firstWhiteSpace, initialPos = 0;
				while (pos != std::string::npos) {
					args.push_back(line.substr(initialPos, pos - initialPos));
					initialPos = pos + 1;

					pos = line.find(" ", initialPos);
				}
				args.push_back(line.substr(initialPos, min(pos, line.size()) - initialPos + 1));

				auto cmd = args[0].c_str();

				if (strcmp(cmd, "vt") == 0) {  // uv
					if (args.size() != 3) {
						logF("Faulty uv, 2 args expected: %s", line.c_str());
						continue;
					}
					uvs.push_back({std::stof(args[1]), std::stof(args[2])});
				} else if (strcmp(cmd, "v") == 0) {  // vertex
					if (args.size() != 4) {
						logF("Faulty vertex, 3 args expected: %s", line.c_str());
						continue;
					}
					vertices.push_back({-std::stof(args[1]), std::stof(args[2]), std::stof(args[3])});
				} else if (strcmp(cmd, "f") == 0) {  // face
					if (args.size() != 5) {
						logF("Faulty face, only quads allowed: %i", args.size() - 1);
						continue;
					}

					face face;
					face.facesPresent = (int)args.size() - 1;
					for (int i = 1; i < args.size(); i++) {
						face::facePart part;
						auto arg = args[i];
						size_t doubleOff = arg.find("//");
						if (doubleOff != std::string::npos) {  //  vertex // normal
							int vertex = std::stoi(arg.substr(0, doubleOff)) - 1;
							int normal = std::stoi(arg.substr(doubleOff + 2)) - 1;
							assert(vertex >= 0);
							assert(normal >= 0);  // negative indices mean relative from last, too lazy for that tbh
							part.vertIndex = vertex;
							part.normalIndex = normal;
						} else {
							size_t firstOff = arg.find("/");
							if (firstOff != std::string::npos) {
								int vertex = std::stoi(arg.substr(0, firstOff)) - 1;
								assert(vertex >= 0);
								part.vertIndex = vertex;

								auto afterSingle = arg.substr(firstOff + 1);
								size_t secondOff = afterSingle.find("/");
								if (secondOff != std::string::npos) {  // vertex / uv / normal
									int uv = std::stoi(afterSingle.substr(0, secondOff)) - 1;
									int normal = std::stoi(afterSingle.substr(secondOff + 1)) - 1;
									assert(uv >= 0);
									assert(normal >= 0);

									part.uvIndex = uv;
									part.normalIndex = normal;
								} else {  // vertex / uv
									assert(false);
								}
							} else
								assert(false);  // vertex only, don't think minecraft even supports this
						}

						face.indices[i - 1] = part;
					}
					faces.push_back(face);
				} else if (strcmp(cmd, "vn") == 0) {  // normal
					if (args.size() != 4) {
						logF("Faulty normal, 3 args expected: %s", line.c_str());
						continue;
					}
					normals.push_back({std::stof(args[1]), std::stof(args[2]), std::stof(args[3])});
				} else
					logF("Unknown command: %s", cmd);
			}

			logF("Modding our roblox geometry");
			// Swap coolroblox's body with new mesh
			{
				auto oldGeo = std::string(reinterpret_cast<char*>(ptrGeometry));
				json geoMod = json::parse(oldGeo);  // If this crashes, coolroblox json is invalid
				auto geoParts = &geoMod.at("minecraft:geometry");

				for (auto it = geoParts->begin(); it != geoParts->end(); it++) {
					auto part = it.value();
					std::string identifier = part["description"]["identifier"].get<std::string>();
					if (identifier.find("animated") != std::string::npos)
						continue;

					auto bones = &part.at("bones");
					for (auto boneIt = bones->begin(); boneIt != bones->end(); boneIt++) {
						auto bone = *boneIt;
						if (bone["cubes"].is_array() || bone["poly_mesh"].is_object())
							continue;
						std::string name = bone["name"];
						std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
						if (name == "body") {
							logF("Found body");
							json polyMesh;
							polyMesh["normalized_uvs"] = true;  // blender does that apparently
							polyMesh["normals"] = normals;
							polyMesh["positions"] = vertices;
							polyMesh["uvs"] = uvs;
							polyMesh["polys"] = faces;
							//auto dump = polyMesh.dump();
							//Logger::WriteBigLogFileF(dump.size() + 20, "%s", dump.c_str());

							boneIt->emplace("poly_mesh", polyMesh);
						}
					}

					it->swap(part);
				}
				newGeometryData = new TextHolder(geoMod.dump());
			}
		}

		SkinData* newSkinData = new SkinData();
		newSkinData->SkinWidth = 128;
		newSkinData->SkinHeight = 128;
		newSkinData->skinData = ptrSteve;
		newSkinData->skinSize = sizeSteve;
		//Logger::WriteBigLogFileF(newGeometryData->getTextLength() + 20, "Geometry: %s", newGeometryData->getText());
		TextHolder* newSkinResourcePatch = new TextHolder(Utils::base64_decode("ewogICAiZ2VvbWV0cnkiIDogewogICAgICAiYW5pbWF0ZWRfZmFjZSIgOiAiZ2VvbWV0cnkuYW5pbWF0ZWRfZmFjZV9wZXJzb25hXzRjZGJiZmFjYTI0YTk2OGVfMF8wIiwKICAgICAgImRlZmF1bHQiIDogImdlb21ldHJ5LnBlcnNvbmFfNGNkYmJmYWNhMjRhOTY4ZV8wXzAiCiAgIH0KfQo="));

		__int64 res = oFunc(_this, privateKeyManager, a3, selfSignedId, serverAddress, clientRandomId, skinId, newSkinData, capeData, animatedImageDataArr, newSkinResourcePatch, newGeometryData, skinAnimationData, isPremiumSkin, isPersonaSkin, deviceId, inputMode, uiProfile, guiScale, languageCode, sendEduModeParams, tenantId, unused, platformUserId, thirdPartyName, thirdPartyNameOnly, platformOnlineId, platformOfflineId, isCapeOnClassicSkin, capeId);

		if (hMemoryGeometry)
			FreeResource(hMemoryGeometry);
		if (hMemorySteve)
			FreeResource(hMemorySteve);

		delete newGeometryData;
		delete newSkinData;
		delete newSkinResourcePatch;
		return res;
	} else {
		__int64 res = oFunc(_this, privateKeyManager, a3, selfSignedId, serverAddress, clientRandomId, skinId, skinData, capeData, animatedImageDataArr, skinResourcePatch, skinGeometryData, skinAnimationData, isPremiumSkin, isPersonaSkin, deviceId, inputMode, uiProfile, guiScale, languageCode, sendEduModeParams, tenantId, unused, platformUserId, thirdPartyName, thirdPartyNameOnly, platformOnlineId, platformOfflineId, isCapeOnClassicSkin, capeId);
		return res;
	}
}

void Hooks::InventoryTransactionManager_addAction(C_InventoryTransactionManager* a1, C_InventoryAction* a2) {
	static auto Func = g_Hooks.InventoryTransactionManager_addActionHook->GetFastcall<void, C_InventoryTransactionManager*, C_InventoryAction*>();
	Func(a1, a2);
}
