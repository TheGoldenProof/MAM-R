#include "Config.h"
#include "Globe.h"
#include "Graphics\Camera.h"
#include "Graphics\Drawable\Drawables.h"
#include "Graphics\Render\FrameController.h"
#include "imgui\imgui.h"
#include "Scene\Circle3D.h"
#include "Util\MyMath.h"

Circle3D::Circle3D(Globe& gb, const std::string& name) : MidiScene(gb, name) {
	CBD::RawLayout layout;
	layout.Add(CBD::Float, "planeX");
	pPlayPlanePBuf = std::make_unique<PixelConstantBufferCaching>(gb.Gfx(), CBD::Buffer(std::move(layout)), 0);
}

void Circle3D::Init(Globe& gb) {
	MidiScene::Init(gb);
	if (auto opCam = gb.Cams().GetCamera("Camera0"); opCam) {
		opCam.value().get().SetHomePos({ -480.0f, 0, 0 });
		opCam.value().get().SetHomeRotation({ 0, Math::to_rad(90.0f), 0 });
		opCam.value().get().Reset();
	}
	camFOV = 60.0f;
	gb.FrameCtrl().fov = Math::to_rad(camFOV);
}

void Circle3D::Draw(Globe& gb) {
	pPlayPlanePBuf->GetBuffer()["planeX"] = playX;
	pPlayPlanePBuf->Bind(gb.Gfx());

	for (const auto& track : trackVisuals) {
		track->Draw(gb.FrameCtrl());
	}

	MidiScene::Draw(gb);
}

void Circle3D::InitVisuals(Globe& gb) {
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
			.uniqueName = std::format("c3d0qbc{:d}", i),
			.maxQuadCount = noteCount * 6,
			.layer = 0,
			.vertexShader = "DarkenPlayed_VS.cso",
			.pixelShader = "DarkenPlayed_PS.cso",
		};
		auto pQbatch = std::make_unique<QuadBatchColored>(gb.Gfx(), std::move(desc));
		const f32 trackRadius = startRadius + i * deltaRadius;
		for (usize j = 0; j < noteCount; j++) {
			const MIDI::Note& note = track.notes[j];
			const f32 noteLength = note.lengthTicks * lengthScale;
			auto velScale = [&note, this](f32 val) {return val * (note.velocity / 128.0f * velocityFactor + (1.0f - velocityFactor)); };

			const f32 noteX = note.startTick * lengthScale + noteLength * 0.5f + noteHSpacing / 2;
			const f32 noteTheta = angleOffset + Math::map<f32>(note.pitch, track.lowestNote, track.highestNote + 1.0f, -Math::PI<f32>, Math::PI<f32>);
			const f32 noteYBase = trackRadius * sin(noteTheta);
			const f32 noteZBase = trackRadius * cos(noteTheta);

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
				const f32 theta = Math::to_rad(360.0f / noteType * k - 90.0f) + noteRotation - noteTheta;
				edesc.rotation.x = theta;
				edesc.position.y = noteYBase + radius * sin(theta) * noteHeight;
				edesc.position.z = noteZBase - radius * cos(theta) * noteHeight;
				pQbatch->AddOneQuad(edesc);
			}

			endD.position.x = note.startTick * lengthScale + noteHSpacing;
			endD.rotation.z = noteRotation - noteTheta;
			endD.rotation.y = Math::HALF_PI<f32>;

			if (noteType == 4) {
				// Left
				endD.position.y = noteYBase;
				endD.position.z = noteZBase;
				endD.size.x = noteHeight;
				pQbatch->AddOneQuad(endD);
				// Right
				endD.position.x = note.startTick * lengthScale + noteLength;
				endD.rotation.y = -Math::HALF_PI<f32>;
				endD.rotation.z = -noteRotation + noteTheta;
				pQbatch->AddOneQuad(endD);

			} else if (noteType == 3) {
				constexpr f32 root3on2 = 1.7320508075688772935274463415059f / 2.0f;
				constexpr f32 root3on12 = 1.7320508075688772935274463415059f / 12.0f;
				// Left
				endD.size.x = noteHeight;
				endD.size.y = noteHeight * root3on2;
				endD.position.y = noteYBase + root3on12 * noteHeight * cos(noteRotation - noteTheta);
				endD.position.z = noteZBase + root3on12 * noteHeight * sin(noteRotation - noteTheta);
				pQbatch->AddOneTriangle(endD);
				// Right
				endD.position.x = note.startTick * lengthScale + noteLength;
				endD.rotation.y = -Math::HALF_PI<f32>;
				endD.rotation.z = -noteRotation + noteTheta;
				pQbatch->AddOneTriangle(endD);
			}

		}

		pQbatch->SetPos({ 0, 0, 0 });

		pQbatch->FlushChanges(gb.Gfx());
		trackVisuals.emplace_back(std::move(pQbatch));
	}
}

void Circle3D::ClearVisuals(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
	trackVisuals.clear();
}

void Circle3D::MovePlay(Globe& gb, f32 dx) {
	dx *= lengthScale;
	playX += dx;

	if (auto opCam = gb.Cams().GetActiveCamera(); opCam) {
		Camera& cam = opCam.value().get();
		auto camPos = cam.GetPos();
		camPos.x += dx;
		cam.SetPos(camPos);
	}
}

void Circle3D::DrawGUI(Globe& gb) {
	#define VALTM(statement) if (statement) lastValueChange = std::chrono::steady_clock::now()

	MidiScene::DrawGUI(gb);

	gb.Cams().SpawnWindowFor("Camera0");

	/*if (ImGui::Begin("Keybinds")) {
		ImGui::Text("W/A/S/D/E/Q: Camera Position");
		ImGui::Text("Arrow Keys: Camera Rotation");
	}
	ImGui::End();*/

	if (ImGui::Begin("MIDI Controls")) {
		ImGui::PushItemWidth(128.0f);
		VALTM(ImGui::InputFloat("Track Start Radius", &startRadius, 1.0f, 5.0f));
		VALTM(ImGui::InputFloat("Track Radius Delta", &deltaRadius, 1.0f, 5.0f));
		VALTM(ImGui::SliderAngle("Angle Offset", &angleOffset, -180.0f, 180.0f));
		ImGui::PopItemWidth();

		if (ImGui::TreeNode("Note Options")) {
			ImGui::PushItemWidth(128.0f);
			VALTM(ImGui::SliderFloat("Velocity Intensity", &velocityFactor, 0.0f, 1.0f));
			if (ImGui::SliderInt("Note Sides", &noteType, 3, 4)) reloadVisuals = autoReload;
			VALTM(ImGui::SliderAngle("Note Rotation", &noteRotation, -180.0f, 180.0f));
			VALTM(ImGui::InputFloat("Height", &noteHeight));
			VALTM(ImGui::InputFloat("Length Factor", &lengthScale));
			VALTM(ImGui::InputFloat("Horizontal Spacing", &noteHSpacing));
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

void Circle3D::WriteConfig(Globe& gb) {
	MidiScene::WriteConfig(gb);

	Config& cfg = gb.Cfg();
	cfg.Set("c3d.camFOV", camFOV);
	cfg.Set("c3d.startRadius", startRadius);
	cfg.Set("c3d.deltaRadius", deltaRadius);
	cfg.Set("c3d.angleOffset", angleOffset);
	cfg.Set("c3d.velFactor", velocityFactor);
	cfg.Set("c3d.noteType", noteType);
	cfg.Set("c3d.noteRotation", noteRotation);
	cfg.Set("c3d.noteHeight", noteHeight);
	cfg.Set("c3d.noteHSpacing", noteHSpacing);
	cfg.Set("c3d.trackColors", trackColors.data(), trackColors.size());

	if (auto opCam = gb.Cams().GetCamera("Camera0"); opCam) {
		Camera& cam = opCam.value().get();
		cfg.Set("c3d.cam.homePos", cam.GetHomePos());
		cfg.Set("c3d.cam.homeRot", cam.GetHomeRotation());
	}
}

void Circle3D::ReadConfig(Globe& gb) {
	MidiScene::ReadConfig(gb);

	Config& cfg = gb.Cfg();
	cfg.Get("c3d.camFOV", &camFOV);
	cfg.Get("c3d.startRadius", &startRadius);
	cfg.Get("c3d.deltaRadius", &deltaRadius);
	cfg.Get("c3d.angleOffset", &angleOffset);
	cfg.Get("c3d.velFactor", &velocityFactor);
	cfg.Get("c3d.noteType", &noteType);
	cfg.Get("c3d.noteRotation", &noteRotation);
	cfg.Get("c3d.noteHeight", &noteHeight);
	cfg.Get("c3d.noteHSpacing", &noteHSpacing);
	if (auto trackColorCount = cfg()["c3d.trackColors"].size(); trackColorCount != 0) {
		trackColors.resize(trackColorCount / (sizeof(f32) * 4));
		cfg.Get("c3d.trackColors", trackColors.data());
	}

	if (auto opCam = gb.Cams().GetCamera("Camera0"); opCam) {
		Camera& cam = opCam.value().get();
		DirectX::XMFLOAT3 homePos(0, 0, 0);
		DirectX::XMFLOAT3 homeRot(0, 0, 0);
		cfg.Get("c3d.cam.homePos", &homePos);
		cfg.Get("c3d.cam.homeRot", &homeRot);
		cam.SetHomePos(homePos);
		cam.SetHomeRotation(homeRot);
		cam.Reset();
	}
}
