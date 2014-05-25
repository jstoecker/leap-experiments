#version 330

uniform sampler3D tex_mask;
uniform sampler3D tex_volume;
//uniform bool cursor_on;
uniform vec3 cursor_ws;
uniform float cursor_size;

in vec3 fs_texcoord;
in vec3 fs_position_ws;
out vec4 display_color;

void main()
{
	if (texture(tex_mask, fs_texcoord).r > 0.5) {
		discard;
	}
	
    vec4 color = vec4(1.0, 0.0, 1.0, 0.01);
    
    if (fs_position_ws.x >= cursor_ws.x - cursor_size &&
        fs_position_ws.x <= cursor_ws.x + cursor_size &&
        fs_position_ws.y >= cursor_ws.y - cursor_size &&
        fs_position_ws.y <= cursor_ws.y + cursor_size &&
        fs_position_ws.z >= cursor_ws.z - cursor_size &&
        fs_position_ws.z <= cursor_ws.z + cursor_size)
    {
        color = vec4(1.0, 0.0, 0.0, 0.2);
    }
    
    display_color = color;
}