#include <iostream>
#include "../headers/window.h"
#include "../headers/shader.h"
#include <GLM/glm.hpp>
#include "../headers/definitions.h"
#include "../headers/mesh_loader.h"

void render_image()
{
	GLuint tex_id;
	Shader sh_quad("shaders/pass.vert.glsl", "shaders/quad_texture.frag.glsl");
	quad screen_quad = {0, 0};		
	upload_texture(tex_id, "images/circles.jpg");	
	sh_quad.use();
	sh_quad.setInt("kuwabara", 0);
	bind_texture_unit(0, tex_id);
	draw_quad(screen_quad);		
}
arcball arc_cam;
glm::mat4 M;
camera_props cfg;

int main(int argc, char **argv)
{
	if (!glfwInit())
	{
		std::cout << "cant initialize glfw" << std::endl;
		return -1;
	}

	//arcball camera initialization
	arc_cam.init(640.0f, 480.0f);

	//callback lambdas
	keycallback kb = [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		switch (key)
		{
		case GLFW_KEY_L:
			GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
			break;
		case GLFW_KEY_P:
			GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_POINT));
			break;
		case GLFW_KEY_F:
			GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			break;
		}
	};

	mousebuttoncallback mbtn_cb = [](GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			arc_cam.active = true;
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			arc_cam.lastx = int(xpos);
			arc_cam.lasty = int(ypos);

		}
		else {

			arc_cam.active = false;

		}
	};

	mousecallback mcb = [](GLFWwindow* window, double xpos, double ypos) {
		if (arc_cam.active)
		{

			arc_cam.curx = xpos;
			arc_cam.cury = ypos;
			arc_cam.update_mat(M, cfg.V);

			arc_cam.lastx = arc_cam.curx;
			arc_cam.lasty = arc_cam.cury;

		}
	};

	/*-----------------------------------------------------------------------------------------------*/

	gl_window window = { nullptr, "mesh color test", 640.0f, 480.0f, kb, mcb, mbtn_cb };

	bool success = create_gl_window(window);

	if (!success)
	{
		std::cout << "cant create gl window" << std::endl;
		return -1;
	}

	set_gl_window_properties();

	std::string knight_path = "obj/mini_box/mini_knight.obj" ;
	std::string kirby_path = "obj/Kirby/kirby.obj";
	std::string flash_path = "obj/flash/flash_new.obj";
	GLuint tex_id;
	Shader drawMesh("shaders/standard_mvp.vert.glsl", "shaders/albedo_shade.frag.glsl");
	mesh_loader mesh(kirby_path.c_str());
	
	//how many models
	std::cout << "mesh has : " << mesh.models.size() << " models\n" << std::endl;
		
	cfg = { glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0, 0.0, -1.0f) , glm::vec3(0.0f, 1.0f, 0.0f), 640.0f, 480.0f, 0.1f, 500.0f, 60.0f };
	build_fixed_camera(cfg);	

	M = glm::mat4(1);
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), -mesh.bb_mid);
	
	float flash_scale = 1.0f;
	float kirby_scale = 0.08f;
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(kirby_scale));

	//bring to center and then scale down
	M = scale * translation;	
	
	std::cout << "pre transformation mesh`s center of mass at :  \n" << mesh.bb_mid << std::endl;	
	
	//initializing mesh colors
	//mesh_colors mc(mesh.models[0], "obj/flash/FL_CW_A_1.png");
	//rect2D r(1024,1024);
	//build_mesh_color_texture(mc, r);
	//r.export_bmp2();
	//rect2D line(5, 1);
	//std::vector<rgb> cols;
	//build_linear_mc(mc2, cols);
	//line.data = cols;
	//line.export_bmp2("linear.bmp");

	//for (auto a : cols) {
	//	std::cout << (int)a.r << " " << (int)a.g << " " << (int)a.b << std::endl;
	//}			
	
	//"obj/kirby/kdiff.png"
	//"obj/flash/FL_CW_A_1.png"
	//"obj/mini_box_knight/mini_knight.png"
	mesh_colors2 mc2(mesh.models[0], "obj/kirby/kdiff.png", 3);
	rect2D r2(mc2.wid, mc2.hei);
	custom_mesh_color_texture(mc2, r2);
	r2.export_bmp2("custom_mc.bmp");	
		
	//std::cout << "test: " << mc2.wid * mc2.hei * mc2.ch << std::endl;

	std::cout << "mesh colors has : " << mc2.faces.size() << " faces" << std::endl;

	//for (size_t i = 0; i < mc2.faces.size(); i++) {
	//	if (mc2.faces[i].v_index[0] >= 1048576) {
	//		std::cout << "found bug at " << i << " " << mc2.faces[i].v_index[0] << std::endl;
	//	}
	//	else if (mc2.faces[i].v_index[1] >= 1048576) {
	//		std::cout << "found bug at " << i << " " << mc2.faces[i].v_index[0] << std::endl;
	//	}
	//	else if (mc2.faces[i].v_index[2] >= 1048576) {
	//		std::cout << "found bug at " << i << " " << mc2.faces[i].v_index[0] << std::endl;
	//	}
	//}
	
	//debug print
	//print_mesh_colors_index();

	GLuint t_id;
	gen_rectangle_texture(r2, t_id);

	while (!glfwWindowShouldClose(window.wnd))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 MVP = cfg.P * cfg.V * M;

		drawMesh.use();
		drawMesh.setMat4("MVP", MVP);
				
		drawMesh.setInt("mesh_color", 1);
		bind_texture_unit(1,t_id);		

		mesh.Draw(drawMesh);	
		//auto i = glGetUniformLocation(drawMesh.ID, "mesh_color");
		//auto j = glGetUniformLocation(drawMesh.ID, "texture_diffuse1");
		//std::cout << i << std::endl;
		//std::cout << j << std::endl;

		
		glfwSwapBuffers(window.wnd);
		glfwPollEvents();
	}	
	
	std::cout << "program terminated" << std::endl;

	/*clean up block*/
	glfwTerminate();	

	
	return 0;
}
