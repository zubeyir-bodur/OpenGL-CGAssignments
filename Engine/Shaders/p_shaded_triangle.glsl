#version 150 core

// Vertex shader
#ifdef COMPILING_VS
layout(location = 0) in vec4 v_position;
layout(location = 1) in vec4 v_normal;

out vec3 N, L, E;

uniform mat4 u_MV;
uniform mat4 u_P;
uniform vec4 u_light_position;

void main()
{
	vec3 vertex_pos = (u_MV * v_position).xyz;

	mat4 normal_matrix = transpose(inverse(u_MV));
	if(u_light_position.w == 0.0)
	{
		L = normalize(u_light_position.xyz);
	}
    else
	{
		L = normalize(u_light_position.xyz - vertex_pos);
	}
	E =  -normalize(vertex_pos);
    N = normalize(vec3(normal_matrix * v_normal).xyz);

	gl_Position = u_P * u_MV * v_position;
}

// Pixel (fragment) shader
#elif defined (COMPILING_FS)
in vec3 N, L, E;
uniform vec4 u_color; 
uniform vec4 u_ambient;	 
uniform vec4 u_diffuse;	 
uniform vec4 u_specular; 
uniform float u_shininess;

void main()
{    
    vec4 fragment_color;
    vec3 H = normalize( L + E );
    vec4 ambient_color = u_ambient;

    float Kd = max( dot(L, N), 0.0 );
    vec4  diffuse_color = Kd*u_diffuse;

    float Ks = pow( max(dot(N, H), 0.0), u_shininess );
    vec4  specular_color = Ks * u_specular;
    
    if( dot(L, N) < 0.0 ) 
	{
		specular_color = vec4(0.0, 0.0, 0.0, 1.0);
	}

    fragment_color = (ambient_color + diffuse_color + specular_color) * u_color;
    fragment_color.a = 1.0;
	
	gl_FragColor = fragment_color;
}
#endif