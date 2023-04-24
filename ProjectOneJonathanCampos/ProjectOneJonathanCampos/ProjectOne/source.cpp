#include <iostream>
#include <GLEW/include/GL/glew.h>
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h" //camera class
#include "mesh.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //image loading util 


using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Jonathan Campos"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;


    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    // Shader program
    GLuint gProgramId;

    Meshes meshes;

    //Texture ID
    GLuint gWoodTexture;
    GLuint gCashewTexture;
    GLuint gJarLidTexture;
    GLuint gRubberbandTexture;
    GLuint gComputerColorTexture;
    GLuint gComputerTopTexture;


    glm::vec2 gUVScale(5.0f, 5.0f);
    GLuint gTexWrapMode = GL_REPEAT;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

}

// Init Program and Window Size
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec3 normal; //VAP position 1 for normal
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec2 vertexTextureCoordinate;
out vec3 vertexFragmentPos; // For outgoing color or pixels to fragment shader
out vec3 vertexColor;


//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates

    vertexTextureCoordinate = textureCoordinate.xy;

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment or pixel position in world space only (excludes view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // Gets normal vectors in world space only and excludes normal translation properties


}
);

/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;


out vec4 fragmentColor; // For ongoing color to gpu

//Uniform or global variables for object color, light color, light position, and camera/view position

uniform vec3 objectColor;
uniform vec3 ambientColor;
uniform vec3 lightColor1;
uniform vec3 lightColor2;

uniform vec3 lightPos1;
uniform vec3 lightPos2;

uniform vec3 viewPosition;
uniform sampler2D uTexture;
uniform vec2 uvScale;
uniform float ambientStrength = 0.2f;
uniform float specularIntensity1 = 0.8f;
uniform float highlightSize1 = 20.0f;
uniform float specularIntensity2 = 0.8f;
uniform float highlightSize2 = 20.0f;
uniform bool ubHasTexture;


void main()
{

    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

//Calculate Ambient lighting*/
    vec3 ambient = ambientStrength * lightColor1; // Generate ambient light color.

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit.
    vec3 lightDirection1 = normalize(lightPos1 - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube.
    float impact1 = max(dot(norm, lightDirection1), 0.0);// Calculate diffuse impact by generating dot product of normal and light.
    vec3 diffuse1 = impact1 * lightColor1; // Generate diffuse light color.


    vec3 lightDirection2 = normalize(lightPos2 - vertexFragmentPos); // Calculate distance between light source and fragments pixels
    float impact2 = max(dot(norm, lightDirection2), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse2 = impact2 * lightColor2; // Generate diffuse light color

    //Calculate Specular lighting*/
    vec3 viewDir1 = normalize(viewPosition - vertexFragmentPos); // Calculate view direction.
    vec3 reflectDir = reflect(-lightDirection1, norm);// Calculate reflection vector.



    //Calculate specular component.
    float specularComponent1 = pow(max(dot(viewDir1, reflectDir), 0.0), highlightSize1);
    vec3 specular1 = specularIntensity1 * specularComponent1 * lightColor1;
    vec3 reflectDir2 = reflect(-lightDirection2, norm);// Calculate reflection vector


    //Calculate specular component
    float specularComponent2 = pow(max(dot(viewDir1, reflectDir2), 0.0), highlightSize2);
    vec3 specular2 = specularIntensity2 * specularComponent2 * lightColor2;


    // Calculate Phong result.
    // Combine Phong result with texture color.
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate);
    vec3 phong1;
    vec3 phong2;

    if (ubHasTexture == true)
    {
        phong1 = (ambient + diffuse1 + specular1) * textureColor.xyz;
        phong2 = (ambient + diffuse2 + specular2) * textureColor.xyz;
    }
    else
    {
        phong1 = (ambient + diffuse1 + specular1) * objectColor.xyz;
        phong2 = (ambient + diffuse2 + specular2) * objectColor.xyz;
    }

    fragmentColor = vec4(phong1 + phong2, 1.0f); // Multiplies the Phong result with the texture color to obtain the final fragment color.
}
);



// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = 0; i < width * channels; ++i)
        {
            unsigned char tmp = image[index1 + i];
            image[index1 + i] = image[index2 + i];
            image[index2 + i] = tmp;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    meshes.CreateMeshes();

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Load texture (relative to project's directory)
    const char* texFilename = "wood.jpg";
    if (!UCreateTexture(texFilename, gWoodTexture))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "cashew.jpg";
    if (!UCreateTexture(texFilename, gCashewTexture))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "JarLid.jpg";
    if (!UCreateTexture(texFilename, gJarLidTexture))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "rubberBand.jpg";
    if (!UCreateTexture(texFilename, gRubberbandTexture))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "computerColor.jpg";
    if (!UCreateTexture(texFilename, gComputerColorTexture))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "computerTop.jpg";
    if (!UCreateTexture(texFilename, gComputerTopTexture))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }


    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


    // render loop
    while (!glfwWindowShouldClose(gWindow))
    {

        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    meshes.DestroyMeshes();


    // Release texture
    UDestroyTexture(gWoodTexture);
    // Release texture
    UDestroyTexture(gCashewTexture);
    // Release texture
    UDestroyTexture(gRubberbandTexture);

    // Release texture
    UDestroyTexture(gJarLidTexture);

    // Release texture
    UDestroyTexture(gComputerColorTexture);


    // Release shader program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program 
}

// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    // Register callback functions for mouse
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);


    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    // GLEW: initialize
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;
    return true;
}


bool isOrtho = false;

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 3.0f;


    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


    if (!isOrtho && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (!isOrtho && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (!isOrtho && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (!isOrtho && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (!isOrtho && glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (!isOrtho && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        isOrtho = !isOrtho;
    }


}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (isOrtho) {
        return;
    }

    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;


    gCamera.ProcessMouseMovement(xoffset, yoffset);

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void URender()
{

    //Init matrices so they are not null
    glm::mat4 model;
    glm::mat4 scale;
    glm::mat4 projection;
    glm::mat4 rotation;
    glm::mat4 translation;
    GLint objectColorLoc;
    GLint modelLoc;
    GLint viewLoc;
    GLint projLoc;

    GLint viewPosLoc;
    GLint ambStrLoc;
    GLint ambColLoc;
    GLint light1ColLoc;
    GLint light1PosLoc;
    GLint light2ColLoc;
    GLint light2PosLoc;
    GLint specInt1Loc;
    GLint highlghtSz1Loc;
    GLint specInt2Loc;
    GLint highlghtSz2Loc;
    GLint uHasTextureLoc;
    bool ubHasTextureVal;



    // Enable z-depth
    glEnable(GL_DEPTH_TEST);
    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the shader to be used
    glUseProgram(gProgramId);

    glm::mat4 view = gCamera.GetViewMatrix();

    if (isOrtho) {
        // Orthographic projection
        float orthoSize = 10.0f;
        projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 100.0f);
        view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    }
    else {
        // Perspective projection
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        // camera/view transformation
        glm::mat4 view = gCamera.GetViewMatrix();
    }

    // Outputs the matrices into the Vertex Shader
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");
    objectColorLoc = glGetUniformLocation(gProgramId, "uObjectColor");
    viewPosLoc = glGetUniformLocation(gProgramId, "viewPosition");
    ambStrLoc = glGetUniformLocation(gProgramId, "ambientStrength");
    ambColLoc = glGetUniformLocation(gProgramId, "ambientColor");
    light1ColLoc = glGetUniformLocation(gProgramId, "lightColor1");
    light1PosLoc = glGetUniformLocation(gProgramId, "lightPos1");
    light2ColLoc = glGetUniformLocation(gProgramId, "lightColor2");
    light2PosLoc = glGetUniformLocation(gProgramId, "lightPos2");
    specInt1Loc = glGetUniformLocation(gProgramId, "specularIntensity1");
    highlghtSz1Loc = glGetUniformLocation(gProgramId, "highlightSize1");
    specInt2Loc = glGetUniformLocation(gProgramId, "specularIntensity2");
    highlghtSz2Loc = glGetUniformLocation(gProgramId, "highlightSize2");
    uHasTextureLoc = glGetUniformLocation(gProgramId, "ubHasTexture");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLuint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));


    //set the camera view location
    glUniform3f(viewPosLoc, gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);

    //set ambient lighting strength
    glUniform1f(ambStrLoc, 0.1f);
    //set ambient color
    glUniform3f(ambColLoc, 0.5f, 0.5f, 0.5f);

    //set light 1 properties
   // glUniform3f(light1ColLoc, 1.0f, 1.0f, 1.0f);
    glUniform3f(light1ColLoc, 1.0f, 0.95f, 0.85f); // slightly warm white color

    glUniform3f(light1PosLoc, -15.0f, 2.5f, -10.0f);

    //set light 2 properties
    //glUniform3f(light2ColLoc, 1.0f, 1.0f, 1.0f);
    glUniform3f(light2ColLoc, 1.0f, 0.95f, 0.85f); // slightly warm white color

    glUniform3f(light2PosLoc, 15.0f, 20.0f, -15.0f);

    //set specular intensity
    glUniform1f(specInt1Loc, 1.0f);
    glUniform1f(specInt2Loc, 1.0f);

    //set specular highlight size
    glUniform1f(highlghtSz1Loc, 25.0f);
    glUniform1f(highlghtSz2Loc, 50.0f);

    ubHasTextureVal = true;
    glUniform1i(uHasTextureLoc, ubHasTextureVal);


    //Plane Wood//
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gPlaneMesh.vao);

    // Bind the texture to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gWoodTexture);


    // 1. Scales the object
    scale = glm::scale(glm::vec3(15.0f, 1.0f, 15.0f));
    // 2. Rotate the object
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glProgramUniform4f(gProgramId, objectColorLoc, 0.1f, 0.1f, 0.1f, 0.1f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);



    //Computer Side
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gBoxMesh.vao);

    // Bind the texture to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gComputerColorTexture);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(7.0f, 7.0f, 2.5f));
    // 2. Rotate the object
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(10.0f, 3.5f, -3.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);


    //Computer BACK 
 // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gBoxMesh.vao);

    // Bind the texture to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gJarLidTexture);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(0.2f, 7.0f, 2.5f));
    // 2. Rotate the object
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(13.6f, 3.5f, -3.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);


    //Computer Top
    // 
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gBoxMesh.vao);

    // Bind the texture to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gComputerTopTexture);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(2.9f, 0.1f, 7.3f));
    // 2. Rotate the object
    rotation = glm::rotate(80.095f, glm::vec3(0.0, 2.0f, 0.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(9.7f,7.0f, -2.7f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);


    //Computer Front

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gBoxMesh.vao);

    // Bind the texture to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gJarLidTexture);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(0.5f, 7.0f, 2.5f));
    // 2. Rotate the object
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(6.3f, 3.5f, -3.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);


    //Computer Side

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gBoxMesh.vao);

    // Bind the texture to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gJarLidTexture);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(7.3f, 7.0f, 0.5f));
    // 2. Rotate the object
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(9.7f, 3.5f, -1.5f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);


    //Jar lid//

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gTorusMesh.vao);

    // Bind the texture to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gJarLidTexture);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(1.1f, 1.0f, 1.0f));
    // 2. Rotate the object
    rotation = glm::rotate(-90.05f, glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(0.0f, 0.1f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);
    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);



    //Rubber Band Ball//

    glBindVertexArray(meshes.gSphereMesh.vao);
    // Bind the texture to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gRubberbandTexture);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(0.7f, 0.7f, 0.7f));
    // 2. Rotate the object
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(3.0f, 0.68f, -5.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.0f, 1.0f, 0.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);



    //Jar Object//
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gCylinderMesh.vao);

    // Bind the texture to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCashewTexture);


    // 1. Scales the object
    scale = glm::scale(glm::vec3(1.0f, 3.2f, 1.0f));
    // 2. Rotate the object
    rotation = glm::rotate(0.0f, glm::vec3(0.0, 0.0f, 1.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(0.0f, 0.1f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glProgramUniform4f(gProgramId, objectColorLoc, 0.25f, 0.68f, 0.75f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
    glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);



    glfwSwapBuffers(gWindow); // Flips the the back buffer with the front buffer every frame.
    // glfw: swap buffers and poll IO events 

}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }
}

void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}

void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
