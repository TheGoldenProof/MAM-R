#include "Config.h"
#include "Globe.h"
#include "Graphics\Camera.h"
#include "Graphics\Drawable\Drawables.h"
#include "imgui\imgui.h"
#include "MIDI\RawMidi.h"
#include "Scene\TestScene.h"
#include "Util\MyMath.h"
#include "Windows\Keyboard.h"
#include "Windows\Mouse.h"
#include <cmath>
#include <format>

TestScene::TestScene(Globe& gb) : MidiScene(gb, "testScene0") {
	if (auto opCam = gb.Cams().GetCamera("Camera0"); opCam) {
		opCam.value().get().SetHomePos({ 0, 0, -960.0f });
		opCam.value().get().Reset();
	}

	CBD::RawLayout layout;
	layout.Add(CBD::Float, "planeX");
	pPlayPlanePBuf = std::make_unique<PixelConstantBufferCaching>(gb.Gfx(), CBD::Buffer(std::move(layout)), 0);

	trackColors = {
		{1.00f, 0.00f, 0.00f, 1.0f},
		{1.00f, 0.25f, 0.00f, 1.0f},
		{1.00f, 0.50f, 0.00f, 1.0f},
		{1.00f, 0.75f, 0.00f, 1.0f},
		{1.00f, 1.00f, 0.00f, 1.0f},
		{0.75f, 1.00f, 0.00f, 1.0f},
		{0.50f, 1.00f, 0.00f, 1.0f},
		{0.25f, 1.00f, 0.00f, 1.0f},
		{0.00f, 1.00f, 0.00f, 1.0f},
		{0.00f, 1.00f, 0.25f, 1.0f},
		{0.00f, 1.00f, 0.50f, 1.0f},
		{0.00f, 1.00f, 0.75f, 1.0f},
		{0.00f, 1.00f, 1.00f, 1.0f},
		{0.00f, 0.75f, 1.00f, 1.0f},
		{0.00f, 0.50f, 1.00f, 1.0f},
		{0.00f, 0.25f, 1.00f, 1.0f},
		{0.00f, 0.00f, 1.00f, 1.0f},
		{0.25f, 0.00f, 1.00f, 1.0f},
		{0.50f, 0.00f, 1.00f, 1.0f},
		{0.75f, 0.00f, 1.00f, 1.0f},
		{1.00f, 0.00f, 1.00f, 1.0f},
		{1.00f, 0.00f, 0.75f, 1.0f},
		{1.00f, 0.00f, 0.50f, 1.0f},
		{1.00f, 0.00f, 0.25f, 1.0f},
	};
}

void TestScene::Draw(Globe& gb) {
	pPlayPlanePBuf->GetBuffer()["planeX"] = playX;
	pPlayPlanePBuf->Bind(gb.Gfx());

	if (zSpacing != zSpacingPrev) {
		for (u32 i = 0; i < trackVisuals.size(); i++) {
			auto& trackVis = *trackVisuals[i];
			auto trackPos = trackVis.GetPos();
			trackPos.z = i * zSpacing + 1;
			trackVis.SetPos(trackPos);
		}
		zSpacingPrev = zSpacing;
	}

	for (const auto& track : trackVisuals) {
		track->Draw(gb.FrameCtrl());
	}

	MidiScene::Draw(gb);
	UpdateInputs(gb);
}

void TestScene::InitVisuals(Globe& gb) {
	usize maxIndexCount = 0;
	usize skippedTracks = 0;
	for (usize fi = 0; fi < midi.GetTracks().size(); fi++) {
		const auto& trR = trackReorder[fi];
		if (!trR.second) { skippedTracks++; continue; }
		const usize i = fi - skippedTracks;
		const MIDI::Track& track = midi.GetTracks()[trR.first];
		auto trackColor = trackColors[i % trackColors.size()];
		const usize noteCount = std::min(0x7fffffff/24ull, track.notes.size());
		if (noteCount == 0) continue;
		QuadBatchColored::BatchDesc desc{
			.uniqueName = std::format("ts0qbc{:d}", i),
			.maxQuadCount = noteCount * 6,
			.layer = 0,
			.vertexShader = "DarkenPlayed_VS.cso",
			.pixelShader = "DarkenPlayed_PS.cso",
		};
		auto pQbatch = std::make_unique<QuadBatchColored>(gb.Gfx(), std::move(desc));
		for (usize j = 0; j < noteCount; j++) {
			const MIDI::Note& note = track.notes[j];
			const f32 noteLength = note.lengthTicks * lengthScale;
			auto velScale = [&note, this](f32 val) {return val * (note.velocity / 128.0f * velocityFactor + (1.0f - velocityFactor)); };

			const f32 noteX = note.startTick * lengthScale + noteLength * 0.5f + noteHSpacing / 2;
			const f32 noteYBase = (note.pitch - 64) * (noteHeight + noteVSpacing);

			QuadBatchColored::QuadDesc edesc{
				.position = {noteX, 0, 0},
				.rotation = {0, 0, 0},
				.size = {noteLength - noteHSpacing, noteHeight},
				.singleColor = true,
				.colors = {{{velScale(trackColor[0]), velScale(trackColor[1]), velScale(trackColor[2]), 1.0f}, {}, {}, {}}}
			};

			QuadBatchColored::QuadDesc endD = edesc;

			constexpr f32 inradii[] = { 0.0f, 0.0f, 0.0f, 0.2886751346f, 0.5f, 0.6881909602f, 0.8660254038f };
			const f32 radius = inradii[noteType];
			for (i32 k = 0; k < noteType; k++) {
				const f32 theta = Math::to_rad(360.0f / noteType * k - 90.0f) + noteRotation;
				edesc.rotation.x = theta;
				edesc.position.y = noteYBase + radius * sin(theta) * noteHeight;
				edesc.position.z = -radius * cos(theta) * noteHeight;
				pQbatch->AddOneQuad(edesc);
			}

			endD.position.x = note.startTick * lengthScale + noteHSpacing;
			endD.rotation.z = noteRotation;
			endD.rotation.y = Math::HALF_PI;

			if (noteType == 4) {
				// Left
				endD.position.y = noteYBase;
				endD.size.x = noteHeight;
				pQbatch->AddOneQuad(endD);
				// Right
				endD.position.x = note.startTick * lengthScale + noteLength;
				endD.rotation.y = -Math::HALF_PI;
				pQbatch->AddOneQuad(endD);

			} else if (noteType == 3) {
				constexpr f32 root3on2 = 1.7320508075688772935274463415059f / 2.0f;
				constexpr f32 root3on12 = 1.7320508075688772935274463415059f / 12.0f;
				// Left
				endD.size.x = noteHeight;
				endD.size.y = noteHeight * root3on2;
				endD.position.y = noteYBase + root3on12 * noteHeight * cos(noteRotation);
				endD.position.z = root3on12 * noteHeight * sin(noteRotation);
				pQbatch->AddOneTriangle(endD);
				// Right
				endD.position.x = note.startTick * lengthScale + noteLength;
				endD.rotation.y = -Math::HALF_PI;
				pQbatch->AddOneTriangle(endD);
			}

		}

		pQbatch->SetPos({ 0, 0, i * zSpacing + 1 });

		pQbatch->FlushChanges(gb.Gfx());
		const usize indexCount = pQbatch->GetIndexCount();
		maxIndexCount = std::max(maxIndexCount, indexCount);
		trackVisuals.emplace_back(std::move(pQbatch));
	}

	DEBUG_LOG(std::format("Max index count: {:d}\n", maxIndexCount).c_str());
}

void TestScene::ClearVisuals(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
	trackVisuals.clear();
}

void TestScene::MovePlay(Globe& gb, f32 dx) {
	dx *= lengthScale;
	//auto plPos = pPlayLine->GetPos();
	//plPos.x += dx;
	//pPlayLine->SetPos(plPos);
	playX += dx;

	if (auto opCam = gb.Cams().GetActiveCamera(); opCam) {
		Camera& cam = opCam.value().get();
		auto camPos = cam.GetPos();
		camPos.x += dx;
		cam.SetPos(camPos);
	}
}

void TestScene::DrawGUI(Globe& gb) {
	MidiScene::DrawGUI(gb);
	if (ImGui::Begin("Keybinds")) {
		ImGui::Text("W/A/S/D/E/Q: Camera Position");
		ImGui::Text("Arrow Keys: Camera Rotation");
	}
	ImGui::End();

	if (ImGui::Begin("MIDI Controls")) {
		ImGui::SetNextItemWidth(128.0f);
		ImGui::InputFloat("Track Depth Spacing", &zSpacing, 0.5f, 5.0f);
		
		if (ImGui::TreeNode("Note Options")) {
			ImGui::PushItemWidth(128.0f);
			ImGui::SliderFloat("Velocity Intensity", &velocityFactor, 0.0f, 1.0f);
			if (ImGui::SliderInt("Note Sides", &noteType, 3, 4)) reloadMidi = true;
			ImGui::SliderAngle("Note Rotation", &noteRotation, -180.0f, 180.0f);
			ImGui::InputFloat("Height", &noteHeight);
			ImGui::InputFloat("Vertical Spacing", &noteVSpacing);
			ImGui::InputFloat("Length Factor", &lengthScale);
			ImGui::InputFloat("Horizontal Spacing", &noteHSpacing);
			ImGui::PopItemWidth();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Track Colors")) {
			const f32 buttonDim = ImGui::GetFrameHeight();
			const ImVec2 buttonSize(buttonDim, buttonDim);
			for (u32 i = 0; i < trackColors.size(); i++) {
				if (ImGui::Button(std::format("-##del{:d}", i).c_str(), buttonSize)) {
					trackColors.erase(trackColors.begin() + i);
					i--;
					reloadMidi = true;
				}
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				if (ImGui::Button(std::format("+##trRadd{:d}", i).c_str(), buttonSize)) {
					trackColors.insert(trackColors.begin() + i, trackColors[i]);
					i++;
					reloadMidi = true;
				}
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				if (ImGui::Button(std::format("^##trRup{:d}", i).c_str(), buttonSize) && i != 0) {
					trackColors[i].swap(trackColors[i-1]);
					reloadMidi = true;
				}
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				if (ImGui::Button(std::format("v##trRdown{:d}", i).c_str(), buttonSize) && i != trackColors.size()-1) {
					trackColors[i].swap(trackColors[i+1]);
					reloadMidi = true;
				}
				ImGui::SameLine();
				usize trackIndex = i < midi.GetTracks().size() ? trackReorder[i].first : i;
				std::string trackName = trackIndex < midi.GetTracks().size() ? midi.GetTracks()[trackIndex].name : "";
				std::string trackDisplayName = trackName.empty() ? std::format("Track {:d}", trackIndex + 1) : trackName;
				ImGui::ColorEdit3(std::format("{:s}##track{:d}", trackDisplayName, trackIndex).c_str(), trackColors[i].data(), ImGuiColorEditFlags_NoInputs);
			}

			if (ImGui::Button("+##addEnd", buttonSize)) {
				trackColors.push_back(trackColors.front());
				reloadMidi = true;
			}

			ImGui::TreePop();
		}

	}
	ImGui::End();
}

void TestScene::WriteConfig(Globe& gb) {
	MidiScene::WriteConfig(gb);

	Config& cfg = gb.Cfg();
	cfg.Set("ts.zSpacing", zSpacing);
	cfg.Set("ts.velFactor", velocityFactor);
	cfg.Set("ts.noteType", noteType);
	cfg.Set("ts.noteRotation", noteRotation);
	cfg.Set("ts.noteHeight", noteHeight);
	cfg.Set("ts.noteVSpacing", noteVSpacing);
	cfg.Set("ts.noteHSpacing", noteHSpacing);
	cfg.Set("ts.trackColors", trackColors.data(), trackColors.size());

	if (auto opCam = gb.Cams().GetCamera("Camera0"); opCam) {
		Camera& cam = opCam.value().get();
		cfg.Set("ts.cam.homePos", cam.GetHomePos());
		cfg.Set("ts.cam.homeRot", cam.GetHomeRotation());
	}
}

void TestScene::ReadConfig(Globe& gb) {
	MidiScene::ReadConfig(gb);

	Config& cfg = gb.Cfg();
	cfg.Get("ts.zSpacing", &zSpacing);
	cfg.Get("ts.velFactor", &velocityFactor);
	cfg.Get("ts.noteType", &noteType);
	cfg.Get("ts.noteRotation", &noteRotation);
	cfg.Get("ts.noteHeight", &noteHeight);
	cfg.Get("ts.noteVSpacing", &noteVSpacing);
	cfg.Get("ts.noteHSpacing", &noteHSpacing);
	if (auto trackColorCount = cfg()["ts.trackColors"].size(); trackColorCount != 0) {
		trackColors.resize(trackColorCount / (sizeof(f32) * 4));
		cfg.Get("ts.trackColors", trackColors.data());
	}

	if (auto opCam = gb.Cams().GetCamera("Camera0"); opCam) {
		Camera& cam = opCam.value().get();
		DirectX::XMFLOAT3 homePos(0, 0, 0);
		DirectX::XMFLOAT3 homeRot(0, 0, 0);
		cfg.Get("ts.cam.homePos", &homePos);
		cfg.Get("ts.cam.homeRot", &homeRot);
		cam.SetHomePos(homePos);
		cam.SetHomeRotation(homeRot);
		cam.Reset();
	}
}

void TestScene::UpdateInputs(Globe& gb) {
	const f32 speed = 300.0f * gb.TargetFrameDt();
	DirectX::XMFLOAT3 ds = { 0, 0, 0 };
	const f32 rotSpeed = Math::to_rad(60.0f) * gb.TargetFrameDt();
	f32 rotX = 0, rotY = 0;
	if (gb.Kbd().KeyIsPressed('A')) {
		ds.x -= speed;
	}
	if (gb.Kbd().KeyIsPressed('D')) {
		ds.x += speed;
	}
	if (gb.Kbd().KeyIsPressed('E')) {
		ds.y += speed;
	}
	if (gb.Kbd().KeyIsPressed('Q')) {
		ds.y -= speed;
	}
	if (gb.Kbd().KeyIsPressed('W')) {
		ds.z += speed;
	}
	if (gb.Kbd().KeyIsPressed('S')) {
		ds.z -= speed;
	}
	if (gb.Kbd().KeyIsPressed(VK_LEFT)) {
		rotX -= rotSpeed;
	}
	if (gb.Kbd().KeyIsPressed(VK_RIGHT)) {
		rotX += rotSpeed;
	}
	if (gb.Kbd().KeyIsPressed(VK_UP)) {
		rotY += rotSpeed;
	}
	if (gb.Kbd().KeyIsPressed(VK_DOWN)) {
		rotY -= rotSpeed;
	}

	std::optional<Mouse::Event> opEvent;
	while ((opEvent = gb.Mouse().Read()).has_value()) {
		switch (opEvent.value().GetType()) {
		case Mouse::Event::Type::WheelUp:
			ds.z += 240.0f;
			break;
		case Mouse::Event::Type::WheelDown:
			ds.z -= 240.0f;
			break;
		}
	}

	if (ds.x != 0 || ds.y != 0 || ds.z != 0 || rotX != 0 || rotY != 0) {
		if (auto opCam = gb.Cams().GetActiveCamera(); opCam) {
			opCam.value().get().Translate(ds);
			opCam.value().get().Rotate(rotX, rotY);
		}
	}
}


