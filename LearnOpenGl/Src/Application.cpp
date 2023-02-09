#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Shader.h"
#include <string>
#include <fstream>
#include <sstream>

#include "Model.h"
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


float mixvalue = 0.2f;
glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cam_Target = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cam_Front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cam_Direction = glm::normalize(camera_pos - cam_Target);

glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cam_Right = glm::normalize(glm::cross(up, cam_Direction));

glm::vec3 cam_Up = glm::normalize(glm::cross(cam_Direction, cam_Up));

float DeltaTime = 0.0f;
float lastFrame = 0.0f;

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 400.0f, lastY = 300.0f;
float fov = 45.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);


const char *VertexShaderSource =
"#version 330 core \n"
"layout (location = 0) in vec3 aPos;\n"
///"layout (location = 1) in vec3 aColor;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoord;\n"

"out vec3 first_Color;\n"

"out vec2 TexCoord;\n"
"out vec3 FragPos;\n"
"uniform mat4 transform;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec3 normal;\n"
"void main ()\n"
"{\n"
"     FragPos = vec3(model * vec4(aPos, 1.0));\n"
"      normal = mat3(transpose(inverse(model))) * aNormal;\n"
"     gl_Position =  projection * view * vec4(FragPos, 1.0f);\n"

"     TexCoord = vec2(aTexCoord.x, aTexCoord.y) ;\n"

"}\0";

const char* FragmentShaderSource =
"#version 330 core\n"


"struct materials {\n"
//"vec3 ambient;\n"
"sampler2D diffuse;\n"
"sampler2D specular;\n"
"sampler2D emission;\n"
"float shininess;\n"
"};\n"


"struct Dir_Light {\n"
"vec3 ambient;\n"
"vec3 diffuse;\n"
"vec3 specular;\n"
"vec3 direction;\n"
"};\n"

"struct Point_Light {\n"
"vec3 position;\n"

"float constant;\n"
"float quadratic;\n"
"float linear;\n"

"vec3 ambient;\n"
"vec3 diffuse;\n"
"vec3 specular;\n"

"};\n"

"struct Flash_Light {\n"
"vec3 position;\n"
"vec3 direction;\n"
"float cutoff;\n"
"float cutoffouter;\n"

"float constant;\n"
"float quadratic;\n"
"float linear;\n"

"vec3 ambient;\n"
"vec3 diffuse;\n"
"vec3 specular;\n"

"};\n"




"out vec4 FragColor;\n"
"in vec3 FragPos;\n"
"in vec2 TexCoord;\n"
"in vec3 normal;\n"
//"uniform vec4 Our_Color;\n"
//"uniform float mixvalue;\n"
//"uniform sampler2D texture1;\n"
//"uniform sampler2D texture2; \n"
//" uniform vec3 ObjectColor ;\n"
//" uniform vec3 LightColor;\n"
//" uniform vec3 LightPos;\n"
//" uniform vec3 ViewPos; \n"
"#define NR_POINT_LIGHTS 4\n"
"uniform vec3 viewPos;\n"
"uniform Dir_Light dirLight;\n"
"uniform Point_Light pointLights[NR_POINT_LIGHTS];\n"
"uniform Flash_Light spotLight;\n"
"uniform materials material;\n"


// Functions

"vec3 CalcDirLight(Dir_Light light, vec3 normal, vec3 viewDir);\n"
"vec3 CalcPointLight(Point_Light light, vec3 normal, vec3 fragPos, vec3 viewDir);\n"
"vec3 CalcSpotLight(Flash_Light light, vec3 normal, vec3 fragPos, vec3 viewDir);\n"


"vec3 CalcDirLight(Dir_Light light, vec3 normal, vec3 viewDir)\n"
"{\n"

"      vec3 light_dir = normalize(-light.direction);\n"

"      float diff = max(dot(normal,light_dir), 0.0f);\n"

"      vec3 reflect_dir = reflect(-light_dir, normal);\n"

"      float spec = pow(max(dot(viewDir, reflect_dir), 0.0), material.shininess);\n"

"      vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord).rgb); \n"
"      vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord).rgb);\n"
"      vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord).rgb);\n"

"      return (ambient + diffuse + specular); \n"

"}\n"


"vec3 CalcPointLight(Point_Light light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
"{\n"
"      vec3 light_dir = normalize(light.position - fragPos);\n"
"      float diff = max(dot(normal,light_dir), 0.0f);\n"
"      vec3 reflect_dir = reflect(-light_dir, normal);\n"
"      float spec = pow(max(dot(viewDir, reflect_dir), 0.0), material.shininess);\n"
//Attenuation

"      float distance = length(light.position - fragPos);\n"
"float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"

"      vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord).rgb); \n"
"      vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord).rgb);\n"
"      vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord).rgb);\n"

"        ambient *= attenuation;\n"
"        diffuse *= attenuation;\n"
"        specular *= attenuation;\n"
"      return (ambient + diffuse + specular); \n"

"}\n"

"vec3 CalcSpotLight(Flash_Light light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
"{\n"

"      vec3 light_dir = normalize(light.position - fragPos);\n"
"      float diff = max(dot(normal,light_dir), 0.0f);\n"
"      vec3 reflect_dir = reflect(-light_dir, normal);\n"
"      float spec = pow(max(dot(viewDir, reflect_dir), 0.0), material.shininess);\n"
//Attenuation

"      float distance = length(light.position - fragPos);\n"
"      float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"

//Flash_Light
"       float theta = dot(light_dir, normalize(-light.direction));\n"
"       float epsilon = light.cutoff - light.cutoffouter;\n"
"       float intensity = clamp((theta - light.cutoffouter) / epsilon, 0.0, 1.0);\n"

"      vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord).rgb); \n"
"      vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord).rgb);\n"
"      vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord).rgb);\n"

"        ambient *= attenuation * intensity;\n"
"        diffuse *= attenuation * intensity;\n"
"        specular *= attenuation * intensity;\n"

"      return (ambient + diffuse + specular); \n"

"}\n"



"void main(){\n"

"vec3 norm  = normalize(normal);\n"
"vec3 viewDir = normalize(viewPos - FragPos);\n"

//DirectionLight
"vec3 result = CalcDirLight(dirLight,  norm,  viewDir);\n "

//PointLight
"for (int i = 0; i < NR_POINT_LIGHTS; i++)\n"
"        result +=  CalcPointLight(pointLights[i], norm, FragPos, viewDir);\n"

//FlashLight
"        result += CalcSpotLight(spotLight,  norm ,  FragPos,  viewDir);\n"

"        FragColor = vec4(result,1.0f);\n"


"}\n\0";






const char* VertexShaderSource2 =
"#version 330 core \n"
"layout (location = 0) in vec3 aPos;\n"
///"layout (location = 1) in vec3 aColor;\n"
//"layout (location = 1) in vec2 aTexCoord;\n"

"out vec3 first_Color;\n"
"out vec2 TexCoord;\n"
"uniform mat4 transform;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"

"void main ()\n"
"{\n"
"     gl_Position =  projection * view * model * vec4(aPos, 1.0);\n"
//"     TexCoord = vec2(aTexCoord.x, aTexCoord.y) ;\n"
"}\0";



const char* FragmentShaderSource2 =
"#version 330 core\n"
"in vec3 first_Color;\n"
"uniform vec3 Light_Color;\n"
"out vec4 DragColor;\n"
"void main(){\n"
"       DragColor = vec4(1.0f) ;\n"
"}\n\0";


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{

   //glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
   //glm::mat4 trans = glm::mat4(2.0f);
   //trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));
   //vec = trans * vec;
   //std::cout << vec.x <<"," << vec.y << "," << vec.z << std::endl;
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    

    // glfw window creation
// --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DO IT!!!", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

   

    stbi_set_flip_vertically_on_load(true);

    Shader ourShader("Shader/Vertex.shader", "Shader/Fragment.Shader");

    const char* Model_Path = "res/Textures/3D_Model/backpack.mobj";

    Model ourModel(Model_Path);



    glEnable(GL_DEPTH_TEST);


    float Positions[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };  

    glm::vec3 CubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  3.0f, -7.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    glm::vec3 pointLightPositions[] = {
       glm::vec3(0.7f,  0.2f,  2.0f),
       glm::vec3(2.3f, -3.3f, -4.0f),
       glm::vec3(-4.0f,  2.0f, -12.0f),
       glm::vec3(0.0f,  0.0f, -3.0f)
    };
    //unsigned int IndexArray[] = {
    //    0, 1, 3, // first triangle
    //    1, 2, 3  // second triangle
    //};


    // TEXTURES

  

    // VERTEX SHADER
    

    

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(vertexShader);


    unsigned int vertexShader2;
    vertexShader2 = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader2, 1, &VertexShaderSource2, NULL);
    glCompileShader(vertexShader2);

    // ERROR CHECKING IN SHADERS
    int success;
    char Infolog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, Infolog);
        std::cout << "Failed To Compile\n" << Infolog << std::endl;
    }


    // FRAGMENT SHADER
    

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &FragmentShaderSource, NULL);
    glCompileShader(fragmentShader);


    unsigned int fragmentShader2;
    fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
    
    glShaderSource(fragmentShader2, 1, &FragmentShaderSource2, NULL);
    glCompileShader(fragmentShader2);

    int success1;
    char Infolog1[512];

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success1);

    if (!success1) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, Infolog1);
        std::cout << "Failed To Compile\n" << Infolog1 << std::endl;
    }

    // Attaching the Shaders and Creating the program

    

    unsigned int LightShaderProgram;
    LightShaderProgram = glCreateProgram();
    glAttachShader(LightShaderProgram, vertexShader2);
    glAttachShader(LightShaderProgram, fragmentShader2);
    glLinkProgram(LightShaderProgram);
    glUseProgram(LightShaderProgram);
    

    unsigned int ShaderProgram;
    ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, vertexShader);
    glAttachShader(ShaderProgram, fragmentShader);
    glLinkProgram(ShaderProgram);
    glUseProgram(ShaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    

    //unsigned int EBO;
    //glGenBuffers(1, &EBO);

    

    unsigned int VBO[2], VAO[2];
    glGenVertexArrays(2, &VAO[0]);
    glGenBuffers(2, &VBO[0]);

    glBindVertexArray(VAO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Positions), Positions, GL_STATIC_DRAW);



    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Normals Co-ordinates
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int LightCubeVAO;
    glGenVertexArrays(1, &LightCubeVAO);

    glBindVertexArray(LightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

   //
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
   

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    const char* path = "res/Textures/Steel_Container.png";
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        
        
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
   //
   //
   //
   // // Texture 2
    unsigned int texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    const char* path2 = "res/Textures/Steel_Edges.png";
    data = stbi_load(path2, &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // // Texture 3
    unsigned int texture3;
    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    const char* path3 = "res/Textures/matrix.jpg";
    data = stbi_load(path3, &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
 




    
   // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        
       
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);






   
       float timer = static_cast<float>(glfwGetTime());
       float currentFrame = timer;
       DeltaTime = currentFrame - lastFrame;
       lastFrame = currentFrame;
             
          /// MODEL LOADING
        ourShader.use();
        
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(camera_pos, camera_pos + cam_Front, up);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        
        // render the loaded model

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(timer * 90), glm::vec3(0.0f, 1.0f, 0.0f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);
    
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO[0]);
    glDeleteBuffers(1, &VBO[0]);
    //glDeleteBuffers(1, &EBO);
    

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    std::cin.get();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    const float CameraSpeed = 2.5f * DeltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        
        mixvalue += 0.001f;
        if (mixvalue > 1.0f) {
            mixvalue = 1.0f;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {

        mixvalue -= 0.001f;
        if (mixvalue < 0.0f) {
            mixvalue = 0.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera_pos += cam_Front * CameraSpeed;

    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera_pos -= cam_Front * CameraSpeed;

    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera_pos -=   glm::cross(cam_Front, up) * CameraSpeed;

    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera_pos +=   glm::cross(cam_Front, up) * CameraSpeed;

    }
    camera_pos.y = 0.0f;

}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cam_Front = glm::normalize(front);

}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}