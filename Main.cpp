
# include <Siv3D.hpp>

ImageRGBA16F CreateNormalMap(const ImageR32F& heightMap)
{
	ImageRGBA16F normalMap(heightMap.size);

	const double d = 0.15;

	for (int32 y = 0; y < heightMap.height; ++y)
	{
		for (int32 x = 0; x < heightMap.width; ++x)
		{
			const Point n(x, Max(0, y - 1));
			const Point w(Max(0, x - 1), y);
			const Point e(Min(heightMap.width - 1, x + 1), y);
			const Point s(x, Min(heightMap.height - 1, y + 1));

			const float nH = heightMap[n].r;
			const float wH = heightMap[w].r;
			const float eH = heightMap[e].r;
			const float sH = heightMap[s].r;

			const Float3 normal = Float3(wH - eH, d, sH - nH).normalized();
			normalMap[y][x] = RGBA16F(normal.x, normal.y, normal.z, 1.0);
		}
	}

	return normalMap;
}

void Main()
{
	Window::Resize(1024, 640);
	const Color skyColor(50, 140, 250);
	const double fogDensity = 0.001;
	Graphics::SetBackground(skyColor);
	Graphics3D::SetFog(Fog::SquaredExponential(skyColor, fogDensity));
	Graphics3D::SetAmbientLight(ColorF(0.2, 0.3, 0.4));

	PerlinNoise n(Random(0, 9999));
	ImageR32F heightMap(1024, 1024);
	for (auto p : step(heightMap.size))
	{
		heightMap[p].r = static_cast<float>(n.octaveNoise0_1(p.x / 200.0, p.y / 200.0, 8));
	}
	const Texture textureHeight{ heightMap };
	const Texture textureNormal{ CreateNormalMap(heightMap) };

	const VertexShader vsTerrain(L"Terrain.hlsl");
	const PixelShader psTerrain(L"Terrain.hlsl");
	ConstantBuffer<Float4> cbParam(Float4(1.0, 0.0, 0.0, 0.0));
	if (!vsTerrain || !psTerrain)
		return;

	const Texture textureGround(L"Example/Grass.jpg", TextureDesc::For3D);
	const Mesh terrainBase(MeshData::TerrainBase(1000, 256));

	GUI gui(GUIStyle::Default);
	gui.add(GUIText::Create(L"テクスチャ"));
	gui.addln(L"x", GUISlider::Create(0.0, 1.0, 1.0, 200));
	gui.add(GUIText::Create(L"等高線"));
	gui.addln(L"y", GUISlider::Create(-1.0, 1.0, 0.0, 200));

	while (System::Update())
	{
		Graphics3D::FreeCamera();
		cbParam->x = static_cast<float>(gui.slider(L"x").value);
		cbParam->y = static_cast<float>(gui.slider(L"y").value);

		Graphics3D::SetTexture(ShaderStage::Vertex, 0, textureHeight);
		Graphics3D::SetSamplerState(ShaderStage::Vertex, 0, SamplerState::ClampLinear);
		Graphics3D::SetTexture(ShaderStage::Pixel, 1, textureNormal);
		Graphics3D::BeginVS(vsTerrain);
		Graphics3D::BeginPS(psTerrain);
		Graphics3D::SetConstant(ShaderStage::Pixel, 1, cbParam);

		terrainBase.draw(textureGround);

		Graphics3D::EndPS();
		Graphics3D::EndVS();
		Graphics3D::SetTexture(ShaderStage::Pixel, 1, none);
		Graphics3D::SetTexture(ShaderStage::Vertex, 0, none);
	}
}
