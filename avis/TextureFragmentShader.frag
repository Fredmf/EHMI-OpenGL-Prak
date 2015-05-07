// Interpolated values from the vertex shaders
varying vec2 UV;
varying vec3 Position_worldspace;
varying vec3 Normal_cameraspace;
varying vec3 EyeDirection_cameraspace;
varying vec3 LightDirection_a_cameraspace;
varying vec3 LightDirection_b_cameraspace;
varying vec3 LightDirection_c_cameraspace;
varying vec3 LightDirection_d_cameraspace;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform mat4 MV;
uniform vec3 LightPosition_a_worldspace;
uniform vec3 LightPosition_b_worldspace;
uniform vec3 LightPosition_c_worldspace;
uniform vec3 LightPosition_d_worldspace;

void main(){

// Light emission properties
// You probably want to put them as uniforms

vec3 LightColor_a = vec3(0.87,0.0,0.28);
float LightPower_a = 250.0;

vec3 LightColor_b = vec3(1.0,0.0,0.0);
float LightPower_b = 250.0;

vec3 LightColor_c = vec3(0.0,1.0,0.0);
float LightPower_c = 250.0;

vec3 LightColor_d = vec3(0.0,0.0,1.0);
float LightPower_d = 250.0;

// Material properties
vec3 MaterialDiffuseColor = texture2D( myTextureSampler, UV ).rgb;
vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

// Distance to the light
float distance_a = length( LightPosition_a_worldspace - Position_worldspace );
float distance_b = length( LightPosition_b_worldspace - Position_worldspace );
float distance_c = length( LightPosition_c_worldspace - Position_worldspace );
float distance_d = length( LightPosition_d_worldspace - Position_worldspace );

// Normal of the computed fragment, in camera space
vec3 n = normalize( Normal_cameraspace );
// Direction of the light (from the fragment to the light)
vec3 l_a = normalize( LightDirection_a_cameraspace );
vec3 l_b = normalize( LightDirection_b_cameraspace );
vec3 l_c = normalize( LightDirection_c_cameraspace );
vec3 l_d = normalize( LightDirection_d_cameraspace );
// Cosine of the angle between the normal and the light direction,
// clamped above 0
// - light is at the vertical of the triangle -> 1
// - light is perpendicular to the triangle -> 0
// - light is behind the triangle -> 0
float cosTheta_a = clamp( dot( n,l_a ), 0.0, 1.0 );
float cosTheta_b = clamp( dot( n,l_b ), 0.0, 1.0 );
float cosTheta_c = clamp( dot( n,l_c ), 0.0, 1.0 );
float cosTheta_d = clamp( dot( n,l_d ), 0.0, 1.0 );

// Eye vector (towards the camera)
vec3 E = normalize(EyeDirection_cameraspace);
// Direction in which the triangle reflects the light
vec3 R_a = reflect(-l_a,n);
vec3 R_b = reflect(-l_b,n);
vec3 R_c = reflect(-l_c,n);
vec3 R_d = reflect(-l_d,n);
// Cosine of the angle between the Eye vector and the Reflect vector,
// clamped to 0
// - Looking into the reflection -> 1
// - Looking elsewhere -> < 1
float cosAlpha_a = clamp( dot( E,R_a ), 0.0, 1.0 );
float cosAlpha_b = clamp( dot( E,R_b ), 0.0, 1.0 );
float cosAlpha_c = clamp( dot( E,R_c ), 0.0, 1.0 );
float cosAlpha_d = clamp( dot( E,R_d ), 0.0, 1.0 );

vec3 amb = MaterialAmbientColor;
vec3 dif_a = MaterialDiffuseColor * LightColor_a * LightPower_a * cosTheta_a / (distance_a*distance_a);
vec3 dif_b = MaterialDiffuseColor * LightColor_b * LightPower_b * cosTheta_b / (distance_b*distance_b);
vec3 dif_c = MaterialDiffuseColor * LightColor_c * LightPower_c * cosTheta_c / (distance_c*distance_c);
vec3 dif_d = MaterialDiffuseColor * LightColor_d * LightPower_d * cosTheta_d / (distance_d*distance_d);
vec3 spec_a = MaterialSpecularColor * LightColor_a * LightPower_a * pow(cosAlpha_a, 5.0) / (distance_a*distance_a);
vec3 spec_b = MaterialSpecularColor * LightColor_b * LightPower_b * pow(cosAlpha_b, 5.0) / (distance_b*distance_b);
vec3 spec_c = MaterialSpecularColor * LightColor_c * LightPower_c * pow(cosAlpha_c, 5.0) / (distance_c*distance_c);
vec3 spec_d = MaterialSpecularColor * LightColor_d * LightPower_d * pow(cosAlpha_d, 5.0) / (distance_d*distance_d);

vec3 dif = dif_a + dif_b + dif_c + dif_d;
//vec3 spec = spec0 + spec1 + spec2 + spec3;

gl_FragColor.a = 0.5;
gl_FragColor.rgb = amb + dif + spec_a;

/* gl_FragColor.rgb =
// Ambient : simulates indirect lighting
MaterialAmbientColor +
// Diffuse : "color" of the object
MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
// Specular : reflective highlight, like a mirror
MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
*/
}
