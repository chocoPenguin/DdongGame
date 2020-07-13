//*******************************************************************
// Copyright 2011-2018 Sungkil Lee
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*******************************************************************

#ifndef __CGUT_H__
#define __CGUT_H__

// minimum standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <set>

// enforce not to use /MD or /MDd flag
#if defined(_MSC_VER)
	#ifdef _DLL 
		#error Use /MT at Configuration > C/C++ > Code Generation > Run-time Library
	#endif
	#include <direct.h>
	#include <io.h>
#endif

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"	// http://www.glfw.org
#include "glad/glad.h"	// https://github.com/Dav1dde/glad
						// visit http://glad.dav1d.de/ to generate your own glad.h/glad.c of a different version
						// suggested profile: OpenGL, gl Version 4.6, core profile

// explicitly link libraries
#if defined(_MSC_VER) && _MSC_VER>1910
	#pragma comment( lib, "glfw3.lib" )		// static lib for VC2017
#elif defined(_MSC_VER)
	#pragma comment( lib, "glfw3dll.lib" )	// dynamic lib for other VC version
#endif

//*************************************
// OpenGL versions
struct gl_version_t
{
	int major=0;
	int minor=0;
	int major_glsl=0;
	int minor_glsl=0;
	static gl_version_t& instance(){ static gl_version_t v; return v; }
	int gl(){ return instance().major*10+instance().minor;}
	int glsl(){ return instance().major_glsl*10+instance().minor_glsl;}
};

//*************************************
// module file path
#ifdef _MSC_VER
struct module_path_t
{
	char path[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	module_path_t(){ GetModuleFileNameA( 0, path, _MAX_PATH ); _splitpath_s( path, drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT); }
};
#endif

//*************************************
// common structures
struct mem_t
{
	char*	ptr = nullptr;
	size_t	size = 0;
};

struct vertex // will be used for all the course examples
{
    vec3 pos;	// position
	vec3 norm;	// normal vector; we will use this for vertex color for this example
    vec2 tex;	// texture coordinate; ignore this for the moment
};

struct mesh
{
	std::vector<vertex>	vertex_list;
	std::vector<uint>	index_list;
	GLuint				vertex_buffer = 0;
	GLuint				index_buffer = 0;
	GLuint				texture = 0;
};

//*************************************
// utility functions
inline mem_t cg_read_binary( const char* file_path )
{
	mem_t m;
	FILE* fp = fopen( file_path, "rb" ); if(fp==nullptr){ printf( "[error] Unable to open %s\n", file_path ); return mem_t(); }
	fseek( fp, 0L, SEEK_END);
	m.size = ftell(fp);
	m.ptr = (char*) malloc(m.size+1);		// +1 for string
	fseek( fp, 0L, SEEK_SET );				// reset file pointer
	fread( m.ptr, m.size, 1, fp );
	memset(m.ptr+m.size,0,sizeof(char));	// for string
	fclose(fp);
	return m;
}

inline char* cg_read_shader( const char* file_path )
{
#ifdef _MSC_VER
	if(strlen(file_path)<2){ printf( "%s(): file_path (%s) is too short\n",__FUNCTION__, file_path ); return nullptr; }

	// get the full path of shader file
	module_path_t mpath;
	char shader_file_path[_MAX_PATH]; sprintf_s( shader_file_path, "%s%s%s", mpath.drive, mpath.dir, file_path );

	// get the full path of a shader file
	if(_access(shader_file_path,0)!=0){ printf( "%s(): %s not exists\n",__FUNCTION__, shader_file_path ); return nullptr; }
	return cg_read_binary( shader_file_path ).ptr;
#else
	return cg_read_binary( file_path ).ptr;
#endif
}

inline bool cg_validate_shader( GLuint shaderID, const char* shaderName )
{
	const int MAX_LOG_LENGTH=4096;
	static char msg[MAX_LOG_LENGTH] = {0};
	GLint shaderInfoLogLength;

	glGetShaderInfoLog( shaderID, MAX_LOG_LENGTH, &shaderInfoLogLength, msg );
	if( shaderInfoLogLength>1 && shaderInfoLogLength<=MAX_LOG_LENGTH )
		printf( "[Shader: %s]\n%s\n", shaderName, msg );

	GLint shaderCompileStatus; glGetShaderiv( shaderID, GL_COMPILE_STATUS, &shaderCompileStatus);
	if(shaderCompileStatus==GL_TRUE) return true;

	glDeleteShader( shaderID );
	return false;
}

inline bool cg_validate_program( GLuint programID, const char* programName )
{
	const int MAX_LOG_LENGTH=4096;
	static char msg[MAX_LOG_LENGTH] = {0};
	GLint programInfoLogLength;

	glGetProgramInfoLog( programID, MAX_LOG_LENGTH, &programInfoLogLength, msg );
	if( programInfoLogLength>1 && programInfoLogLength<=MAX_LOG_LENGTH )
		printf( "[Program: %s]\n%s\n", programName, msg );

	GLint programLinkStatus; glGetProgramiv( programID, GL_LINK_STATUS, &programLinkStatus);
	if(programLinkStatus!=GL_TRUE){ glDeleteProgram(programID); return false; }

	glValidateProgram( programID );
	glGetProgramInfoLog( programID, MAX_LOG_LENGTH, &programInfoLogLength, msg );
	if( programInfoLogLength>1 && programInfoLogLength<=MAX_LOG_LENGTH )
		printf( "[Program: %s]\n%s\n", programName, msg );

	GLint programValidateStatus; glGetProgramiv( programID, GL_VALIDATE_STATUS, &programValidateStatus);
	if( programValidateStatus!=GL_TRUE ){ glDeleteProgram(programID); return false; }

	return true;
}

#ifdef _MSC_VER
inline const char* cg_conf_path()
{
	static module_path_t mpath;
	static char conf_dir[_MAX_PATH]={0}, conf_path[_MAX_PATH]={0};
	if(!conf_path[0])
	{
		char temp[_MAX_PATH]; GetTempPathA( _MAX_PATH-1, temp );
		if(temp[0]){ size_t s = strlen(temp); if(temp[s-1]!='\\'){ temp[s]='\\'; temp[s+1]='\0'; } }
		if(!conf_dir[0]) sprintf( conf_dir, "%s.cgbase\\", temp );
		if(_access(conf_dir,0)!=0) _mkdir( conf_dir );
		sprintf( conf_path, "%s%s%s.conf", conf_dir, mpath.fname, mpath.ext );
	}
	return conf_path;
}
#endif

inline ivec2 cg_default_window_size()
{
#ifdef GL_ES_VERSION_2_0
	return ivec2( 576, 1024 );	// initial window size similary to smartphone 
#else
	return ivec2( 1280, 720 );	// initial window size
#endif
}

inline GLFWwindow* cg_create_window( const char* name, int& width, int& height, bool show_window=true )
{
	// give GLFW hints for window and OpenGL context
	glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
	glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
#ifdef GL_ES_VERSION_2_0
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );				// minimum requirement for OpenGL ES 2/3
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );				// minimum requirement for OpenGL ES 2/3
	glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
#else
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE );			// legacy GPUs common in students
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE );	// legacy GPUs common in students
#endif

	// high-dpi aware window scaling
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	if(mon)
	{
		float xscale, yscale; glfwGetMonitorContentScale( mon, &xscale, &yscale );
		const int width0 = width; if(xscale>1&&xscale<16) width = int(width*xscale);
		const int height0 = height; if(yscale>1&&yscale<16) height = int(height*yscale);
		if(width!=width0||height!=height0) printf( "window resized to %dx%d for high-dpi display\n", width, height );
	}

	// create a windowed mode window and its OpenGL context
	GLFWwindow* win = glfwCreateWindow( width, height, name, nullptr, nullptr ); if(!win){ printf( "Failed to create GLFW window.\n" ); glfwTerminate(); return nullptr; }

	// get the screen size and locate the window in the center
	ivec2 win_pos(-1,-1);
#ifdef _MSC_VER
	FILE* fp = fopen( cg_conf_path(), "r" );
	if(fp){ fscanf( fp, "window_pos_x = %d\nwindow_pos_y = %d\n", &win_pos.x, &win_pos.y ); fclose(fp); }
#endif

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	ivec2 screen_size = ivec2( mode->width, mode->height );
	if(win_pos.x<0||win_pos.x>=screen_size.x||win_pos.y<0||win_pos.y>=screen_size.y) win_pos = ivec2((screen_size.x-width)/2, (screen_size.y-height)/2);
	glfwSetWindowPos( win, win_pos.x, win_pos.y );
	glfwSetWindowSize( win, width, height ); // make sure to update the size again to avoid DPI mismatch problems

	// make context and show window
	glfwMakeContextCurrent(win);
	if(show_window) glfwShowWindow( win );

	return win;
}

inline void cg_destroy_window( GLFWwindow* window )
{
	// save window position
#ifdef _MSC_VER
	if(glfwGetWindowAttrib(window,GLFW_VISIBLE))
	{
		int x, y; glfwGetWindowPos( window, &x, &y );
		FILE* fp = fopen( cg_conf_path(), "w" ); if(fp){ fprintf( fp, "window_pos_x = %d\nwindow_pos_y = %d\n", x, y ); fclose(fp); }
	}
#endif

	glfwDestroyWindow(window);
	glfwTerminate();
}

inline bool cg_init_extensions( GLFWwindow* window )
{
	glfwMakeContextCurrent(window);	// make sure the current context again

#ifdef GL_ES_VERSION_2_0
	if(!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)){ printf( "init_extensions(): Failed in gladLoadGLES2Loader()\n" ); glfwTerminate(); return false; }
#else
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ printf( "init_extensions(): Failed in gladLoadGLLoader()\n" ); glfwTerminate(); return false; }
#endif

	// print GL/GLSL versions
	gl_version_t& v = gl_version_t::instance();
	printf( "Using %s on %s, %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR) );
	glGetIntegerv(GL_MAJOR_VERSION, &v.major );
	glGetIntegerv(GL_MINOR_VERSION, &v.minor ); while(v.minor>10) v.minor/=10;
	const char* strGLSLver = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	if( v.gl() < 20 ) printf( "Warning: OpenGL version may be too low to run modern-style OpenGL\n" );

	// warning for lower version
#ifdef GL_ES_VERSION_2_0
	printf( "Using %s\n", strGLSLver );
	if(strGLSLver){ sscanf( strGLSLver, "OpenGL ES GLSL ES %d.%d", &v.major_glsl, &v.minor_glsl ); while(v.minor_glsl>10) v.minor_glsl/=10; }
	if( v.glsl() < 10 ) printf( "Warning: GLSL ES %.1f may not support shader programs.\n", v.glsl()*0.1f );
#else
	printf( "Using OpenGL GLSL %s\n", strGLSLver );
	if(strGLSLver){ sscanf( strGLSLver, "%d.%d", &v.major_glsl, &v.minor_glsl ); while(v.minor_glsl>10) v.minor_glsl/=10; }
	if( v.glsl() < 13 ) printf( "Warning: GLSL %.1f may not support shader programs.\n", v.glsl()*0.1f );
#endif

	// load and check extensions
	std::set<std::string> ext_set;
#ifdef GL_ES_VERSION_2_0
	size_t ext_len = strlen((const char*)glGetString(GL_EXTENSIONS));
	std::vector<char> ext((char*)glGetString(GL_EXTENSIONS),(char*)glGetString(GL_EXTENSIONS)+ext_len+2);
	for( char* t=strtok( &ext[0], " \t\n" ); t; t=strtok( nullptr, " \t\n" ) ) ext_set.insert(t);
#else
	int next; glGetIntegerv(GL_NUM_EXTENSIONS,&next);
	for( int k=0; k<next; k++) ext_set.insert((const char*)glGetStringi(GL_EXTENSIONS,k));
	#define CHECK_GL_EXT(ext) if(ext_set.find( "GL_ARB_"#ext )==ext_set.end() ) printf( "Warning: GL_ARB_" #ext " not supported.\n" );
		CHECK_GL_EXT( shading_language_100 );	// check your platform supports GLSL
		CHECK_GL_EXT( vertex_buffer_object );	// BindBuffers, DeleteBuffers, GenBuffers, IsBuffer, BufferData, BufferSubData, GenBufferSubData, ...
		CHECK_GL_EXT( vertex_shader );			// functions related to vertex shaders
		CHECK_GL_EXT( fragment_shader );		// functions related to fragment shaders
		CHECK_GL_EXT( shader_objects );			// functions related to program and shaders
	#undef CHECK_GL_EXT
#endif

	printf( "\n" );
	return true;
}

inline const char* shader_type_name( GLenum shader_type )
{
	if( shader_type==0x8B31 ) return "vertex shader";
	if( shader_type==0x8B30 ) return "fragment shader";
	if( shader_type==0x8DD9 ) return "geometry shader";
	if( shader_type==0x8E88 ) return "tess control shader";
	if( shader_type==0x8E87 ) return "tess evaluation shader";
	if( shader_type==0x91B9 ) return "compute shader";
	return "unknown shader_type";
}

inline bool strstr( const char* src, std::initializer_list<const char*> substr )
{
	for( auto* s : substr ) if(strstr(src,s)) return true;
	return false;
}

inline GLuint cg_create_shader( const char* shader_source, GLenum shader_type, std::string& log )
{
	if(!shader_source){ printf( "%s(): shader_source == nullptr\n", __FUNCTION__ ); return 0; }

	std::string stn = std::string("[")+shader_type_name(shader_type)+"]";
	std::string src = shader_source;
	std::string macro;

	if(!strstr(shader_source,"#version"))
	{
		gl_version_t& v = gl_version_t::instance();
		char buff[1024]; sprintf( buff, "#version %d\n", v.glsl()*10 ); macro += buff;
		sprintf( buff, "%-18s '#version %d es' added automatically.\n", stn.c_str(), v.glsl()*10 ); log += buff;
	}

	std::vector<const char*> src_list;
	std::vector<GLint> src_size_list;
	if(!macro.empty()){ src_list.push_back(macro.c_str()); src_size_list.push_back(macro.size()); }
	src_list.push_back(shader_source); src_size_list.push_back(strlen(shader_source));

	GLuint shader = glCreateShader( shader_type );
	glShaderSource( shader, src_list.size(), &src_list[0], &src_size_list[0] );
	glCompileShader( shader );
	if(!cg_validate_shader( shader, shader_type_name(shader_type))){ printf( "Unable to compile %s\n", shader_type_name(shader_type) ); return 0; }

	return shader;
}

inline GLuint cg_create_program_from_string( const char* vertex_shader_source, const char* fragment_shader_source )
{
	// try to create a program
	GLuint program = glCreateProgram();
	glUseProgram( program );

	// create shaders
	std::string log;
	GLuint vertex_shader = cg_create_shader( vertex_shader_source, GL_VERTEX_SHADER, log );
	GLuint fragment_shader = cg_create_shader( fragment_shader_source, GL_FRAGMENT_SHADER, log );
	if(!log.empty()) printf( "%s\n", log.c_str() );
	if(!vertex_shader||!fragment_shader) return 0;

	// attach vertex/fragments shaders
	glAttachShader( program, vertex_shader );
	glAttachShader( program, fragment_shader );

	// try to link program
	glLinkProgram( program );
	if(!cg_validate_program( program, "program" )){ printf( "Unable to link program\n" ); return 0; }

	return program;
}

inline GLuint cg_create_program( const char* vert_path, const char* frag_path )
{
	const char* vertex_shader_source = cg_read_shader( vert_path ); if(vertex_shader_source==NULL) return 0;
	const char* fragment_shader_source = cg_read_shader( frag_path ); if(fragment_shader_source==NULL) return 0;

	// try to create a program
	GLuint program = cg_create_program_from_string( vertex_shader_source, fragment_shader_source );

	// deallocate string
	free((void*)vertex_shader_source);
	free((void*)fragment_shader_source);
	return program;
}

inline mesh* cg_load_mesh( const char* vert_binary_path, const char* index_binary_path )
{
	mesh* new_mesh = new mesh();

	// load vertex buffer
	mem_t v = cg_read_binary(vert_binary_path);
	if(v.size%sizeof(vertex)){ printf( "%s is not a valid vertex binary file\n", vert_binary_path ); return nullptr; }
	new_mesh->vertex_list.resize( v.size/sizeof(vertex) );
	memcpy( &new_mesh->vertex_list[0], v.ptr, v.size );

	// load index buffer
	mem_t i = cg_read_binary(index_binary_path);
	if(i.size%sizeof(uint)){ printf( "%s is not a valid index binary file\n", index_binary_path ); return nullptr; }
	new_mesh->index_list.resize( v.size/sizeof(uint) );
	memcpy( &new_mesh->index_list[0], i.ptr, i.size );

	// release memory
	if(v.ptr) free(v.ptr);
	if(i.ptr) free(i.ptr);

	// create a vertex buffer
	glGenBuffers( 1, &new_mesh->vertex_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, new_mesh->vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*new_mesh->vertex_list.size(), &new_mesh->vertex_list[0], GL_STATIC_DRAW );

	// create a index buffer
	glGenBuffers( 1, &new_mesh->index_buffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, new_mesh->index_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*new_mesh->index_list.size(), &new_mesh->index_list[0], GL_STATIC_DRAW );

	return new_mesh;
}

inline void cg_bind_vertex_attributes( uint program, std::array<const char*,3> attrib_names={"position","normal","texcoord"} )
{
	size_t attrib_size[] = { sizeof(vertex().pos), sizeof(vertex().norm), sizeof(vertex().tex) };
	for( size_t k=0, byte_offset=0; k<3; k++, byte_offset+=attrib_size[k-1] )
	{
		GLuint loc = glGetAttribLocation( program, attrib_names[k] ); if(loc>=3) continue;
		glEnableVertexAttribArray( loc );
		glVertexAttribPointer( loc, attrib_size[k]/sizeof(GLfloat), GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*) byte_offset );
	}
}

#endif // __CGUT_H__
