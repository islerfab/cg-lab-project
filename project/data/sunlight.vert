$B_SHADER_VERSION
#ifdef GL_ES
precision mediump float;
#endif

varying vec3 surfaceToCamera;
varying float y;
uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;
attribute vec4 Position;

void main() {
	vec4 posViewSpace = ModelViewMatrix*Position;
	surfaceToCamera = -posViewSpace.xyz;
	y = Position.y;
	gl_Position = ProjectionMatrix*posViewSpace;
}
