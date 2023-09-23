#include "Globe.h"
#include "Graphics\Camera.h"
#include "Graphics\Drawable\Drawables.h"
#include "imgui\imgui.h"
#include "MIDI\Enums.h"
#include "MIDI\RawMidi.h"
#include "Scene\MidiScene.h"
#include "TGLib\TGLib_Util.h"
#include "Util\MyMath.h"
#include "Windows\Window.h"
#include <fstream>

MidiScene::MidiScene(Globe& gb, const std::string& name) : Scene(gb, name) {
	CBD::RawLayout layout;
	layout.Add(CBD::Float4, "imgTint");
	pBGImgTintBuf = std::make_unique<PixelConstantBufferCaching>(gb.Gfx(), std::move(layout), 1);
}

void MidiScene::Init(Globe& gb) {
	pBGImgTintBuf->GetBuffer()["imgTint"] = bgImgTint;
	pBGImgTintBuf->Bind(gb.Gfx());
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

	if (!imagePath.empty()) {
		QuadTextured::QuadDesc desc{
			.uniqueName = "BG image",
			.size = {1.0f, 1.0f},
			.layer = 16,
			.vertexShader = "BGImg_VS.cso",
			.pixelShader = "BGImg_PS.cso",
			.texture = imagePath,
			.sizeMode = SIZE_MODE_ABSOLUTE,
		};
		pBGImg = std::make_unique<QuadTextured>(gb.Gfx(), std::move(desc));
		pBGImg->SetScale({ static_cast<f32>(gb.Gfx().GetWidth()), static_cast<f32>(gb.Gfx().GetHeight()) });
		imagePath.clear();
	}

	if (clearBGImg && pBGImg) pBGImg = nullptr;

	UpdateTPS(gb);

	if (midiOffset != midiOffsetPrev) {
		const f32 diff = static_cast<f32>(midiOffset - midiOffsetPrev);
		MovePlay(gb, MillisToPixels(diff));
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
	if (pBGImg) pBGImg->Draw(gb.FrameCtrl());

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
	MovePlay(gb, MillisToPixels(static_cast<f32>(midiOffset)));
	sound.Open(gb.Audio(), audioPath.c_str());
	sound.SetVolume(volume);
	sound.SetOffset(audioOffset);
}

void MidiScene::DrawGUI(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
	bool bOpenMIDI = false;
	bool bOpenAudio = false;
	bool bOpenImage = false;

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("Open MIDI", nullptr, &bOpenMIDI);
			ImGui::MenuItem("Open Audio", nullptr, &bOpenAudio);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (ImGui::Begin("Audio Controls")) {
		ImGui::PushItemWidth(128.0f);
		ImGui::SliderFloat("Volume", &volume, 0.0, 2.0);
		i32 fakeOffset = -audioOffset;
		if (ImGui::InputInt("Offset (ms)", &fakeOffset)) audioOffset = -fakeOffset;
		ImGui::PopItemWidth();
	}
	ImGui::End();

	if (ImGui::Begin("MIDI Controls")) {
		ImGui::SetNextItemWidth(128.0f);
		ImGui::InputInt("Offset (ms)", &midiOffset);
	}
	ImGui::End();

	if (ImGui::Begin("Misc Controls")) {
		ImGui::ColorEdit3("Background Color", gb.clearColor, ImGuiColorEditFlags_NoInputs);
		ImGui::Text("Background Image"); ImGui::SameLine();
		bOpenImage = ImGui::Button("Open##bgImg"); ImGui::SameLine();
		clearBGImg = ImGui::Button("Clear##bgImg");
		if (pBGImg && ImGui::TreeNode("Image Options##bgImg")) {
			ImGui::PushItemWidth(128.0f);
			pBGImg->SpawnControlWindowInner();
			ImGui::PopItemWidth();
			if (ImGui::Button("Stretch to window"))
				pBGImg->SetScale({ static_cast<f32>(gb.Gfx().GetWidth()), static_cast<f32>(gb.Gfx().GetHeight()) });
			if (ImGui::ColorEdit4("Image Tint", (float*)&bgImgTint, ImGuiColorEditFlags_NoInputs)) {
				pBGImgTintBuf->GetBuffer()["imgTint"] = bgImgTint;
				pBGImgTintBuf->Bind(gb.Gfx());
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();

	if (ImGui::Begin("Keybinds")) {
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

	if (bOpenImage) {
		std::vector<std::pair<const wchar_t*, const wchar_t*>> fileTypes = {
			{L"Image", L"*.png;*.jpg;*.jpeg;*.tiff;*.tga;*.gif;*.bmp"},
			{L"PNG", L"*.png;"},
			{L"JPEG", L"*.jpg;*.jpeg;"},
			{L"TIFF", L"*.tiff;"},
			{L"TGA", L"*.tga;"},
			{L"GIF", L"*.gif;"},
			{L"BMP", L"*.bmp"},
			{L"All files", L"*.*"}
		};
		imagePath = gb.Wnd().OpenFile(fileTypes, 1, L".png");
	}
}

f32 MidiScene::MillisToPixels(f32 millis) const {
	f32 odx = 0;
	const auto midiDiv = midi.GetHeader().division;
	switch (midiDiv.fmt) {
	case MIDI::MIDI_DIVISION_FORMAT_TPQN:
		odx = millis * 1000.0f * midiDiv.ticksPerQuarter / (tempoMicros);
		break;
	case MIDI::MIDI_DIVISION_FORMAT_SMPTE:
		odx = millis / (1000.0f * -midiDiv.smpte * midiDiv.ticksPerFrame);
		break;
	}
	return odx;
}

void MidiScene::InitMidi(Globe& gb) {
	currentTick = 0;
	tempoMicros = 500000;
	tempoMapIndex = 0;
	ClearVisuals(gb);

	if (midiPath.empty()) return;

	MIDI::RawMidi rawMidi(gb, midiPath);

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