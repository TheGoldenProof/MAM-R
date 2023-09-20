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
	QuadColored::QuadDesc playLineDesc{
		.uniqueName = "play line",
		.size = {3.0f, static_cast<f32>(gb.Gfx().GetHeight())},
		.layer = 16,
		.vertexShader = "Colored_VS.cso",
		.pixelShader = "Colored_PS.cso",
		.singleColor = true,
		.colors = {{255, 255, 255, 127}, {}, {}, {}}
	};
	auto upPlayLine = std::make_unique<QuadColored>(gb.Gfx(), std::move(playLineDesc));
	pPlayLine = upPlayLine.get();
	drawables.emplace(std::move(upPlayLine));
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

	if (volume != volumePrev)
		sound.SetVolume(volume);
	volumePrev = volume;

	if (audioOffset != audioOffsetPrev)
		sound.SetOffset(audioOffset);
	audioOffsetPrev = audioOffset;

	if (!isPlaying) return;

	UpdateTPS(gb);

	if (auto opCam = gb.Cams().GetActiveCamera(); opCam) {
		auto div = midi.GetHeader().division;
		switch (div.fmt) {
		case MIDI::MIDI_DIVISION_FORMAT_TPQN:
			opCam.value().get().Translate({ noteScaleFactor, 0, 0 });
			break;
		case MIDI::MIDI_DIVISION_FORMAT_SMPTE:
			opCam.value().get().Translate({ (6e7f / tempoMicros) * gb.TargetTPS(), 0, 0 }); // no idea if this is right. it might just need to be noteScaleFactor as well.
			break;
		}
	}

	currentTick++;
}

void MidiScene::Draw(Globe& gb) {
	Scene::Draw(gb);

	for (auto& drawable : visuals) {
		drawable->Draw(gb.FrameCtrl());
	}

	if (gb.Gfx().IsImguiEnabled()) DrawGUI(gb);

	pPlayLine->SetScale(1.0f, static_cast<f32>(gb.Gfx().GetHeight()) / MAMR_WINH);

	if (gb.Kbd().KeyPressed(VK_SPACE)) {
		isPlaying = !isPlaying;
		if (sound.IsPlaying()) {
			sound.Pause();
		} else {
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
}

void MidiScene::Reset(Globe& gb) {
	if (auto opCam = gb.Cams().GetActiveCamera(); opCam) opCam.value().get().Reset();
	InitMidi(gb);
	sound.Open(gb.Audio(), audioPath.c_str());
	sound.SetVolume(volume);
	sound.SetOffset(audioOffset);
}

void MidiScene::ClearVisuals(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
	visuals.clear();
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
		ImGui::End();
	}

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