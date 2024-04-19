// main
#include <cgl.h>


struct GLFWwindow;

struct GLFWmonitor;



static string __G0={"",0};


static const i32 glfw__GLFW_FALSE=0;


extern i32 glfwInit();
extern struct GLFWwindow* glfwCreateWindow(i32 width,i32 height,i8* title,struct GLFWmonitor* monitor,struct GLFWwindow* share);
extern i32 glfwWindowShouldClose(struct GLFWwindow* window);
extern void glfwPollEvents();
extern void glfwSwapBuffers(struct GLFWwindow* window);
extern void glfwDestroyWindow(struct GLFWwindow* window);
extern void glfwTerminate();


int main(){
	i32 __0=glfwInit();
	struct GLFWwindow* __1=glfwCreateWindow(1280,720,__G0.ptr,(struct GLFWmonitor*)(0),(struct GLFWwindow*)(0));
	struct GLFWwindow* *window=__1;
	i32 __2=glfwWindowShouldClose(window);
	while(__2==glfw__GLFW_FALSE){
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
