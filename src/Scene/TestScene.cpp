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
	for (usize i = 0; i < midi.GetTracks().size(); i++) {
		const MIDI::Track& track = midi.GetTracks()[i];
		auto trackColor = trackColors[i % trackColors.size()];
		const usize noteCount = min(16383, track.notes.size());
		if (noteCount == 0) continue;
		QuadBatchColored::BatchDesc desc{
			.uniqueName = std::format("ts0qbc{:d}", i),
			.maxQuadCount = noteCount,
			.layer = 0,
			.vertexShader = "DarkenPlayed_VS.cso",
			.pixelShader = "DarkenPlayed_PS.cso",
		};
		auto pQbatch = std::make_unique<QuadBatchColored>(gb.Gfx(), std::move(desc));
		for (usize j = 0; j < noteCount; j++) {
			const MIDI::Note& note = track.notes[j];
			const f32 noteLength = note.lengthTicks * lengthScale;
			auto velScale = [&note, this](f32 val) {return val * (note.velocity / 128.0f * velocityFactor + (1.0f - velocityFactor)); };
			QuadBatchColored::QuadDesc ndesc{
				.position = {note.startTick * lengthScale + noteLength * 0.5f + 1, (note.pitch - 64) * 12.0f, 0},
				.rotation = {0, 0, 0},
				.size = {noteLength - 2, 10.0f},
				.singleColor = true,
				.colors = {{{velScale(trackColor[0]), velScale(trackColor[1]), velScale(trackColor[2]), 1.0f}, {}, {}, {}}}
			};
			pQbatch->AddOneQuad(std::move(ndesc));
		}

		pQbatch->SetPos({ 0, 0, i * zSpacing + 1 });

		pQbatch->FlushChanges(gb.Gfx());
		trackVisuals.emplace_back(std::move(pQbatch));
	}
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
	if (ImGui::Begin("Controls")) {
		ImGui::Text("W/A/S/D/E/Q: Camera Position");
		ImGui::Text("Arrow Keys: Camera Rotation");
	}
	ImGui::End();

	if (ImGui::Begin("MIDI Controls")) {
		ImGui::InputFloat("Depth", &zSpacing, 0.5f, 5.0f);
		ImGui::SliderFloat("Velocity Intensity", &velocityFactor, 0.0f, 1.0f);

		if (ImGui::TreeNode("Track Colors")) {
			for (u32 i = 0; i < trackColors.size(); i++) {
				ImGui::ColorEdit3(std::format("Track {:d}", i + 1).c_str(), trackColors[i].data());
			}

			if (ImGui::Button("-")) {
				if (trackColors.size() > 1) trackColors.pop_back();
			}
			ImGui::SameLine();
			if (ImGui::Button("+")) {
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


