#version 330
//layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

//uniform vec2 window_size;
//uniform vec2 text_translate;
uniform mat4 text_matrix;
//uniform

void main()
{
	//vec2 vertCoords, offset;
	//offset.x = -(window_size.x / 2.0);
	//offset.y =   window_size.y / 2.0;
	//vertCoords.x = (offset.x + vertex.x + text_translate.x) / (window_size.x / 2.0);
	//vertCoords.y = (offset.y + vertex.y - text_translate.y) / (window_size.y / 2.0);
    gl_Position = text_matrix * vec4(vertex.xy, -0.1, 1.0);
    TexCoords = vertex.zw;
}  
