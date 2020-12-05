#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"

//struct that represent a light
struct sLight {
	vec3 pos = vec3(50.0,50.0,0.0); //position
	vec3 ambient; //ambient color
	vec3 diffuse = vec3(1.0,1.0,1.0); //diffuse color
	vec3 specular = vec3(1.0, 1.0, 1.0); //specular color
};

enum eMaterialType : int {STANDARD, SKYBOX, PBR};

class Material {
public:

	eMaterialType type;

	Shader* shader = NULL;
	Texture* texture = NULL;
	vec4 color;

	virtual void setUniforms(Camera* camera, Matrix44 model) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera * camera) = 0;
	virtual void renderInMenu() = 0;
	virtual void update(double seconds_elapsed) = 0;
};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	StandardMaterial(const char* texturename);
	~StandardMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
	void update(double seconds_elapsed);
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

class VolumeMaterial : public StandardMaterial {
public:

	//several floats
	float slider; float brightness; float step_factor; float threshold;

	//several bools (activate/deactivate in the IMGUI)
	bool slider_bool; bool jittering; 
	bool tf_bool; bool clipping;
	bool lighting; bool fixed_scaling;

	Texture* tf = NULL; //transfer function
	Vector4 volume_clipping; //clipping plane

	//light factors
	Vector3 light_pos; Vector3 light_color; 

	VolumeMaterial();
	VolumeMaterial(Texture* _texture);
	~VolumeMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
	void update(double seconds_elapsed);
};



#endif