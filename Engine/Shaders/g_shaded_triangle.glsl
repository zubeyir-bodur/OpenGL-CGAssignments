#version 150 core

// Vertex shader
#ifdef COMPILING_VS
layout(location = 0) in vec4 v_position;
layout(location = 1) in vec4 v_normal;
layout(location = 2) in vec2 v_parametric_coords;
layout(location = 3) in vec4 v_tangent_vector;

out vec4 f_color;

uniform mat4 u_MV;
uniform mat4 u_P;
uniform vec4 u_light_position;
uniform vec4 u_ambient;	 
uniform vec4 u_diffuse;	 
uniform vec4 u_specular; 
uniform float u_shininess;
uniform sampler2D u_bump_texture;

void main()
{
	vec3 vertex_pos = (u_MV * v_position).xyz;
	vec3 N, L, E, H;
	vec3 T, B;

	mat4 normal_matrix = transpose(inverse(u_MV));
    N = normalize(vec3(normal_matrix * v_normal).xyz);
	T  = normalize(vec3(normal_matrix * v_tangent_vector).xyz);
    B = cross(N, T);
	if(u_light_position.w == 0.0)
	{		
		L.x = dot(T, u_light_position.xyz);
		L.y = dot(B, u_light_position.xyz);
		L.z = dot(N, u_light_position.xyz);
	}
    else
	{		
		L.x = dot(T, u_light_position.xyz - vertex_pos);
		L.y = dot(B, u_light_position.xyz - vertex_pos);
		L.z = dot(N, u_light_position.xyz - vertex_pos);
	}
	L = normalize(L);
	
	E.x = dot(T, -vertex_pos);
    E.y = dot(B, -vertex_pos);
    E.z = dot(N, -vertex_pos);
	E = normalize(E);

	// Convert N to texture space
	vec4 N_ = texture2D(u_bump_texture, v_parametric_coords);
	N = normalize(2.0*N_.xyz - 1.0);
	H = normalize(L + E);

	// Compute terms in the illumination equation
    vec4 ambient = u_ambient;

    float Kd = max( dot(L, N), 0.0 );
    vec4  diffuse = Kd*u_diffuse;

    float Ks = pow( max(dot(N, H), 0.0), u_shininess );
    vec4  specular = Ks * u_specular;
    if( dot(L, N) < 0.0 ) 
    {
	    specular = vec4(0.0, 0.0, 0.0, 1.0);
    } 

    f_color = ambient + diffuse + specular;
    f_color.a = 1.0;

	gl_Position = u_P * u_MV * v_position;
}

// Pixel (fragment) shader
#elif defined (COMPILING_FS)

in vec4 f_color;

uniform vec4 u_color;

void main()
{    
    gl_FragColor = f_color * u_color;
}
#endif