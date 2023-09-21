#include "Globe.h"
#include "Graphics\Camera.h"
#include "Graphics\Drawable\Drawables.h"
#include "imgui\imgui.h"
#include "MIDI\Enums.h"
#include "MIDI\RawMidi.h"
#include "Scene\MidiScene.h"
#include "Util\MyMath.h"
#include "Windows\Window.h"
#include <fstream>

MidiScene::MidiScene(Globe& gb, const std::string& name) : Scene(gb, name) {}

void MidiScene::Init(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
}

void MidiScene::Update(Globe& gb) {
	if (needsReset) {
		Reset(gb);
		reloadMidi = false;
		reloadAudio = false;
		needsReset = false;
	}
	if (reloadMidi) {
		InitMidi(gb);
		reloadMidi = false;
	}

	if (reloadAudio) {
		sound.Open(gb.Audio(), audioPath.c_str());
		sound.SetVolume(volume);
		sound.SetOffset(audioOffset);
		reloadAudio = false;
	}

	if (volume != volumePrev) {
		sound.SetVolume(volume);
		volumePrev = volume;
	}

	if (audioOffset != audioOffsetPrev) {
		sound.SetOffset(audioOffset);
		audioOffsetPrev = audioOffset;
	}

	UpdateTPS(gb);

	if (midiOffset != midiOffsetPrev) {
		f32 odx = (midiOffset - midiOffsetPrev) * lengthScale;
		MovePlay(gb, odx);
		midiOffsetPrev = midiOffset;
	}

	if (!isPlaying) return;

	f32 dx = 0;

	auto div = midi.GetHeader().division;
	switch (div.fmt) {
	case MIDI::MIDI_DIVISION_FORMAT_TPQN:
		dx = 1.0f;
		break;
	case MIDI::MIDI_DIVISION_FORMAT_SMPTE:
		dx = (6e7f / tempoMicros) * gb.TargetTPS(); // no idea if this is right. it might just need to be noteScaleFactor as well.
		break;
	}

	MovePlay(gb, dx);

	currentTick++;
}

void MidiScene::Draw(Globe& gb) {
	Scene::Draw(gb);

	if (gb.Gfx().IsImguiEnabled()) DrawGUI(gb);

	if (gb.Kbd().KeyPressed(VK_SPACE)) {
		isPlaying = !isPlaying;
		bool soundPlaying = sound.IsPlaying();
		if (!isPlaying && soundPlaying) {
			sound.Pause();
		} else if (isPlaying && !soundPlaying) {
			if (sound.IsPaused()) {
				sound.Resume();
			} else {
				sound.Play(gb.Audio());
			}
		}
	}

	if (gb.Kbd().KeyPressed(VK_F5)) {
		isPlaying = false;
		sound.Stop();
		needsReset = true;
	}
	if (gb.Kbd().KeyPressed(VK_F6)) {
		reloadMidi = true;
	}
}

void MidiScene::Reset(Globe& gb) {
	if (auto opCam = gb.Cams().GetActiveCamera(); opCam) opCam.value().get().Reset();
	playX = 0.0f;
	InitMidi(gb);
	MovePlay(gb, static_cast<f32>(midiOffset));
	sound.Open(gb.Audio(), audioPath.c_str());
	sound.SetVolume(volume);
	sound.SetOffset(audioOffset);
}

void MidiScene::DrawGUI(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
	bool bOpenMIDI = false;
	bool bOpenAudio = false;

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("Open MIDI", nullptr, &bOpenMIDI);
			ImGui::MenuItem("Open Audio", nullptr, &bOpenAudio);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (ImGui::Begin("Audio Controls")) {
		ImGui::SliderFloat("Volume", &volume, 0.0, 2.0);
		ImGui::InputInt("Offset (ms)", &audioOffset);
	}
	ImGui::End();

	if (ImGui::Begin("MIDI Controls")) {
		ImGui::InputInt("Offset (frames)", &midiOffset);
	}
	ImGui::End();

	if (ImGui::Begin("Misc Controls")) {
		ImGui::ColorEdit3("Background", gb.clearColor);
	}
	ImGui::End();

	if (ImGui::Begin("Controls")) {
		ImGui::Text("Space: Play/Pause");
		ImGui::Text("F1: Toggle GUI");
		ImGui::Text("F3: Toggle debug info");
		ImGui::Text("F5: Reset");
		ImGui::Text("F6: Refresh MIDI");
		ImGui::Text("F11: Toggle Fullscreen");
	}
	ImGui::End();

	if (bOpenMIDI) {
		std::vector<std::pair<const wchar_t*, const wchar_t*>> fileTypes = { {L"MIDI file", L"*.mid;*.midi"}, {L"All files", L"*.*"} };
		midiPath = gb.Wnd().OpenFile(std::move(fileTypes), 1, L".mid");
		if (auto oCam = gb.Cams().GetActiveCamera(); oCam) oCam.value().get().Reset();
		isPlaying = false;
		reloadMidi = true;
	}

	if (bOpenAudio) {
		std::vector<std::pair<const wchar_t*, const wchar_t*>> fileTypes = { 
			// idk what all sound files can even be opened so im just gonna leave it as this.
			{L"All files", L"*.*"} 
		};
		audioPath = gb.Wnd().OpenFile(fileTypes, 1, L".mp3");
		sound.Open(gb.Audio(), audioPath.c_str());
		if (auto oCam = gb.Cams().GetActiveCamera(); oCam) oCam.value().get().Reset();
		isPlaying = false;
		reloadAudio = true;
	}
}

void MidiScene::InitMidi(Globe& gb) {
	currentTick = 0;
	tempoMicros = 500000;
	tempoMapIndex = 0;
	ClearVisuals(gb);

	if (midiPath.empty()) return;

	MIDI::RawMidi rawMidi(gb, midiPath);

	std::ofstream ofs("midi_dump.txt");
	rawMidi.DebugPrint(ofs);

	midi.Cook(std::move(rawMidi));

	InitVisuals(gb);

	UpdateTPS(gb);
}

void MidiScene::UpdateTPS(Globe& gb) {
	const auto& tempoMap = midi.GetTempoMap();
	if (tempoMapIndex >= tempoMap.size()) return;

	if (const auto p = midi.GetTempoMap()[tempoMapIndex]; currentTick == p.first) {
		if (tempoMicros == p.second) return;
		tempoMicros = p.second;

		auto div = midi.GetHeader().division;
		switch (div.fmt) {
		case MIDI::MIDI_DIVISION_FORMAT_TPQN:
			gb.TargetTPS((1e6f / tempoMicros) * div.ticksPerQuarter);
			break;
		case MIDI::MIDI_DIVISION_FORMAT_SMPTE:
			gb.TargetTPS(-div.smpte * static_cast<f32>(div.ticksPerFrame));
			break;
		}

		tempoMapIndex++;
	}

}