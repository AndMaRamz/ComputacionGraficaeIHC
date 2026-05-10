// KEYFRAMES

//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <fstream>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

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

//variables para animación
float movCoche;
float movOffset;
float rotllanta;
float rotllantaOffset;
bool avanza;
float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0;
float angulovaria = 0.0f;
float anguloAleteo = 0.0f;
float velocidadAleteo = 10.0f;  
float amplitudAleteo = 50.0f;  

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;
Texture NumerosTexture;
Texture Numero1Texture;
Texture Numero2Texture;

Model Kitt_M;
Model Llanta_M;
Model Pista_M;
Model Nave_M;
Model Ala_M;
Model Ala2_M;
Model Aeolipile_base_M;
Model Aeolipile_M;

Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;

// Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// INPUT DE KEYFRAMES
void inputKeyframes(bool* keys);
float ciclo1, ciclo2, ciclo3, ciclo4, ciclo5, ciclo6, ciclo7, ciclo8, contador = 0;
float reproduciranimacion, habilitaranimacion, guardoFrame, reinicioFrame = 0;
float ciclo9, ciclo10, ciclo11, ciclo12, ciclo13, ciclo14, ciclo15, ciclo16, ciclo17, 
ciclo18, ciclo19, ciclo20, ciclo21, ciclo22, ciclo23, ciclo24 = 0;
float terminoAnimacion = 0;  // Control para printf 

// luz direccional
DirectionalLight mainLight;

// para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Shader
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

//función para teclado de keyframes 
void inputKeyframes(bool* keys);

//cálculo del promedio de las normales para sombreado de Phong
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


	unsigned int flechaIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat flechaVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int scoreIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat scoreVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

	};

	unsigned int numeroIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	};

	GLfloat numeroVertices[] = {
		-0.5f, 0.0f, 0.5f,		0.0f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.5f,		0.25f, 0.67f,		0.0f, -1.0f, 0.0f,
		0.5f, 0.0f, -0.5f,		0.25f, 1.0f,		0.0f, -1.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

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

	Mesh* obj5 = new Mesh();
	obj5->CreateMesh(flechaVertices, flechaIndices, 32, 6);
	meshList.push_back(obj5);

	Mesh* obj6 = new Mesh();
	obj6->CreateMesh(scoreVertices, scoreIndices, 32, 6);
	meshList.push_back(obj6);

	Mesh* obj7 = new Mesh();
	obj7->CreateMesh(numeroVertices, numeroIndices, 32, 6);
	meshList.push_back(obj7);

}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}


///////////////////////////////KEYFRAMES/////////////////////

bool animacion = false;

//NEW// Keyframes
float posAvionX = 2.0f, posAvionY = 2.0f, posAvionZ = 0.0f;
float movX = 0.0f, movY = 0.0f, movZ = 0.0f;
float giroY = 0, giroX = 0, giroZ = 0;

#define MAX_FRAMES 50 
int i_max_steps = 100; 
int i_curr_steps = 7;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float movX;			//Variable para PosicionX
	float movY;			//Variable para PosicionY
	float movZ;			//Variable para PosicionZ
	float movXInc;		//Variable para IncrementoX
	float movYInc;		//Variable para IncrementoY
	float movZInc;		//Variable para IncrementoZ
	float giroY;		//Variable para giroY
	float giroX;		//Variable para giroX
	float giroZ;		//Variable para giroZ
	float giroXInc;		//Variable para IncrementogiroX
	float giroYInc;		//Variable para IncrementogiroY
	float giroZInc;		//Variable para IncrementogiroZ
}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 7;			
bool play = false;
int playIndex = 0;

void cargarKeyframes(void)
{
	std::ifstream file("keyframes.txt");



	while (file >> KeyFrame[FrameIndex].movX
		>> KeyFrame[FrameIndex].movY
		>> KeyFrame[FrameIndex].movZ
		>> KeyFrame[FrameIndex].giroX
		>> KeyFrame[FrameIndex].giroY
		>> KeyFrame[FrameIndex].giroZ)
	{
		FrameIndex++;
	}

	file.close();

}

void saveFrame(void) //tecla L
{

	printf("frameindex %d\n", FrameIndex);

	KeyFrame[FrameIndex].movX = movX;
	KeyFrame[FrameIndex].movY = movY;
	KeyFrame[FrameIndex].giroY = giroY;
	KeyFrame[FrameIndex].movZ = movZ;
	KeyFrame[FrameIndex].giroX = giroX;
	KeyFrame[FrameIndex].giroZ = giroZ;
	
	std::ofstream file("keyframes.txt", std::ios::app);
	if (file.is_open())
	{
		file << movX << " "
			<< movY << " "
			<< movZ << " "
			<< giroX << " "
			<< giroY << " "
			<< giroZ << "\n";
		file.close();
	}

	FrameIndex++;
}

void resetElements(void) //Tecla 0
{
	movX = KeyFrame[0].movX;
	movY = KeyFrame[0].movY;
	movZ = KeyFrame[0].movZ;
	giroX = KeyFrame[0].giroX;
	giroY = KeyFrame[0].giroY;
	giroZ = KeyFrame[0].giroZ;
}

void interpolation(void)
{
	KeyFrame[playIndex].movXInc = (KeyFrame[playIndex + 1].movX - KeyFrame[playIndex].movX) / i_max_steps;
	KeyFrame[playIndex].movYInc = (KeyFrame[playIndex + 1].movY - KeyFrame[playIndex].movY) / i_max_steps;
	KeyFrame[playIndex].movZInc = (KeyFrame[playIndex + 1].movZ - KeyFrame[playIndex].movZ) / i_max_steps;
	KeyFrame[playIndex].giroYInc = (KeyFrame[playIndex + 1].giroY - KeyFrame[playIndex].giroY) / i_max_steps;
	KeyFrame[playIndex].giroXInc = (KeyFrame[playIndex + 1].giroX - KeyFrame[playIndex].giroX) / i_max_steps;
	KeyFrame[playIndex].giroZInc = (KeyFrame[playIndex + 1].giroZ - KeyFrame[playIndex].giroZ) / i_max_steps;

}

void animate(void)
{
	if (play)
	{
		if (i_curr_steps >= i_max_steps) //fin de animación entre frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//Fin de toda la animación con último frame?
			{
				if (terminoAnimacion < 1)
				{
					printf("Termino la animacion. Presiona 0 para reproducir de nuevo.\n");
					terminoAnimacion++;
				}
				playIndex = 0;
				play = false;
				anguloAleteo = 0.0f;  // Resetear aleteo al terminar
			}
			else //Interpolación del próximo cuadro
			{
				i_curr_steps = 0; //Resetea contador
				interpolation();
			}
		}
		else
		{
			//Dibujar Animación
			movX += KeyFrame[playIndex].movXInc;
			movY += KeyFrame[playIndex].movYInc;
			movZ += KeyFrame[playIndex].movZInc;
			giroX += KeyFrame[playIndex].giroXInc;
			giroY += KeyFrame[playIndex].giroYInc;
			giroZ += KeyFrame[playIndex].giroZInc;
			i_curr_steps++;
		}
	}
}

///////////////* FIN KEYFRAMES*////////////////////////////

int main()
{
	SetConsoleOutputCP(65001);
	
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.5f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	Nave_M = Model();
	Nave_M.LoadModel("Models/NaveSola.obj");
	Ala_M = Model();
	Ala_M.LoadModel("Models/ala.obj");
	Ala2_M = Model();
	Ala2_M.LoadModel("Models/ala2.obj");

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

	//luz direccional
	mainLight = DirectionalLight(
		1.0f, 1.0f, 1.0f,
		0.3f, 0.3f,
		0.0f, 0.0f, -1.0f);

	//luces puntuales
	unsigned int pointLightCount = 0;
	pointLights[0] = PointLight(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 2.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;
	
	//linterna
	unsigned int spotLightCount = 0;
	/*spotLights[0] = SpotLight(
		1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;*/

	//luz fija
	spotLights[1] = SpotLight(
		0.0f, 0.0f, 1.0f,
		1.0f, 2.0f,
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);
	spotLightCount++;


	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	printf("Barra espaciadora -> Reproducir animacion\n");
	printf("0 -> Habilitar reproduccion de nuevo\n");
	printf("L -> Guardar frame\n");
	printf("P -> Habilitar guardar nuevo frame\n");
	printf("1-2 -> Mover en +X\n");
	printf("R-T -> Mover en -X\n");
	printf("7-8 -> Mover en subir)\n");
	printf("5-6 -> Mover en bajar\n");
	printf("9-Q -> Mover en +Z\n");
	printf("N-M -> Mover en -Z\n");
	printf("Y-U -> Rotar en eje +X\n");
	printf("H-J -> Rotar en eje -X\n");
	printf("3-4 -> Rotar en eje Y\n");
	printf("I-O -> Rotar en eje Z\n\n");

	movCoche = 0.0f;
	movOffset = 0.01f;
	rotllanta = 0.0f;
	rotllantaOffset = 10.0f;

	glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);
	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset(0.0f, 0.0f);
	//Keyframes
	glm::vec3 posblackhawk = glm::vec3(2.0f, 0.0f, 0.0f);

	//KEYFRAMES DECLARADOS INICIALES
	KeyFrame[0].movX = 0.0f;
	KeyFrame[0].movY = 0.0f;

	KeyFrame[1].movX = 1.0f;
	KeyFrame[1].movY = 2.0f;

	KeyFrame[2].movX = 2.0f;
	KeyFrame[2].movY = 0.0f;

	KeyFrame[3].movX = 3.0f;
	KeyFrame[3].movY = -2.0f;

	KeyFrame[4].movX = 3.0f;
	KeyFrame[4].movY = -2.0f;
	KeyFrame[4].giroY = 180.0f;

	KeyFrame[5].movX = 0.0f;
	KeyFrame[5].movY = 0.0f;
	KeyFrame[5].giroY = 180;

	KeyFrame[6].movX = 0.0f;
	KeyFrame[6].movY = 0.0f;

	////Loop mientras no se cierra la ventana
	saveFrame();
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		angulovaria += 10.0f * deltaTime;

		if (movCoche < 30.0f)
		{
			movCoche -= movOffset * deltaTime;
		}
		rotllanta += rotllantaOffset * deltaTime;

		// aleteo
		if (play)
		{
			anguloAleteo += velocidadAleteo * deltaTime;
			if (anguloAleteo > 360.0f)
				anguloAleteo -= 360.0f;
		}

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		//para keyframes
		inputKeyframes(mainWindow.getsKeys());
		animate();

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
		uniformTextureOffset = shaderList[0].getOffsetLocation();

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// luz ligada a la cámara de tipo flash
		/*lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());*/

		// información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		// reiniciar variables antes de que sean enviadas al shader
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		//nave
		model = glm::mat4(1.0);
		posblackhawk = glm::vec3(posAvionX + movX, posAvionY + movY, posAvionZ + movZ);
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		model = glm::translate(model, posblackhawk);
		model = glm::rotate(model, 180*toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, (giroX)*toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, (giroY)*toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, (giroZ)*toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		modelaux = model; 
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Nave_M.RenderModel();

		// calcula  ángulo de aleteo 
		float aleteoActual = amplitudAleteo * sin(anguloAleteo * toRadians);
		
		// ala izq
		model = modelaux;
		model = glm::translate(model, glm::vec3(-0.3f, 0.3f, -0.3f));
		model = glm::rotate(model, aleteoActual * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Ala_M.RenderModel();
		
		// ala der
		model = modelaux;
		model = glm::rotate(model, -aleteoActual * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Ala2_M.RenderModel();
		
		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}


void inputKeyframes(bool* keys)
{
	if (keys[GLFW_KEY_SPACE])
	{
		if (reproduciranimacion < 1)
		{
			if (play == false && (FrameIndex > 1))
			{
				resetElements();
				//First Interpolation				
				interpolation();
				play = true;
				playIndex = 0;
				i_curr_steps = 0;
				anguloAleteo = 0.0f;  // reset aleteo
				reproduciranimacion++;
				printf("\n presiona 0 para habilitar reproducir de nuevo la animación\n");
				habilitaranimacion = 0;

			}
			else
			{
				play = false;

			}
		}
	}
	if (keys[GLFW_KEY_0])
	{
		if (habilitaranimacion < 1 && reproduciranimacion>0)
		{
			printf("Ya puedes reproducir de nuevo la animación con la tecla de barra espaciadora\n");
			habilitaranimacion++;
			reproduciranimacion = 0;
			terminoAnimacion = 0;  // reset para imprimir de nuevo cuando termine

		}
	}
	if (keys[GLFW_KEY_L])
	{
		if (guardoFrame < 1)
		{
			saveFrame();
			printf("movX es: %f\n", movX);
			printf("movY es: %f\n", movY);
			printf("movZ es: %f\n", movZ);
			printf("giroX es: %f\n", giroX);
			printf("giroY es: %f\n", giroY);
			printf("giroZ es: %f\n", giroZ);
			printf("presiona P para habilitar guardar otro frame\n");
			guardoFrame++;
			reinicioFrame = 0;
		}
	}
	if (keys[GLFW_KEY_P])
	{
		if (reinicioFrame < 1)
		{
			reinicioFrame++;
			guardoFrame = 0;
			printf("Ya puedes guardar otro frame presionando la tecla L\n");
		}
	}
	if (keys[GLFW_KEY_1])
	{
		if (ciclo1 < 1)
		{
			movX += 1.0f;
			printf("\n movX es: %f\n", movX);
			ciclo1++;
			ciclo2 = 0;
			printf("\n Presiona la tecla 2 para poder habilitar la variable\n");
		}
	}
	if (keys[GLFW_KEY_2])
	{
		if (ciclo2 < 1)
		{
			ciclo1 = 0;
			ciclo2++;
			printf("\n Ya puedes modificar tu variable presionando la tecla 1\n");
		}
	}

	if (keys[GLFW_KEY_3])
	{
		if (ciclo3 < 1)
		{
			giroY += 5.0f;
			printf("\n giroY es: %f\n", giroY);
			ciclo3++;
			ciclo4 = 0;
			printf("\n Presiona la tecla 4 para poder habilitar la variable\n");
		}
	}

	if (keys[GLFW_KEY_4])
	{
		if (ciclo4 < 1)
		{
			ciclo3 = 0;
			ciclo4++;
			printf("\n Ya puedes modificar tu variable presionando la tecla 3\n");
		}
	}

	if (keys[GLFW_KEY_5])
	{
		if (ciclo5 < 1)
		{
			movY -= 1.0f;
			printf("\n movY es: %f\n", movY);
			ciclo5++;
			ciclo6 = 0;
			printf("\n Presiona la tecla 6 para poder habilitar la variable\n");
		}
	}

	if (keys[GLFW_KEY_6])
	{
		if (ciclo6 < 1)
		{
			ciclo5 = 0;
			ciclo6++;
			printf("\n Ya puedes modificar tu variable presionando la tecla 5\n");
		}
	}

	if (keys[GLFW_KEY_7])
	{
		if (ciclo7 < 1)
		{
			movY += 1.0f;
			printf("\n movY es: %f\n", movY);
			ciclo7++;
			ciclo8 = 0;
			printf("\n Presiona la tecla 8 para poder habilitar la variable\n");
		}
	}

	if (keys[GLFW_KEY_8])
	{
		if (ciclo8 < 1)
		{
			ciclo7 = 0;
			ciclo8++;
			printf("\n Ya puedes modificar tu variable presionando la tecla 7\n");
		}
	}

	if (keys[GLFW_KEY_9])
	{
		if (ciclo9 < 1)
		{
			movZ += 1.0f;
			printf("\n movZ es: %f\n", movZ);
			ciclo9++;
			ciclo10 = 0;
			printf("\n Presiona la tecla Q para poder habilitar la variable\n");
		}
	}

	if (keys[GLFW_KEY_Q])
	{
		if (ciclo10 < 1)
		{
			ciclo9 = 0;
			ciclo10++;
			printf("\n Ya puedes modificar tu variable presionando la tecla 9\n");
		}
	}

	if (keys[GLFW_KEY_N])
	{
		if (ciclo11 < 1)
		{
			movZ -= 1.0f;
			printf("\n movZ es: %f\n", movZ);
			ciclo11++;
			ciclo12 = 0;
			printf("\n Presiona la tecla M para poder habilitar la variable\n");
		}
	}

	if (keys[GLFW_KEY_M])
	{
		if (ciclo12 < 1)
		{
			ciclo11 = 0;
			ciclo12++;
			printf("\n Ya puedes modificar tu variable presionando la tecla N\n");
		}
	}

	if (keys[GLFW_KEY_R])
	{
		if (ciclo13 < 1)
		{
			movX -= 1.0f;
			printf("\n movX es: %f\n", movX);
			ciclo13++;
			ciclo14 = 0;
			printf("\n Presiona la tecla T para poder habilitar la variable\n");
		}
	}

	if (keys[GLFW_KEY_T])
	{
		if (ciclo14 < 1)
		{
			ciclo13 = 0;
			ciclo14++;
			printf("\n Ya puedes modificar tu variable presionando la tecla R\n");
		}
	}

	if (keys[GLFW_KEY_Y])
	{
		if (ciclo15 < 1)
		{
			giroX += 5.0f;
			printf("\n giroX es: %f\n", giroX);
			ciclo15++;
			ciclo16 = 0;
			printf("\n Presiona la tecla U para poder habilitar la variable\n");
		}
	}

	if (keys[GLFW_KEY_U])
	{
		if (ciclo16 < 1)
		{
			ciclo15 = 0;
			ciclo16++;
			printf("\n Ya puedes modificar tu variable presionando la tecla Y\n");
		}
	}

	if (keys[GLFW_KEY_I])
	{
		if (ciclo17 < 1)
		{
			giroZ += 5.0f;
			printf("\n giroZ es: %f\n", giroZ);
			ciclo17++;
			ciclo18 = 0;
			printf("\n Presiona la tecla O para poder habilitar la variable\n");
		}
	}

	if (keys[GLFW_KEY_O])
	{
		if (ciclo18 < 1)
		{
			ciclo17 = 0;
			ciclo18++;
			printf("\n Ya puedes modificar tu variable presionando la tecla I\n");
		}
	}

	if (keys[GLFW_KEY_H])
	{
		if (ciclo19 < 1)
		{
			giroX -= 5.0f;
			printf("\n giroX es: %f\n", giroX);
			ciclo19++;
			ciclo20 = 0;
			printf("\n Presiona la tecla J para poder habilitar la variable\n");
		}
	}

	if (keys[GLFW_KEY_J])
	{
		if (ciclo20 < 1)
		{
			ciclo19 = 0;
			ciclo20++;
			printf("\n Ya puedes modificar tu variable presionando la tecla H\n");
		}
	}

}