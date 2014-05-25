#version 330

uniform sampler3D tex_mask;
uniform sampler3D tex_volume;
#uniform bool cursor_on;
#uniform vec3 cursor_ws;

in vec3 fs_texcoord;
in vec3 fs_position_ws;
out vec4 display_color;

void main()
{
	if (texture(tex_mask, fs_texcoord).r > 0.5) {
		discard;
	}
	
    // get raw value stored in volume (normalized to [0, 1])
    float value = texture(tex_volume, fs_texcoord).r;

    display_color = vec4(1.0, 0.0, 1.0, 0.1);
}