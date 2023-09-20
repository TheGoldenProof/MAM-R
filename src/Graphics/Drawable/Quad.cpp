#include "Graphics/Bindable/Bindables.h"
#include "Graphics/Drawable/Quad.h"
#include "Util/MyMath.h"
#include "imgui/imgui.h"

Quad::Quad(const std::string& name) : uniqueName(name) {}

void Quad::SetPos(DirectX::XMFLOAT3 pos_) noexcept {
	pos = pos_;
}

void Quad::SetRotation(f32 pitch_, f32 yaw_, f32 roll_) noexcept {
	pitch = pitch_;
	yaw = yaw_;
	roll = roll_;
}

void Quad::SetScale(f32 w, f32 h) noexcept {
	scaleW = w;
	scaleH = h;
}

DirectX::XMMATRIX Quad::GetTransformXM() const noexcept {
	DirectX::XMVECTOR pos_ = DirectX::XMVectorSet(pos.x, pos.y, pos.z, 0.0f);
	DirectX::XMVECTOR rot_ = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	DirectX::XMVECTOR scl_ = DirectX::XMVectorSet(scaleW, scaleH, 1.0f, 1.0f);
	return DirectX::XMMatrixAffineTransformation(scl_, DirectX::XMVectorZero(), rot_, pos_);
}

void Quad::SpawnControlWindow(Graphics& gfx, const std::string& name) noexcept {
	constexpr f32 hp = (f32)Math::HALF_PI;
	UNREFERENCED_PARAMETER(gfx);
	if (ImGui::Begin(name.c_str())) {
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &pos.x, -8.0f, 8.0f);
		ImGui::SliderFloat("Y", &pos.y, -8.0f, 8.0f);
		ImGui::SliderFloat("Z", &pos.z, -8.0f, 8.0f);
		ImGui::Text("Orientation");
		ImGui::SliderFloat("Roll", &roll, -hp, hp);
		ImGui::SliderFloat("Pitch", &pitch, -hp, hp);
		ImGui::SliderFloat("Yaw", &yaw, -hp, hp);

		class Probe : public TechniqueProbe {
		public:
			void OnSetTechnique() override {
				using namespace std::string_literals;
				ImGui::TextColored({0.4f, 1.0f, 0.6f, 1.0f}, pTech->GetName().c_str());
				bool active = pTech->IsActive();
				ImGui::Checkbox(("Active##"s + std::to_string(techId)).c_str(), &active);
				pTech->SetActive(active);
			}
			bool OnVisitBuffer(CBD::Buffer& buf) {
				namespace dx = DirectX;
				bool dirty = false;
				const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };
				auto tag =[tagScratch = std::string{}, tagString = "##" + std::to_string(bufId)](const char* label) mutable {
					tagScratch = label + tagString;
					return tagScratch.c_str();
				};
				if (auto v = buf["scale"]; v.Exists()) {
					dcheck(ImGui::SliderFloat(tag("Scale"), &v, 1.0f, 2.0f, "%.3f", ImGuiSliderFlags_Logarithmic));
				}
				return dirty;
			}
		} probe;

		Accept(probe);
	}
	ImGui::End();
}

QuadTextured::QuadTextured(Graphics& gfx, const QuadDesc& desc) // some of the least favorite code ive written
	: QuadTextured(gfx, desc, std::move(desc.texture.front() == '@' ?
		static_cast<std::shared_ptr<Texture>>(RenderTargetTexture::Resolve(gfx, desc.texture, 0))
		: static_cast<std::shared_ptr<Texture>>(SurfaceTexture::Resolve(gfx, desc.texture, 0)))) {
}

QuadTextured::QuadTextured(Graphics& gfx, const QuadDesc& desc, std::shared_ptr<Texture> pTex)
	: Quad(desc.uniqueName) {
	using Type = Vtx::VertexLayout::ElementType;

	f32 sizeW = desc.size.x;
	f32 sizeH = desc.size.y;
	if (pTex->IsErrored()) {
		sizeW = 64.0f;
		sizeH = 64.0f;
	} else if (desc.sizeMode == SIZE_MODE_SCALE) {
		sizeW *= pTex->GetWidth() * desc.uvSize.x;
		sizeH *= pTex->GetHeight() * desc.uvSize.y;
	}

	Vtx::VertexLayout l;
	l.Append(Vtx::VertexLayout::Position3D);
	l.Append(Vtx::VertexLayout::Texture2D);
	Vtx::VertexBuffer vbuf(l, 4);
	vbuf[0].Attr<Type::Position3D>() = { -sizeW / 2,  sizeH / 2, 0 };
	vbuf[1].Attr<Type::Position3D>() = { sizeW / 2,  sizeH / 2, 0 };
	vbuf[2].Attr<Type::Position3D>() = { -sizeW / 2, -sizeH / 2, 0 };
	vbuf[3].Attr<Type::Position3D>() = { sizeW / 2, -sizeH / 2, 0 };
	vbuf[0].Attr<Type::Texture2D>() = { desc.uvOffset.x, desc.uvOffset.y };
	vbuf[1].Attr<Type::Texture2D>() = { desc.uvOffset.x + desc.uvSize.x, desc.uvOffset.y };
	vbuf[2].Attr<Type::Texture2D>() = { desc.uvOffset.x, desc.uvOffset.y + desc.uvSize.y };
	vbuf[3].Attr<Type::Texture2D>() = { desc.uvOffset.x + desc.uvSize.x, desc.uvOffset.y + desc.uvSize.y };

	const std::string geometryTag = "$quadT." + std::to_string(sizeW) + "x" + std::to_string(sizeH);
	pVerticies = VertexBuffer::Resolve(gfx, geometryTag + "." + uniqueName, std::move(vbuf));
	std::vector<u16> indicies{ 0,1,2,1,3,2 };
	pIndicies = IndexBuffer::Resolve(gfx, geometryTag, indicies.data(), indicies.size());
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Technique main("Main");
	QueueStep only(desc.layer);
	only.AddBindable(std::move(pTex));
	only.AddBindable(Sampler::Resolve(gfx, desc.filter));

	auto pVs = VertexShader::Resolve(gfx, desc.vertexShader);
	auto pVsbc = pVs->GetBytecode();
	only.AddBindable(std::move(pVs));

	only.AddBindable(PixelShader::Resolve(gfx, desc.pixelShader));

	only.AddBindable(InputLayout::Resolve(gfx, l, pVsbc));
	only.AddBindable(std::make_shared<TransformCBuf>(gfx));

	main.AddStep(std::move(only));
	AddTechnique(std::move(main));
}

QuadColored::QuadColored(Graphics& gfx, const QuadDesc& desc) : Quad(desc.uniqueName) {
	using Type = Vtx::VertexLayout::ElementType;

	f32 sizeW = desc.size.x;
	f32 sizeH = desc.size.y;

	Vtx::VertexLayout l;
	l.Append(Vtx::VertexLayout::Position3D);
	l.Append(Vtx::VertexLayout::Byte4Color);
	Vtx::VertexBuffer vbuf(l, 4);
	vbuf[0].Attr<Type::Position3D>() = { -sizeW / 2,  sizeH / 2, 0 };
	vbuf[1].Attr<Type::Position3D>() = { sizeW / 2,  sizeH / 2, 0 };
	vbuf[2].Attr<Type::Position3D>() = { -sizeW / 2, -sizeH / 2, 0 };
	vbuf[3].Attr<Type::Position3D>() = { sizeW / 2, -sizeH / 2, 0 };
	for (u32 i = 0; i < 4; i++) {
		auto color = desc.colors[desc.singleColor ? 0 : i];
		vbuf[i].Attr<Type::Byte4Color>() = { color[0], color[1], color[2], color[3] };
	}

	const std::string geometryTag = "$quadC." + std::to_string(sizeW) + "x" + std::to_string(sizeH);
	pVerticies = VertexBuffer::Resolve(gfx, geometryTag + "." + uniqueName, std::move(vbuf));
	std::vector<u16> indicies{ 0,1,2,1,3,2 };
	pIndicies = IndexBuffer::Resolve(gfx, geometryTag, indicies.data(), indicies.size());
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Technique main("Main");
	QueueStep only(desc.layer);

	auto pVs = VertexShader::Resolve(gfx, desc.vertexShader);
	auto pVsbc = pVs->GetBytecode();
	only.AddBindable(std::move(pVs));

	only.AddBindable(PixelShader::Resolve(gfx, desc.pixelShader));

	only.AddBindable(InputLayout::Resolve(gfx, l, pVsbc));
	only.AddBindable(std::make_shared<TransformCBuf>(gfx));

	main.AddStep(std::move(only));
	AddTechnique(std::move(main));
}
