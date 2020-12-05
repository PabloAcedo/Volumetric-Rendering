varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;
varying vec3 v_eye_local_position; //camera local position

uniform float u_time;
uniform vec4 u_color;

uniform float u_slider;
uniform float u_factor; //factor for the sample step
uniform float u_brightness;
uniform float u_threshold;

//textures
uniform sampler3D u_vol_texture;
uniform sampler2D u_tf;

//booleans
uniform bool u_jittering;
uniform bool u_slider_bool;
uniform bool u_tf_bool;
uniform bool u_lighting; //activate/deactivate lighting
uniform bool u_vol_clip;

uniform vec4 u_clip; //to perform volume clipping

uniform vec3 u_light_pos; //light pos
uniform vec3 u_light_color; //light color


//solve jittering
vec3 random_offset(vec3 sample_position, vec3 step){
	float pos_offset = fract(sin(dot(gl_FragCoord.xy,vec2(12.9898,78.233)))*43758.5453);//pseudorandom
	return sample_position + step*pos_offset; //new sample position
}

vec3 compute_gradient(sampler3D texture, vec3 pos, float h){
	//following the formula
	float x = texture3D(texture, pos + vec3(h,0,0)).x - texture3D(texture, pos- vec3(h,0,0)).x;
	float y = texture3D(texture, pos + vec3(0,h,0)).x - texture3D(texture, pos - vec3(0,h,0)).x;
	float z = texture3D(texture, pos + vec3(0,0,h)).x - texture3D(texture, pos - vec3(0,0,h)).x;
	return vec3(x,y,z)/(2*h);
}

void main()
{	
	//eye in local space
	vec3 camera_local_pos = v_eye_local_position;

	//vertex local coordinate (init position)
	vec3 sample_position = v_position;

	//compute ray direction
	vec3 ray_direction = normalize(sample_position - camera_local_pos);

	//sample step (small sample of the ray direction)
	vec3 sample_step = ray_direction/u_factor;

	if(u_jittering)
		sample_position = random_offset(sample_position,sample_step);

	vec4 final_color = vec4(0.0); //init color

	vec3 N = vec3(0.0);

	vec4 sample_color = vec4(0.0);


	//iterate
	for(int i = 0; i< 200; i++){

		//volume clipping
		if( u_vol_clip && (dot(vec4(sample_position,0.0), u_clip)+u_clip.w) > 0.0){
			sample_position += sample_step;

			//check early termination
			if(final_color.a >= 0.99){ 
				break;
			}		

			if(sample_position.x <= -1.0 || sample_position.x >= 1.0 || sample_position.y <= -1.0 || 
				sample_position.y >= 1.0 || sample_position.z <= -1.0 || sample_position.z >= 1.0){
				break;
			}

			//skip volume parts
			continue;
		}

		//slider
		if(u_slider_bool)
			sample_position.z = u_slider*2.0 - 1.0;
		
		vec3 tex_coord = (sample_position+1.0)/(2.0); //local to tex coords

		float d = texture3D(u_vol_texture, tex_coord).x; //take density sample
		
		//density thresholding
		if(d < u_threshold){
			sample_position += sample_step;
			if(final_color.a >= 0.99){ 
				break;
			}		

			if(sample_position.x <= -1.0 || sample_position.x >= 1.0 || sample_position.y <= -1.0 || 
				sample_position.y >= 1.0 || sample_position.z <= -1.0 || sample_position.z >= 1.0){
				break;
			}
			continue;
		}
		
		//simple color or tf 
		if(u_tf_bool)
			sample_color = texture2D(u_tf,vec2(d,1));
		else
			sample_color = vec4(d);
		
		//compute final color	
		sample_color.xyz *= sample_color.a;
		final_color += length(sample_step)*(1.0 -final_color.a)*sample_color;

		if(u_lighting){

			//debug
			vec3 light_pos = u_light_pos*-1.0;
			//light_pos.x = u_light_pos.x*-1.0;
			//debug
			
			vec3 L = normalize(light_pos - sample_position); //light vector (to light)
			float h = length(sample_step); //h constant factor
			vec3 N = normalize(compute_gradient(u_vol_texture, tex_coord, h)); //normal
			float NdotL = (dot(N,L)+1.0)/(2.0); //shading
			sample_color.xyz = sample_color.xyz*vec3(NdotL)*u_light_color; //compute sample color
			sample_color.a = 1.0; //maximum opacity
			final_color += sample_color*(1.0-final_color.a); //combine with final color	
		}

		sample_position += sample_step;
		
		//early termination		
		if(final_color.a >= 0.99){ 
			break;
		}		

		if(sample_position.x <= -1.0 || sample_position.x >= 1.0 || sample_position.y <= -1.0 || 
		sample_position.y >= 1.0 || sample_position.z <= -1.0 || sample_position.z >= 1.0){
			break;
		}
	}


	gl_FragColor = final_color*u_color*u_brightness;
}
