/*
* 
* Tarea teoría: Codigo para crear un toroide
* Andrea Mata Ramírez
* 
*/
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
#include <gtc\constants.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

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

Texture pisoTexture;
Texture ToroideTexture;

Skybox skybox;

Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime  = 0.0f;
static double limitFPS = 1.0 / 60.0;

DirectionalLight mainLight;
PointLight  pointLights[MAX_POINT_LIGHTS];
SpotLight   spotLights[MAX_SPOT_LIGHTS];

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";


struct ToroideConfig {
    int      resHor;
    int      resVert;
    GLfloat  radioInt;
    GLfloat  radioExt;

    glm::vec3 posicion;
    GLfloat   rotacionX;   // grados
    glm::vec3 escala;
    Material* material;
    int meshIndex;
};

// lista de toroides con sus configuraciones
std::vector<ToroideConfig> toroides;

// piso
void CreateObjects()
{
    unsigned int floorIndices[] = { 0, 2, 1, 1, 2, 3 };

    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,  0.0f,  0.0f,  0.0f, -1.0f, 0.0f,
         10.0f, 0.0f, -10.0f,  10.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f,  10.0f,  0.0f,  10.0f, 0.0f, -1.0f, 0.0f,
         10.0f, 0.0f,  10.0f,  10.0f, 10.0f, 0.0f, -1.0f, 0.0f
    };

    Mesh* obj1 = new Mesh();
    obj1->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(obj1);
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

/*
*   se usa ec. paramétrica:
* 
*       X(phi, theta) = (R + r*cos(phi)) * cos(theta)
*       Y(phi, theta) = (R + r*cos(phi)) * sin(theta)
*       Z(phi, theta) = r * sin(phi)
* 
*   R = radioExt
*   r = radioInt
* 
*   la normal en cada vertx apunta desde dentro hacia afuera
*   n = (cos(phi)*cos(theta), cos(phi)*sin(theta), sin(phi))
* 
*   vertices almacenan: pos(3) + uv(2) + normal(3) = 8 floats
*/


Mesh* generarMeshToroide(int resHor, int resVert, GLfloat radioTubo, GLfloat radioExt)
{
    std::vector<GLfloat>      vertices;
    std::vector<unsigned int> indices;

    const GLfloat paso_phi   = (2.0f * glm::pi<GLfloat>()) / resHor;
    const GLfloat paso_theta = (2.0f * glm::pi<GLfloat>()) / resVert;

    // vertices
    for (int i = 0; i <= resHor; i++) {
        GLfloat phi = i * paso_phi;
        GLfloat cosPhi = glm::cos(phi);
        GLfloat sinPhi = glm::sin(phi);

        for (int j = 0; j <= resVert; j++) {
            GLfloat theta    = j * paso_theta;
            GLfloat cosTheta = glm::cos(theta);
            GLfloat sinTheta = glm::sin(theta);

            // ec parametrica
            GLfloat x = (radioExt + radioTubo * cosPhi) * cosTheta;
            GLfloat y = (radioExt + radioTubo * cosPhi) * sinTheta;
            GLfloat z =  radioTubo * sinPhi;

            // posicion
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // coordenadas para textura
            vertices.push_back((GLfloat)j / resVert);
            vertices.push_back((GLfloat)i / resHor);

            // normal
            vertices.push_back(-cosPhi * cosTheta);
            vertices.push_back(-cosPhi * sinTheta);
            vertices.push_back(-sinPhi);
        }
    }

    // indx
    for (int i = 0; i < resHor; i++) {
        unsigned int k1 = i       * (resVert + 1);
        unsigned int k2 = (i + 1) * (resVert + 1);

        for (int j = 0; j < resVert; j++) {
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);

            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);

            k1++; k2++;
        }
    }

    Mesh* mesh = new Mesh();
    mesh->CreateMesh(vertices.data(), indices.data(),
                     (unsigned int)vertices.size(),
                     (unsigned int)indices.size());
    return mesh;
}

void agregarToroide(
    int resHor, int resVert, GLfloat radioInt, GLfloat radioExt, glm::vec3 posicion, 
    GLfloat rotacionX, glm::vec3 escala, Material* material
){
    ToroideConfig cfg;
    cfg.resHor = resHor;
    cfg.resVert = resVert;
    cfg.radioInt = radioInt;
    cfg.radioExt = radioExt;
    cfg.posicion = posicion;
    cfg.rotacionX = rotacionX;
    cfg.escala = escala;
    cfg.material = material;
    cfg.meshIndex = (int)meshList.size();
    meshList.push_back(generarMeshToroide(resHor, resVert, radioInt, radioExt));
    toroides.push_back(cfg);
}


int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();

    Material_brillante = Material(4.0f, 256);
    Material_opaco     = Material(0.3f, 4);

    // resHor, resVert, radioTubo, radioExt
    agregarToroide(
        50, 50, 0.5f, 2.0f,
        glm::vec3(0.0f, 10.0f, 10.0f), -90.0f, glm::vec3(3.0f),
        &Material_brillante);

    agregarToroide(
        60, 20, 1.0f, 3.0f,
        glm::vec3(0.0f, 20.0f, 0.0f), -90.0f, glm::vec3(3.0f),
        &Material_brillante);

    agregarToroide(
        30, 60, 2.0f, 4.0f,
        glm::vec3(0.0f, 30.0f, -15.0f), -90.0f, glm::vec3(3.0f),
        &Material_brillante);

    agregarToroide(
        20, 20, 3.0f, 5.0f,
        glm::vec3(0.0f, 40.0f, -40.0f), -90.0f, glm::vec3(3.0f),
        &Material_brillante);

    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),-60.0f, 0.0f, 0.5f, 0.5f);

    pisoTexture = Texture("Textures/piso.tga");
    pisoTexture.LoadTextureA();
    ToroideTexture = Texture("Textures/Toroide.png");
    ToroideTexture.LoadTextureA();

    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");
    skybox = Skybox(skyboxFaces);

    mainLight = DirectionalLight(
        1.0f, 1.0f, 1.0f,
        0.3f, 0.3f,
        0.0f, 0.0f, -1.0f);

    unsigned int pointLightCount = 0;
    unsigned int spotLightCount  = 0;

    spotLights[0] = SpotLight(
        1.0f, 1.0f, 1.0f,
        0.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        5.0f);
    spotLightCount++;

    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;
    GLuint uniformView = 0;
    GLuint uniformEyePosition = 0;
    GLuint uniformSpecularIntensity = 0;
    GLuint uniformShininess = 0;
    GLuint uniformTextureOffset = 0;
    GLuint uniformColor = 0;

    glm::mat4 projection = glm::perspective(45.0f,(GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
    glm::vec3 lowerLight(0.0f);


    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime   = now - lastTime;
        deltaTime  += (now - lastTime) / limitFPS;
        lastTime    = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

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

        lowerLight = camera.getCameraPosition();
        lowerLight.y -= 0.3f;
        spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        // piso
        glm::vec3 color   = glm::vec3(1.0f);
        glm::vec2 toffset = glm::vec2(0.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor,        1, glm::value_ptr(color));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[0]->RenderMesh();

        // toroides
        ToroideTexture.UseTexture();
        glUniform3fv(uniformColor,        1, glm::value_ptr(glm::vec3(1.0f)));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(glm::vec2(0.0f)));

        for (const ToroideConfig& t : toroides) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, t.posicion);
            model = glm::scale(model, t.escala);
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            t.material->UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshList[t.meshIndex]->RenderMesh();
        }

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
