#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

double padPositionX = 0;
double padPositionZ = 0;

float nearPlane = 0.1f;
float farPlane = 350.f;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

SceneNode* rootNode;
SceneNode* boxNode;
SceneNode* ballNode;
SceneNode* ball2Node;
SceneNode* padNode;
SceneNode* ballLightNode;
SceneNode* staticLightNode;
SceneNode* staticLightNode2;
SceneNode* staticLightNode3;
SceneNode* animatedLightNode;
SceneNode* textureAtlasNode;
SceneNode* textEmptyNode;

double ballRadius = 3.0f;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
Gloom::Shader* overlayShader;
;
sf::Sound* sound;

const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 padDimensions(30, 3, 40);

glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.2f);
glm::vec3 boxCenter(0, -10, -80);
glm::vec3 cameraPosition;
glm::mat4 orthoProject;

CommandLineOptions options;

bool hasStarted        = false;
bool hasLost           = false;
bool jumpedToNextFrame = false;
bool isPaused          = false;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;
void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    padPositionX -= mouseSensitivity * deltaX / windowWidth;
    padPositionZ -= mouseSensitivity * deltaY / windowHeight;

    if (padPositionX > 1) padPositionX = 1;
    if (padPositionX < 0) padPositionX = 0;
    if (padPositionZ > 1) padPositionZ = 1;
    if (padPositionZ < 0) padPositionZ = 0;

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}

//// A few lines to help you if you've never used c++ structs
//struct LightSource {
//    LightSource() {
//        lightColor = glm::vec3(4);
//        float constant = 1.0;
//        float linear = 0.0009;
//        float quadratic = 0.0032;
//    }
//    glm::vec3 lightColor;
//    float constant;
//    float linear;
//    float quadratic;
// };
//LightSource lightSources[3];

std::vector<SceneNode*> lightSources;
int NumLightProcessed = 0;

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    options = gameOptions;

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    
    orthoProject = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight), nearPlane, farPlane);
    //orthoProject = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, nearPlane, farPlane);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    shader->activate();

    overlayShader = new Gloom::Shader();
    overlayShader->makeBasicShader("../res/shaders/overlay.vert", "../res/shaders/overlay.frag");

    // Create meshes
    Mesh pad = cube(padDimensions, glm::vec2(30, 40), true);
    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);
    Mesh sphere = generateSphere(1.0, 40, 40);
    Mesh sphere2 = generateSphere(1.0, 40, 40);
    Mesh textMesh = generateTextGeometryBuffer("Click to begin!", 39 / 29, 0.5);
    Mesh textEmptyMesh = generateTextGeometryBuffer("               ", 39 / 29, 0.5);

    // ------ Fill buffers ------ //
    unsigned int ballVAO = generateBuffer(sphere);
    unsigned int ball2VAO = generateBuffer(sphere2);
    unsigned int boxVAO  = generateBuffer(box);
    unsigned int padVAO  = generateBuffer(pad);
    unsigned int textVAO = generateBuffer(textMesh);
    unsigned int textEmptyVAO = generateBuffer(textEmptyMesh);

    // ------ Construct scene ------ //
    //create scene nodes 
    rootNode = createSceneNode();
    boxNode  = createSceneNode();
    padNode  = createSceneNode();
    ballNode = createSceneNode();
    ball2Node = createSceneNode();
    ball2Node->position = boxCenter;
    textureAtlasNode = createSceneNode();
    //textureAtlasNode->position = glm::vec3(0, 0, -80);
    textureAtlasNode->position = boxCenter;

    textEmptyNode = createSceneNode();
    textEmptyNode->position = boxCenter;
    /*for (int i = 0; i < 3; ++i) {
        lightSources.push_back(createSceneNode());
    }*/

    //for (auto& node : lightSources) {
    //    node = createSceneNode();
    //    //node->nodeType = POINT_LIGHT;
    //}

    ballLightNode = createSceneNode();
    ballLightNode->nodeType = POINT_LIGHT;
    ballLightNode->lightColor = glm::vec3(10, 10, 10);
    //ballLightNode->lightColor = glm::vec3(8);

    staticLightNode = createSceneNode();
    staticLightNode->nodeType = POINT_LIGHT;
    staticLightNode->lightColor = glm::vec3(16,0,0);
    staticLightNode->position= glm::vec3(-5,0,0);
    
    staticLightNode2 = createSceneNode();
    staticLightNode2->nodeType = POINT_LIGHT;
    staticLightNode2->lightColor = glm::vec3(0,16,0);
    staticLightNode2->position= glm::vec3(0,0,50);
    //staticLightNode2->lightColor = glm::vec3(8);
     
    staticLightNode3 = createSceneNode();
    staticLightNode3->nodeType = POINT_LIGHT;
    staticLightNode3->lightColor = glm::vec3(0,0,16);
    staticLightNode3->position = glm::vec3(5,0,0);



    //animatedLightNode = createSceneNode();
    //animatedLightNode->nodeType = POINT_LIGHT;
    //animatedLightNode->position = glm::vec3(0, 0, 1);
    //animatedLightNode->lightColor = glm::vec3(5, 0, 0);
    //animatedLightNode->lightColor = glm::vec3(8);

    // attatch to scene graph
    rootNode->children.push_back(boxNode);

    rootNode->children.push_back(padNode);

    rootNode->children.push_back(ballNode);
    //rootNode->children.push_back(ball2Node);

    rootNode->children.push_back(textureAtlasNode);
    rootNode->children.push_back(textEmptyNode);
    ballNode->children.push_back(ballLightNode);
    rootNode->children.push_back(staticLightNode);
    rootNode->children.push_back(staticLightNode2);
    rootNode->children.push_back(staticLightNode3);
    //padNode->children.push_back(animatedLightNode);
    
    //ballNode->children.push_back(lightSources[0]);
    //lightSources[0]->position = glm::vec3(0, 0, 2);
    //rootNode->children.push_back(lightSources[1]);
    //padNode->children.push_back(lightSources[2]);

    // assign VAO
    boxNode->vertexArrayObjectID  = boxVAO;
    boxNode->VAOIndexCount        = box.indices.size();

    padNode->vertexArrayObjectID  = padVAO;
    padNode->VAOIndexCount        = pad.indices.size();

    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount       = sphere.indices.size();    
    
    ball2Node->vertexArrayObjectID = ball2VAO;
    ball2Node->VAOIndexCount       = sphere.indices.size();

    textureAtlasNode->vertexArrayObjectID = textVAO;
    textureAtlasNode->VAOIndexCount = textMesh.indices.size();
    
    textEmptyNode->vertexArrayObjectID = textEmptyVAO;
    textEmptyNode->VAOIndexCount = textEmptyMesh.indices.size();

    // ------------------- textures ------------------- // 

    textureAtlasNode->nodeType = OVERLAY;
    textureAtlasNode->diffuseTexture = loadPNGFile("../res/textures/charmap.png");
    unsigned int atlasDiffuseID = generateTexture(textureAtlasNode->diffuseTexture, false);
    textureAtlasNode->diffuseTextureID = atlasDiffuseID;
    
    textEmptyNode->nodeType = OVERLAY;
    //textEmptyNode->diffuseTexture = loadPNGFile("../res/textures/charmap.png");
    unsigned int emptyDiffuseID = generateTexture(textureAtlasNode->diffuseTexture, false);
    textEmptyNode->diffuseTextureID = emptyDiffuseID;



    boxNode->nodeType = TEXTURED_GEOMETRY;
    boxNode->diffuseTexture= loadPNGFile("../res/textures/Brick03_col.png");
    boxNode->normalTexture = loadPNGFile("../res/textures/Brick03_nrm.png");
    boxNode->roughnessTexture = loadPNGFile("../res/textures/Brick03_rgh.png");

    //boxNode->diffuseTexture = brickDiffuse;
    //boxNode->normalTexture = brickNormal;

    unsigned int brickDiffuseID = generateTexture(boxNode->diffuseTexture, false);
    boxNode->diffuseTextureID = brickDiffuseID;
    unsigned int brickNormalID = generateTexture(boxNode->normalTexture, false);
    boxNode->normalTextureID = brickNormalID;    
    unsigned int roughnessTextureID = generateTexture(boxNode->roughnessTexture, false);
    boxNode->roughnessTextureID = roughnessTextureID;
    
    
    //padNode->nodeType = TEXTURED_GEOMETRY;
    //padNode->diffuseTextureID = brickDiffuseID;
    //padNode->normalTextureID= brickNormalID;

    //auto textAtlas = loadPNGFile("../res/textures/charmap.png");
    //unsigned int textAtlasID = generateTexture(textAtlas, true);


    //textureAtlasNode->diffuseTextureID = textAtlasID;
    //textureAtlasNode->normalTextureID = brickNormalID;

    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();

    const float ballBottomY = boxNode->position.y - (boxDimensions.y/2) + ballRadius + padDimensions.y;
    const float ballTopY    = boxNode->position.y + (boxDimensions.y/2) - ballRadius;
    const float BallVerticalTravelDistance = ballTopY - ballBottomY;

    const float cameraWallOffset = 30; // Arbitrary addition to prevent ball from going too much into camera

    const float ballMinX = boxNode->position.x - (boxDimensions.x/2) + ballRadius;
    const float ballMaxX = boxNode->position.x + (boxDimensions.x/2) - ballRadius;
    const float ballMinZ = boxNode->position.z - (boxDimensions.z/2) + ballRadius;
    const float ballMaxZ = boxNode->position.z + (boxDimensions.z/2) - ballRadius - cameraWallOffset;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }

    if(!hasStarted) {
        if (mouseLeftPressed) {
            if (options.enableMusic) {
                sound = new sf::Sound();
                sound->setBuffer(*buffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            totalElapsedTime = debug_startTime;
            gameElapsedTime = debug_startTime;
            hasStarted = true;

            textureAtlasNode->VAOIndexCount = textEmptyNode->VAOIndexCount;
            textureAtlasNode->vertexArrayObjectID = textEmptyNode->vertexArrayObjectID;
        }

        ballPosition.x = ballMinX + (1 - padPositionX) * (ballMaxX - ballMinX);
        ballPosition.y = ballBottomY;
        ballPosition.z = ballMinZ + (1 - padPositionZ) * ((ballMaxZ+cameraWallOffset) - ballMinZ);
    } else {
        totalElapsedTime += timeDelta;
        if(hasLost) {
            if (mouseLeftReleased) {
                hasLost = false;
                hasStarted = false;
                currentKeyFrame = 0;
                previousKeyFrame = 0;
            }
        } else if (isPaused) {
            if (mouseRightReleased) {
                isPaused = false;
                if (options.enableMusic) {
                    sound->play();
                }
            }
        } else {
            gameElapsedTime += timeDelta;
            if (mouseRightReleased) {
                isPaused = true;
                if (options.enableMusic) {
                    sound->pause();
                }
            }
            // Get the timing for the beat of the song
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (gameElapsedTime < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = gameElapsedTime - frameStart;
            double frameDuration = frameEnd - frameStart;
            double fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            double ballYCoord;

            KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);

            // Synchronize ball with music
            if (currentOrigin == BOTTOM && currentDestination == BOTTOM) {
                ballYCoord = ballBottomY;
            } else if (currentOrigin == TOP && currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance;
            } else if (currentDestination == BOTTOM) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * (1 - fractionFrameComplete);
            } else if (currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * fractionFrameComplete;
            }

            // Make ball move
            const float ballSpeed = 60.0f;
            ballPosition.x += timeDelta * ballSpeed * ballDirection.x;
            ballPosition.y = ballYCoord;
            ballPosition.z += timeDelta * ballSpeed * ballDirection.z;

            // Make ball bounce
            if (ballPosition.x < ballMinX) {
                ballPosition.x = ballMinX;
                ballDirection.x *= -1;
            } else if (ballPosition.x > ballMaxX) {
                ballPosition.x = ballMaxX;
                ballDirection.x *= -1;
            }
            if (ballPosition.z < ballMinZ) {
                ballPosition.z = ballMinZ;
                ballDirection.z *= -1;
            } else if (ballPosition.z > ballMaxZ) {
                ballPosition.z = ballMaxZ;
                ballDirection.z *= -1;
            }

            if(options.enableAutoplay) {
                padPositionX = 1-(ballPosition.x - ballMinX) / (ballMaxX - ballMinX);
                padPositionZ = 1-(ballPosition.z - ballMinZ) / ((ballMaxZ+cameraWallOffset) - ballMinZ);
            }

            // Check if the ball is hitting the pad when the ball is at the bottom.
            // If not, you just lost the game! (hehe)
            if (jumpedToNextFrame && currentOrigin == BOTTOM && currentDestination == TOP) {
                double padLeftX  = boxNode->position.x - (boxDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x);
                double padRightX = padLeftX + padDimensions.x;
                double padFrontZ = boxNode->position.z - (boxDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z);
                double padBackZ  = padFrontZ + padDimensions.z;

                if (   ballPosition.x < padLeftX
                    || ballPosition.x > padRightX
                    || ballPosition.z < padFrontZ
                    || ballPosition.z > padBackZ
                ) {
                    hasLost = true;
                    if (options.enableMusic) {
                        sound->stop();
                        delete sound;
                    }
                }
            }
        }
    }

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), nearPlane, farPlane);

    //orthoProject = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight), nearPlane, farPlane);

    cameraPosition = glm::vec3(0, 2, -20);

    // Some math to make the camera move in a nice way
    float lookRotation = -0.6 / (1 + exp(-5 * (padPositionX-0.5))) + 0.3;
    glm::mat4 cameraTransform =
                    glm::rotate(0.3f + 0.2f * float(-padPositionZ*padPositionZ), glm::vec3(1, 0, 0)) *
                    glm::rotate(lookRotation, glm::vec3(0, 1, 0)) *
                    glm::translate(-cameraPosition);

    glm::mat4 VP = projection * cameraTransform;

    // Move and rotate various SceneNodes
    boxNode->position = { 0, -10, -80 };

    ballNode->position = ballPosition;
    ballNode->scale = glm::vec3(ballRadius);
    ballNode->rotation = { 0, totalElapsedTime*2, 0 };

    padNode->position  = {
        boxNode->position.x - (boxDimensions.x/2) + (padDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x),
        boxNode->position.y - (boxDimensions.y/2) + (padDimensions.y/2),
        boxNode->position.z - (boxDimensions.z/2) + (padDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z)
    };

    updateNodeTransformations(rootNode, VP, cameraTransform);


}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar, glm::mat4 viewTransformation) {
    glm::mat4 transformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);

    node->modelMatrix = transformationMatrix; // M
    node->modelViewMatrix = viewTransformation * transformationMatrix; // MV
    node->currentTransformationMatrix = transformationThusFar * transformationMatrix; // model view projection matrix

    switch(node->nodeType) {
        case GEOMETRY: break;
        case POINT_LIGHT: break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix, viewTransformation);
    }
}

void renderNode(SceneNode* node) {

    auto screenPos = glm::vec3(node->modelMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));

    
    // Common uniforms for phong shader 
    shader->activate();
    glUniformMatrix4fv(shader->getUniformFromName("MVP"), 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix)); // MVP

    glUniformMatrix4fv(shader->getUniformFromName("modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(node->modelViewMatrix));

    glUniform3fv(shader->getUniformFromName("textPos"), 1, glm::value_ptr(screenPos));

    glUniformMatrix4fv(shader->getUniformFromName("modelMatrix"), 1, GL_FALSE, glm::value_ptr(node->modelMatrix));

    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(node->modelMatrix)));
    glUniformMatrix3fv(shader->getUniformFromName("normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));


    glUniform1i(shader->getUniformFromName("useTexture"), 0);

    
    //unsigned int diffuseTextureID = generateTexture(node->normalTexture, false);
    //node->normalTextureID = diffuseTextureID;
    //unsigned int normalTextureID = generateTexture(node->diffuseTexture, false);
    //node->diffuseTextureID = normalTextureID;

    std::string number = std::to_string(NumLightProcessed);
    std::string numSprite = std::to_string(0);
    //std::string numPBR = std::to_string(0);
    switch(node->nodeType) {
        case GEOMETRY:
            shader->activate();
            if(node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case TEXTURED_GEOMETRY:
            shader->activate();

            glUniform1i(shader->getUniformFromName("useTexture"), 1);
            glBindTextureUnit(1, node->diffuseTextureID);
            glBindTextureUnit(2, node->normalTextureID);
            glBindTextureUnit(3, node->roughnessTextureID);
            
        /*    glBindTextureUnit(shader->getUniformFromName("texture_in.diffuse"), node->diffuseTextureID);
            glBindTextureUnit(shader->getUniformFromName("texture_in.normal"), node->normalTextureID);*/

            if (node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case OVERLAY: 
            overlayShader->activate();
            // Common uniforms for overlay shader 
            //glBindTextureUnit(overlayShader->getUniformFromName(("textSprite[" + numSprite + "].position").c_str()), textureID);
            glBindTextureUnit(3, node->diffuseTextureID);
            //glBindTextureUnit(overlayShader->getUniformFromName("atlas.color"), node->diffuseTextureID);

            glUniformMatrix4fv(overlayShader->getUniformFromName("modelMatrix"), 1, GL_FALSE, glm::value_ptr(node->modelMatrix));
            glUniformMatrix4fv(overlayShader->getUniformFromName("modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(node->modelViewMatrix));
            glUniformMatrix4fv(overlayShader->getUniformFromName("orthoMatrix"), 1, GL_FALSE, glm::value_ptr(orthoProject));
    
            glm::mat4 customMatrix(1.0f);
            //customMatrix = customMatrix * glm::translate(glm::vec3(0,0,0));
            glUniformMatrix4fv(overlayShader->getUniformFromName("customMatrix"), 1, GL_FALSE, glm::value_ptr(customMatrix));


            if (node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }


            //auto pos = (node->currentTransformationMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));

            break;
        case SPRITE: 
            if (node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            //auto pos = (node->currentTransformationMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));
            //glUniform3fv(shader->getUniformFromName(("textSprite[" + numSprite + "].position").c_str()), 1, glm::value_ptr(glm::vec3(pos.x, pos.y, pos.z)));
            break;
        case POINT_LIGHT: 
            shader->activate();
            //glUniform1ui(7, lightSources.size());
            auto pos = (node->currentTransformationMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));
            glUniform3fv(shader->getUniformFromName(("pointLights["+ number +"].position").c_str()), 1, glm::value_ptr(glm::vec3(pos.x, pos.y, pos.z)));
            //glUniform3fv(shader->getUniformFromName(("pointLights["+ number +"].lightColor").c_str()), 1, glm::value_ptr(lightSources[NumLightProcessed].lightColor));
            glUniform3fv(shader->getUniformFromName(("pointLights["+ number +"].lightColor").c_str()), 1, glm::value_ptr(node->lightColor));

            glUniform1f(shader->getUniformFromName(("pointLight[" + number + "].constant").c_str()), node->constant);
            glUniform1f(shader->getUniformFromName(("pointLight[" + number + "].linear").c_str()), node->linear);
            glUniform1f(shader->getUniformFromName(("pointLight[" + number + "].quadratic").c_str()), 0.0f);
            NumLightProcessed++;
            break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {

    //unsigned int diffuseTextureID = generateTexture(boxNode->normalTexture, false);
    //boxNode->normalTextureID = diffuseTextureID;
    //unsigned int normalTextureID = generateTexture(boxNode->diffuseTexture, false);
    //boxNode->diffuseTextureID = normalTextureID;
    //
    //unsigned int atlasNormalID = generateTexture(textureAtlasNode->diffuseTexture, false);
    //textureAtlasNode->diffuseTextureID = atlasNormalID;

    glUniform3fv(shader->getUniformFromName("viewPos"), 1, glm::value_ptr(cameraPosition));
    glUniform3fv(overlayShader->getUniformFromName("viewPos"), 1, glm::value_ptr(cameraPosition));
    glUniform3fv(shader->getUniformFromName("lightTest"), 1, glm::value_ptr(glm::vec3(1, 0, 0)));

    auto ballPos = glm::vec3(ballNode->modelMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));
    glUniform3fv(shader->getUniformFromName("ballPos"), 1, glm::value_ptr(ballPos));
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    renderNode(rootNode);
    NumLightProcessed = 0; // reset 
}
