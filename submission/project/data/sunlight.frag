$B_SHADER_VERSION
#ifdef GL_ES
precision mediump float;
#endif

uniform vec3 Kd;
uniform vec3 waterAmbient;
varying vec3 surfaceToCamera;
varying float depth;


void main() {
	vec4 diffuse = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 color = diffuse * vec4(Kd, 1.0);

	// Fog effect
	float fogFactor = pow(1.005, -distance(vec3(0.0, 0.0, 0.0), surfaceToCamera));
	color.rgb = color.rgb * fogFactor + (1.0 - fogFactor) * waterAmbient;

	// Make it fade with the depth
	color.a *= abs(depth) / 500.0;

	gl_FragColor = clamp(color, 0.0, 1.0);
}
