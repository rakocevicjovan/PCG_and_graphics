//from shadertoy, I don't really understand this tbh just for educational purposes...

float3 blackbody(float t)
{
	t *= 2000.f;

	float u = (0.860117757 + 1.54118254e-4 * t + 1.28641212e-7 * t*t)
		/ (1.0 + 8.42420235e-4 * t + 7.08145163e-7 * t*t);

	float v = (0.317398726 + 4.22806245e-5 * t + 4.20481691e-8 * t*t)
		/ (1.0 - 2.89741816e-5 * t + 1.61456053e-7 * t*t);

	float x = 3. * u / (2.0*u - 8.0*v + 4.0);
	float y = 2. * v / (2.0*u - 8.0*v + 4.0);
	float z = 1. - x - y;

	float Y = 1.0;
	float X = Y / y * x;
	float Z = Y / y * z;

	float3x3 XYZtoRGB = float3x3
		(
			3.2404542, -1.5371385, -0.4985314,
			-0.9692660, 1.8760108, 0.0415560,
			0.0556434, -0.2040259, 1.0572252
			);

	return max(float3(0., 0., 0.), mul(float3(X, Y, Z), XYZtoRGB) * pow(t * 0.0004, 4.0));
}