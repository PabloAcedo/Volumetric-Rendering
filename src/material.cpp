#include "material.h"
#include "texture.h"
#include "application.h"


StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

StandardMaterial::StandardMaterial(const char* texturename)
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs"); //assign texture shader to the material
	texture = Texture::Get(texturename); //get the texture
}

StandardMaterial::~StandardMaterial()
{

}

void StandardMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);

	shader->setUniform("u_color", color);

	if (texture)
		shader->setUniform("u_texture", texture);
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	//set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
}

void StandardMaterial::update(double s_e) {

}

WireframeMaterial::WireframeMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial()
{

}

void WireframeMaterial::render(Mesh* mesh, Matrix44 model, Camera * camera)
{
	if (shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//enable shader
		shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

/***********************************************************************************************/
//volume material
VolumeMaterial::VolumeMaterial() {
	slider = 0.5;
}

VolumeMaterial::VolumeMaterial(Texture* _texture) {

	//init float values
	slider = 0.5; brightness = 10.0; threshold = 0.2; step_factor = 100.0;

	//init bools
	slider_bool = false; jittering = true; tf_bool = false; lighting = false; fixed_scaling = true; clipping = true;
	//get volume 3D texture
	texture = _texture;

	//get volume shader
	shader = Shader::Get("data/shaders/volume_rendering.vs", "data/shaders/volume_rendering.fs");

	//get transfer function texture
	tf = Texture::Get("data/textures/tf_lut.png");
	
	//clipping plane
	volume_clipping = Vector4(0.0,8.0,0.0,-4.0);

	//init light
	light_pos = Vector3(50.0, 50.0, 0);
	light_color = Vector3(1.0, 1.0, 1.0);
	
}


VolumeMaterial::~VolumeMaterial() {

}

void VolumeMaterial::setUniforms(Camera* camera, Matrix44 model) {
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_pos", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_time", Application::instance->time);

	shader->setUniform("u_color", color);

	if (texture)
		shader->setUniform("u_vol_texture", texture,0);

	shader->setUniform("u_slider", slider);
	shader->setUniform("u_slider_bool", slider_bool);
	shader->setUniform("u_brightness", brightness);

	if (tf)
		shader->setUniform("u_tf", tf, 1); //transfer function

	shader->setUniform("u_jittering", jittering);

	shader->setUniform("u_factor", step_factor);

	shader->setUniform("u_tf_bool", tf_bool);

	shader->setUniform("u_clip", volume_clipping);
	shader->setUniform("u_vol_clip", clipping);

	shader->setUniform("u_light_pos", light_pos);
	shader->setUniform("u_light_color", light_color);
	shader->setUniform("u_lighting", lighting);

	shader->setUniform("u_threshold", threshold);

}

void VolumeMaterial::render(Mesh* mesh, Matrix44 model, Camera * camera) {
	//set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void VolumeMaterial::renderInMenu() {

	ImGui::Checkbox("Auto scale", &fixed_scaling);
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
	ImGui::Checkbox("Slider", &slider_bool);
	ImGui::Checkbox("Jittering", &jittering);
	ImGui::Checkbox("Transfer function", &tf_bool);
	ImGui::Checkbox("Use clipping plane", &clipping);

	if(slider_bool)
		ImGui::SliderFloat("Slice", &slider, 0.01, 1.0);

	ImGui::SliderFloat("Brightness", &brightness, 1.0, 10.0);

	ImGui::SliderFloat("Step factor", &step_factor, 50.0, 100.0);

	ImGui::SliderFloat("Threshold", &threshold, 0.0, 1.0);

	ImGui::Checkbox("Direct Light", &lighting);

	if (lighting) {
		ImGui::DragFloat3("Light Position", (float*)&light_pos); //light position edit
		ImGui::ColorEdit3("Light Color", (float*)&light_color); // Edit 3 floats representing a color
	}

}

void VolumeMaterial::update(double seconds_elapsed) {

	//auto scale if fixed_scaling activated (this is done to have the correct values in slider mode),
	//if this is activated the model matrix could not be modified.
	if (slider_bool && fixed_scaling) {
		Application::instance->node_list[0]->model.setScale(4.0, 4.0, 0.25);
	}
	else if(fixed_scaling){
		Application::instance->node_list[0]->model.setScale(5.0, 5.0, 9.0);
	}
}

/***********************************************************************************************/