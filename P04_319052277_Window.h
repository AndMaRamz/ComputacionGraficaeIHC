//*
//CÓDIGO PARA MONO 
#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
    Window();
    Window(GLint windowWidth, GLint windowHeight);
    int Initialise();

    GLfloat getBufferWidth() { return bufferWidth; }
    GLfloat getBufferHeight() { return bufferHeight; }

    bool getShouldClose() {
        return glfwWindowShouldClose(mainWindow);
    }

    bool* getsKeys() { return keys; }

    GLfloat getXChange();
    GLfloat getYChange();

    void swapBuffers() { glfwSwapBuffers(mainWindow); }

    // ROTACIONES GLOBALES
    GLfloat getrotay() { return rotay; }
    GLfloat getrotax() { return rotax; }
    GLfloat getrotaz() { return rotaz; }

    // PATAS
    GLfloat getPata1_1() { return pata1_1; }
    GLfloat getPata1_2() { return pata1_2; }
    GLfloat getPata2_1() { return pata2_1; }
    GLfloat getPata2_2() { return pata2_2; }
    GLfloat getPata3_1() { return pata3_1; }
    GLfloat getPata3_2() { return pata3_2; }
    GLfloat getPata4_1() { return pata4_1; }
    GLfloat getPata4_2() { return pata4_2; }

    // OTROS
    GLfloat getCola() { return cola; }
    GLfloat getOrejaDer() { return orejaDer; }
    GLfloat getOrejaIzq() { return orejaIzq; }
    GLfloat getarticulacionCuello() { return articulacionCuello; }

    ~Window();

private:
    GLFWwindow* mainWindow;
    GLint width, height;

    GLfloat rotax, rotay, rotaz;
    GLfloat pata1_1, pata1_2, pata2_1, pata2_2;
    GLfloat pata3_1, pata3_2, pata4_1, pata4_2;
    GLfloat cola, orejaDer, orejaIzq, articulacionCuello;

    bool keys[1024];

    GLint bufferWidth, bufferHeight;

    GLfloat lastX, lastY, xChange, yChange;
    bool mouseFirstMoved;

    void createCallbacks();

    static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
    static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};
//*/

/*
//CÓDIGO PARA GRÚA 
#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);
	}
	bool* getsKeys() { return keys; }
	GLfloat getXChange();
	GLfloat getYChange();
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }
	GLfloat getrotay() { return rotay; }
	GLfloat getrotax() { return rotax; }
	GLfloat getrotaz() { return rotaz; }
	GLfloat getarticulacion1() { return articulacion1; }
	GLfloat getarticulacion2() { return articulacion2; }
	GLfloat getarticulacion3() { return articulacion3; }
	GLfloat getarticulacion4() { return articulacion4; }
	GLfloat getarticulacion5() { return articulacion5; }
	GLfloat getarticulacion6() { return articulacion6; }
	GLfloat getllanta1() { return llanta1; }
	GLfloat getllanta2() { return llanta2; }
	GLfloat getllanta3() { return llanta3; }
	GLfloat getllanta4() { return llanta4; }

	~Window();
private:
	GLFWwindow* mainWindow;
	GLint width, height;
	GLfloat rotax, rotay, rotaz, articulacion1, articulacion2, articulacion3, articulacion4, articulacion5, articulacion6, llanta1, llanta2, llanta3, llanta4;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	bool mouseFirstMoved;
	void createCallbacks();
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};
*/