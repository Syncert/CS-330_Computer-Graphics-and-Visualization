#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

//Mesh
#include <mesh.h>
#include <mesh_utility.h>

//shader header
#include <shader.hpp>


using namespace std; // Standard namespace


// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "CS330 Showcase - Cluttered Desk"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 1920;
    const int WINDOW_HEIGHT = 1080;


    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    // Mesh objects (scene)
    Mesh saltShakerCapMesh;
    Mesh saltShakerBodyMesh;

    Mesh pepperShakerCapMesh;
    Mesh pepperShakerBodyMesh;

    Mesh saltContainerMesh;
    Mesh pepperContainerMesh;

    Mesh sphereMesh1;
    Mesh sphereMesh2;

    Mesh sphereHolder;

    Mesh tableMesh;

    //Debug Mesh 
    Mesh debugMesh;


    // Shader program
    Shader* ourShader = nullptr;
    Shader* debugShader = nullptr;

    //Camera Position
    glm::vec3 gCameraPos = glm::vec3(-1.0f, 0.75f, 20.0f);
    glm::vec3 gCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 gCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    //camera speed
    float gDeltaTime = 0.0f; // Time between current frame and last frame
    float gLastFrame = 0.0f;
    float gCameraSpeed = 2.5f; //camera speed constant

    //Camera Rotation Angles

    float gYaw = -90.0f; // Initial yaw angle (facing forward)
    float gPitch = 0.0f; // Initial pitch angle
    float gZoom = 45.0f; // Initial zoom level
    float gMouseSensitivity = 0.05f; // Mouse sensitivity for camera rotation

    // Constants for camera movement and pivot
    float fov = 45.0f;

    // Global variables for mouse tracking

    //initial state to detect first mouse movement
    bool firstMouse = true;

    //store last frames mouse partition
    float lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;

    // Global variable to track the current projection type
    bool isPerspective = true;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
//utility
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void URender();

void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Dynamically allocate Shader object (if using pointer)
    ourShader = new Shader("shaders/vertex_texture_key_fill_lights.glsl", "shaders/fragment_texture_key_fill_lights.glsl");
    debugShader = new Shader("shaders/vertex_debug.glsl", "shaders/fragment_debug.glsl");


    //create meshes
    
    //salt shaker
    saltShakerCapMesh = CreateCircleShakerCapMesh("textures","shaker_cap.jpg",false);
    saltShakerBodyMesh = CreatePrismShakerBodyMesh("textures", "salt.jpg",false, 5.0);
    //pepper shaker
    pepperShakerCapMesh = CreateCircleShakerCapMesh("textures", "shaker_cap.jpg", false);
    pepperShakerBodyMesh = CreatePrismShakerBodyMesh("textures", "black_pepper.jpg", false, 5.0);

    //salt container
    saltContainerMesh = CreateCylinderMesh("textures", "salt_logo.jpg", false, 2);

    ////pepper container
    pepperContainerMesh = CreateCubePepperMesh("textures", "pepper_logo.jpg", false, 1.0);

    ////sphere 1
    sphereMesh1 = CreateSphereMesh("textures", "sphere_gem.jpg", false, 4.0);
    //
    ////sphere 2
    sphereMesh2 = CreateSphereMesh("textures", "sphere_gem_2.png", false, 4.0);
    //
    ////sphere holder
    sphereHolder = CreateCubeMesh("textures", "table_top.jpg", false, 3.0);

    //tabletop
    tableMesh = CreatePlaneMesh("textures", "table_top_2.jpg", false, 1.0);

    //debug mesh
    debugMesh = CreateDebugCubeMesh();

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // input
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Cleanup
    delete ourShader; // Release dynamically allocated Shader object
    ourShader = nullptr;
    delete debugShader;
    debugShader = nullptr;

    exit(EXIT_SUCCESS); // Terminates the program successfully
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
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }


    //mouse callbacks
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);


    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    float cameraOffset = gCameraSpeed * gDeltaTime; //camera offset calc
    static bool isWireframe = false;         // Keeps track of wireframe state
    static bool tabKeyReleased = true;       // Tracks if the TAB key has been released


    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


    // Move forward or diagonally forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        // Calculate the direction vector based on yaw
        glm::vec3 direction(cos(glm::radians(gYaw)), 0.0f, sin(glm::radians(gYaw)));
        gCameraPos += cameraOffset * direction;
    }

    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        glm::vec3 direction(cos(glm::radians(gYaw)), 0.0f, sin(glm::radians(gYaw)));
        gCameraPos -= cameraOffset * direction;
    }

    // Move left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        glm::vec3 left(cos(glm::radians(gYaw - 90.0f)), 0.0f, sin(glm::radians(gYaw - 90.0f)));
        gCameraPos -= cameraOffset * left;
    }

    // Move right
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        glm::vec3 right(cos(glm::radians(gYaw - 90.0f)), 0.0f, sin(glm::radians(gYaw - 90.0f)));
        gCameraPos += cameraOffset * right;
    }
    // Move up
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        gCameraPos += cameraOffset * gCameraUp;
    }


    // Move down
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        gCameraPos -= cameraOffset * gCameraUp;
    }

    // Toggle perspective and orthographic
    static bool pKeyPressed = false; // Track the press state to prevent toggling multiple times per press
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (!pKeyPressed) {
            isPerspective = !isPerspective;
            pKeyPressed = true;
        }
    }
    else {
        pKeyPressed = false;
    }
    //toggle wireframe for troubleshooting. utility funtionality :) 
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && tabKeyReleased) {
        isWireframe = !isWireframe;
        tabKeyReleased = false; // Set to false until the key is released
        glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_LINE : GL_FILL);
    }

    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) {
        tabKeyReleased = true; // Reset to true when the key is released
    }

    //calculate time-related variables
    float currentFrame = glfwGetTime();
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;
}

// glfw: Whenever the mouse moves, this callback is called.
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false; //ensure block wont run again during frame
    }

    //calculate mouse offset delta from last frame
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    //sensitivity factor to scale mouse movement
    float sensitivity = 0.1f; //change value as needed
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    //adjust yaw and pitch angles based on movement
    gYaw += xoffset;
    gPitch += yoffset;

    //ensuring that out of bound gPitch does not lead to screen flipping
    if (gPitch > 89.0f)
        gPitch = 89.0f;
    if (gPitch < -89.0f)
        gPitch = -89.0f;

    //calculating new front vector from updated yaw and gPitch
    glm::vec3 front{};
    front.x = cos(glm::radians(gYaw)) * cos(glm::radians(gPitch)); //cos of yaw * cos angle calculates new direction on x - axis
    front.y = sin(glm::radians(gPitch)); //gPitch determines if camera is oriented directionally up/down
    front.z = sin(glm::radians(gYaw)) * cos(glm::radians(gPitch)); //sin of yaw * cos angle calculates new direction on z - axis
    gCameraFront = glm::normalize(front); //normalize and assign to gCameraFront
}

// glfw: Whenever the mouse scroll wheel scrolls, this callback is called.
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Adjust the camera speed based on scroll wheel movement.
    gCameraSpeed += yoffset * 0.5f; // Increase/decrease camera speed
    // Clamp the camera speed to a specific range to prevent it from becoming too fast or too slow.
    if (gCameraSpeed < 0.25f)
        gCameraSpeed = 0.25f; // Prevent camera from stopping.
    if (gCameraSpeed > 25.0f)
        gCameraSpeed = 25.0f; // Prevent camera from moving too fast.

}

// glfw: Handle mouse button events.
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

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//function called to render each frame
void URender()
{

    glEnable(GL_DEPTH_TEST); // Enable z-depth
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Clear the frame and z buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader->use(); // Activate Shader

    // Camera and Projection setup
    glm::mat4 view = glm::lookAt(gCameraPos, gCameraPos + gCameraFront, gCameraUp);
    view = glm::rotate(view, glm::radians(gYaw), glm::vec3(0.0f, 1.0f, 0.0f)); // Apply yaw rotation to the camera's view

    glm::mat4 projection;
    if (isPerspective) {
        projection = glm::perspective(glm::radians(fov), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else {
        float scale = 1.0f + (45.0f - fov) / 45.0f;
        float orthoWidth = scale * (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
        float orthoHeight = scale;
        projection = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, 0.1f, 100.0f);
    }

    //LIGHTING SETUP
    
    // Key light properties
    glm::vec3 keyLightPos(-11.0f, 4.0f, -1.0f); //positioning to the left and upwards
    glm::vec3 keyLightColor(0.0f, 0.4f, 1.0f); // Blue color for key light
    float keyLightIntensity = 1.0f; // 100% intensity

    // Fill light properties
    glm::vec3 fillLightPos(-2.0f, 3.0f, 13.0f);
    glm::vec3 fillLightColor(1.0f, 1.0f, 1.0f); // White color for fill light
    float fillLightIntensity = 0.7f; // 70% intensity

    // Pass light and transformation properties to shaders
    // Set light properties using the Shader class methods
    ourShader->setVec3("keyLight.position", keyLightPos);
    ourShader->setVec3("keyLight.color", keyLightColor);
    ourShader->setFloat("keyLight.intensity", keyLightIntensity);

    ourShader->setVec3("fillLight.position", fillLightPos);
    ourShader->setVec3("fillLight.color", fillLightColor);
    ourShader->setFloat("fillLight.intensity", fillLightIntensity);

    // Pass view and projection matrices to the shader
    ourShader->setMat4("view", view);
    ourShader->setMat4("projection", projection);

    //Define Draw Method
    GLuint method = GL_TRIANGLES;

    /*
    * Salt Shaker Body Draw
    */

    // Define transformations for the shaker body //
    glm::mat4 saltBodyScale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f)); // Adjust scale as needed

    //Define angle in degrees, convert to radians, input to rotation function
    float angleInDegrees = 0.0f; // Angle in degrees
    float angleInRadians = glm::radians(angleInDegrees); // Convert to radians

    glm::mat4 saltBodyRotation = glm::mat4(1.0f); //glm::rotate(angleInRadians, glm::vec3(2.0f, 1.5f, 0.0f)); // Adjust rotation as needed
    glm::mat4 saltBodyTranslation = glm::translate(glm::vec3(-2.5f, -2.985f, -4.5f)); // Adjust translation as needed
    glm::mat4 saltBodyModel = saltBodyTranslation * saltBodyRotation * saltBodyScale;

    // Pass the model matrix to the shader
    ourShader->setMat4("model", saltBodyModel);

    // Now, draw the shaker body mesh
    saltShakerBodyMesh.Draw(*ourShader);

    /*
    * Salt Shaker Cap Draw
    */

    // Define transformations for the shaker cap //
    glm::mat4 saltCapScale = glm::scale(glm::vec3(0.60f, 0.60f, 0.60f)); // Adjust scale as needed

    //Define angle in degrees, convert to radians, input to rotation function
    angleInDegrees = 25.0f; // Angle in degrees
    angleInRadians = glm::radians(angleInDegrees); // Convert to radians

    glm::mat4 saltCapRotation = glm::mat4(1.0f); //no rotation glm::rotate(angleInRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 saltCapTranslation = glm::translate(glm::vec3(-2.5f, -1.985f, -4.5f)); // translate to sit on top of shaker body
    glm::mat4 saltCapModel = saltCapTranslation * saltCapRotation * saltCapScale;

    // Pass the model matrix to the shader
    ourShader->setMat4("model", saltCapModel);

    // Now, draw the shaker body mesh
    saltShakerCapMesh.Draw(*ourShader);

    /*
    * Pepper Shaker Body Draw
    */
    // Define transformations for the shaker body //
    glm::mat4 pepperBodyScale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f)); // Adjust scale as needed

    //Define angle in degrees, convert to radians, input to rotation function
    angleInDegrees = 0.0f; // Angle in degrees
    angleInRadians = glm::radians(angleInDegrees); // Convert to radians

    glm::mat4 pepperBodyRotation = glm::mat4(1.0f); //glm::rotate(angleInRadians, glm::vec3(2.0f, 1.5f, 0.0f)); // Adjust rotation as needed
    glm::mat4 pepperBodyTranslation = glm::translate(glm::vec3(-2.5f, -2.985f, 4.5f)); // Adjust translation as needed
    glm::mat4 pepperbodyModel = pepperBodyTranslation * pepperBodyRotation * pepperBodyScale;

    // Pass the model matrix to the shader
    ourShader->setMat4("model", pepperbodyModel);

    // Now, draw the shaker body mesh
    pepperShakerBodyMesh.Draw(*ourShader);

    /*
    * Pepper Shaker Cap Draw
    */

    // Define transformations for the shaker cap //
    glm::mat4 pepperCapScale = glm::scale(glm::vec3(0.60f, 0.60f, 0.60f)); // Adjust scale as needed

    //Define angle in degrees, convert to radians, input to rotation function
    angleInDegrees = 25.0f; // Angle in degrees
    angleInRadians = glm::radians(angleInDegrees); // Convert to radians

    glm::mat4 pepperCapRotation = glm::mat4(1.0f); //no rotation glm::rotate(angleInRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 pepperCapTranslation = glm::translate(glm::vec3(-2.5f, -1.985f, 4.5f)); // translate to sit on top of shaker body
    glm::mat4 pepperCapModel = pepperCapTranslation * pepperCapRotation * pepperCapScale;

    // Pass the model matrix to the shader
    ourShader->setMat4("model", pepperCapModel);

    // Now, draw the shaker body mesh
    pepperShakerCapMesh.Draw(*ourShader);

    /*
    * Salt Container Draw
    */

    //Define transformations for the salt container
    glm::mat4 saltContainerScale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f)); // Adjust scale as needed

        //Define angle in degrees, convert to radians, input to rotation function
    angleInDegrees = 180.0f; // Angle in degrees
    angleInRadians = glm::radians(angleInDegrees);

    glm::mat4 saltContainerRotation = glm::rotate(angleInRadians, glm::vec3(1.0f, 0.0f, 1.7f));
    glm::mat4 saltContainerTranslation = glm::translate(glm::vec3(-4.5f, -1.985f, -2.5f)); // translate to sit on top of shaker body
    glm::mat4 saltContainerModel = saltContainerTranslation * saltContainerRotation * saltContainerScale;

    // Pass the model matrix to the shader
    ourShader->setMat4("model", saltContainerModel);

    // Now, draw the shaker body mesh
    saltContainerMesh.Draw(*ourShader);

    /*
    * Pepper Container Draw
    */
    glm::mat4 pepperContainerScale = glm::scale(glm::vec3(1.0f, 2.0f, 2.0f)); // Adjust scale as needed

    //Define angle in degrees, convert to radians, input to rotation function
    angleInDegrees = 180.0f; // Angle in degrees
    angleInRadians = glm::radians(angleInDegrees);

    glm::mat4 pepperContainerRotation = glm::rotate(angleInRadians, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 pepperContainerTranslation = glm::translate(glm::vec3(-4.5f, -1.985f, 2.0f)); // translate to sit on top of shaker body
    glm::mat4 pepperContainerModel = pepperContainerTranslation * pepperContainerRotation * pepperContainerScale;

    // Pass the model matrix to the shader
    ourShader->setMat4("model", pepperContainerModel);

    // Now, draw the shaker body mesh
    pepperContainerMesh.Draw(*ourShader);

    /*
    * Sphere 1 Draw
    */
    glm::mat4 sphereScale = glm::scale(glm::vec3(0.8f, 0.8f, 0.8f)); //adjust scale

    glm::mat4 sphereRotation = glm::mat4(1.0f); //No rotation
    glm::mat4 sphereTranslation = glm::translate(glm::vec3(0.7f, -2.2f, 0.6f)); //translate
    glm::mat4 sphereModel = sphereTranslation * sphereRotation * sphereScale;

    //Pass matrix to Shader
    ourShader->setMat4("model", sphereModel);
    //Draw
    sphereMesh1.Draw(*ourShader);

    /*
    * Sphere 2 Draw
    */
    sphereScale = glm::scale(glm::vec3(0.8f, 0.8f, 0.8f)); //adjust scale

    //Define angle in degrees, convert to radians, input to rotation function
    angleInDegrees = 180.0f; // Angle in degrees
    angleInRadians = glm::radians(angleInDegrees);

    sphereRotation = glm::mat4(1.0f); //No rotation
    sphereTranslation = glm::translate(glm::vec3(1.3f, -2.2f, 1.4f)); //translate
    sphereModel = sphereTranslation * sphereRotation * sphereScale;

    //Pass matrix to Shader
    ourShader->setMat4("model", sphereModel);
    //Draw
    sphereMesh2.Draw(*ourShader);

    /*
    * Sphere Holder Draw
    */
    glm::mat4 sphereHolderScale = glm::scale(glm::vec3(1.0f, 0.5f, 2.0f)); //adjust scale

    //Define angle in degrees, convert to radians, input to rotation function
    angleInDegrees = 36.0f; // Angle in degrees
    angleInRadians = glm::radians(angleInDegrees);

    glm::mat4 sphereHolderRotation = glm::rotate(angleInRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 sphereHolderTranslation = glm::translate(glm::vec3(1.0f, -2.72f, 1.0f)); //translate
    glm::mat4 sphereHolderModel = sphereHolderTranslation * sphereHolderRotation * sphereHolderScale;

    //Pass matrix to Shader
    ourShader->setMat4("model", sphereHolderModel);
    //Draw
    sphereHolder.Draw(*ourShader);

    /*
    * Table Draw
    */

    ////Define transformations for plane mesh //
    glm::mat4 planeScale = glm::scale(glm::vec3(15.0f, 5.0f, 10.0f));

    //Define angle in degrees, convert to radians, input to rotation function
    angleInDegrees = 90.0f; // Angle in degrees
    angleInRadians = glm::radians(angleInDegrees); // Convert to radians

    glm::mat4 planeRotation = glm::rotate(angleInRadians, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 planeTranslation = glm::translate(glm::vec3(-2.0f, -3.0f, 0.0f));
    //glm::mat4 planeTranslation = glm::mat4(1.0f);
    glm::mat4 planeModel = planeTranslation * planeRotation * planeScale;

    //glm::mat4 planeModel = glm::mat4(1.0f); //no transformations
    
    // Pass the model matrix to the shader
    ourShader->setMat4("model", planeModel);

    // Now, draw the shaker body mesh
    tableMesh.Draw(*ourShader);

    /*
    * DEBUG LIGHTS
    */

    ////initiate debug shader
    //debugShader->use(); // Activate Shader

    ////Disable depth test
    //glDisable(GL_DEPTH_TEST);

    //// Assuming gCameraPos is already defined and updated
    //debugShader->setVec3("viewPos", gCameraPos);

    //// Set the view and projection matrices for the debug shader
    //debugShader->setMat4("view", view);
    //debugShader->setMat4("projection", projection);


    //// Render Key Light Debug Mesh
    //glm::mat4 keyLightModel = glm::translate(glm::mat4(1.0f), keyLightPos); // Translate to key light position
    //keyLightModel = glm::scale(keyLightModel, glm::vec3(1.0f)); // Scale down for visibility
    //debugShader->setMat4("model", keyLightModel);
    //debugShader->setVec3("lightColor", keyLightColor); // Set light color for visual differentiation
    //// Assuming you have a function to draw your debug mesh
    //debugMesh.Draw(*debugShader);

    //// Render Fill Light Debug Mesh
    //glm::mat4 fillLightModel = glm::translate(glm::mat4(1.0f), fillLightPos); // Translate to fill light position
    //fillLightModel = glm::scale(fillLightModel, glm::vec3(1.0f)); // Scale down for visibility
    //debugShader->setMat4("model", fillLightModel);
    //debugShader->setVec3("lightColor", fillLightColor); // Set light color for visual differentiation
    //// Reuse the same debug mesh drawing function
    //debugMesh.Draw(*debugShader);

    ////Re-Enable Depth Test
    //glEnable(GL_DEPTH_TEST);

    ///*
    //* END LIGHTING DEBUG
    //*/

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow); // Flips the the back buffer with the front buffer every frame.
}

