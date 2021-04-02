#pragma once

#include <gl/glew.h>
#include <GLFW\glfw3.h>
#include <string>
#include <iostream>
#include "gl_macro.h"
#include <GLM/glm.hpp>

typedef void(*keycallback)(GLFWwindow* window, int key, int scancode, int action, int mods);
typedef void(*mousecallback)(GLFWwindow* window, double xpos, double ypos);
typedef void(*mousebuttoncallback)(GLFWwindow* window, int button, int action, int mods);

struct gl_window
{
	GLFWwindow *wnd;
	std::string name;
	float width;
	float height;
	keycallback keycb;
	mousecallback mousecb;
	mousebuttoncallback mousebtncb;

};

void set_gl_window_properties(const glm::vec4& col = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
{
	
	glClearColor(col.r, col.g, col.b, col.a);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
}

void set_window_callbacks(gl_window& w)
{
	glfwSetKeyCallback(w.wnd, w.keycb);
	glfwSetCursorPosCallback(w.wnd, w.mousecb);
	glfwSetMouseButtonCallback(w.wnd, w.mousebtncb);
}

bool create_gl_window(gl_window& w)
{
	bool ret = false;
	if (w.wnd == nullptr)
	{
		w.wnd = glfwCreateWindow(w.width, w.height, w.name.c_str(), NULL, NULL);
		glfwMakeContextCurrent(w.wnd);
		
		GLenum error;
		if ((error = glewInit()) == GLEW_OK)
		{
			ret = true;					
			set_window_callbacks(w);				
		}
		else
		{
			std::cout << "failed to init glew" << std::endl;
			std::cout << glewGetErrorString(error) << std::endl;
		}		
	}
	else
	{
		std::cout << "window handle isnt null, can't create window" << std::endl;
	}
	return ret;
	
}



