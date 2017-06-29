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
varying vec3 lightVectorViewSpace_0;
varying float intensityBasedOnDist_0;
uniform vec4 lightPositionViewSpace_1;
uniform float lightIntensity_1;
uniform float lightAttenuation_1;
uniform float lightRadius_1;
uniform vec3 lightDiffuseColor_1;
uniform vec3 lightSpecularColor_1;
varying vec3 lightVectorViewSpace_1;
varying float intensityBasedOnDist_1;
uniform vec4 lightPositionViewSpace_2;
uniform float lightIntensity_2;
uniform float lightAttenuation_2;
uniform float lightRadius_2;
uniform vec3 lightDiffuseColor_2;
uniform vec3 lightSpecularColor_2;
varying vec3 lightVectorViewSpace_2;
varying float intensityBasedOnDist_2;
uniform vec4 lightPositionViewSpace_3;
uniform float lightIntensity_3;
uniform float lightAttenuation_3;
uniform float lightRadius_3;
uniform vec3 lightDiffuseColor_3;
uniform vec3 lightSpecularColor_3;
varying vec3 lightVectorViewSpace_3;
varying float intensityBasedOnDist_3;
varying vec2 texCoordVarying;
varying vec3 normalVaryingViewSpace;
varying vec3 surfaceToCameraViewSpace;
varying vec3 surfaceToCamera2;
varying vec3 n;
varying vec2 causticTexCoord;
uniform sampler2D DiffuseMap;
uniform sampler2D CustomMap_1;
uniform sampler2D NormalMap;
uniform sampler2D SpecularMap;
uniform vec3 ambientColor;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
uniform float offset;
uniform vec3 waterAmbient;


void main() {
	vec4 diffuse = vec4(0.0,0.0,0.0,1.0);
	vec4 specular = vec4(0.0,0.0,0.0,0.0);
	float specularCoefficient = 0.0;
	vec3 surfaceToCamera = normalize(surfaceToCameraViewSpace);
	vec3 surfaceNormal = normalize(n);

	vec3 cameraDirection = normalize(vec3(0.0, 0.0, 1.0));

	
	float intensity = 0.0;
	
	if (intensityBasedOnDist_0 > 0.0
			&& (intensity = max(dot(surfaceNormal, normalize(surfaceToCamera2)), 0.0)) > 0.0
			&& dot(normalize(surfaceToCamera2), -cameraDirection) > 0.95) {
		intensity = clamp(intensity, 0.0, 1.0);
		diffuse += vec4(lightDiffuseColor_0 * (intensity * intensityBasedOnDist_0), 0.0);
		specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-normalize(lightVectorViewSpace_0), surfaceNormal))), Ns);
		specular += vec4(lightSpecularColor_0 * (specularCoefficient * intensity * intensityBasedOnDist_0), 0.0);
	}
	
	if (intensityBasedOnDist_1 > 0.0 && (intensity = max(dot(surfaceNormal, normalize(lightVectorViewSpace_1)), 0.0)) > 0.0){
		intensity = clamp(intensity, 0.0, 1.0);
		diffuse += vec4(lightDiffuseColor_1 * (intensity * intensityBasedOnDist_1), 0.0);
		specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-normalize(lightVectorViewSpace_1), surfaceNormal))), Ns);
		specular += vec4(lightSpecularColor_1 * (specularCoefficient * intensity * intensityBasedOnDist_1), 0.0);
	}
	
	if (intensityBasedOnDist_2 > 0.0 && (intensity = max(dot(surfaceNormal, normalize(lightVectorViewSpace_2)), 0.0)) > 0.0){
		intensity = clamp(intensity, 0.0, 1.0);
		diffuse += vec4(lightDiffuseColor_2 * (intensity * intensityBasedOnDist_2), 0.0);
		specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-normalize(lightVectorViewSpace_2), surfaceNormal))), Ns);
		specular += vec4(lightSpecularColor_2 * (specularCoefficient * intensity * intensityBasedOnDist_2), 0.0);
	}
	
	if (intensityBasedOnDist_3 > 0.0 && (intensity = max(dot(surfaceNormal, normalize(lightVectorViewSpace_3)), 0.0)) > 0.0){
		intensity = clamp(intensity, 0.0, 1.0);
		diffuse += vec4(lightDiffuseColor_3 * (intensity * intensityBasedOnDist_3), 0.0);
		specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-normalize(lightVectorViewSpace_3), surfaceNormal))), Ns);
		specular += vec4(lightSpecularColor_3 * (specularCoefficient * intensity * intensityBasedOnDist_3), 0.0);
	}
	
	diffuse = diffuse * vec4(Kd,1.0) * texture2D(DiffuseMap, texCoordVarying);
	specular = specular  * vec4(Ks, 0.0);

	// Caustic effect
	vec2 uv = causticTexCoord;
	vec2 dt;
	dt.x = sin(uv.y * 50.0 + offset / 2.0) / 250.0;
	dt.y = sin(uv.x * 10.0 + offset / 2.0) / 100.0;
	
	vec4 causticColor = texture2D(CustomMap_1, uv + dt) / 2.0;	// Reduce intensity a bit by dividing it by 2
	vec4 color = diffuse+specular+causticColor;
	// Fog effect
	float fogFactor = pow(1.01, -distance(vec3(0.0, 0.0, 0.0), surfaceToCamera2));
	color.rgb = color.rgb * fogFactor + (1.0 - fogFactor) * waterAmbient;
	
	gl_FragColor = clamp(color, 0.0, 1.0);
}