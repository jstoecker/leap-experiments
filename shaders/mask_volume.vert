#version 330

uniform vec3 bounds_min;
uniform vec3 bounds_size;
uniform mat4 model_view_projection;

layout (location=0) in vec4 vs_position;
out vec3 fs_texcoord;
out vec3 fs_position_ws;

void main()
{
    fs_texcoord = (vs_position.xyz - bounds_min) / bounds_size;
    gl_Position = model_view_projection * vs_position;
	fs_voxel_position_ws = vs_position.xyz;
}
