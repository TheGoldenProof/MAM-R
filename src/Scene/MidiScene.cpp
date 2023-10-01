#include "Config.h"
#include "Globe.h"
#include "Graphics\Camera.h"
#include "Graphics\Drawable\Drawables.h"
#include "Graphics\Render\FrameController.h"
#include "imgui\imgui.h"
#include "MIDI\Enums.h"
#include "MIDI\RawMidi.h"
#include "Scene\MidiScene.h"
#include "Scene\SceneController.h"
#include "TGLib\TGLib_Util.h"
#include "Util\MyMath.h"
#include "Windows\Window.h"
#include <fstream>

MIDI::RawMidi MidiScene::rawMidi;
bool MidiScene::midiPathChanged = false;
std::wstring MidiScene::midiPath;
MIDI::CookedMidi MidiScene::midi;
i32 MidiScene::midiOffset = 0;
i32 MidiScene::midiOffsetPrev = 0;
std::wstring MidiScene::audioPath;
Sound MidiScene::sound;
f32 MidiScene::volume = 1.0f;
f32 MidiScene::volumePrev = 1.0f;
i32 MidiScene::audioOffset = 0;
i32 MidiScene::audioOffsetPrev = 0;
std::wstring MidiScene::imagePath;
std::unique_ptr<QuadTextured> MidiScene::pBGImg;
std::unique_ptr<PixelConstantBufferCaching> MidiScene::pBGImgTintBuf;
DirectX::XMFLOAT4 MidiScene::bgImgTint = { 1.0f, 1.0f, 1.0f, 1.0f };
f32 MidiScene::playX = 0.0f;
std::vector<std::pair<usize, bool>> MidiScene::trackReorder; // <index, enabled>
std::unordered_map<std::string, bool> MidiScene::sceneSel;

MidiScene::MidiScene(Globe& gb, const std::string& name) : Scene(gb, name) {
	CBD::RawLayout layout;
	layout.Add(CBD::Float4, "imgTint");
	pBGImgTintBuf = std::make_unique<PixelConstantBufferCaching>(gb.Gfx(), std::move(layout), 1);
}

void MidiScene::Init(Globe& gb) {
	pBGImgTintBuf->GetBuffer()["imgTint"] = bgImgTint;
	pBGImgTintBuf->Bind(gb.Gfx());
	needsReset = true;
}

void MidiScene::Update(Globe& gb) {
	if (needsReset) {
		Reset(gb);
		reloadMidi = false;
		reloadAudio = false;
		needsRestart = false;
		needsReset = false;
	}
	if (needsRestart) {
		Restart(gb);
		needsRestart = false;
	}
	if (reloadMidi) {
		InitMidi(gb);
		reloadMidi = false;
		lastValueChange = std::chrono::steady_clock::time_point::max();
	}
	if (reloadVisuals || (autoReload && 
			lastValueChange != std::chrono::steady_clock::time_point::max() && 
			std::chrono::steady_clock::now() - lastValueChange > std::chrono::nanoseconds(500'000'000))) {
		ClearVisuals(gb);
		InitVisuals(gb);
		reloadVisuals = false;
		lastValueChange = std::chrono::steady_clock::time_point::max();
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
		const std::chrono::microseconds diff(1000 * (midiOffset - midiOffsetPrev));
		MovePlay(gb, MicrosToPixels(currentTime.count(), diff.count()));
		currentTime += diff;
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
		dx = gb.TargetTickDt() / (-div.smpte * div.ticksPerFrame);
		break;
	}

	MovePlay(gb, dx);

	if (currentTime.count() >= 0) currentTick++;
	currentTime += gb.TargetTickDt_dur<i64, std::micro>();
}

void MidiScene::Draw(Globe& gb) {
	for (auto& p : sceneSel)
		if (p.second) { gb.SceneCtrl().SetNextActive(p.first); p.second = false; return; }

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
	
	if (gb.Kbd().KeyPressed('R')) {
		isPlaying = false;
		sound.Stop();
		needsRestart = true;
	}
	if (gb.Kbd().KeyPressed(VK_F5)) {
		isPlaying = false;
		sound.Stop();
		needsReset = true;
	}
	if (gb.Kbd().KeyPressed(VK_F6)) {
		if (gb.Kbd().KeyIsPressed(VK_SHIFT)) {
			reloadMidi = true;
		} else 
			reloadVisuals = true;
	}
}

void MidiScene::Restart(Globe& gb) {
	if (auto opCam = gb.Cams().GetActiveCamera(); opCam) opCam.value().get().Reset();
	playX = 0.0f;
	currentTime = std::chrono::microseconds(1000 * midiOffset);
	currentTick = 0;
	MovePlay(gb, MicrosToPixels(0, 1000 * midiOffset));
	sound.SetOffset(audioOffset);
}

void MidiScene::Reset(Globe& gb) {
	InitMidi(gb);
	sound.Open(gb.Audio(), audioPath.c_str());
	sound.SetVolume(volume);
	Restart(gb);
}

void MidiScene::Denit(Globe& gb) {
	ClearVisuals(gb);
}

void MidiScene::DrawGUI(Globe& gb) {
	if (gb.Gfx().IsImDebugEnabled()) {
		if (ImGui::Begin("debug")) {
			ImGui::Text("time: %d", currentTime.count());
			ImGui::Text("next tempo: %d", midi.GetTempoMap().next(currentTime.count()).first);
		}
		ImGui::End();
	}

	bool bOpenMIDI = false;
	bool bOpenAudio = false;
	bool bOpenImage = false;
	bool bOpenConfig = false;
	bool bSaveConfig = false;
	bool bSaveConfigAs = false;

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			bOpenMIDI = ImGui::MenuItem("Open MIDI", nullptr, nullptr);
			bOpenAudio = ImGui::MenuItem("Open Audio", nullptr, nullptr);
			bOpenConfig = ImGui::MenuItem("Open Config", nullptr, nullptr);
			bSaveConfig = ImGui::MenuItem("Save Config", nullptr, nullptr);
			bSaveConfigAs = ImGui::MenuItem("Save Config As", nullptr, nullptr);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			for (const auto& scName : gb.SceneCtrl().GetSceneNames()) {
				sceneSel[scName] = ImGui::MenuItem(scName.c_str(), nullptr, nullptr, scName != name);
			}
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
		if (ImGui::InputInt("Offset (ms)", &midiOffset)) lastValueChange = std::chrono::steady_clock::now();
		if (ImGui::TreeNode("Track order")) {
			const f32 buttonDim = ImGui::GetFrameHeight();
			const ImVec2 buttonSize(buttonDim, buttonDim);
			for (u32 i = 0; i < midi.GetTracks().size(); i++) {
				if (ImGui::Button(std::format("^##trRup{:d}", i).c_str(), buttonSize) && i != 0) {
					std::swap(trackReorder[i], trackReorder[i-1]);
					reloadMidi = autoReload;
				}
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				if (ImGui::Button(std::format("v##trRdown{:d}", i).c_str(), buttonSize) && i != trackReorder.size() - 1) {
					std::swap(trackReorder[i], trackReorder[i+1]);
					reloadMidi = autoReload;
				}
				ImGui::SameLine();
				usize trackIndex = trackReorder[i].first;
				const std::string& trackName = midi.GetTracks()[trackIndex].name;
				std::string trackDisplyName = trackName.empty() ? std::format("Track {:d}", trackIndex + 1) : trackName;
				if (ImGui::Checkbox(std::format("{:s}##track{:d}", std::move(trackDisplyName), trackIndex).c_str(), &trackReorder[i].second)) {
					reloadMidi = autoReload;
				}

			}
			ImGui::TreePop();
		}
	}
	ImGui::End();

	if (ImGui::Begin("Misc Controls")) {
		ImGui::Checkbox("Auto-apply certain changes", &autoReload);
		if (ImGui::InputFloat("FOV", &camFOV, 1.0f, 10.0f, "%.0f")) {
			camFOV = std::clamp(camFOV, 1.0f, 179.0f);
			gb.FrameCtrl().fov = Math::to_rad(camFOV);
		}
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
		ImGui::Text("R: Restart");
		ImGui::Text("F1: Toggle GUI");
		ImGui::Text("F3: Toggle debug info");
		ImGui::Text("F5: Reload Everything");
		ImGui::Text("F6: Refresh MIDI");
		ImGui::Text("F11: Toggle Fullscreen");
	}
	ImGui::End();

	if (bOpenMIDI) {
		std::vector<std::pair<const wchar_t*, const wchar_t*>> fileTypes = { {L"MIDI file", L"*.mid;*.midi"}, {L"All files", L"*.*"} };
		midiPath = gb.Wnd().OpenFile(std::move(fileTypes), 1, L".mid");
		midiPathChanged = true;
		isPlaying = false;
		reloadMidi = true;
		needsRestart = true;
	}

	if (bOpenAudio) {
		std::vector<std::pair<const wchar_t*, const wchar_t*>> fileTypes = { 
			// idk what all sound files can even be opened so im just gonna leave it as this.
			{L"All files", L"*.*"} 
		};
		audioPath = gb.Wnd().OpenFile(fileTypes, 1, L".mp3");
		sound.Open(gb.Audio(), audioPath.c_str());
		isPlaying = false;
		reloadAudio = true;
		needsRestart = true;
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

	if (bOpenConfig) {
		gb.Cfg().Open(gb);
		ReadConfig(gb);
	}
	if (bSaveConfig) {
		WriteConfig(gb);
		gb.Cfg().Save(gb);
	}
	if (bSaveConfigAs) {
		WriteConfig(gb);
		gb.Cfg().SaveAs(gb);
	}
}

f32 MidiScene::MicrosToPixels(i64 start, i64 micros) const {
	f32 odx = 0;
	const auto& tempoMap = midi.GetTempoMap();

	while (micros != 0) {
		i64 timeSpentInTempo = 0;
		u32 _tempo = tempoMap[start].second;

		decltype(tempoMap._map)::const_iterator nextIt;
		if (micros < 0) {
			nextIt = tempoMap._map.lower_bound(start);
			if (nextIt == tempoMap._map.begin()) {
				timeSpentInTempo = micros;
			} else {
				nextIt--;
				if (nextIt->first < (start + micros)) {
					timeSpentInTempo = micros;
				} else {
					timeSpentInTempo = nextIt->first - start;
				}
			}
			_tempo = nextIt->second;
		} else {
			nextIt = tempoMap._map.upper_bound(start);
			if (nextIt == tempoMap._map.end() || nextIt->first > (start + micros)) {
				timeSpentInTempo = micros;
			} else {
				timeSpentInTempo = nextIt->first - start;
			}
		}
		start += timeSpentInTempo;
		micros -= timeSpentInTempo;

		const auto midiDiv = midi.GetHeader().division;
		switch (midiDiv.fmt) {
		case MIDI::MIDI_DIVISION_FORMAT_TPQN:
			odx += timeSpentInTempo * midiDiv.ticksPerQuarter / (_tempo);
			break;
		case MIDI::MIDI_DIVISION_FORMAT_SMPTE:
			odx += timeSpentInTempo / (1e6f * -midiDiv.smpte * midiDiv.ticksPerFrame);
			break;
		}
	}

	return odx;
}

void MidiScene::InitMidi(Globe& gb) {
	currentTick = 0;
	currentTempo = 500000;
	ClearVisuals(gb);

	if (midiPath.empty()) return;

	if (midiPathChanged) {
		rawMidi.Open(gb, midiPath);
		trackReorder.clear();
		midiPathChanged = false;
	}

	midi.Cook(std::move(rawMidi));

	const usize trackCount = midi.GetTracks().size();
	if (trackReorder.size() > trackCount) {
		trackReorder.resize(trackCount);
	} else {
		usize trackReorderSize = 0;
		while ((trackReorderSize = trackReorder.size()) < trackCount) {
			trackReorder.emplace_back(trackReorderSize, true);
		}
	}

	UpdateTPS(gb);

	InitVisuals(gb);
}

void MidiScene::UpdateTPS(Globe& gb) {
	if (const auto newTempo = midi.GetTempoMap()[currentTime.count()].second; currentTempo != newTempo) {
		currentTempo = newTempo;

		auto div = midi.GetHeader().division;
		switch (div.fmt) {
		case MIDI::MIDI_DIVISION_FORMAT_TPQN:
			gb.TargetTickDt<u32, std::micro>(currentTempo / div.ticksPerQuarter);
			break;
		case MIDI::MIDI_DIVISION_FORMAT_SMPTE:
			gb.TargetTPS(-div.smpte * static_cast<f32>(div.ticksPerFrame));
			break;
		}
	}

}

void MidiScene::WriteConfig(Globe& gb) {
	Config& cfg = gb.Cfg();
	cfg.Set("bgColor", gb.clearColor, _countof(gb.clearColor));
}

void MidiScene::ReadConfig(Globe& gb) {
	Config& cfg = gb.Cfg();
	cfg.Get("bgColor", gb.clearColor, _countof(gb.clearColor));

	reloadMidi = autoReload;
}
