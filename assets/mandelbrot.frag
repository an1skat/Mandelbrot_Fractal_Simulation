uniform vec2 resolution;

uniform float Rm;
uniform float RM;
uniform float Im;
uniform float IM;

uniform int maxIt;

const float EscapeRadius = 4.0;

vec2 complexFromPixel(vec2 pixel) {
	vec2 uv = pixel / resolution;
	return vec2(
		mix(Rm, RM, uv.x),
		mix(IM, Im, uv.y)
	);
}

bool isDefinitelyInside(vec2 c) {
	float x = c.x - 0.25;
	float r = x*x + c.y*c.y;

	bool insideMainCardiod = r * (r + x) <= 0.25 * c.y * c.y;
	bool insidePeriodTwoBulb = (c.x + 1.0) * (c.x + 1.0) + c.y * c.y <= 0.0625;

	return insideMainCardiod || insidePeriodTwoBulb;
}

int calcIterations(vec2 c, out float radius2) {
	float zx = 0.0;
	float zy = 0.0;
	float zx2 = 0.0;
	float zy2 = 0.0;
	int it = 0;

	while (zx2 + zy2 <= EscapeRadius && it < maxIt) {
		zy = 2.0 * zx * zy + c.y;
    zx = zx2 - zy2 + c.x;

		zx2 = zx * zx;
		zy2 = zy * zy;

		++it;
	}

	radius2 = zx2 + zy2;
	return it;
}

vec3 palette(float smoothIt) {
	float r = mod(10.0 * smoothIt, 255.0) / 255.0;
	float g = mod(5.0 * smoothIt, 255.0) / 255.0;
	float b = smoothIt / float(maxIt);

	return vec3(r, g, b);
}

void main() {
	vec2 c = complexFromPixel(gl_FragCoord.xy);

	if (isDefinitelyInside(c)) {
		gl_FragColor = vec4(vec3(0.0), 1.0);
		return;
	}

	float radius2 = 0.0;
	int it = calcIterations(c, radius2);

	if (it >= maxIt) {
		gl_FragColor = vec4(vec3(0.0), 1.0);
		return;
	}

	float smoothIt = float(it) - log2(log2(radius2));
	gl_FragColor = vec4(palette(smoothIt), 1.0);
}