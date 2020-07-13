#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "circle.h"			// circle class definition
#include <fstream>
#include <queue>
#include<conio.h>
#include <Windows.h>
#include<stdio.h>


//*******************************************************************
// forward declarations for freetype text
void text_init();
void render_text(std::string text, GLint x, GLint y, GLfloat scale, vec4 color);

//*************************************
// global constants
static const char*	window_name = "cgcirc";
static const char*	vert_shader_path = "../bin/shaders/circ.vert";
static const char*	frag_shader_path = "../bin/shaders/circ.frag";
uint				NUM_TESS = 72 * 36 * 6;		// initial tessellation factor of the circle as a polygon

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2( 1024, 576 );	// initial window size

//*************************************
// OpenGL objects
GLuint	program			= 0;	// ID holder for GPU program
GLuint	vertex_buffer	= 0;	// ID holder for vertex buffer
GLuint	index_buffer	= 0;	// ID holder for index buffer
 
//*************************************
// global variables
int		frame = 0;						// index of rendering frames
float	t = 0.0f;						// current simulation parameter
int		color = 0;			// use circle's color?
bool	b_index_buffer = true;			// use index buffering?
bool	b_wireframe = false;
bool	rotating = true;
bool	start = false;
bool	quit = false;
std::queue<int>	map;
int	map_size = 0;

std::vector<step_t>	steps;
auto	main_cube = std::move(create_cube());
struct { bool add=false, sub=false; operator bool() const { return add||sub; } } b; // flags of keys for smooth changes

//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_cube_vertices;	// host-side vertices


//*******************************************************************
// scene object
mesh* pMesh = nullptr;
camera		cam;

//*************************************
void update()
{
	// update projection matrix
	cam.aspect_ratio = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dnear, cam.dfar);

	float t = float(glfwGetTime());
	float scale = 1.0f + float(cos(t * 1.5f)) * 0.05f;
	mat4 model_matrix = mat4::scale(scale, scale, scale);

	// tricky aspect correction matrix for non-square window
	float aspect = window_size.x/float(window_size.y);
	mat4 aspect_matrix =
	{
		min(1 / aspect,1.0f), 0, 0, 0,
		0, min(aspect,1.0f), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	// update common uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "view_matrix");			if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);
	uloc = glGetUniformLocation(program, "model_matrix");			if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, model_matrix);
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// render texts
	if (!start) {
		render_text("Ddong Game!", 100, 100, 1.0f, vec4(0.9f, 0.9f, 0.9f, 1.0f));
		render_text("Press right when floor is green", 100, 140, 0.5f, vec4(50 / 255.0f, 120 / 225.0f, 20 / 225.0f, 0.7f));
		render_text("Press left when floor is red", 100, 170, 0.5f, vec4(148 / 255.0f, 20 / 225.0f, 20 / 225.0f, 1.0f));
		render_text("Press space bar when there is a square", 100, 200, 0.5f, vec4(107 / 255.0f, 236 / 225.0f, 219 / 225.0f, 1.0f));
		render_text("Press any key to start/restart", 100, 520, 0.5f, vec4(0.9f, 0.9f, 0.9f, 1.0f));
	}
	render_text("Score:", 800, 520, 0.5f, vec4(107 / 255.0f, 236 / 225.0f, 219 / 225.0f, 1.0f));
	render_text(std::to_string(main_cube.score), 900, 520, 0.5f, vec4(107 / 255.0f, 236 / 225.0f, 219 / 225.0f, 1.0f));

	// notify GL that we use our own program and buffers
	glUseProgram( program );
	if(vertex_buffer)	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	if(index_buffer)	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

	// bind vertex attributes to your shader program
	cg_bind_vertex_attributes( program );
	
	if (start) {
	
		t += 0.005f;
		float tmp = main_cube.roll(&steps, &map, &start);
		cam.eye.x = -100 + tmp;
		cam.at.x = tmp;
		cam.view_matrix = mat4::look_at(cam.eye, cam.at, cam.up);
	}

	main_cube.update(t);
	// update per-circle uniforms
	GLint uloc;
	uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, main_cube.color);	// pointer version
	uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, main_cube.model_matrix);

	// per-circle draw calls
	if (b_index_buffer)	glDrawElements(GL_TRIANGLES, NUM_TESS, GL_UNSIGNED_INT, nullptr);
	else				glDrawArrays(GL_TRIANGLES, 0, NUM_TESS); // NUM_TESS = N

	for (auto& c : steps) {
		c.update(t);

		// update per-circle uniforms
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, c.color);	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, c.model_matrix);

		// per-circle draw calls
		if (b_index_buffer)	glDrawElements(GL_TRIANGLES, NUM_TESS, GL_UNSIGNED_INT, nullptr);
		else				glDrawArrays(GL_TRIANGLES, 0, NUM_TESS); // NUM_TESS = N
	}

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "\n" );
}

std::vector<vertex> create_cube_verticese(uint N, cube_t main_cube, std::vector<step_t> steps) {
	std::vector<vertex> v;	// origin
	
	// Main cube
	v.push_back({ vec3(-1.0f, -1.0f, -1.0f), vec3(1,0,0)});
	v.push_back({ vec3(1.0f, -1.0f, -1.0f), vec3(1,0,0)});
	v.push_back({ vec3(-1.0f, 1.0f, -1.0f), vec3(1,0,0)});;
	v.push_back({ vec3(1.0f, 1.0f, -1.0f), vec3(1,0,0)});
	v.push_back({ vec3(-1.0f, -1.0f, 1.0f), vec3(1,0,0)});
	v.push_back({ vec3(1.0f, -1.0f, 1.0f), vec3(1,0,0)});
	v.push_back({ vec3(-1.0f, 1.0f, 1.0f), vec3(1,0,0)});
	v.push_back({ vec3(1.0f, 1.0f, 1.0f), vec3(1,0,0)});

	for (auto c : steps) {
		v.push_back({ vec3(-1.0f, -1.0f, -1.0f), vec3(1,0,0), vec2(1.0f, 1.0f) });
		v.push_back({ vec3(1.0f, -1.0f, -1.0f), vec3(1,0,0), vec2(1.0f, 0.9f) });
		v.push_back({ vec3(-1.0f, 1.0f, -1.0f), vec3(1,0,0), vec2(1.0f, 0.8f) });;
		v.push_back({ vec3(1.0f, 1.0f, -1.0f), vec3(1,0,0), vec2(1.0f, 0.7f) });
		v.push_back({ vec3(-1.0f, -1.0f, 1.0f), vec3(1,0,0), vec2(1.0f, 0.6f) });
		v.push_back({ vec3(1.0f, -1.0f, 1.0f), vec3(1,0,0), vec2(1.0f, 0.5f) });
		v.push_back({ vec3(-1.0f, 1.0f, 1.0f), vec3(1,0,0), vec2(1.0f, 0.4f) });
		v.push_back({ vec3(1.0f, 1.0f, 1.0f), vec3(1,0,0), vec2(1.0f, 0.3f) });
	}
	return v;
}

void update_vertex_buffer( const std::vector<vertex>& vertices, uint N , std::vector<step_t> steps, cube_t main_cube)
{
	// clear and create new buffers
	if(vertex_buffer)	glDeleteBuffers( 1, &vertex_buffer );	vertex_buffer = 0;
	if(index_buffer)	glDeleteBuffers( 1, &index_buffer );	index_buffer = 0;

	// check exceptions
	if(vertices.empty()){ printf("[error] vertices is empty.\n"); return; }

	int cube_number = steps.size();
	// create buffers
	for (int ccount=0;ccount<=cube_number;ccount++)
	{
		std::vector<uint> indices;

		indices.push_back(ccount * 8 + 1);
		indices.push_back(ccount * 8 + 0);
		indices.push_back(ccount * 8 + 2);
		indices.push_back(ccount * 8 + 1);
		indices.push_back(ccount * 8 + 2);
		indices.push_back(ccount * 8 + 3);
		
		indices.push_back(ccount * 8 + 0);
		indices.push_back(ccount * 8 + 4);
		indices.push_back(ccount * 8 + 2);
		indices.push_back(ccount * 8 + 4);
		indices.push_back(ccount * 8 + 6);
		indices.push_back(ccount * 8 + 2);
		
		indices.push_back(ccount * 8 + 2);
		indices.push_back(ccount * 8 + 6);
		indices.push_back(ccount * 8 + 3);
		indices.push_back(ccount * 8 + 6);
		indices.push_back(ccount * 8 + 7);
		indices.push_back(ccount * 8 + 3);
		
		indices.push_back(ccount * 8 + 3);
		indices.push_back(ccount * 8 + 7);
		indices.push_back(ccount * 8 + 1);
		indices.push_back(ccount * 8 + 7);
		indices.push_back(ccount * 8 + 5);
		indices.push_back(ccount * 8 + 1);
		
		indices.push_back(ccount * 8 + 1);
		indices.push_back(ccount * 8 + 5);
		indices.push_back(ccount * 8 + 4);
		indices.push_back(ccount * 8 + 1);
		indices.push_back(ccount * 8 + 4);
		indices.push_back(ccount * 8 + 0);

		indices.push_back(ccount * 8 + 4);
		indices.push_back(ccount * 8 + 5);
		indices.push_back(ccount * 8 + 6);
		indices.push_back(ccount * 8 + 6);
		indices.push_back(ccount * 8 + 5);
		indices.push_back(ccount * 8 + 7);
		

		// generation of vertex buffer: use vertices as it is
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

		// geneation of index buffer
		glGenBuffers( 1, &index_buffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*indices.size(), &indices[0], GL_STATIC_DRAW );
	}
}


void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		start = true;
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_R) rotating = !rotating;
		else if (key == GLFW_KEY_HOME) {
			cam.eye = vec3(-150, -200, 0);
			cam.at = vec3(0, 0, 0);
			cam.up = vec3(0, 0, 1);
			cam.view_matrix = mat4::look_at(cam.eye, cam.at, cam.up);
		}
		else if (key == GLFW_KEY_RIGHT) {
			steps.at(main_cube.next_index).angle_status++;
		}
		else if (key == GLFW_KEY_LEFT) {
			steps.at(main_cube.next_index).angle_status--;
		}
		else if (key == GLFW_KEY_SPACE) {
			steps.at(main_cube.next_index + 8).box_status--;
		}
	}
	else if(action==GLFW_RELEASE)
	{
		if(key==GLFW_KEY_KP_ADD||(key==GLFW_KEY_EQUAL&&(mods&GLFW_MOD_SHIFT)))	b.add = false;
		else if(key==GLFW_KEY_KP_SUBTRACT||key==GLFW_KEY_MINUS) b.sub = false;
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
}

void motion(GLFWwindow* window, double x, double y)
{
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glLineWidth( 1.0f );
	glClearColor( 10/255.0f, 10/255.0f, 10/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	unit_cube_vertices = std::move(create_cube_verticese(NUM_TESS, main_cube, steps));
	update_vertex_buffer(unit_cube_vertices, NUM_TESS, steps, main_cube);

	// setup freetype
	text_init();

	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	std::ifstream in("map.txt");
	std::string s;

	for (map_size = 0; map_size < 1000 && !in.eof(); map_size++) {
		in >> s;
		map.push(stoi(s));
	}

	steps = std::move(create_steps());

	// initialization
	if(!glfwInit()){ printf( "[error] failed in glfwInit()\n" ); return 1; }

	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	double now = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		// register event callbacks
		glfwSetWindowSizeCallback(window, reshape);	// callback for window resizing events
		glfwSetKeyCallback(window, keyboard);			// callback for keyboard events
		glfwSetMouseButtonCallback(window, mouse);	// callback for mouse click inputs
		glfwSetCursorPosCallback(window, motion);		// callback for mouse movements

		if ((glfwGetTime() >= now + 0.005)) {
			now = glfwGetTime();
			glfwPollEvents();	// polling and processing of events
			update();			// per-frame update
			render();			// per-frame render
		}
	}
	
	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
