
/*
Práctica 7: Iluminación 1
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

//TEXTURAS
Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture OctaTexture;

//MODELOS
Model ChasisAutoModelo;
Model CofreAutoModelo;
Model LlantaDelDerModelo;
Model LlantaDelIzqModelo;
Model LlantaTrasDerModelo;
Model LlantaTrasIzqModelo;
Model Nave;
Model Farola;
Model Pecera;
Model TapaPecera;
Model antena_pez;
Model cuerpo_pez;
Model foco_pez;

Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;

//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
bool farolaEncendida = true;
bool pezLuzEncendida = true;
SpotLight spotLights[MAX_SPOT_LIGHTS];
bool luzRojaActiva = false;
bool luzAmarillaActiva = true;

int colorIndex = 0;
glm::vec3 coloresFaro[6] = {
	glm::vec3(1.0f, 0.0f, 0.0f), // rojo
	glm::vec3(1.0f, 1.0f, 0.0f), // amarillo
	glm::vec3(0.0f, 1.0f, 0.0f), // verde
	glm::vec3(0.0f, 1.0f, 1.0f), // cian
	glm::vec3(0.0f, 0.0f, 1.0f), // azul
	glm::vec3(1.0f, 0.0f, 1.0f)  // magenta
};

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";

//función de calculo de normales por promedio de vértices 
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4,5,6,
	   4,6,7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,


	};

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);

	Mesh* obj4 = new Mesh();
	obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);

}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

void CrearDado()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,

		// back
		8, 9, 10,
		10, 11, 8,

		// left
		12, 13, 14,
		14, 15, 12,
		// bottom
		16, 17, 18,
		18, 19, 16,
		// top
		20, 21, 22,
		22, 23, 20,

		// right
		4, 5, 6,
		6, 7, 4,

	};	GLfloat cubo_vertices[] = {
			// front
			//x		y		z		S		T			NX		NY		NZ
			-0.5f, -0.5f,  0.5f,	0.26f,  0.34f,		0.0f,	0.0f,	1.0f,	//0
			0.5f, -0.5f,  0.5f,		0.49f,	0.34f,		0.0f,	0.0f,	1.0f,	//1
			0.5f,  0.5f,  0.5f,		0.49f,	0.66f,		0.0f,	0.0f,	1.0f,	//2
			-0.5f,  0.5f,  0.5f,	0.26f,	0.66f,		0.0f,	0.0f,	1.0f,	//3
			// right
			//x		y		z		S		T
			0.5f, -0.5f,  0.5f,	    0.0f,  0.0f,		-1.0f,	0.0f,	0.0f,
			0.5f, -0.5f,  -0.5f,	1.0f,	0.0f,		-1.0f,	0.0f,	0.0f,
			0.5f,  0.5f,  -0.5f,	1.0f,	1.0f,		-1.0f,	0.0f,	0.0f,
			0.5f,  0.5f,  0.5f,	    0.0f,	1.0f,		-1.0f,	0.0f,	0.0f,
			// back
			-0.5f, -0.5f, -0.5f,	0.0f,  0.0f,		0.0f,	0.0f,	1.0f,
			0.5f, -0.5f, -0.5f,		1.0f,	0.0f,		0.0f,	0.0f,	1.0f,
			0.5f,  0.5f, -0.5f,		1.0f,	1.0f,		0.0f,	0.0f,	1.0f,
			-0.5f,  0.5f, -0.5f,	0.0f,	1.0f,		0.0f,	0.0f,	1.0f,

			// left
			//x		y		z		S		T
			-0.5f, -0.5f,  -0.5f,	0.0f,  0.0f,		1.0f,	0.0f,	0.0f,
			-0.5f, -0.5f,  0.5f,	1.0f,	0.0f,		1.0f,	0.0f,	0.0f,
			-0.5f,  0.5f,  0.5f,	1.0f,	1.0f,		1.0f,	0.0f,	0.0f,
			-0.5f,  0.5f,  -0.5f,	0.0f,	1.0f,		1.0f,	0.0f,	0.0f,

			// bottom
			//x		y		z		S		T
			-0.5f, -0.5f,  0.5f,	0.0f,  0.0f,		0.0f,	1.0f,	0.0f,
			0.5f,  -0.5f,  0.5f,	1.0f,	0.0f,		0.0f,	1.0f,	0.0f,
			 0.5f,  -0.5f,  -0.5f,	1.0f,	1.0f,		0.0f,	1.0f,	0.0f,
			-0.5f, -0.5f,  -0.5f,	0.0f,	1.0f,		0.0f,	1.0f,	0.0f,

			//UP
			 //x		y		z		S		T
			 -0.5f, 0.5f,  0.5f,	0.0f,  0.0f,		0.0f,	-1.0f,	0.0f,
			 0.5f,  0.5f,  0.5f,	1.0f,	0.0f,		0.0f,	-1.0f,	0.0f,
			  0.5f, 0.5f,  -0.5f,	1.0f,	1.0f,		0.0f,	-1.0f,	0.0f,
			 -0.5f, 0.5f,  -0.5f,	0.0f,	1.0f,		0.0f,	-1.0f,	0.0f,

	};

	Mesh* dado = new Mesh();
	dado->CreateMesh(cubo_vertices, cubo_indices, 192, 36);
	meshList.push_back(dado);

}

void CrearDadoOct()
{
	unsigned int oct_indices[] = {
		0,1,2,
		3,4,5,
		6,7,8,
		9,10,11,
		12,13,14,
		15,16,17,
		18,19,20,
		21,22,23
	};

	GLfloat oct_vertices[] = {

		//x       y      z         u     v       nx   ny   nz

		// Cara 1 (cara mujer con aro)
		0.0f,	0.5f,	0.0f,     0.25f,0.98f,	0.0f,-1.0f,-1.0f,
		-0.5f,	0.0f,	0.0f,     0.02f,0.75f,	0.0f,-1.0f,-1.0f,
		0.0f,	0.0f,	0.5f,     0.5f,0.75f,	0.0f,-1.0f,-1.0f,

		// Cara 2 (cisne)
		0.0f,	0.5f,	0.0f,     0.75f,0.5f,	-1.0f,0.0f,-1.0f,
		0.0f,	0.0f,	0.5f,     0.98f,0.74f,  -1.0f,0.0f,-1.0f,
		0.5f,	0.0f,	0.0f,     0.55f,0.74f,	-1.0f,0.0f,-1.0f,

		// Cara 3 (paisaje chino)
		0.0f,	0.5f,	0.0f,	 0.75f,	0.5f,	0.0f,0.0f,1.0f,
		0.5f,	0.0f,	0.0f,	 0.53f,	0.26f,	0.0f,0.0f,1.0f,
		0.0f,	0.0f,	-0.5f,	 0.98f, 0.26f,	0.0f,0.0f,1.0f,

		// Cara 4 (mano en agua)
		0.0f,	0.5f,	0.0f,	 0.25f,	0.02f,  1.0f,-1.0f,0.0f,
		0.0f,	0.0f,	-0.5f,	 0.5f,	0.25f,  1.0f,-1.0f,0.0f,
		-0.5f,	0.0f,	0.0f,	 0.0f,  0.25f,  1.0f,-1.0f,0.0f,

		// Cara 5 (flor)
		0.0f,	-0.5f,	0.0f,    0.25f,	0.5f,   0.0f,1.0f,-1.0f,
		0.0f,	0.0f,	0.5f,    0.5f,	0.75f,  0.0f,1.0f,-1.0f,
		-0.5f,	0.0f,	0.0f,    0.0f,	0.75f,  0.0f,1.0f,-1.0f,

		// Cara 6 (persona con máscara)
		0.0f,	-0.5f,	0.0f,    0.5f,	0.75f,   0.0f,1.0f,-1.0f,
		0.5f,	0.0f,	0.0f,    0.25f,	0.5f,    0.0f,1.0f,-1.0f,
		0.0f,	0.0f,	0.5f,    0.75f,	0.5f,    0.0f,1.0f,-1.0f,

		// Cara 7 (caballo)
		0.0f,	-0.5f,	0.0f,    0.5f,	0.25f,	 -1.0f,1.0f,0.0f,
		0.0f,	0.0f,	-0.5f,   0.74f,	0.5f,    -1.0f,1.0f,0.0f,
		0.5f,	0.0f,	0.0f,    0.26f,	0.5f,    -1.0f,1.0f,0.0f,

		// Cara 8 (ojos)
		0.0f,	-0.5f,	0.0f,    0.25f,	0.5f,    0.0f,0.0f,1.0f,
		-0.5f,	0.0f,	0.0f,    0.0f,	0.25f,   0.0f,0.0f,1.0f,
		0.0f,	0.0f,	-0.5f,   0.5f,	0.25f,   0.0f,0.0f,1.0f
	};

	Mesh* dadoOct = new Mesh();
	dadoOct->CreateMesh(oct_vertices, oct_indices, 192, 24);
	meshList.push_back(dadoOct);

}

int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();
	glEnable(GL_DEPTH_TEST);
	CreateObjects();
	CrearDado();
	CrearDadoOct();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	AgaveTexture = Texture("Textures/Agave.tga");
	AgaveTexture.LoadTextureA();
	OctaTexture = Texture("Textures/OctaTexture.jpg");
	OctaTexture.LoadTexture();

	// MODELOS
	ChasisAutoModelo = Model();
	ChasisAutoModelo.LoadModel("Models/Chasis.obj");
	CofreAutoModelo = Model();
	CofreAutoModelo.LoadModel("Models/Cofre.obj");
	LlantaDelDerModelo = Model();
	LlantaDelDerModelo.LoadModel("Models/LlDelDer.obj");
	LlantaDelIzqModelo = Model();
	LlantaDelIzqModelo.LoadModel("Models/LlDelIzq.obj");
	LlantaTrasDerModelo = Model();
	LlantaTrasDerModelo.LoadModel("Models/LlTrasDer.obj");
	LlantaTrasIzqModelo = Model();
	LlantaTrasIzqModelo.LoadModel("Models/LlTrasIzq.obj");
	Nave = Model();
	Nave.LoadModel("Models/nave.obj");
	Farola = Model();
	Farola.LoadModel("Models/farol.obj");
	Pecera = Model();
	Pecera.LoadModel("Models/Pecera.obj");
	antena_pez = Model();
	antena_pez.LoadModel("Models/antena_pez.obj");
	cuerpo_pez = Model();
	cuerpo_pez.LoadModel("Models/cuerpo_pez.obj");
	foco_pez = Model();
	foco_pez.LoadModel("Models/foco_pez.obj");

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);


	//luz direccional, sólo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.3f,
		0.0f, 0.0f, -1.0f);

	// LUCES PUNTUALES
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Declaración de primer luz puntual
	pointLights[0] = PointLight(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		-6.0f, 1.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	pointLights[1] = PointLight(
		1.0f, 0.95f, 0.8f,   // blanco
		1.5f, 3.0f,         // intensidad
		9.0f, 4.0f, 7.0f,   // posición (altura del foco)
		1.0f, 0.09f, 0.032f);
	pointLightCount++;

	pointLights[2] = PointLight(
		0.0f, 0.0f, 1.0f,   // azul
		2.0f, 2.0f,         // intensidad
		9.0f, 4.0f, 7.0f,   // posición (altura del foco)
		1.0f, 1.0f, 1.0f);
	pointLightCount++;

	unsigned int spotLightCount = 0;

	//SPOTLIGHTS
	//linterna
	spotLights[0] = SpotLight(
		1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;

	//luz fija
	spotLights[1] = SpotLight(
		0.0f, 1.0f, 0.0f,
		1.0f, 2.0f,
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);
	spotLightCount++;
	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	//Luz de auto
	spotLights[2] = SpotLight(
		0.0f, 0.0f, 1.0f,   // azul
		10.0f, 10.0f,         // intensidad
		0.0f, 0.0f, 0.0f,   // posición inicial
		0.0f, 0.0f, 1.0f,   // dirección
		1.0f, 0.20f, 0.20f,
		8.0f);
	spotLightCount++;

	//Luz nave
	spotLights[3] = SpotLight(
		1.0f, 1.0f, 0.0f,   // amarillo
		1.0f, 1.0f,         // intensidad
		0.0f, 0.0f, 0.0f,   // posición inicial
		0.0f, -1.0f, 0.0f,  // dirección (abajo)
		1.0f, 0.20f, 0.20f,	// atenuación
		25.0f);				// ángulo del cono
	spotLightCount++;

	//Luz nave
	spotLights[4] = SpotLight(
		1.0f, 0.0f, 0.0f,   // rojo
		5.0f, 5.0f,         // intensidad
		0.0f, 0.0f, 0.0f,   // posición inicial
		0.0f, -1.0f, 0.0f,  // dirección (abajo)
		1.0f, 0.20f, 0.20f,	// atenuación
		35.0f);				// ángulo del cono
	spotLightCount++;

	// Luz pez 2
	spotLights[5] = SpotLight(
		1.0f, 0.4f, 0.8f,   // rosa
		3.0f, 3.0f,         // intensidad
		0.0f, 0.0f, 0.0f,   // posición inicial
		0.0f, -1.0f, 0.0f,  // dirección inicial
		1.0f, 0.2f, 0.1f,   // atenuación
		20.0f);             // ángulo del cono
	spotLightCount++;

	//se crean mas luces puntuales y spotlight 

	GLuint uniformProjection = 0,
		uniformModel = 0,
		uniformView = 0,
		uniformEyePosition = 0,
		uniformSpecularIntensity = 0,
		uniformShininess = 0;

	GLuint uniformColor = 0;

	glm::vec3 posicionAuto;
	glm::vec3 frenteAuto;
	glm::vec3 posicionFaro;
	glm::vec3 posicionHeli;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	int ejeActivo = 1;
	glm::vec3 direccionPez = glm::vec3(0.0f, -1.0f, 0.0f);
	float velocidadDireccion = 0.02f;

	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//Recibir eventos del usuario
		glfwPollEvents();

		//Control de teclas

		// farola
		static bool teclaF = false;
		if (mainWindow.getsKeys()[GLFW_KEY_F] && !teclaF)
		{
			farolaEncendida = !farolaEncendida;
			teclaF = true;
		}
		if (!mainWindow.getsKeys()[GLFW_KEY_F])
		{
			teclaF = false;
		}

		// pez
		static bool teclaP = false;
		if (mainWindow.getsKeys()[GLFW_KEY_P] && !teclaP)
		{
			pezLuzEncendida = !pezLuzEncendida;   // apagar / prender luz del pez
			teclaP = true;
		}
		if (!mainWindow.getsKeys()[GLFW_KEY_P])
		{
			teclaP = false;
		}

		// color de luz de faro de auto 
		static bool teclaC = false;
		if (mainWindow.getsKeys()[GLFW_KEY_C] && !teclaC)
		{
			colorIndex++;
			if (colorIndex > 5)
				colorIndex = 0;

			teclaC = true;
		}
		if (!mainWindow.getsKeys()[GLFW_KEY_C])
		{
			teclaC = false;
		}

		// luces de nave
		static bool teclaH = false;
		static bool teclaJ = false;
		if (mainWindow.getsKeys()[GLFW_KEY_H] && !teclaH)
		{
			luzRojaActiva = true;
			luzAmarillaActiva = false;
			teclaH = true;
		}
		if (!mainWindow.getsKeys()[GLFW_KEY_H])
		{
			teclaH = false;
		}
		if (mainWindow.getsKeys()[GLFW_KEY_J] && !teclaJ)
		{
			luzRojaActiva = false;
			luzAmarillaActiva = true;
			teclaJ = true;
		}
		if (!mainWindow.getsKeys()[GLFW_KEY_J])
		{
			teclaJ = false;
		}

		// spotlight del pez
		static bool teclaX = false;
		static bool teclaY = false;
		static bool teclaZ = false;
		if (mainWindow.getsKeys()[GLFW_KEY_X])
		{
			ejeActivo = 1;
		}
		if (mainWindow.getsKeys()[GLFW_KEY_Y])
		{
			ejeActivo = 2;
		}
		if (mainWindow.getsKeys()[GLFW_KEY_Z])
		{
			ejeActivo = 3;
		}

		// mover dirección del spotlight
		if (mainWindow.getsKeys()[GLFW_KEY_LEFT])
		{
			if (ejeActivo == 1)
				direccionPez.x -= velocidadDireccion;

			if (ejeActivo == 2)
				direccionPez.y -= velocidadDireccion;

			if (ejeActivo == 3)
				direccionPez.z -= velocidadDireccion;
		}
		if (mainWindow.getsKeys()[GLFW_KEY_RIGHT])
		{
			if (ejeActivo == 1)
				direccionPez.x += velocidadDireccion;

			if (ejeActivo == 2)
				direccionPez.y += velocidadDireccion;

			if (ejeActivo == 3)
				direccionPez.z += velocidadDireccion;
		}
		direccionPez = glm::normalize(direccionPez);

		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// luz ligada a la cámara de tipo flash
		//sirve para que en tiempo de ejecución (dentro del while) se cambien propiedades de la luz
		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());
		//spotLights[1].SetPos(poscoche + glm::vec(x, y, cofre));

		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);

		PointLight lucesActivas[3];
		int contadorLuces = 0;
		lucesActivas[contadorLuces++] = pointLights[0];

		// farola
		if (farolaEncendida)
		{
			lucesActivas[contadorLuces++] = pointLights[1];
		}

		// luz del pez
		if (pezLuzEncendida)
		{
			lucesActivas[contadorLuces++] = pointLights[2];
		}

		shaderList[0].SetPointLights(lucesActivas, contadorLuces);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		meshList[2]->RenderMesh();

		//Instancia del coche 
		model = glm::mat4(1.0);
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, mainWindow.getMovGral()));
		modelaux = model;
		model = glm::scale(model, glm::vec3(0.06f, 0.06f, 0.06f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		ChasisAutoModelo.RenderModel();

		posicionAuto = glm::vec3(modelaux[3]);
		frenteAuto = glm::normalize(glm::vec3(modelaux * glm::vec4(0, 0, 1, 0)));
		posicionFaro = posicionAuto + frenteAuto * 2.5f + glm::vec3(0.0f, 0.2f, 0.0f);

		// Luz faro auto
		spotLights[2].SetFlash(posicionFaro, frenteAuto);
		spotLights[2].SetColor(
			coloresFaro[colorIndex].x,
			coloresFaro[colorIndex].y,
			coloresFaro[colorIndex].z
		);

		//Llanta delantera izquierda
		model = modelaux;
		model = glm::scale(model, glm::vec3(0.06f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		LlantaDelIzqModelo.RenderModel();

		//Llanta trasera izquierda
		model = modelaux;
		model = glm::scale(model, glm::vec3(0.06f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		LlantaTrasIzqModelo.RenderModel();

		//Llanta delantera derecha
		model = modelaux;
		model = glm::scale(model, glm::vec3(0.06f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		LlantaDelDerModelo.RenderModel();

		//Llanta trasera derecha
		model = modelaux;
		model = glm::scale(model, glm::vec3(0.06f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		LlantaTrasDerModelo.RenderModel();

		//Cofre
		model = modelaux;
		model = glm::scale(model, glm::vec3(0.06f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		CofreAutoModelo.RenderModel();

		//Nave
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(mainWindow.getMovGral2(), 5.0f, 6.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Nave.RenderModel();
		posicionHeli = glm::vec3(model[3]);

		// Luces de la nave
		if (luzAmarillaActiva)
		{
			spotLights[3].SetFlash(
				posicionHeli + glm::vec3(-0.9f, -0.5f, 0.3f),
				glm::vec3(-1.3f, -1.0f, 0.3f)
			);
		}
		else
		{
			spotLights[3].SetFlash(glm::vec3(0.0f), glm::vec3(0.0f));
		}

		if (luzRojaActiva)
		{
			spotLights[4].SetFlash(
				posicionHeli + glm::vec3(0.9f, -0.5f, -0.3f),
				glm::vec3(1.3f, -1.0f, -0.3f)
			);
		}
		else
		{
			spotLights[4].SetFlash(glm::vec3(0.0f), glm::vec3(0.0f));
		}

		// Farola
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(9.0f, -1.0f, 7.0f));
		modelaux = model;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Farola.RenderModel();

		//	dado de 8 caras y texturizado por código
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(2.0f, 4.0f, -2.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		OctaTexture.UseTexture();
		meshList[5]->RenderMesh();
		
		// pez abisal
		glm::mat4 modelPez = glm::mat4(1.0f);

		glm::vec3 posPez = glm::vec3(
			mainWindow.getPezX(),
			mainWindow.getPezY(),
			mainWindow.getPezZ()
		);

		modelPez = glm::translate(modelPez, posPez);
		modelPez = glm::scale(modelPez, glm::vec3(0.4f));

		pointLights[2].SetPosition(
			posPez.x,
			posPez.y,
			posPez.z
		);
		// Spotlight del pez
		spotLights[5].SetFlash(
			posPez,
			direccionPez
		);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		// Antena
		model = modelPez;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		antena_pez.RenderModel();
		// Cuerpo
		model = modelPez;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		cuerpo_pez.RenderModel();
		// Foco
		model = modelPez;
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		foco_pez.RenderModel();

		// Pecera
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(15.0f, -1.0f, -8.0f));
		model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Pecera.RenderModel();

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		//Agave ¿qué sucede si lo renderizan antes del coche y el helicóptero?
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}