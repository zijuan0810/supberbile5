#version 150 
// multibuffer.fs
// outputs to 3 buffers: normal color, greyscale, 
// and luminance adjusted color

in vec4 vFragColor; 
in vec2 vTexCoord; 

uniform int bUseTexture;
uniform sampler2D textureUnit0; 
uniform samplerBuffer lumCurveSampler; 

void main(void) 
{ 
    vec4 vColor; 
    vec4 lumFactor; 
    
    if (bUseTexture != 0) 
	    vColor = texture(textureUnit0, vTexCoord);
    else 
	    vColor = vFragColor;
	     
	// Untouched output goes to first buffer
    // 对第一个缓冲区进行原样输出
	gl_FragData[0] = vColor;

	// Black and white to second buffer
    // 对第二个缓冲区进行灰度输出
    float grey = dot(vColor.rgb, vec3(0.3, 0.59, 0.11));
	gl_FragData[1] = vec4(grey, grey, grey, 1.0f);
	
    // 第三个缓冲区处理
	// clamp input color to make sure it is between 0.0 and 1.0
    // 对输入颜色进行截取，以确保其值在0.0至1.0之间
	vColor = clamp(vColor, 0.0f, 1.0f);
	 
    int offset = int(vColor.r * (1024 - 1)); 
    lumFactor.r = texelFetch(lumCurveSampler, offset ).r;
    
    offset = int(vColor.g * (1024 - 1)); 
    lumFactor.g = texelFetch(lumCurveSampler, offset ).r;
    
    offset = int(vColor.b * (1024 - 1)); 
    lumFactor.b = texelFetch(lumCurveSampler, offset ).r;
    
    lumFactor.a = 1.0f;

    gl_FragData[2] = lumFactor; 
}