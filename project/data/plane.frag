$B_SHADER_VERSION
#ifdef GL_ES
precision mediump float;
#endif

uniform vec4 lightPositionViewSpace_0;
uniform float lightIntensity_0;
uniform float lightAttenuation_0;
uniform float lightRadius_0;
uniform vec3 lightDiffuseColor_0;
uniform vec3 lightSpecularColor_0;
varying vec3 lightVectorTangentSpace_0;
varying float intensityBasedOnDist_0;
uniform vec4 lightPositionViewSpace_1;
uniform float lightIntensity_1;
uniform float lightAttenuation_1;
uniform float lightRadius_1;
uniform vec3 lightDiffuseColor_1;
uniform vec3 lightSpecularColor_1;
varying vec3 lightVectorTangentSpace_1;
varying float intensityBasedOnDist_1;
uniform vec4 lightPositionViewSpace_2;
uniform float lightIntensity_2;
uniform float lightAttenuation_2;
uniform float lightRadius_2;
uniform vec3 lightDiffuseColor_2;
uniform vec3 lightSpecularColor_2;
varying vec3 lightVectorTangentSpace_2;
varying float intensityBasedOnDist_2;
uniform vec4 lightPositionViewSpace_3;
uniform float lightIntensity_3;
uniform float lightAttenuation_3;
uniform float lightRadius_3;
uniform vec3 lightDiffuseColor_3;
uniform vec3 lightSpecularColor_3;
varying vec3 lightVectorTangentSpace_3;
varying float intensityBasedOnDist_3;
uniform sampler2D CustomMap_1;
uniform sampler2D DiffuseMap;
varying vec3 surfaceToCameraTangentSpace;
varying vec3 n;
varying vec3 surfaceToCamera;
varying vec2 causticTexCoord;
uniform vec3 Kd;
uniform float offset;
uniform vec3 waterAmbient;
varying vec4 texCoordVarying;
uniform float Ns;

void main() {
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 surfaceNormal = normalize(n);

	// The camera in view space goes towards the z-axis
	vec3 cameraDirection = normalize(vec3(0.0, 0.0, 1.0));

	float intensity = 0.0;

	// HEAD LAMP
	// IF it's in distance (as defined in the light source)
	if (intensityBasedOnDist_0 > 0.0
			// AND the surface points towards us
			&& (intensity = max(dot(surfaceNormal, normalize(surfaceToCamera)), 0.0)) > 0.0
			// AND the point is in the cone of the headlamp
			&& dot(normalize(surfaceToCamera), -cameraDirection) > 0.95) {
		intensity = clamp(intensity, 0.0, 1.0);
		diffuse += vec4(lightDiffuseColor_0 * (intensity * intensityBasedOnDist_0), 0.0);
		float specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-normalize(lightVectorTangentSpace_1), surfaceNormal))), Ns);
		specular += vec4(lightSpecularColor_1 * (specularCoefficient * intensity * intensityBasedOnDist_1), 0.0);
	}
	
	// Second Light
	if (intensityBasedOnDist_1 > 0.0 && (intensity = max(dot(surfaceNormal, normalize(lightVectorTangentSpace_1)), 0.0)) > 0.0){
		intensity = clamp(intensity, 0.0, 1.0);
		diffuse += vec4(lightDiffuseColor_1 * (intensity * intensityBasedOnDist_1), 0.0);
	}

	// Third Light
	if (intensityBasedOnDist_2 > 0.0 && (intensity = max(dot(surfaceNormal, normalize(lightVectorTangentSpace_2)), 0.0)) > 0.0){
		intensity = clamp(intensity, 0.0, 1.0);
		diffuse += vec4(lightDiffuseColor_2 * (intensity * intensityBasedOnDist_2), 0.0);
	}

	// Fourth Light
	if (intensityBasedOnDist_3 > 0.0 && (intensity = max(dot(surfaceNormal, normalize(lightVectorTangentSpace_3)), 0.0)) > 0.0){
		intensity = clamp(intensity, 0.0, 1.0);
		diffuse += vec4(lightDiffuseColor_3 * (intensity * intensityBasedOnDist_3), 0.0);
	}

	// Caustic effect
	vec2 uv = causticTexCoord;
	vec2 dt;
	dt.x = sin(uv.y * 50.0 + offset / 2.0) / 250.0;
	dt.y = sin(uv.x * 10.0 + offset / 2.0) / 100.0;
	
	vec3 causticColor = texture2D(CustomMap_1, uv + dt).xyz / 2.0;	// Reduce intensity a bit by dividing it by 2
	vec3 textureColor = texture2D(DiffuseMap, texCoordVarying.st).xyz;

	vec4 color = vec4(textureColor, 1.0) + vec4(causticColor, 1.0) + specular;

	// Fog effect
	float fogFactor = pow(1.01, -distance(vec3(0.0, 0.0, 0.0), surfaceToCamera));
	color.rgb = color.rgb * fogFactor + (1.0 - fogFactor) * waterAmbient;

	//color = texture2DProj(DiffuseMap, texCoordVarying);
	
	gl_FragColor = clamp(color, 0.0, 1.0);
	// gl_FragColor = vec4(waterAmbient, 1.0);
}
