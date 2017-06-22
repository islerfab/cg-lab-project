$B_SHADER_VERSION
#ifdef GL_ES
precision mediump float;
#endif

varying float depth;
varying vec3 surfaceToCamera;
uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;
attribute vec4 Position;

void main() {
	vec4 posViewSpace = ModelViewMatrix*Position;
	surfaceToCamera = -posViewSpace.xyz;
	depth = Position.z;
	gl_Position = ProjectionMatrix*posViewSpace;
}
