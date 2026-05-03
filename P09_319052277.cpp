
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

//para iluminaci�n
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

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;
Texture NumerosTexture;
Texture Numero1Texture;
Texture Numero2Texture;
Texture SmokeTexture;
Texture FireTexture;

Model Kitt_M;
Model Llanta_M;
Model Pista_M;
Model Nave_M;
Model Ala_M;
Model Aeolipile_base_M;
Model Aeolipile_M;
Model CatapultaBase_M;
Model CatapultaBrazo_M;
Model Bola_M;

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
SpotLight spotLights[MAX_SPOT_LIGHTS];

//variables generales
float tiempo = 0.0f;
float angulovaria = 0.0f;

// variables del coche
float movCoche = 0.0f;
float movOffset = 0.01f;
float rotllanta = 0.0f;
float rotllantaOffset = 10.0f;

// variables de textura animada
float toffsetflechau = 0.0f;
float toffsetflechav = 0.0f;
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0f;

// Pos inicial canasta de catapulta
const glm::vec3 BOLA_INICIO(-3.7f, 4.9f, 0.2f);
const float BOLA_HOYO_X = 20.0f;
const float BOLA_HOYO_Z = 18.0f;

// Variables practica animacion compleja
int   estado = 0;
float tiempoEstado = 0.0f;
float rotacionBola = 0.0f;
float anguloBrazo = 0.0f;
float humoEscala = 0.0f;
float bola_t = 0.0f;
bool  bolaActiva = false;
bool  bolaEnHoyo = false;
glm::vec3 bolaPos = BOLA_INICIO;
glm::vec3 bolaLanzada = BOLA_INICIO; // posicion real del cesto al momento del lanzamiento
bool  fPresionadaAntes = false;

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";


/*
* Para la animación compleja se utilizó una máquina de estados 
* Estados:
*  0 : Reposo
*  1 : Se enciende el fuego 
*  2 : La bola gira con el fuego aun encendido
*  3 : Comienza a salir humo que crece + fuego + movimiento de bola
*  4 : Se apaga el fuego mientras el humo crece hasta un máximo 
*  5 : En ese máximo se activa la catapulta y lanza la bola, humo se apaga
*  6 : Pelota lanzada, rebota y regresa a la catapulta
*/



// Reset de variables
void ResetAnimacion(int& estado, float& tiempoEstado,
    float& rotacionBola, float& anguloBrazo,
    float& humoEscala, float& bola_t,
    bool& bolaActiva, glm::vec3& bolaPos,
    bool& bolaEnHoyo)
{
    estado = 0;
    tiempoEstado = 0.0f;
    rotacionBola = 0.0f;
    anguloBrazo = 0.0f;
    humoEscala = 0.0f;
    bola_t = 0.0f;
    bolaActiva = false;
    bolaEnHoyo = false;
    bolaPos = BOLA_INICIO;
}


void ActualizarEstados(int& estado, float& tiempoEstado,
    float& humoEscala, bool& bolaActiva,
    float& bola_t, float deltaTime)
{
    if (estado == 0) return;

    tiempoEstado += deltaTime;

    switch (estado)
    {
    case 1: // enciende fuego / espera / bola empieza a girar
        if (tiempoEstado >= 200.0f)
        {
            estado = 2;
            tiempoEstado = 0.0f;
        }
        break;

    case 2: // bola girando + fuego / espera / humo aparece
        if (tiempoEstado >= 200.0f)
        {
            estado = 3;
            tiempoEstado = 0.0f;
        }
        break;

    case 3: // humo crece + fuego / espera / fuego se apaga
        if (tiempoEstado >= 170.0f)
        {
            estado = 4;
            tiempoEstado = 0.0f;
        }
        break;

    case 4: // fuego apagado + humo crece hasta max
        break;

    case 5: // catapulta lanza en humo max
        estado = 6;
        tiempoEstado = 0.0f;
        bolaActiva = true;
        bola_t = 0.0f;
        break;

    case 6: // pelota en lanzamiento
        break;
    }
}


void ActualizarBola(int& estado, bool& bolaActiva,float& bola_t, glm::vec3& bolaPos, bool& bolaEnHoyo, const glm::vec3& bolaLanzada, float deltaTime)
{
    if (!bolaActiva) return;
    bola_t += deltaTime;
    const float T_vuelo  = 200.0f;
    const float T_rebote = 80.0f;
    const glm::vec3 BOLA_DESTINO(bolaLanzada.x - 30.0f, -2.0f, bolaLanzada.z + 2.0f); // aterrizaje
    const glm::vec3 BOLA_REBOTE(BOLA_DESTINO.x - 5.0f, -2.0f, BOLA_DESTINO.z); // rebote

    if (!bolaEnHoyo)
    {
        // canasta hasta piso
        float t = bola_t / T_vuelo;
        if (t > 1.0f) t = 1.0f;
        bolaPos.x = bolaLanzada.x + (BOLA_DESTINO.x - bolaLanzada.x) * t;
        bolaPos.y = bolaLanzada.y + (BOLA_DESTINO.y - bolaLanzada.y) * t + 8.0f * sin(3.14159f * t);  // altura del arco
        bolaPos.z = bolaLanzada.z + (BOLA_DESTINO.z - bolaLanzada.z) * t;
        if (bola_t >= T_vuelo)
        {
            bolaPos    = BOLA_DESTINO;
            bolaEnHoyo = true;
            bola_t     = 0.0f;
        }
    }
    else
    {
        // rebote
        float t = bola_t / T_rebote;
        if (t > 1.0f) t = 1.0f;

        bolaPos.x = BOLA_DESTINO.x + (BOLA_REBOTE.x - BOLA_DESTINO.x) * t;
        bolaPos.z = BOLA_DESTINO.z;
        bolaPos.y = -2.0f + 2.5f * sin(3.14159f * t);

		// regresa a canasta 
        if (bola_t >= T_rebote)
        {
            bolaPos    = bolaLanzada;
            bolaActiva = false;
            bolaEnHoyo = false;
            estado     = 0;
        }
    }
}


void ActualizarAeolipile(float& rotacionBola, int estado, float deltaTime)
{
    if (estado >= 2)
    {
        rotacionBola -= 10.0f * deltaTime;
        if (rotacionBola >= 360.0f) rotacionBola -= 360.0f;
    }
}

void ActualizarCatapulta(float& anguloBrazo, int estado, float deltaTime)
{
    if (estado == 4 && anguloBrazo < 45.0f)
    {
        anguloBrazo += 60.0f * deltaTime;
        if (anguloBrazo > 45.0f) anguloBrazo = 45.0f;
    }
}

void ActualizarHumo(float& humoEscala, int& estado, float& tiempoEstado, bool& bolaActiva, float& bola_t, float deltaTime)
{
    if (estado == 3 || estado == 4)
    {
        // humo crece
        humoEscala += 0.15f * deltaTime;
        if (humoEscala >= 4.5f)
        {
            humoEscala   = 4.5f;
            // catapulta se activa
            if (estado == 4)
            {
                estado       = 5;
                tiempoEstado = 0.0f;
            }
        }
    }
    else if (estado == 5)
    {
        // humo desaparece
        humoEscala -= 0.15f * deltaTime;
        if (humoEscala < 0.0f) humoEscala = 0.0f;
    }
    else if (estado == 0 || estado == 1 || estado == 2)
    {
        humoEscala = 0.0f;
    }
}

//c�lculo del promedio de las normales para sombreado de Phong
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
	meshList.push_back(obj6); // todos los n�meros

	Mesh* obj7 = new Mesh();
	obj7->CreateMesh(numeroVertices, numeroIndices, 32, 6);
	meshList.push_back(obj7); // solo un n�mero

}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main()
{
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
	AgaveTexture = Texture("Textures/Agave.tga");
	AgaveTexture.LoadTextureA();
	FlechaTexture = Texture("Textures/flechas.tga");
	FlechaTexture.LoadTextureA();
	NumerosTexture = Texture("Textures/numerosbase.tga");
	NumerosTexture.LoadTextureA();
	Numero1Texture = Texture("Textures/numero1.tga");
	Numero1Texture.LoadTextureA();
	Numero2Texture = Texture("Textures/numero2.tga");
	Numero2Texture.LoadTextureA();
	SmokeTexture = Texture("Textures/Smoke.PNG");
	SmokeTexture.LoadTextureA();
	glBindTexture(GL_TEXTURE_2D, SmokeTexture.getTextureID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	FireTexture = Texture("Textures/Fire.PNG");
	FireTexture.LoadTextureA();


	Kitt_M = Model();
	Kitt_M.LoadModel("Models/kitt_optimizado.obj");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/llanta_optimizada.obj");
	Pista_M = Model();
	Pista_M.LoadModel("Models/pista.obj");
	Nave_M = Model();
	Nave_M.LoadModel("Models/nave.obj");
	Ala_M = Model();
	Ala_M.LoadModel("Models/ala.obj");
	Aeolipile_base_M = Model();
	Aeolipile_base_M.LoadModel("Models/Aeolipile_base.obj");
	Aeolipile_M = Model();
	Aeolipile_M.LoadModel("Models/Aeolipile.obj");
	CatapultaBase_M = Model();
	CatapultaBase_M.LoadModel("Models/CatapultaBase.obj");
	CatapultaBrazo_M = Model();
	CatapultaBrazo_M.LoadModel("Models/CatapultaBrazo.obj");
	Bola_M = Model();
	Bola_M.LoadModel("Models/Bola.obj");


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


	//luz direccional, s�lo 1 y siempre debe de existir
	mainLight = DirectionalLight(
		1.0f, 1.0f, 1.0f,
		0.5f, 0.5f,
		0.0f, -1.0f, -1.0f);
	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Primer luz puntual
	pointLights[0] = PointLight(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 2.5f, 1.5f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	unsigned int spotLightCount = 0;
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
		0.0f, 0.0f, 1.0f,
		1.0f, 2.0f,
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		15.0f);
	spotLightCount++;



	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	movCoche = 0.0f;
	movOffset = 0.01f;
	rotllanta = 0.0f;
	rotllantaOffset = 10.0f;

	glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		tiempo += deltaTime;
		angulovaria += 0.5f * deltaTime;
		movCoche -= movOffset * deltaTime;
		rotllanta += rotllantaOffset * deltaTime;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// tecla F 
		bool fAhora = mainWindow.getsKeys()[GLFW_KEY_F];
		if (fAhora && !fPresionadaAntes && estado == 0)
		{
			ResetAnimacion(estado, tiempoEstado, rotacionBola, anguloBrazo,humoEscala, bola_t, bolaActiva, bolaPos, bolaEnHoyo);
			estado = 1;
		}
		fPresionadaAntes = fAhora;

		// actualizaciones
		ActualizarEstados(estado, tiempoEstado, humoEscala, bolaActiva, bola_t, deltaTime);
		ActualizarAeolipile(rotacionBola, estado, deltaTime);
		ActualizarCatapulta(anguloBrazo, estado, deltaTime);
		ActualizarHumo(humoEscala, estado, tiempoEstado, bolaActiva, bola_t, deltaTime);


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
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition,camera.getCameraPosition().x,camera.getCameraPosition().y,camera.getCameraPosition().z);

		// Linterna ligada a la camara
		lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		color   = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

		// piso
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		// auto
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(movCoche - 50.0f, 0.5f, -2.0f));
		modelaux = model;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Kitt_M.RenderModel();

		// Llanta delantera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(7.0f, -0.5f, 8.0f));
		model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		color = glm::vec3(0.5f, 0.5f, 0.5f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// Llanta trasera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(15.5f, -0.5f, 8.0f));
		model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// Llanta delantera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(7.0f, -0.5f, 1.5f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// Llanta trasera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(15.5f, -0.5f, 1.5f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -rotllanta * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		// ========================== RENDER PRÁCTICA 9 ==============================

		// aeolipile
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		// base
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Aeolipile_base_M.RenderModel();
		// bola giratoria
		model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
		model = glm::rotate(model, rotacionBola * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Aeolipile_M.RenderModel();


		// catapulta
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		// base
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(5.0f, -2.0f, 0.0f));
		model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.5f, 3.5f, 3.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		CatapultaBase_M.RenderModel();
		// brazo 
		glm::mat4 brazoPivot(1.0f);
		brazoPivot = glm::translate(brazoPivot, glm::vec3(4.49f, -1.9f, 0.0f));
		brazoPivot = glm::rotate(brazoPivot, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		brazoPivot = glm::rotate(brazoPivot, 30.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		brazoPivot = glm::rotate(brazoPivot, -anguloBrazo * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(brazoPivot, glm::vec3(3.5f, 3.5f, 3.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		CatapultaBrazo_M.RenderModel();
		
		// canasta
		{
			glm::vec4 cestoLocal(-3.7f, 4.9f, 0.2f, 1.0f);
			glm::vec3 cestoActual = glm::vec3(brazoPivot * cestoLocal);
			if (!bolaActiva || (bolaActiva && !bolaEnHoyo && bola_t <= deltaTime))
				bolaLanzada = cestoActual;
		}
		ActualizarBola(estado, bolaActiva, bola_t, bolaPos, bolaEnHoyo, bolaLanzada, deltaTime);


		// transparencias
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// fuego
		if (estado >= 1 && estado <= 3)
		{
			float escalaFuego = 1.0f + 0.3f * sin(tiempo * 3.0f);
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			toffset = glm::vec2(0.0f, 0.0f);
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, -0.7f, 0.0f));
			model = glm::rotate(model, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f * escalaFuego));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			FireTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[4]->RenderMesh();
		}


		// humo 
		if (estado >= 3 && humoEscala > 0.0f)
		{
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			toffset = glm::vec2(0.0f, 0.0f);
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(1.0f, 3.0f, 0.0f));
			model = glm::rotate(model, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f * (humoEscala - 1.0f)));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, humoEscala));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			SmokeTexture.UseTexture();
			meshList[4]->RenderMesh();
		}


		// pelota
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		if (estado == 6 && bolaActiva)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, bolaPos);
			model = glm::scale(model, glm::vec3(1.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Bola_M.RenderModel();
		}
		else
		{
			model = glm::translate(brazoPivot, glm::vec3(-3.7f, 4.9f, 0.2f));
			model = glm::scale(model, glm::vec3(1.3f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Bola_M.RenderModel();
		}


		// flecha
		toffsetflechau += 0.001f;
		if (toffsetflechau > 1.0f) toffsetflechau = 0.0f;
		toffset = glm::vec2(toffsetflechau, 0.0f);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -6.0f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		FlechaTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[4]->RenderMesh();

		// plano con num
		toffsetnumerou = 0.0f;
		toffsetnumerov = 0.0f;
		toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-6.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[5]->RenderMesh();

		// num 1
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-10.0f, 2.0f, -6.0f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[6]->RenderMesh();

		for (int i = 1; i < 4; i++)
		{
			toffsetnumerou += 0.25f;
			toffset = glm::vec2(toffsetnumerou, 0.0f);
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(-10.0f - (i * 3.0f), 2.0f, -6.0f));
			model = glm::rotate(model, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			NumerosTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[6]->RenderMesh();
		}

		for (int j = 1; j < 5; j++)
		{
			toffsetnumerou += 0.25f;
			toffset = glm::vec2(toffsetnumerou, -0.33f);
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(-7.0f - (j * 3.0f), 5.0f, -6.0f));
			model = glm::rotate(model, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			color = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			NumerosTexture.UseTexture();
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			meshList[6]->RenderMesh();
		}

		// num cambiante
		toffsetnumerocambiau += 0.25f;
		if (toffsetnumerocambiau > 1.0f) toffsetnumerocambiau = 0.0f;
		toffset = glm::vec2(toffsetnumerocambiau, 0.0f);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-10.0f, 10.0f, -6.0f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		NumerosTexture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[6]->RenderMesh();

		// cambio autom de texturas 
		toffset = glm::vec2(0.0f, 0.0f);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-13.0f, 10.0f, -6.0f));
		model = glm::rotate(model, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Numero1Texture.UseTexture();
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[5]->RenderMesh();

		glDisable(GL_BLEND);
		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}

