#version 330

out vec4 vFragColor;

uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;

uniform sampler2D cloudTexture;
uniform float dissolveFactor;

smooth in vec3 vVaryingNormal;
smooth in vec3 vVaryingLightDir;
smooth in vec2 vVaryingTexCoord;

void main(void)
{
	vec4 vCloudSample = texture2D(cloudTexture, vVaryingTexCoord);
	if (vCloudSample.r < dissolveFactor)
		discard;

	// 点乘获取漫反射强度
	float diff = max(0.0, dot(normalize(vVaryingNormal), normalize(vVaryingLightDir)));
	// 用强度乘以漫反射颜色，将alpha值设为1.0
	vFragColor = diff * diffuseColor;
	// 添加环境光
	vFragColor += ambientColor;
	// 镜面光
	vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal)));

	float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
	if (diff != 0) {
		float fSpec = pow(spec, 128.0);
		vFragColor.rgb += vec3(fSpec, fSpec, fSpec);
	}
}
