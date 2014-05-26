#version 330

uniform sampler3D tex_mask;
uniform sampler3D tex_volume;
//uniform bool cursor_on;
uniform vec3 cursor_ws;
uniform float cursor_size;

in vec3 fs_texcoord;
in vec3 fs_position_ws;
out vec4 display_color;

bool inCursor()
{
    return
        fs_position_ws.x >= cursor_ws.x - cursor_size &&
        fs_position_ws.x <= cursor_ws.x + cursor_size &&
        fs_position_ws.y >= cursor_ws.y - cursor_size &&
        fs_position_ws.y <= cursor_ws.y + cursor_size &&
        fs_position_ws.z >= cursor_ws.z - cursor_size &&
        fs_position_ws.z <= cursor_ws.z + cursor_size;
}

void main()
{
    bool masked = texture(tex_mask, fs_texcoord).r > 0.5;
    bool in_cursor = inCursor();
    
    vec4 color;
    float value = texture(tex_volume, fs_texcoord).r;
    if (value < 0.1) {
        discard;
    } else if (value < 0.5) {
        
        if (in_cursor) {
            color = vec4(0.0, 1.0, 0.0, 0.5);
        } else {
            color = vec4(0.0, 1.0, 0.0, 0.05);
        }
        
        if (masked) {
            color.r = 1.0;
            color.a = 0.5;
        }
        
    } else {
        
        if (masked) {
            discard;
        }
        
        if (in_cursor) {
            color = vec4(1.0, 0.0, 0.0, 0.5);
        } else {
            color = vec4(1.0, 0.0, 0.0, 0.05);

        }
        
    }
    

    
    display_color = color;
}