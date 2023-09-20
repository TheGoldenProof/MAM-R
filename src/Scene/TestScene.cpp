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

TestScene::TestScene(Globe& gb) : MidiScene(gb, "testScene0") {}

void TestScene::Draw(Globe& gb) {
	MidiScene::Draw(gb);
	UpdateInputs(gb);
}

void TestScene::InitVisuals(Globe& gb) {
	for (usize i = 0; i < midi.GetTracks().size(); i++) {
		const MIDI::Track& track = midi.GetTracks()[i];
		auto trackColor = colors[(i * 8) % _countof(colors)];
		const usize noteCount = min(16383, track.notes.size());
		if (noteCount == 0) continue;
		QuadBatchColored::BatchDesc desc{
			.uniqueName = std::format("ts0qbc{:d}", i),
			.maxQuadCount = noteCount,
			.layer = 0
		};
		auto pQbatch = std::make_unique<QuadBatchColored>(gb.Gfx(), std::move(desc));
		for (usize j = 0; j < noteCount; j++) {
			const MIDI::Note& note = track.notes[j];
			const f32 noteLength = note.lengthTicks * noteScaleFactor;
			QuadBatchColored::QuadDesc ndesc{
				.position = {note.startTick * noteScaleFactor + noteLength * 0.5f + 1, (note.pitch - 64) * 12.0f, i * 15.0f},
				.rotation = {0, 0, 0},
				.size = {noteLength - 2, 10.0f},
				.singleColor = true,
				.colors = {{trackColor.r, trackColor.g, trackColor.b, static_cast<u8>(note.velocity / 2 + 128)}, {}, {}, {}}
			};
			pQbatch->AddOneQuad(std::move(ndesc));
		}

		pQbatch->FlushChanges(gb.Gfx());
		visuals.emplace(std::move(pQbatch));
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
			ds.z += Graphics::depthScale * 1.5f;
			break;
		case Mouse::Event::Type::WheelDown:
			ds.z -= Graphics::depthScale * 1.5f;
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


