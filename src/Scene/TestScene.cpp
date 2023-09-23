#include "Globe.h"
#include "Graphics\Camera.h"
#include "Graphics\Drawable\Drawables.h"
#include "imgui\imgui.h"
#include "MIDI\RawMidi.h"
#include "Scene\TestScene.h"
#include "Util\MyMath.h"
#include "Windows\Keyboard.h"
#include "Windows\Mouse.h"
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
	for (usize i = 0; i < midi.GetTracks().size(); i++) {
		const MIDI::Track& track = midi.GetTracks()[i];
		auto trackColor = trackColors[i % trackColors.size()];
		const usize noteCount = min(0x7fffffff/24, track.notes.size());
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

			// Front
			QuadBatchColored::QuadDesc fdesc{
				.position = {note.startTick * lengthScale + noteLength * 0.5f + noteHSpacing/2, (note.pitch - 64) * (noteHeight + noteVSpacing), 0},
				.rotation = {0, 0, 0},
				.size = {noteLength - noteHSpacing, noteHeight},
				.singleColor = true,
				.colors = {{{velScale(trackColor[0]), velScale(trackColor[1]), velScale(trackColor[2]), 1.0f}, {}, {}, {}}}
			};
			pQbatch->AddOneQuad(fdesc);

			QuadBatchColored::QuadDesc ndesc = fdesc;

			// Top
			ndesc.position.y = fdesc.position.y + noteHeight / 2;
			ndesc.position.z = noteHeight / 2;
			ndesc.rotation.x += Math::HALF_PI;
			pQbatch->AddOneQuad(ndesc);

			// Back
			ndesc.position.y = fdesc.position.y;
			ndesc.position.z = noteHeight;
			ndesc.rotation.x += Math::HALF_PI;
			pQbatch->AddOneQuad(ndesc);

			// Bottom
			ndesc.position.y = fdesc.position.y - noteHeight / 2;
			ndesc.position.z = noteHeight / 2;
			ndesc.rotation.x += Math::HALF_PI;
			pQbatch->AddOneQuad(ndesc);

			// Left
			ndesc.position.x = note.startTick * lengthScale + noteHSpacing;
			ndesc.position.y = fdesc.position.y;
			ndesc.rotation.x = 0;
			ndesc.rotation.y = Math::HALF_PI;
			ndesc.size.x = noteHeight;
			pQbatch->AddOneQuad(ndesc);

			// Right
			ndesc.position.x = note.startTick * lengthScale + noteLength;
			ndesc.rotation.y = -Math::HALF_PI;
			pQbatch->AddOneQuad(ndesc);

		}

		pQbatch->SetPos({ 0, 0, i * zSpacing + 1 });

		pQbatch->FlushChanges(gb.Gfx());
		const u32 indexCount = pQbatch->GetIndexCount();
		maxIndexCount = max(maxIndexCount, indexCount);
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
				}
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				if (ImGui::Button(std::format("+##add{:d}", i).c_str(), buttonSize)) {
					trackColors.insert(trackColors.begin() + i, trackColors[i]);
					i++;
				}
				ImGui::SameLine();
				ImGui::ColorEdit3(std::format("Track {:d}", i + 1).c_str(), trackColors[i].data(), ImGuiColorEditFlags_NoInputs);
			}

			if (ImGui::Button("+##addEnd", buttonSize)) {
				trackColors.push_back(trackColors.front());
			}

			ImGui::TreePop();
		}

	}
	ImGui::End();
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

