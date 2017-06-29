$B_SHADER_VERSION
#ifdef GL_ES
precision mediump float;
#endif

uniform vec3 waterAmbient;

void main() {

	gl_FragColor = vec4(waterAmbient, 1.0);

}
