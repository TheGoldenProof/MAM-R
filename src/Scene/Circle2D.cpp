#include "Config.h"
#include "imgui\imgui.h"
#include "Globe.h"
#include "Graphics\Camera.h"
#include "Graphics\Drawable\Drawables.h"
#include "Graphics\Render\FrameController.h"
#include "Scene\Circle2D.h"
#include "Util\MyMath.h"

Circle2D::Circle2D(Globe& gb, const std::string& name) : MidiScene(gb, name) {
	angleOffset = Math::to_rad(90.0f);
	angleRange = Math::TWO_PI<f32>;
}

void Circle2D::Init(Globe& gb) {
	MidiScene::Init(gb);
	if (auto opCam = gb.Cams().GetCamera("Camera1"); opCam) {
		opCam.value().get().SetHomePos({ 0, 0, -480.0f });
		opCam.value().get().SetHomeRotation({ 0, 0, 0 });
		opCam.value().get().Reset();
	}
	camFOV = 90.0f;
	gb.FrameCtrl().fov = Math::to_rad(camFOV);
}

void Circle2D::Draw(Globe& gb) {
	for (const auto& track : trackVisuals) {
		track->Draw(gb.FrameCtrl());
	}

	if (pPlayingVisuals) {
		UpdateVisuals(gb);
		if (pPlayingVisuals->GetIndexCount() > 0) pPlayingVisuals->Draw(gb.FrameCtrl());
	}

	MidiScene::Draw(gb);
}

void Circle2D::InitVisuals(Globe& gb) {
	usize skippedTracks = 0;
	for (usize fi = 0; fi < midi.GetTracks().size(); fi++) {
		const auto& trR = trackReorder[fi];
		if (!trR.second) { skippedTracks++; continue; }
		const usize i = fi - skippedTracks;
		const MIDI::Track& track = midi.GetTracks()[trR.first];
		auto trackColor = trackColors[i % trackColors.size()];
		const usize noteCount = std::min(0x7fffffff / 24ull, track.notes.size());
		if (noteCount == 0) continue;
		QuadBatchColored::BatchDesc desc{
			.uniqueName = std::format("c2d0qbc{:d}", i),
			.maxQuadCount = noteCount,
			.layer = 16,
			.vertexShader = "Colored_VS.cso",
			.pixelShader = "Colored_PS.cso",
		};
		auto pQbatch = std::make_unique<QuadBatchColored>(gb.Gfx(), std::move(desc));
		const f32 trackRadius = startRadius + i * deltaRadius;
		for (u8 j = track.lowestNote; j <= track.highestNote; j++) {
			const f32 noteTheta = angleOffset + Math::map<f32>(j, track.lowestNote, track.highestNote + (alignEnds?0.0f:1.0f), -angleRange*0.5f, angleRange*0.5f);
			const f32 noteXBase = -trackRadius * cos(noteTheta);
			const f32 noteYBase = trackRadius * sin(noteTheta);

			QuadBatchColored::QuadDesc qdesc{
				.position = {0, 0, 0},
				.rotation = {0, 0, noteRotation - noteTheta},
				.size = {noteSize, noteSize},
				.singleColor = true,
				.colors = {{{trackColor[0], trackColor[1], trackColor[2], noteAlpha}, {}, {}, {}}}
			};

			if (noteType == 4) {
				// Left
				qdesc.position.x = noteXBase;
				qdesc.position.y = noteYBase;
				pQbatch->AddOneQuad(qdesc);
			} else if (noteType == 3) {
				constexpr f32 root3on2 = 1.7320508075688772935274463415059f / 2.0f;
				constexpr f32 root3on12 = 1.7320508075688772935274463415059f / 12.0f;
				// Left
				qdesc.size.y = noteSize * root3on2;
				qdesc.position.x = noteXBase - root3on12 * noteSize * sin(noteRotation - noteTheta) * 1.0f;
				qdesc.position.y = noteYBase + root3on12 * noteSize * cos(noteRotation - noteTheta) * 1.0f;
				pQbatch->AddOneTriangle(qdesc);
			}
		}

		//pQbatch->SetPos({ 0, 0, 0 });

		pQbatch->FlushChanges(gb.Gfx());
		trackVisuals.emplace_back(std::move(pQbatch));
	}

	QuadBatchColored::BatchDesc npdesc{
		.uniqueName = "c2dNowPlaying",
		.maxQuadCount = std::max<usize>(1, (midi.GetTracks().size()-skippedTracks) * 128),
		.layer = 17,
		.vertexShader = "Colored_VS.cso",
		.pixelShader = "Colored_PS.cso",
	};
	pPlayingVisuals = std::make_unique<QuadBatchColored>(gb.Gfx(), std::move(npdesc));

	noteFades.resize(midi.GetTracks().size() - skippedTracks);
}

void Circle2D::ClearVisuals(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
	trackVisuals.clear();
	if (pPlayingVisuals) {
		pPlayingVisuals->Clear();
		pPlayingVisuals->FlushChanges(gb.Gfx());
	}
}

void Circle2D::MovePlay(Globe& gb, f32 dx) {
	UNREFERENCED_PARAMETER(gb);
	UNREFERENCED_PARAMETER(dx);
}

void Circle2D::DrawGUI(Globe& gb) {
	#define VALTM(statement) if (statement) lastValueChange = std::chrono::steady_clock::now()

	MidiScene::DrawGUI(gb);

	gb.Cams().SpawnWindowFor("Camera1");

	/*if (ImGui::Begin("Keybinds")) {
		ImGui::Text("W/A/S/D/E/Q: Camera Position");
		ImGui::Text("Arrow Keys: Camera Rotation");
	}
	ImGui::End();*/

	if (ImGui::Begin("MIDI Controls")) {
		ImGui::PushItemWidth(128.0f);
		VALTM(ImGui::InputFloat("Track Start Radius", &startRadius, 1.0f, 5.0f));
		VALTM(ImGui::InputFloat("Track Radius Delta", &deltaRadius, 1.0f, 5.0f));
		VALTM(ImGui::SliderAngle("Angle Range", &angleRange, 0.0f, 360.0f));
		VALTM(ImGui::SliderAngle("Angle Offset", &angleOffset, -180.0f, 180.0f));
		ImGui::PopItemWidth();
		if (ImGui::Checkbox("Both ends align", &alignEnds)) reloadVisuals = autoReload;

		if (ImGui::TreeNode("Note Options")) {
			ImGui::PushItemWidth(128.0f);
			if (ImGui::SliderInt("Note Sides", &noteType, 3, 4)) reloadVisuals = autoReload;
			VALTM(ImGui::SliderAngle("Note Rotation", &noteRotation, -180.0f, 180.0f));
			VALTM(ImGui::InputFloat("Note Size", &noteSize, 1.0f, 10.0f));
			VALTM(ImGui::SliderFloat("Note Alpha", &noteAlpha, 0.0f, 1.0f));
			ImGui::InputFloat("Fade Time", &noteFadeTime);
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
					reloadVisuals = autoReload;
				}
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				if (ImGui::Button(std::format("+##trRadd{:d}", i).c_str(), buttonSize)) {
					trackColors.insert(trackColors.begin() + i, trackColors[i]);
					i++;
					reloadVisuals = autoReload;
				}
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				if (ImGui::Button(std::format("^##trRup{:d}", i).c_str(), buttonSize) && i != 0) {
					trackColors[i].swap(trackColors[i - 1]);
					reloadVisuals = autoReload;
				}
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				if (ImGui::Button(std::format("v##trRdown{:d}", i).c_str(), buttonSize) && i != trackColors.size() - 1) {
					trackColors[i].swap(trackColors[i + 1]);
					reloadVisuals = autoReload;
				}
				ImGui::SameLine();
				usize trackIndex = i < midi.GetTracks().size() ? trackReorder[i].first : i;
				std::string trackName = trackIndex < midi.GetTracks().size() ? midi.GetTracks()[trackIndex].name : "";
				std::string trackDisplayName = trackName.empty() ? std::format("Track {:d}", trackIndex + 1) : trackName;
				VALTM(ImGui::ColorEdit3(std::format("{:s}##track{:d}", trackDisplayName, trackIndex).c_str(), trackColors[i].data(), ImGuiColorEditFlags_NoInputs));
			}

			if (ImGui::Button("+##addEnd", buttonSize)) {
				trackColors.push_back(trackColors.front());
				reloadVisuals = autoReload;
			}

			ImGui::TreePop();
		}

	}
	ImGui::End();

	#undef VALTM
}

void Circle2D::WriteConfig(Globe& gb) {
	MidiScene::WriteConfig(gb);

	Config& cfg = gb.Cfg();
	cfg.Set("c2d.camFOV", camFOV);
	cfg.Set("c2d.startRadius", startRadius);
	cfg.Set("c2d.deltaRadius", deltaRadius);
	cfg.Set("c2d.angleOffset", angleOffset);
	cfg.Set("c2d.angleRange", angleRange);
	cfg.Set("c2d.alignEnds", alignEnds);
	cfg.Set("c2d.noteType", noteType);
	cfg.Set("c2d.noteRotation", noteRotation);
	cfg.Set("c2d.noteSize", noteSize);
	cfg.Set("c2d.noteAlpha", noteAlpha);
	cfg.Set("c2d.noteFadeTime", noteFadeTime);
	cfg.Set("c2d.trackColors", trackColors.data(), trackColors.size());

	if (auto opCam = gb.Cams().GetCamera("Camera1"); opCam) {
		Camera& cam = opCam.value().get();
		cfg.Set("c2d.cam.homePos", cam.GetHomePos());
		cfg.Set("c2d.cam.homeRot", cam.GetHomeRotation());
	}
}

void Circle2D::ReadConfig(Globe& gb) {
	MidiScene::ReadConfig(gb);

	Config& cfg = gb.Cfg();
	cfg.Get("c2d.camFOV", &camFOV);
	cfg.Get("c2d.startRadius", &startRadius);
	cfg.Get("c2d.deltaRadius", &deltaRadius);
	cfg.Get("c2d.angleOffset", &angleOffset);
	cfg.Get("c2d.angleRange", &angleRange);
	cfg.Get("c2d.alignEnds", &alignEnds);
	cfg.Get("c2d.noteType", &noteType);
	cfg.Get("c2d.noteRotation", &noteRotation);
	cfg.Get("c2d.noteSize", &noteSize);
	cfg.Get("c2d.noteAlpha", &noteAlpha);
	cfg.Get("c2d.noteFadeTime", &noteFadeTime);
	if (auto trackColorCount = cfg()["c2d.trackColors"].size(); trackColorCount != 0) {
		trackColors.resize(trackColorCount / (sizeof(f32) * 4));
		cfg.Get("c2d.trackColors", trackColors.data());
	}

	if (auto opCam = gb.Cams().GetCamera("Camera1"); opCam) {
		Camera& cam = opCam.value().get();
		DirectX::XMFLOAT3 homePos(0, 0, 0);
		DirectX::XMFLOAT3 homeRot(0, 0, 0);
		cfg.Get("c2d.cam.homePos", &homePos);
		cfg.Get("c2d.cam.homeRot", &homeRot);
		cam.SetHomePos(homePos);
		cam.SetHomeRotation(homeRot);
		cam.Reset();
	}
}

void Circle2D::UpdateVisuals(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
	if (!pPlayingVisuals) return;
	pPlayingVisuals->Clear();

	usize skippedTracks = 0;
	const f32 realFadeTime = std::max(0.125f, noteFadeTime);

	const auto& tracks = midi.GetTracks();
	//currentlyPlaying.resize(tracks.size());
	for (usize fi = 0; fi < tracks.size(); fi++) {
		const auto& trR = trackReorder[fi];
		if (!trR.second) { skippedTracks++; continue; }
		const usize i = fi - skippedTracks;
		const usize trI = trR.first;
		const MIDI::Track& track = midi.GetTracks()[trI];

		std::bitset<128> bsTrack;
		bsTrack.reset();
		for (const auto& note : tracks[trI].notes) {
			auto ns = bsTrack[note.pitch];
			ns = ns || (currentTime.count() > 0
				&& currentTick >= note.startTick
				&& currentTick <= note.startTick + note.lengthTicks);
		}
		std::array<f32, 128>& nfTrack = noteFades[trI];
		for (u8 bi = track.lowestNote; bi <= track.highestNote; bi++) {
			f32& nf = nfTrack[bi];
			if (bsTrack[bi]) {
				nf = 1.0f;
				continue;
			}
			if (nf == 0.0f) continue;
			if (nf > 0.0f) nf -= gb.TargetFrameDt() / realFadeTime;
			if (nf < 0.0f) nf = 0.0f;
		}

		auto trackColor = trackColors[i % trackColors.size()];
		const f32 trackRadius = startRadius + i * deltaRadius;
		for (u8 j = track.lowestNote; j <= track.highestNote; j++) {
			const f32& nf = nfTrack[j];
			if (nf <= 0.0f) continue;
			const f32 noteTheta = angleOffset + Math::map<f32>(j, track.lowestNote, track.highestNote + (alignEnds?0.0f:1.0f), -angleRange * 0.5f, angleRange * 0.5f);
			const f32 noteXBase = -trackRadius * cos(noteTheta);
			const f32 noteYBase = trackRadius * sin(noteTheta);

			QuadBatchColored::QuadDesc qdesc{
				.position = {0, 0, 0.0f},
				.rotation = {0, 0, noteRotation - noteTheta},
				.size = {noteSize, noteSize},
				.singleColor = true,
				.colors = {{{trackColor[0], trackColor[1], trackColor[2], nf}, {}, {}, {}}}
			};

			if (noteType == 4) {
				// Left
				qdesc.position.x = noteXBase;
				qdesc.position.y = noteYBase;
				pPlayingVisuals->AddOneQuad(qdesc);
			} else if (noteType == 3) {
				constexpr f32 root3on2 = 1.7320508075688772935274463415059f / 2.0f;
				constexpr f32 root3on12 = 1.7320508075688772935274463415059f / 12.0f;
				// Left
				qdesc.size.y = noteSize * root3on2;
				qdesc.position.x = noteXBase - root3on12 * noteSize * sin(noteRotation - noteTheta) * 1.0f;
				qdesc.position.y = noteYBase + root3on12 * noteSize * cos(noteRotation - noteTheta) * 1.0f;
				pPlayingVisuals->AddOneTriangle(qdesc);
			}
		}
	}
	
	//pPlayingVisuals->SetPos({ 0, 0, 0 });
	pPlayingVisuals->FlushChanges(gb.Gfx());
}
