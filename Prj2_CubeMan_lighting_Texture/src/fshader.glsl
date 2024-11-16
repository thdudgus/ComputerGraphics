#version 430

in vec4 fragPos;
in vec4 color;
in vec4 normal;
in vec2 texCoord;

out vec4  fColor;

uniform sampler2D sphereTexture;
uniform mat4 mPVM;
uniform mat4 mProject;
uniform mat4 mView;
uniform mat4 mModel;

void main() 
{ 
    //fColor = color;

	vec4 L = normalize(vec4(3, 3, 5, 0));
	float kd = 0.8, ks = 1.0, ka = 0.8, shininess = 60;
	vec4 Id = color;
	vec4 Is = vec4(1, 1, 1, 1);
	vec4 Ia = color;

	// ambient
	float ambient = ka;

	// diffuse
	vec4 N = normalize(normal);
	float diff = kd * clamp(dot(N, L), 0, 1);

	// specular
	vec4 viewPos = inverse(mView) * vec4(0, 0, 0, 1);
	vec4 V =  normalize(viewPos - fragPos);
	vec4 R = reflect(-L, N);
	float spec = ks * pow(clamp(dot(V, R), 0, 1), shininess);

	fColor = ambient * Ia + diff * Id + spec * Is;
	// if (isTexture == 1) 
	fColor = fColor * texture( sphereTexture, texCoord ).rgba;


}