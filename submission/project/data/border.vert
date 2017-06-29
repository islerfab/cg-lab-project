$B_SHADER_VERSION
#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;
attribute vec4 Position;

void main() {
	vec4 posViewSpace = ModelViewMatrix*Position;
	gl_Position = ProjectionMatrix*posViewSpace;
}
