#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

//*******************************************************************
// stb_truetype object
stbtt_fontinfo font_info;			// font information

GLuint		VAO, VBO;				// vertex array for text objects
GLuint		program_text;			// GPU program for text render

static const char*	font_path = "C:/Windows/Fonts/consola.ttf";
static const char*	vert_text_path = "../bin/shaders/text.vert";		// text vertex shaders
static const char*	frag_text_path = "../bin/shaders/text.frag";		// text fragment shaders

struct stbtt_char_t
{
	GLuint	textureID;				// ID handle of the glyph texture
	ivec2	size;					// Size of glyph
	ivec2	bearing;				// Offset from baseline to left/top of glyph
	GLuint	advance;				// Horizontal offset to advance to next glyph
};
std::map<GLchar, stbtt_char_t> stbtt_char_list;

void create_font_textures()
{
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	for (GLubyte c = 0; c < 128; c++)
	{	
		// Font size (pixel) to scale
		float font_scale = stbtt_ScaleForPixelHeight( &font_info, 48 );

		// Get bitmap and bitmap info of character
		int width, height, offset_x, offset_y;
		unsigned char* bitmap = stbtt_GetCodepointBitmap( 
			&font_info, 									// Font information
			0.0f, 											// Scale in x axis
			font_scale, 									// Scale in y axis
			c, 												// Character
			&width, 										// Width of bitmap
			&height, 										// Height of bitmap
			&offset_x, 										// Left bearing of bitmap
			&offset_y 										// Top bearing of bitmap (negative value)
		);

		// Generate texture
		GLuint texture_text;
		glGenTextures( 1, &texture_text );
		glBindTexture( GL_TEXTURE_2D, texture_text );
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			width,
			height,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			bitmap
		);

		// Release bitmap
		stbtt_FreeBitmap( bitmap, font_info.userdata );

		// Set texture options
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		// Get advance in x axis
		int advance_width = 0;
		stbtt_GetCodepointHMetrics( &font_info, c, &advance_width, nullptr );

		// Now store character for later use
		stbtt_char_t character =
		{
			texture_text,
			ivec2( width, height ),
			ivec2( offset_x, -offset_y ),			// flip y axis
			GLuint( advance_width * font_scale )
		};
		stbtt_char_list.insert( std::pair<GLchar, stbtt_char_t>( c, character ) );
	}

	printf( "Font textures created well using stb_truetype\n\n" );
}

void text_init()
{
	// Read and check font file
	FILE* font_file = fopen( font_path, "rb" );
	//if (!font_file) { printf( "Font file does not exists.\n" ); exit(-1); }

	// Get size of file
	fseek( font_file, 0, SEEK_END );
	const unsigned int size = ftell( font_file );
	fseek( font_file, 0, SEEK_SET );

	// Create font buffer to store
	unsigned char* font_buffer = new unsigned char[size];

	// Read and close file
	std::size_t read_size = fread( font_buffer, 1, size, font_file );
	assert( read_size == size );
	fclose( font_file );

	// Initialize stb truetype
	int result = stbtt_InitFont( &font_info, font_buffer, 0 );
	//if (!result) { printf( "Failed to initialize stb_truetype.\n" );  exit(-1); }

	create_font_textures();

	// Release buffer
	delete[] font_buffer;

	if (!(program_text = cg_create_program( vert_text_path, frag_text_path ))) { glfwTerminate(); return; }

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Vertices below contain x, y, texcoord x, texcoord y
	GLfloat vertices[6][4] = {
			{ 0, 1, 0.0, 0.0 },
			{ 0, 0, 0.0, 1.0 },
			{ 1, 0, 1.0, 1.0 },

			{ 0, 1, 0.0, 0.0 },
			{ 1, 0, 1.0, 1.0 },
			{ 1, 1, 1.0, 0.0 },
	};
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void render_text( std::string text, GLint _x, GLint _y, GLfloat scale, vec4 color )
{
	// Activate corresponding render state	
	extern ivec2 window_size;
	GLfloat x = GLfloat(_x);
	GLfloat y = GLfloat(_y);
	glUseProgram(program_text);
	glUniform4f(glGetUniformLocation(program_text, "textColor"), color.r, color.g, color.b, color.a);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	mat4 text_offset = mat4( 1/(window_size.x/2.0f), 0.0f, 0.0f,-1.0f,		// view space conversion
						 0.0f, 1/(window_size.y/2.0f), 0.0f, 1.0f,
						 0.0f, 0.0f, 1.0f, 0.0f,
						 0.0f, 0.0f, 0.0f, 1.0f
						);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		stbtt_char_t ch = stbtt_char_list[*c];

		mat4 text_size		= mat4(	scale * float(ch.size.x),0.0f,0.0f,0.0f,
									0.0f,scale * float(ch.size.y),0.0f,0.0f,
									0.0f,0.0f,1.0f,0.0f,
									0.0f,0.0f,0.0f,1.0f
									);

		mat4 text_translate = mat4( 1.0f,0.0f,0.0f,	x  + scale * float(ch.bearing.x),
									0.0f,1.0f,0.0f,	-y + scale * float(-(ch.size.y - ch.bearing.y)),
									0.0f,0.0f,1.0f,	0.0f,
									0.0f,0.0f,0.0f,	1.0f
									);

		mat4 text_matrix = mat4();
		
		text_matrix = text_translate * text_size * text_matrix;
		//text_matrix = text_translate * text_matrix;
		text_matrix = text_offset * text_matrix;
		glUniformMatrix4fv(glGetUniformLocation(program_text, "text_matrix"), 1, GL_TRUE, text_matrix);

		//// Render glyph texture over quad
 		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Now advance cursors for next glyph
		x += ch.advance * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}