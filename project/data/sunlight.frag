$B_SHADER_VERSION
#ifdef GL_ES
precision mediump float;
#endif

uniform vec3 Kd;
varying vec3 surfaceToCamera;
varying float y;


void main() {
	vec4 diffuse = vec4(1.0, 1.0, 1.0, 0.8);
	vec4 color = diffuse * vec4(Kd, 1.0);

	// Fog effect
	float fogFactor = pow(1.01, -distance(vec3(0.0, 0.0, 0.0), surfaceToCamera));
	color.rgb = color.rgb * fogFactor + (1.0 - fogFactor) * vec3(0.0, 0.0, 0.0);

	color.a = color.a / 1000.0 * y;
	
	gl_FragColor = clamp(color, 0.0, 1.0);
	// gl_FragColor = vec4(waterAmbient, 1.0);
}
