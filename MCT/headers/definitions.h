#pragma once

#include <gl/glew.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <iostream>
#include <SOIL/SOIL.h>
#include <vector>
#include "gl_macro.h"
#include <algorithm>
#include "model.h"
#include <algorithm>

#define ArrayCount(x) (sizeof(x)/sizeof(x[0]))

//camera properties
struct camera_props {
	glm::vec3 eye;
	glm::vec3 forward;
	glm::vec3 up;
	float width;
	float height;
	float znear;
	float zfar;
	float fov;
	glm::vec3 right;	
	glm::mat4 V;
	glm::mat4 P;	
};

//builds a fixed camera with user selected camera properties
void build_fixed_camera(camera_props& cfg)
{
	cfg.V = glm::lookAt(cfg.eye, cfg.forward, cfg.up);
	cfg.right = glm::normalize(glm::cross(cfg.forward, cfg.up));
	cfg.P = glm::perspective(glm::radians(cfg.fov), cfg.width/cfg.height, cfg.znear, cfg.zfar);	
}

//generates opengl texture, powered by SOIL
void upload_texture(GLuint& id, const std::string& path)
{
	
	id = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (id == 0)
	{
		std::cout << "failed to load opengl texture\n" << SOIL_last_result() << std::endl;
	}
}

//arcball orbital camera
struct arcball {
	int lastx, lasty, curx, cury;
	float w, h, midw, midh;
	bool active;

	void init(float wid, float hei) {
		lastx = 0;
		lasty = 0;
		curx = 0;
		cury = 0;
		w = wid;
		h = hei;
		midw = wid / 2.0f;
		midh = hei / 2.0f;
		active = false;
	}

	glm::vec3 get_vector(int x, int y)
	{
		glm::vec3 P = glm::vec3( (float(x)/midw) - 1.0f,
								 (float(y)/midh) - 1.0f,
							      0.0f);
		P.y = -P.y;
		float OP_pow2 = (P.x * P.x) + (P.y * P.y);
		if (OP_pow2 <= 1.0f)
		{
			P.z = sqrt(1.0f - OP_pow2);
			P = glm::normalize(P);
		}
		else
		{
			P = glm::normalize(P);
		}

		return P;
	}

	void update_mat(glm::mat4& M, glm::mat4& V)
	{
		if (curx != lastx || cury != lasty)
		{
			glm::vec3 v1 = get_vector(lastx, lasty);
			glm::vec3 v2 = get_vector(curx, cury);

			float angle = acos(std::min(1.0f, glm::dot(v1, v2)));
			angle *= 0.05f;

			glm::vec3 camera_axis = glm::cross(v1, v2);
			glm::mat3 inverse = glm::inverse(glm::mat3(V) * glm::mat3(M));
			glm::vec3 model_axis = inverse * camera_axis;

			M = glm::rotate(glm::mat4(1.0f), glm::degrees(angle), camera_axis) * M;

			lastx = curx;
			lasty = cury;
		}
	}
	
};

//prints glm vec3 utility
std::ostream& operator<<(std::ostream& os, const glm::vec3&vec)
{
	os << vec.x << " \n";
	os << vec.y << " \n";
	os << vec.z << " \n";
	return os;
}

union rgb
{
	struct {
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};
	unsigned char c[3];
	rgb(int x, int y, int z) {
		r = x;
		g = y;
		b = z;
	}
	rgb() {};

};


rgb operator*(rgb lhs, double s) {
	return {unsigned char(lhs.c[0]*s), unsigned char(lhs.c[1] * s), unsigned char(lhs.c[2] * s )};
}

rgb operator*(rgb lhs, float s) {
	return { unsigned char(lhs.c[0] * s), unsigned char(lhs.c[1] * s), unsigned char(lhs.c[2] * s) };
}

rgb operator+(rgb lhs, rgb rhs) {
	return { unsigned char(lhs.c[0] + rhs.c[0]), unsigned char(lhs.c[1] + rhs.c[1]), unsigned char(lhs.c[2] + rhs.c[2])};
}

bool operator== (const rgb& lhs, const rgb& rhs) {
	bool ret = false;
	if ((lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b)) {
		ret = true;
	}
	return ret;
}

/* 2D rect custom texture*/
class rect2D {
public:
	rect2D(unsigned int w, unsigned int h, const char* file) {
		wid = w;
		hei = h;
		size = wid * hei;
		data.resize(size);
		build_data(file);
	}

	rect2D(unsigned int w, unsigned int h) {
		wid = w;
		hei = h;
		size = wid * hei;
		data.resize(size);
		for (size_t i = 0; i < data.size(); i++)
		{
			data[i].c[0] = 0x00;
			data[i].c[1] = 0x00;
			data[i].c[2] = 0x00;
		}
	}
	
	//Used if one needs to load images from disk
	void build_data(const char* file)
	{
		int w, h, c;
		unsigned char* img = SOIL_load_image(file, &w, &h, &c, SOIL_LOAD_AUTO);
		std::cout << SOIL_last_result() << std::endl;
		if (img != NULL)
		{
			std::cout << "image details:" << std::endl;
			std::cout << "width: " << wid << std::endl;
			std::cout << "heigth: " << hei << std::endl;
			std::cout << "channels: " << ch << std::endl;
			for (int i = 0; i < wid*hei*ch; i += 3)
			{
				rgb aux = { img[i + 0], img[i + 1], img[i + 2] };
				data.push_back(aux);
			}
			wid = w;
			hei = h;
			ch = c;
		}
		else {
			std::cout << "failed to load image" << std::endl;
		}
		SOIL_free_image_data(img);
	}

	//Save as bmp from previously loaded image
	void export_bmp() {
		//std::cout << "saving image" << std::endl;
		int result = SOIL_save_image("rect2D.bmp", SOIL_SAVE_TYPE_BMP, (int)wid, (int)hei, (int)ch, reinterpret_cast<unsigned char*>(data.data()));
		if (result == 0) {
			std::cout << SOIL_last_result() << std::endl;				
		}
	}

	//Save bmp from custom image
	void export_bmp2(const char* name = "rect2D.bmp") {
		//std::cout << "saving image " << name << std::endl;
		int result = SOIL_save_image(name, SOIL_SAVE_TYPE_BMP, (int)wid, (int)hei, 3, reinterpret_cast<unsigned char*>(data.data()));
		if (result == 0) {
			std::cout << SOIL_last_result() << std::endl;
		}
	}

	
	//insert as 1D array index
	void insert_1D(unsigned int index, const rgb& v) 
	{
		//std::cout << "index to be assigned is: " << index << std::endl;
		//if (index < 0 || index > size)
		//{
		//	std::cout << "index out of range" << std::endl;
		//}
		assert(index >= 0 && index <= size);
		data[index] = v;
	}

	//normalized at
	void insert_n_at(const float idx, const float idy, const rgb& v)
	{
		unsigned int ind_x = (idx * wid) - 1;
		unsigned int ind_y = (idy * hei) - 1;

		unsigned int index = idx * hei + idy;
		if (index > (size - 1))
		{
			std::cout << "index out of range" << std::endl;
		}
		else
		{
			data[index] = v;
		}
	}

	//unnormalized at
	void insert_at(const unsigned int idx, const unsigned int idy, const rgb& v) 
	{
		unsigned int index = idx * hei + idy;
		if (index > (size - 1) )
		{
			std::cout << "vector subscript out of range" << std::endl;
		}
		else
		{
			data[index] = v;
		}
	}
		
	rgb get_pixel_from_uv(double u, double v)
	{
		unsigned int index = u * hei + v;
		return data[index];
	}
	//bilinear
	//https://en.wikipedia.org/wiki/Bilinear_filtering
	rgb getPixel(double u, double v) 
	{
		u = u * (wid - 1) - 0.5;
		v = v * (hei - 1) - 0.5;
		unsigned int x = floor(u);
		unsigned int y = floor(v);
		double u_ratio = u - x;
		double v_ratio = v - y;
		double u_op = 1 - u_ratio;
		double v_op = 1 - v_ratio;
		rgb ul = get_pixel_from_uv(x,     y+1.0);
		rgb ur = get_pixel_from_uv(x+1.0, y+1.0);
		rgb bl = get_pixel_from_uv(x,     y);
		rgb br = get_pixel_from_uv(x+1.0, y);
		return (bl*u_op + br * u_ratio) * v_op + (ul * u_op + ur*u_ratio) * v_ratio;
	}

	std::vector<rgb> data;
	unsigned int wid;
	unsigned int hei;
	unsigned int ch;
	unsigned int size;
};

/* Fullscreen quad */
struct quad {
	GLuint vao;
	GLuint vbo;	
};

void draw_quad(quad& q)
{
	if (q.vao == 0)
	{
		float verts[] = {
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 0.0f
		};

		GLCall(glGenVertexArrays(1, &q.vao));
		GLCall(glGenBuffers(1, &q.vbo));
		GLCall(glBindVertexArray(q.vao));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, q.vbo));
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW));
		GLCall(glEnableVertexAttribArray(0));
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0)));
		GLCall(glEnableVertexAttribArray(1));
		GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3)));
	}

	GLCall(glBindVertexArray(q.vao));
	GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	GLCall(glBindVertexArray(0));
}

void bind_texture_unit(unsigned int unit, GLuint id)
{
	GLCall(glActiveTexture(GL_TEXTURE0 + unit));
	GLCall(glBindTexture(GL_TEXTURE_2D, id));	
}

void bind_texrect_unit(unsigned int unit, GLuint id)
{
	GLCall(glActiveTexture(GL_TEXTURE0 + unit));
	GLCall(glBindTexture(GL_TEXTURE_RECTANGLE, id));	
}

//Mesh Colors adhoc implementation will work with Resolution R=2

/*--mesh colors--*/
union edge {
	struct {
		unsigned int a;
		unsigned int b;
	};
	unsigned int e[2];
};

struct face {

	face(unsigned int i1, unsigned int i2, unsigned int i3) {
		//assuming r = 1, forcing resolution
		R = 1;
		face_r = 0; //(R-1 * R-2) / 2
		edge_r = 1; // R-1
		tri[0] = i1;
		tri[1] = i2;
		tri[2] = i3;

		edges[0].a = i1;
		edges[0].b = i2;
		
		edges[1].a = i2;
		edges[1].b = i3;

		edges[2].a = i3;
		edges[2].b = i1;

	}
	unsigned int tri[3];	//vertices
	edge         edges[3];  //edges

	unsigned int v_index[3]; //vertex colors index
	unsigned int e_index[3]; //edge colors index
	
	unsigned int R;
	unsigned int face_r;
	unsigned int edge_r;
};

struct rface {
	rface(unsigned int i1, unsigned int i2, unsigned int i3, int r = 2) {
		//assuming r = 1, forcing resolution
		R = pow(2, r) - 1;
		face_r = ((R - 1) * (R - 2)) / 2;
		edge_r = R - 1;
		
		v_index.resize(3);
		f_index.resize(face_r);

		tri[0] = i1;
		tri[1] = i2;
		tri[2] = i3;

		edges[0].a = i1;
		edges[0].b = i2;

		edges[1].a = i2;
		edges[1].b = i3;

		edges[2].a = i3;
		edges[2].b = i1;

	}
	unsigned int tri[3];	//vertices
	edge         edges[3];  //edges

	std::vector<unsigned int> v_index;    //vertex colors index
	std::vector<unsigned int> e_index[3]; //edge colors index
	std::vector<unsigned int> f_index;    //face colors index

	unsigned int R;
	unsigned int face_r;
	unsigned int edge_r;
};

namespace {
	unsigned int uv_to_index(double u, double v, int wid, int hei) {
		int indx = ((wid-1) * u);
		int indy = ((hei-1) * v);
		int index = indx * hei + indy;
		return index;
	}
		
	//x, y, z are the barycentric coordinates
	//u, v, w are the triangle uv coordinates
	glm::vec2 barycentric_to_cartesian(double x, double y, double z, glm::vec2 u, glm::vec2 v, glm::vec2 w)
	{
		double _u = x * u.x + y * v.x + z * w.x;
		double _v = x * u.y + y * v.y + z * w.y;
		return glm::vec2(_u, _v);
	}

	//integer lerp
	int ilerp(int min, int max, double val) {
		return (min * (1.0  - val) + max*val);
	}
};

class mesh_colors {	
public:
	mesh_colors(const Model& m, const char* path){
		build_imarray(path);
		vertices = m.vertices;
		indices = m.indices;
		build_faces();
		fill_colors_alt();
		std::cout << "mesh colors created, wid: "<< wid << " hei: " << hei << std::endl;
		std::cout << "size of image data : " << image.size() << std::endl;
	}

	void build_imarray(const char* file)
	{
		int w, h, c;
		unsigned char* img = SOIL_load_image(file, &w, &h, &c, SOIL_LOAD_AUTO);
		std::cout << SOIL_last_result() << std::endl;
		if (img != NULL)
		{
			wid = w;
			hei = h;
			ch = c;
			
			std::cout << "image details:" << std::endl;
			std::cout << "width: " << wid << std::endl;
			std::cout << "heigth: " << hei << std::endl;
			std::cout << "channels: " << ch << std::endl;
			
			for (int i = 0; i < wid*hei*ch; i += 3)
			{
				rgb aux = { img[i + 0], img[i + 1], img[i + 2] };
				image.push_back(aux);
			}
			
		}
		else {
			std::cout << "failed to load image" << std::endl;
		}
		SOIL_free_image_data(img);
	}

	void build_faces()
	{
		for (int i = 0; i < indices.size(); i+=3)
		{
			unsigned int id1 = indices[i + 0] ;
			unsigned int id2 = indices[i + 1] ;
			unsigned int id3 = indices[i + 2] ;
			
			faces.emplace_back(id1, id2, id3);
		}
	}

	void fill_colors() {
		for (size_t i = 0; i < faces.size(); i++) {
			unsigned int a = faces[i].tri[0];
			faces[i].v_index[0] = ::uv_to_index(vertices[a].uv.x, vertices[a].uv.y, wid, hei);

			unsigned int b = faces[i].tri[1];
			faces[i].v_index[1] = ::uv_to_index(vertices[b].uv.x, vertices[b].uv.y, wid, hei);

			unsigned int c = faces[i].tri[2];
			faces[i].v_index[2] = ::uv_to_index(vertices[c].uv.x, vertices[c].uv.y, wid, hei);


			//TODO: Read paper, check how do barycentric coordinates position colors in edges and fill these
			//for resolution r = 1 -> colors are C01, C10, C11
			//P01 -> 0/2, 1/2, 0.5
			//1 - 2
			glm::vec2 coords = barycentric_to_cartesian(0.0, 0.5, 0.5, vertices[a].uv, vertices[b].uv, vertices[c].uv);
			faces[i].e_index[0] = ::uv_to_index(coords.x, coords.y, wid, hei);
			
			//P10 -> 1/2, 0/2, 0.5
			//2 - 3
			coords = barycentric_to_cartesian(0.5, 0.0, 0.5, vertices[a].uv, vertices[b].uv, vertices[c].uv);
			faces[i].e_index[1] = ::uv_to_index(coords.x, coords.y, wid, hei);
			
			//P11 -> 0.5, 0.5, 0
			//3 - 1
			coords = barycentric_to_cartesian(0.5, 0.5, 0.0, vertices[a].uv, vertices[b].uv, vertices[c].uv);
			faces[i].e_index[2] = ::uv_to_index(coords.x, coords.y, wid, hei);
		}
	}

	void fill_colors_alt() {
		for (size_t i = 0; i < faces.size(); i++) {
			unsigned int a = faces[i].tri[0];
			unsigned int b = faces[i].tri[1];
			unsigned int c = faces[i].tri[2];

			int x = ilerp(0, wid , vertices[a].uv.x);
			int y = ilerp(0, hei , vertices[a].uv.y);
			faces[i].v_index[0] = (x*hei) + y;

			x = ilerp(0, wid, vertices[b].uv.x);
			y = ilerp(0, hei, vertices[b].uv.y);
			faces[i].v_index[1] = (x*hei) + y;

			x = ilerp(0, wid, vertices[c].uv.x);
			y = ilerp(0, hei, vertices[c].uv.y);
			faces[i].v_index[2] = (x*hei) + y;

			//P01 -> 0/2, 1/2, 0.5
			//1 - 2
			glm::vec2 coords = barycentric_to_cartesian(0.0, 0.5, 0.5, vertices[a].uv, vertices[b].uv, vertices[c].uv);
			x = ilerp(0, wid, coords.x);
			y = ilerp(0, hei, coords.y);
			faces[i].e_index[0] = (x*hei) + y;

			//P10 -> 1/2, 0/2, 0.5
			//2 - 3
			coords = barycentric_to_cartesian(0.5, 0.0, 0.5, vertices[a].uv, vertices[b].uv, vertices[c].uv);
			x = ilerp(0, wid, coords.x);
			y = ilerp(0, hei, coords.y);
			faces[i].e_index[1] = (x*hei) + y;

			//P11 -> 0.5, 0.5, 0
			//3 - 1
			coords = barycentric_to_cartesian(0.5, 0.5, 0.0, vertices[a].uv, vertices[b].uv, vertices[c].uv);
			x = ilerp(0, wid, coords.x);
			y = ilerp(0, hei, coords.y);
			faces[i].e_index[2] = (x*hei) + y;
		}
	}
	
	
	//image information
	unsigned int wid, hei, ch;
	std::vector<rgb> image;

	std::vector<face> faces;
	std::vector<vertex> vertices;
	std::vector<unsigned int> indices;
};

class mesh_colors2{
public:
	mesh_colors2(const Model& m, const char* path, unsigned int _r) {
		r = _r;
		build_imarray(path);
		vertices = m.vertices;
		indices = m.indices;
		build_faces();
		fill_colors_alt();
		std::cout << "mesh colors created, wid: " << wid << " hei: " << hei << std::endl;
		std::cout << "size of mesh colors image data : " << image.size() << std::endl;
	}

	void build_imarray(const char* file)
	{
		int w, h, c;
		unsigned char* img = SOIL_load_image(file, &w, &h, &c, SOIL_LOAD_AUTO);
		std::cout << SOIL_last_result() << std::endl;
		if (img != NULL)
		{
			wid = w;
			hei = h;
			ch = c;

			std::cout << "image details:" << std::endl;
			std::cout << "width: " << wid << std::endl;
			std::cout << "heigth: " << hei << std::endl;
			std::cout << "channels: " << ch << std::endl;		

			for (int i = 0; i < wid*hei*ch; i += ch)
			{
				rgb aux = { img[i + 0], img[i + 1], img[i + 2] };
				image.push_back(aux);
			}
			

		}
		else {
			std::cout << "failed to load image" << std::endl;
		}
		SOIL_free_image_data(img);
	}

	void build_faces()
	{
		for (int i = 0; i < indices.size(); i += 3)
		{
			unsigned int id1 = indices[i + 0];
			unsigned int id2 = indices[i + 1];
			unsigned int id3 = indices[i + 2];

			//r = 2 -> R = (2^2) - 1 == 3															
			faces.emplace_back(id1, id2, id3, r);
		}
	}

	void fill_colors_alt() {
		for (size_t i = 0; i < faces.size(); i++) {
			unsigned int a = faces[i].tri[0];
			unsigned int b = faces[i].tri[1];
			unsigned int c = faces[i].tri[2];

			int x = ilerp(0, wid, vertices[a].uv.x);
			int y = ilerp(0, hei, vertices[a].uv.y);
			faces[i].v_index[0] = (x*hei) + y;

			x = ilerp(0, wid, vertices[b].uv.x);
			y = ilerp(0, hei, vertices[b].uv.y);
			faces[i].v_index[1] = (x*hei) + y;

			x = ilerp(0, wid, vertices[c].uv.x);
			y = ilerp(0, hei, vertices[c].uv.y);
			faces[i].v_index[2] = (x*hei) + y;

			glm::vec3 bary;		
			glm::vec2 coords;
			double _R = faces[i].R;
			int face_res = faces[i].face_r;
			int edge_res = faces[i].edge_r;
			
			for (int a = 0; a <= _R; a++) {
				for (int b = 0; b <= (_R - a); b++) {
					if ((a == 0 && b == 0) || (a == _R && b == 0) || (a == 0 && b == _R)) {
						continue;
					}

					if (a == 0 && (b > 0 && b < _R)) {
						//C0k
						bary = glm::vec3((double)a / _R, (double)b / _R, 1.0 - ((double)a + (double)b) / _R);
						coords = barycentric_to_cartesian(bary.x, bary.y, bary.z, vertices[a].uv, vertices[b].uv, vertices[c].uv);
						x = ilerp(0, wid, coords.x);
						y = ilerp(0, hei, coords.y);
						faces[i].e_index[0].emplace_back((x*hei) + y);
					}
					else if ((a > 0 && a < _R) && b == 0) {
						//Ck0
						bary = glm::vec3((double)a / _R, (double)b / _R, 1.0 - ((double)a + (double)b) / _R);
						coords = barycentric_to_cartesian(bary.x, bary.y, bary.z, vertices[a].uv, vertices[b].uv, vertices[c].uv);
						x = ilerp(0, wid, coords.x);
						y = ilerp(0, hei, coords.y);
						faces[i].e_index[1].emplace_back((x*hei) + y);
					}
					else if (((a > 0 && a < _R)) && b == (_R - a)) {
						//Ck(R-k)
						bary = glm::vec3((double)a / _R, (double)b / _R, 1.0 - ((double)a + (double)b) / _R);
						coords = barycentric_to_cartesian(bary.x, bary.y, bary.z, vertices[a].uv, vertices[b].uv, vertices[c].uv);
						x = ilerp(0, wid, coords.x);
						y = ilerp(0, hei, coords.y);
						faces[i].e_index[2].emplace_back((x*hei) + y);
					}
					else {
						bary = glm::vec3((double)a / _R, (double)b / _R, 1.0 - ((double)a + (double)b) / _R);
						coords = barycentric_to_cartesian(bary.x, bary.y, bary.z, vertices[a].uv, vertices[b].uv, vertices[c].uv);
						x = ilerp(0, wid, coords.x);
						y = ilerp(0, hei, coords.y);
						faces[i].f_index.emplace_back((x*hei) + y);
					}
				}
			}
		}
	}


	//image information
	unsigned int wid, hei, ch, r;
	std::vector<rgb> image;

	std::vector<rface> faces;
	std::vector<vertex> vertices;
	std::vector<unsigned int> indices;
};

void print_mesh_colors_index(const mesh_colors& m)
{
	
	for (const auto& f : m.faces)
	{
		std::cout << "v_index0:  " << f.v_index[0] << std::endl;
		std::cout << "v_index1:  " << f.v_index[1] << std::endl;
		std::cout << "v_index2:  " << f.v_index[2] << std::endl;

		std::cout << "e_index0:  " << f.e_index[0] << std::endl;
		std::cout << "e_index1:  " << f.e_index[1] << std::endl;
		std::cout << "e_index2:  " << f.e_index[2] << std::endl;
	}
}

struct texture {
	unsigned char *data;
	int w;
	int h;
	int ch;
};

void build_mesh_color_texture(mesh_colors& m, rect2D& r)
{
	for (auto f : m.faces)
	{
		r.insert_1D(f.v_index[0], m.image[f.v_index[0]]);
		r.insert_1D(f.v_index[1], m.image[f.v_index[1]]);
		r.insert_1D(f.v_index[2], m.image[f.v_index[2]]);

		r.insert_1D(f.e_index[0], m.image[f.e_index[0]]);
		r.insert_1D(f.e_index[1], m.image[f.e_index[1]]);
		r.insert_1D(f.e_index[2], m.image[f.e_index[2]]);
	}
	for (size_t i = 0; i < r.data.size(); i++)
	{
		if (r.data[i].r == 0 && r.data[i].g == 0 && r.data[i].b == 0) {
			r.data[i] = rgb(246 , 164, 180);
		}
	}
}

void custom_mesh_color_texture(mesh_colors2& m, rect2D& r)
{
	for (size_t i = 0; i < m.faces.size(); i++)
	{
		r.insert_1D(m.faces[i].v_index[0], m.image[m.faces[i].v_index[0]]);
		r.insert_1D(m.faces[i].v_index[1], m.image[m.faces[i].v_index[1]]);
		r.insert_1D(m.faces[i].v_index[2], m.image[m.faces[i].v_index[2]]);

		for (unsigned int j = 0; j < m.faces[i].edge_r; j++)
		{
			r.insert_1D(m.faces[i].e_index[0][j], m.image[m.faces[i].e_index[0][j]]);

			r.insert_1D(m.faces[i].e_index[1][j], m.image[m.faces[i].e_index[1][j]]);

			r.insert_1D(m.faces[i].e_index[2][j], m.image[m.faces[i].e_index[2][j]]);
		}

		for (unsigned int z = 0; z < m.faces[i].face_r; z++) {
			r.insert_1D(m.faces[i].f_index[z], m.image[m.faces[i].f_index[z]]);
		}
		

	}
	
	//kirby fix
	for (size_t i = 0; i < r.data.size(); i++)
	{
		if ((r.data[i].r == 0) && (r.data[i].g == 0) && (r.data[i].b == 0)) {
			r.data[i] = rgb(246, 164, 180);
		}
	}

}

//void buil_alt_mesh_color(mesh_colors2& m, rect2D& r) {
//	for (size_t i = 0; i < m.faces.size(); i++) {
//		r.insert_1D(m.faces[i].v_index[0], m.image[m.faces[i].v_index[0]]);
//		for (size_t j = 0; j < m.faces[i].edge_r; j++) {
//			r.insert_1D(m.faces[i].e_index[0][j], m.image[m.faces[i].e_index[0][j]]);
//		}
//		r.insert_1D(m.faces[i].v_index[1], m.image[m.faces[i].v_index[1]]);
//
//		r.insert_1D(m.faces[i].v_index[2], m.image[m.faces[i].v_index[2]]);
//	}
//}

//build linear color array
void build_linear_mc(const mesh_colors& m, std::vector<rgb>& out) {
	std::vector<rgb> colors;
	for (const auto& f : m.faces) {
		if (std::find(colors.begin(), colors.end(), m.image[f.v_index[0]]) == colors.end()) {
			colors.emplace_back(m.image[f.v_index[0]]);
		}
		if (std::find(colors.begin(), colors.end(), m.image[f.v_index[1]]) == colors.end()) {
			colors.emplace_back(m.image[f.v_index[1]]);
		}
		if (std::find(colors.begin(), colors.end(), m.image[f.v_index[2]]) == colors.end()) {
			colors.emplace_back(m.image[f.v_index[2]]);
		}
		if (std::find(colors.begin(), colors.end(), m.image[f.e_index[0]]) == colors.end()) {
			colors.emplace_back(m.image[f.e_index[0]]);
		}
		if (std::find(colors.begin(), colors.end(), m.image[f.e_index[1]]) == colors.end()) {
			colors.emplace_back(m.image[f.e_index[1]]);
		}
		if (std::find(colors.begin(), colors.end(), m.image[f.e_index[2]]) == colors.end()) {
			colors.emplace_back(m.image[f.e_index[2]]);
		}
	}
	
	out = colors;
}

void gen_rectangle_texture(const rect2D& r, GLuint& id) 
{
	GLCall(glGenTextures(1, &id));
	GLCall(glBindTexture(GL_TEXTURE_2D, id));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, r.wid, r.hei, 0, GL_RGB, GL_UNSIGNED_BYTE, r.data.data()));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));	
}
