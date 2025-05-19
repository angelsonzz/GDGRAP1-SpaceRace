#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"    
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
using namespace glm;

/* credits:

ball by xuzi
https://www.cgtrader.com/free-3d-models/industrial/other/ball1

Car by koomtoon
https://www.cgtrader.com/free-3d-models/car/racing-car/flying-car-for-future

Artifact Wall Light With Hole in Middle free VR by mynameismohittiwari77
https://www.cgtrader.com/free-3d-models/science/other/artifact-77fb899e-db94-4e88-9a1e-7073a96f4f5b

Texture of the surface of planet Mercury, Jupiter, and Mars
https://www.solarsystemscope.com/textures/

Texture of earth
https://www.cgtrader.com/free-3d-models/space/planet/earth-globe-6d13e69d-b8e2-415b-9349-a8b63b4fbd8b

Texture of meteorite
https://www.cgtrader.com/free-3d-models/space/planet/asteroiden

Model Editor: 3D Editor Online Free
https://3deditoronline.com/

Thin Matrix. (Nov 16, 2024). OpenGL 3D Game Tutorial 19: 3rd Person Camera. 
https://www.youtube.com/watch?v=PoxDDZmctnU

Skybox
https://tools.wwwtyro.net/space-3d/index.html
*/

int cameraMode = -1;
bool raceStarted = false;
bool stopCars = true;
bool day = true;

float perspectiveCameraZoom = 1.5f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    float zoomLimit = 0.95f;
    perspectiveCameraZoom -= yoffset*0.5f;
    if (perspectiveCameraZoom <= zoomLimit) {
        perspectiveCameraZoom = zoomLimit;
    }
}

void getUserInput(GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods) {

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        stopCars=!stopCars;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        cameraMode = 1; //Perspective Camera
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        // Releases the mouse cursor from the window, GLFW_CURSOR_DISABLED
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        // Sets the window close flag to true (closes the window)
        glfwSetWindowShouldClose(window, 1);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        day = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        day = false;
    }
}

class VAO {
private:
    // Mesh Data
    string path;
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> material;
    string warning, error;
    tinyobj::attrib_t attributes;

    vector<GLuint> meshIndices;
    vector<GLfloat> fullVertexData;

    // VAO and VBO
    GLuint vao, vbo;

public:
    VAO(string objFilePath) {
        //Initialization
        path = objFilePath;
        bool success = tinyobj::LoadObj(
            &attributes, //Overall def
            &shapes,  //Refers to the object itself
            &material, //Refers to the texture/image
            &warning,
            &error,
            path.c_str()
        );
        /* We need to instruct the EBO from the Mesh Data */
        for (int i = 0; i < shapes[0].mesh.indices.size(); i++) {
            meshIndices.push_back(shapes[0].mesh.indices[i].vertex_index);
        }

        for (int i = 0; i < shapes[0].mesh.indices.size(); i++) {
            tinyobj::index_t vData = shapes[0].mesh.indices[i];

            fullVertexData.push_back(attributes.vertices[(vData.vertex_index * 3)]);
            fullVertexData.push_back(attributes.vertices[(vData.vertex_index * 3) + 1]);
            fullVertexData.push_back(attributes.vertices[(vData.vertex_index * 3) + 2]);

            fullVertexData.push_back(attributes.normals[(vData.normal_index * 3)]);
            fullVertexData.push_back(attributes.normals[(vData.normal_index * 3) + 1]);
            fullVertexData.push_back(attributes.normals[(vData.normal_index * 3) + 2]);

            fullVertexData.push_back(attributes.texcoords[(vData.texcoord_index * 2)]);
            fullVertexData.push_back(attributes.texcoords[(vData.texcoord_index * 2) + 1]);
        }

        //VAO VBO
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * fullVertexData.size(), fullVertexData.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            8 * sizeof(float),
            (void*)0
        );


        GLintptr normalPtr = 3 * sizeof(float);
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            8 * sizeof(float),
            (void*)normalPtr
        );

        GLintptr uvPtr = 6 * sizeof(float);
        glVertexAttribPointer(
            //0 = position, 1 ?, 2 = UV/Texture data
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            8 * sizeof(GLfloat),
            (void*)uvPtr
        );

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_TEXTURE_2D, 2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~VAO() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    GLuint getVAO() {
        return vao;
    }

    vector<GLfloat> getFullVertexData() {
        return fullVertexData;
    }

};

class Texture {
private:
    //Texture Data
    int img_w, img_h, color_channels; //3 color channels (RGB) or 4 for RGBA
    unsigned char* tex_bytes;
    string texFilePath;
    GLuint texture;
    int texSlot;
    
public:
    Texture(string textureFilePath, int newTexSlot) {
        texSlot = newTexSlot;
        stbi_set_flip_vertically_on_load(true);

        //1st para: path of image
        texFilePath = textureFilePath;
        tex_bytes = stbi_load(
            texFilePath.c_str(), // Convert string into char*
            &img_w,
            &img_h,
            &color_channels, //3 for RGB and 4 for RGBA
            0
        );
        glGenTextures(1, &texture);
        switch (texSlot) {
        case 0:
            glActiveTexture(GL_TEXTURE0); 
            break;
        case 1:
            glActiveTexture(GL_TEXTURE1);
            break;
        case 2:
            glActiveTexture(GL_TEXTURE2);
            break;
        case 3:
            glActiveTexture(GL_TEXTURE3);
            break;
        case 4:
            glActiveTexture(GL_TEXTURE4);
            break;
        case 5:
            glActiveTexture(GL_TEXTURE5);
            break;
        case 6:
            glActiveTexture(GL_TEXTURE6);
            break;
        case 7:
            glActiveTexture(GL_TEXTURE7);
            break;
        }
        glBindTexture(GL_TEXTURE_2D, texture);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

        // If there are only 3 color channels or if it is a JPG
        if (color_channels == 3) {
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGB, //4 == rgba, 3 == rgb
                img_w,
                img_h,
                0, //border
                GL_RGB,
                GL_UNSIGNED_BYTE,
                tex_bytes
            );
        }

        // if there are 4 color channels R,G,B, and A (transparency)
        if (color_channels == 4) {
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA, //4 == rgba, 3 == rgb
                img_w,
                img_h,
                0, //border
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                tex_bytes
            );
        }
  
        //Mipmap
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(tex_bytes);
    }

    GLuint getTexture() {
        return texture;
    }
    int getTexSlot() {
        return texSlot;
    }
};

class NormalMapTexture :public Texture {
    private:
        string normFilePath;
        int img_w2, img_h2, color_channels2; 
        unsigned char* norm_bytes;
        GLuint normTexture;
        int normTexSlot;
    public:
        NormalMapTexture(string textureFilePath, int newTexSlot, string normTextureFilePath, int newNormTexSlot) :Texture(textureFilePath, newTexSlot) {
            normTexSlot = newNormTexSlot;
            stbi_set_flip_vertically_on_load(true);

            //1st para: path of image
            normFilePath = normTextureFilePath;
            norm_bytes = stbi_load(
                normFilePath.c_str(), // Convert string into char*
                &img_w2,
                &img_h2,
                &color_channels2, //3 for RGB and 4 for RGBA
                0
            );


            glGenTextures(1, &normTexture);
            switch (normTexSlot) {
            case 6:
                glActiveTexture(GL_TEXTURE6);
                break;
            case 7:
                glActiveTexture(GL_TEXTURE7);
                break;
            case 8:
                glActiveTexture(GL_TEXTURE8);
                break;
            case 9:
                glActiveTexture(GL_TEXTURE9);
                break;
            }
            glBindTexture(GL_TEXTURE_2D, normTexture);

            // If there are only 3 color channels or if it is a JPG
            if (color_channels2 == 3) {
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RGB, //4 == rgba, 3 == rgb
                    img_w2,
                    img_h2,
                    0, //border
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    norm_bytes
                );
            }

            // if there are 4 color channels R,G,B, and A (transparency)
            if (color_channels2 == 4) {
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RGBA, //4 == rgba, 3 == rgb
                    img_w2,
                    img_h2,
                    0, //border
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    norm_bytes
                );
            }

            //Mipmap
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(norm_bytes);
        }

        GLuint getNormTexture() { return normTexture; }
        int getNormTexSlot() {return normTexSlot;}
};

class Shader {
private:
    GLuint shaderProg, vertexShader, fragShader;
public:
    Shader(std::string vertFilePath, std::string fragFilePath) {
        /* Load and create a file */
        fstream vertSrc(vertFilePath);
        stringstream vertBuff;
        vertBuff << vertSrc.rdbuf();

        /* Convert stream into a character array */
        string vertS = vertBuff.str();
        const char* v = vertS.c_str();

        fstream fragSrc(fragFilePath);
        stringstream fragBuff;
        fragBuff << fragSrc.rdbuf();
        string fragS = fragBuff.str();
        const char* f = fragS.c_str();

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &v, NULL);
        glCompileShader(vertexShader);

        /* Create a fragment shader
        *  Assign source to fragment shader
        *  Compile the Fragment Shader
        */
        fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, &f, NULL);
        glCompileShader(fragShader);
            
        /*
        * Create the shader program
        * Attach the compiled vertex & fragment shader
        */
        shaderProg = glCreateProgram();
        glAttachShader(shaderProg, vertexShader);
        glAttachShader(shaderProg, fragShader);
        glLinkProgram(shaderProg);
    }
    void activate() {
        glUseProgram(shaderProg);
    }
    GLuint getShader() {
        return shaderProg;
    }

    ~Shader() {
        glDeleteShader(vertexShader);
        glDeleteShader(fragShader);
    }
};

class Entity3D {
protected:
    vec3 pos, size, theta;
public:
    Entity3D() {
        pos = vec3(0.0f);
        size = vec3(1.0f);
        theta = vec3(0.0f, 0.0f, 0.0f);
    }
    void setPosX(float newPosX) {
        pos.x = newPosX;
    }
    void setPosY(float newPosY) {
        pos.y = newPosY;
    }
    void setPosZ(float newPosZ) {
        pos.z = newPosZ;
    }
    void setSize(float newSize) {
        size = vec3(newSize, newSize, newSize);
    }
    void setScaleX(float newScaleX) {
        size = vec3(newScaleX, size.y, size.z);
    }
    void setScaleY(float newScaleY) {
        size = vec3(size.x, newScaleY, size.z);
    }
    void setScaleZ(float newScaleZ) {
        size = vec3(size.x, size.y, newScaleZ);
    }
    void setThetaX(float newTheta) {
        theta.x = newTheta;
    }
    void setThetaY(float newTheta) {
        theta.y = newTheta;
    }
    void setThetaZ(float newTheta) {
        theta.z = newTheta;
    }
    void addThetaX(float thetaIncrement) {
        theta.x += thetaIncrement;
    }
    void addThetaY(float thetaIncrement) {
        theta.y += thetaIncrement;
    }
    void addThetaZ(float thetaIncrement) {
        theta.z += thetaIncrement;
    }
    vec3 getPos() {
        return pos;
    }
    vec3 getScale() {
        return size;
    }
    vec3 getTheta() {
        return theta;
    }
};

class Camera : public Entity3D { 
protected:
    float windowWidth, windowHeight;
    vec3 worldUp;
    mat4 cameraPosMatrix;
    vec3 cameraGaze;
    mat4 cameraOrientation, viewMatrix, projectionMatrix;

public:
    Camera() {
        //Empty Constructor
    }
    Camera(float newWindowWidth, float newWindowHeight) {
        windowWidth = newWindowWidth;
        windowHeight = newWindowHeight;

        worldUp = normalize(vec3(0.f, 1.f, 0.f)); //Pointing upward

        pos = vec3(0.0f, 0.0f, 1.0f);

        cameraGaze = vec3(0.0f, 0.0f, 0.0f); // Gaze is where the camera is looking. In this case 0,0,0 is the center of the Model we are looking at

        viewMatrix = lookAt(pos, cameraGaze, worldUp);

        projectionMatrix = mat4(1.0f);
    }
    virtual void update(float newWindowWidth, float newWindowHeight) {
        windowWidth = newWindowWidth;
        windowHeight = newWindowHeight;

        cameraGaze = vec3(0.0f, 0.0f, 0.0f);

        viewMatrix = lookAt(pos, cameraGaze, worldUp);
    }
    mat4 getViewMatrix() {
        return viewMatrix;
    }
    mat4 getProjectionMatrix() {
        return projectionMatrix;
    }
};

class Light : public Entity3D {
protected:
    vec3 lightColor;
    float lightLumens; // Value is from 0.0 (Unlit) to 1.0f (Lit)
    float ambientStr;
    vec3 ambientColor;
    float specStr;
    float specPhong;

public:
    Light() {
        pos = vec3(0.0f, 0.0f, 1.0f);
        lightColor = vec3(1.0f, 1.0f, 1.0f);
        lightLumens = 15.0f;
        ambientStr = 0.1f;
        ambientColor = lightColor;
        specStr = 0.5f;
        specPhong = 15;
    }
    virtual void getUserInput(GLFWwindow* window) {}
    
    void setRGB(vec3 newRGB) {
        lightColor = normalize(vec3(newRGB));
        ambientColor = normalize(vec3(newRGB));
    }
    void setR(float newRedValue) {
        lightColor.x = newRedValue;
        ambientColor.x = newRedValue;
    }
    void setG(float newGreenValue) {
        lightColor.y = newGreenValue;
        ambientColor.y = newGreenValue;
    }
    void setB(float newBlueValue) {
        lightColor.z = newBlueValue;
        ambientColor.z = newBlueValue;
    }
    void setLumens(float newLumens) {
        lightLumens = newLumens;
    }
    vec3 getLightColor() {
        return normalize(lightColor);
    }
    float getLumens() {
        return lightLumens;
    }
    float getAmbientStr() {
        return ambientStr;
    }
    vec3 getAmbientColor() {
        return ambientColor;
    }
    float getSpecStr() {
        return specStr;
    }
    float getSpecPhong() {
        return specPhong;
    }

};

class PointLight :public Light {
private:
public:
    PointLight() {
        pos = vec3(0.0f, 0.0f, 0.0f);
        lightColor = vec3(1.0f, 1.0f, 1.0f);
        lightLumens = 5.0f;
        ambientStr = 0.1f;
        ambientColor = lightColor;
        specStr = 0.5f;
        specPhong = 15;

    }
    void update() {};
};

class DirectionLight : public Light {
private:
    vec3 direction;
public:
    DirectionLight(vec3 newDirection) {
        direction = newDirection;
    }
    vec3 getDirection() {
        return direction;
    }
};

class Model3D : public Entity3D {
protected:
    VAO* modelVAO;
    Texture* texture;
    Shader* modelShader;
    mat4 identity_matrix, transformation_matrix;
    float transparency;

public:
    Model3D() {
        //Empty Default constructor
    }
    Model3D(VAO* newModelVao, Texture* newTexture, Shader* newShader) {
        modelVAO = newModelVao;
        texture = newTexture;
        modelShader = newShader;
        identity_matrix = mat4(1.0f);
        transparency = 1.0f;
    }

    virtual void getUserInput(GLFWwindow* window) {};

    void setTransparency(float newTransparency) {
        transparency = newTransparency;
    }

    void draw(Camera camera, PointLight pointLight, DirectionLight directionLight) {

        modelShader->activate(); //To update the uniformVariables, glUseProgram(shaderProg) first.

        unsigned int viewLoc = glGetUniformLocation(modelShader->getShader(), "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(camera.getViewMatrix()));

        unsigned int projLoc = glGetUniformLocation(modelShader->getShader(), "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(camera.getProjectionMatrix()));

        unsigned int transformLocation = glGetUniformLocation(modelShader->getShader(), "transform");

        transformation_matrix = translate(identity_matrix, pos);
        transformation_matrix = scale(transformation_matrix, size);
        transformation_matrix = rotate(transformation_matrix, radians(theta.x), normalize(vec3(1.0f, 0.0f, 0.0f)));
        transformation_matrix = rotate(transformation_matrix, radians(theta.y), normalize(vec3(0.0f, 1.0f, 0.0f)));
        transformation_matrix = rotate(transformation_matrix, radians(theta.z), normalize(vec3(0.0f, 0.0f, 1.0f)));

        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, value_ptr(transformation_matrix));

        //Shader Update
        switch (texture->getTexSlot()) {
        case 0:
            glActiveTexture(GL_TEXTURE0);
            break;
        case 1:
            glActiveTexture(GL_TEXTURE1);
            break;
        case 2:
            glActiveTexture(GL_TEXTURE2);
            break;
        case 3:
            glActiveTexture(GL_TEXTURE3);
            break;
        case 4:
            glActiveTexture(GL_TEXTURE4);
            break;
        case 5:
            glActiveTexture(GL_TEXTURE5);
            break;
        case 6:
            glActiveTexture(GL_TEXTURE6);
            break;
        }

        switch (texture->getTexSlot()) {
        case 0:
            glActiveTexture(GL_TEXTURE0);
            break;
        case 1:
            glActiveTexture(GL_TEXTURE1);
            break;
        case 2:
            glActiveTexture(GL_TEXTURE2);
            break;
        case 3:
            glActiveTexture(GL_TEXTURE3);
            break;
        case 4:
            glActiveTexture(GL_TEXTURE4);
            break;
        case 5:
            glActiveTexture(GL_TEXTURE5);
            break;
        case 6:
            glActiveTexture(GL_TEXTURE6);
            break;
        }
        glBindTexture(GL_TEXTURE_2D, texture->getTexture());
        GLuint texAddress = glGetUniformLocation(modelShader->getShader(), "tex");
        glUniform1i(texAddress, texture->getTexSlot());

        GLfloat transparencyAddress = glGetUniformLocation(modelShader->getShader(), "transparency");
        glUniform1f(transparencyAddress, transparency);

        GLuint lightAddress = glGetUniformLocation(modelShader->getShader(), "lightPos");
        glUniform3fv(lightAddress, 1, value_ptr(pointLight.getPos()));

        GLuint lightColorAddress = glGetUniformLocation(modelShader->getShader(), "lightColor");
        glUniform3fv(lightColorAddress, 1, value_ptr(pointLight.getLightColor()));

        GLfloat lightLumensAddress = glGetUniformLocation(modelShader->getShader(), "lightLumens");
        glUniform1f(lightLumensAddress, pointLight.getLumens());

        GLuint ambientStrAddress = glGetUniformLocation(modelShader->getShader(), "ambientStr");
        glUniform1f(ambientStrAddress, pointLight.getAmbientStr());

        GLuint ambientColorAddress = glGetUniformLocation(modelShader->getShader(), "ambientColor");
        glUniform3fv(ambientColorAddress, 1, value_ptr(pointLight.getAmbientColor()));

        GLuint cameraPosAddress = glGetUniformLocation(modelShader->getShader(), "cameraPos");
        glUniform3fv(cameraPosAddress, 1, value_ptr(camera.getPos()));

        GLuint specStrAddress = glGetUniformLocation(modelShader->getShader(), "specStr");
        glUniform1f(specStrAddress, pointLight.getSpecStr());

        GLuint specPhongAddress = glGetUniformLocation(modelShader->getShader(), "specPhong");
        glUniform1f(specPhongAddress, pointLight.getSpecPhong());

        GLuint dirLightDirectionAddress = glGetUniformLocation(modelShader->getShader(), "dirLightDirection");
        glUniform3fv(dirLightDirectionAddress, 1, value_ptr(directionLight.getDirection()));

        GLuint dirLightColorAddress = glGetUniformLocation(modelShader->getShader(), "dirLightColor");
        glUniform3fv(dirLightColorAddress, 1, value_ptr(directionLight.getLightColor()));

        GLfloat dirLightLumensAddress = glGetUniformLocation(modelShader->getShader(), "dirLightLumens");
        glUniform1f(dirLightLumensAddress, directionLight.getLumens());

        GLuint dirAmbientStrAddress = glGetUniformLocation(modelShader->getShader(), "dirAmbientStr");
        glUniform1f(dirAmbientStrAddress, directionLight.getAmbientStr());

        GLuint dirAmbientColorAddress = glGetUniformLocation(modelShader->getShader(), "dirAmbientColor");
        glUniform3fv(dirAmbientColorAddress, 1, value_ptr(directionLight.getAmbientColor()));

        GLuint dirSpecStrAddress = glGetUniformLocation(modelShader->getShader(), "dirSpecStr");
        glUniform1f(dirSpecStrAddress, directionLight.getSpecStr());

        GLuint dirSpecPhongAddress = glGetUniformLocation(modelShader->getShader(), "dirSpecPhong");
        glUniform1f(dirSpecPhongAddress, pointLight.getSpecPhong());

        //Bind Current VAO
        glBindVertexArray(modelVAO->getVAO());
        //Draw Current VAO
        glDrawArrays(GL_TRIANGLES, 0, modelVAO->getFullVertexData().size() / 8);
        //Unbind VAO
        glBindVertexArray(0);
        //Set GL_Texture to 0 or default
        glActiveTexture(GL_TEXTURE0);

        // Set glBlendFunc and Equation to "default"
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
    }
};

class NormalMapModel :public Model3D {
    private:
        NormalMapTexture* normTexture;
        float rotateSPD = 0.05f;

    public:
        NormalMapModel(VAO* newModelVao, NormalMapTexture* newNormTexture, Shader* newShader){
            modelVAO = newModelVao;
            modelShader = newShader;
            identity_matrix = mat4(1.0f);
            transparency = 1.0f;
            normTexture = newNormTexture;
        }

        void draw(Camera camera, PointLight pointLight, DirectionLight directionLight) {

            modelShader->activate(); //To update the uniformVariables, glUseProgram(shaderProg) first.

            unsigned int viewLoc = glGetUniformLocation(modelShader->getShader(), "view");
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(camera.getViewMatrix()));

            unsigned int projLoc = glGetUniformLocation(modelShader->getShader(), "projection");
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(camera.getProjectionMatrix()));

            unsigned int transformLocation = glGetUniformLocation(modelShader->getShader(), "transform");

            transformation_matrix = translate(identity_matrix, pos);
            transformation_matrix = scale(transformation_matrix, size);
            transformation_matrix = rotate(transformation_matrix, radians(theta.x), normalize(vec3(1.0f, 0.0f, 0.0f)));
            transformation_matrix = rotate(transformation_matrix, radians(theta.y), normalize(vec3(0.0f, 1.0f, 0.0f)));
            transformation_matrix = rotate(transformation_matrix, radians(theta.z), normalize(vec3(0.0f, 0.0f, 1.0f)));

            glUniformMatrix4fv(transformLocation, 1, GL_FALSE, value_ptr(transformation_matrix));


            
            GLuint texAddress = glGetUniformLocation(modelShader->getShader(), "tex");
            switch(normTexture->getTexSlot()){
            case 0:
                glActiveTexture(GL_TEXTURE0);
                break;
            case 1:
                glActiveTexture(GL_TEXTURE1);
                break;
            case 2:
                glActiveTexture(GL_TEXTURE2);
                break;
            case 3:
                glActiveTexture(GL_TEXTURE3);
                break;
            case 4:
                glActiveTexture(GL_TEXTURE4);
                break;
            case 5:
                glActiveTexture(GL_TEXTURE5);
                break;
            case 6:
                glActiveTexture(GL_TEXTURE6);
                break;
            case 7:
                glActiveTexture(GL_TEXTURE7);
                break;
            }
            glBindTexture(GL_TEXTURE_2D, normTexture->getTexture());
            glUniform1i(texAddress, normTexture->getTexSlot());

            
            GLuint normAddress = glGetUniformLocation(modelShader->getShader(), "tex");
            switch (normTexture->getNormTexSlot()) {
            case 6:
                glActiveTexture(GL_TEXTURE6);
                break;
            case 7:
                glActiveTexture(GL_TEXTURE7);
                break;
            case 8:
                glActiveTexture(GL_TEXTURE8);
                break;
            case 9:
                glActiveTexture(GL_TEXTURE9);
                break;
            }
            glBindTexture(GL_TEXTURE_2D, normTexture->getNormTexture());
            glUniform1i(normAddress, normTexture->getNormTexSlot());

            GLfloat transparencyAddress = glGetUniformLocation(modelShader->getShader(), "transparency");
            glUniform1f(transparencyAddress, transparency);

            GLuint lightAddress = glGetUniformLocation(modelShader->getShader(), "lightPos");
            glUniform3fv(lightAddress, 1, value_ptr(pointLight.getPos()));

            GLuint lightColorAddress = glGetUniformLocation(modelShader->getShader(), "lightColor");
            glUniform3fv(lightColorAddress, 1, value_ptr(pointLight.getLightColor()));

            GLfloat lightLumensAddress = glGetUniformLocation(modelShader->getShader(), "lightLumens");
            glUniform1f(lightLumensAddress, pointLight.getLumens());

            GLuint ambientStrAddress = glGetUniformLocation(modelShader->getShader(), "ambientStr");
            glUniform1f(ambientStrAddress, pointLight.getAmbientStr());

            GLuint ambientColorAddress = glGetUniformLocation(modelShader->getShader(), "ambientColor");
            glUniform3fv(ambientColorAddress, 1, value_ptr(pointLight.getAmbientColor()));

            GLuint cameraPosAddress = glGetUniformLocation(modelShader->getShader(), "cameraPos");
            glUniform3fv(cameraPosAddress, 1, value_ptr(camera.getPos()));

            GLuint specStrAddress = glGetUniformLocation(modelShader->getShader(), "specStr");
            glUniform1f(specStrAddress, pointLight.getSpecStr());

            GLuint specPhongAddress = glGetUniformLocation(modelShader->getShader(), "specPhong");
            glUniform1f(specPhongAddress, pointLight.getSpecPhong());

            GLuint dirLightDirectionAddress = glGetUniformLocation(modelShader->getShader(), "dirLightDirection");
            glUniform3fv(dirLightDirectionAddress, 1, value_ptr(directionLight.getDirection()));

            GLuint dirLightColorAddress = glGetUniformLocation(modelShader->getShader(), "dirLightColor");
            glUniform3fv(dirLightColorAddress, 1, value_ptr(directionLight.getLightColor()));

            GLfloat dirLightLumensAddress = glGetUniformLocation(modelShader->getShader(), "dirLightLumens");
            glUniform1f(dirLightLumensAddress, directionLight.getLumens());

            GLuint dirAmbientStrAddress = glGetUniformLocation(modelShader->getShader(), "dirAmbientStr");
            glUniform1f(dirAmbientStrAddress, directionLight.getAmbientStr());

            GLuint dirAmbientColorAddress = glGetUniformLocation(modelShader->getShader(), "dirAmbientColor");
            glUniform3fv(dirAmbientColorAddress, 1, value_ptr(directionLight.getAmbientColor()));

            GLuint dirSpecStrAddress = glGetUniformLocation(modelShader->getShader(), "dirSpecStr");
            glUniform1f(dirSpecStrAddress, directionLight.getSpecStr());

            GLuint dirSpecPhongAddress = glGetUniformLocation(modelShader->getShader(), "dirSpecPhong");
            glUniform1f(dirSpecPhongAddress, pointLight.getSpecPhong());

            //Bind Current VAO
            glBindVertexArray(modelVAO->getVAO());
            //Draw Current VAO
            glDrawArrays(GL_TRIANGLES, 0, modelVAO->getFullVertexData().size() / 8);
            //Unbind VAO
            glBindVertexArray(0);
            //Set GL_Texture to 0 or default
            glActiveTexture(GL_TEXTURE0);

            // Set glBlendFunc and Equation to "default"
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
        }
        void update() {
            theta.y += rotateSPD;
        }
};

class Kart : public Model3D {
protected:
    string name;
    bool activated;
    float turningSPD, roll;
    float speed, maxSPD, acceleration;
    vec3 kartDir;
    double startTime, endTime;
public:
    Kart() {
        //empty constructor
    }
    Kart(VAO* newModelVao, Texture* newTexture, Shader* newShader, string newName, float maximumSpeed, float newAcceleration) {
        modelVAO = newModelVao;
        texture = newTexture;
        modelShader = newShader;
        identity_matrix = mat4(1.0f);
        transparency = 1.0f;

        name = newName;
        activated = false;

        maxSPD = maximumSpeed;
        speed = 0.0f;
        acceleration = newAcceleration;
        kartDir = vec3(pos.x, pos.y, pos.z + 1.0f);
        turningSPD = 0.075;
        roll = 0.0f;

        startTime = 0.0;
        endTime = 0.0;
    };
    virtual void update() {

        // Steering/Rolling
        if (roll > 0.0f) {
            roll -= turningSPD / 1.5;
        }
        if (roll < 0.0f) {
            roll += turningSPD / 1.5;
        }
        if (roll > 55.f) {
            roll = 55.f;
        }
        if (roll < -55.0f) {
            roll = -55.0f;
        }
        theta.z = roll;

        //Basic Acceleration Deceleration Movement
        if (speed > maxSPD) {
            speed = maxSPD;
        }

        if (activated) {
            speed += acceleration;
        }
        pos += kartDir * speed;

        // Deceleration
        if (!activated) {
            speed -= (acceleration * 0.77);
        }
        if (speed <= 0.0f) {
            speed = 0.0f;
        }
    }
    void setStartTime(double newStartTime) {
        startTime = newStartTime;
    }
    void setEndTime(double newEndTime) {
        endTime = newEndTime;
    }
    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }
    void setAcceleration(float newAcceleration) {
        acceleration = newAcceleration;
    }
    void printTime() {
        cout << "KART: "<<name<<" : Time :"<<endTime - startTime<<":"<<endl;
    }
    void toggleActivation() {
        activated = !activated;
    }
    bool getActivation() {
        return activated;
    }
};

class PlayerKart : public Kart {
private:
    float turningSPD, roll;
    float speed, maxSPD, acce, acceleration;
    bool accelerating, reverse;
    vec3 kartDir;
    enum directions{
        STRAIGHT, RIGHT, LEFT
    };
    directions steeringDir;
public:
    PlayerKart(VAO* newModelVao, Texture* newTexture, Shader* newShader, string newName, float maximumSpeed, float newAcceleration){
        modelVAO = newModelVao;
        texture = newTexture;
        modelShader = newShader;
        identity_matrix = mat4(1.0f);
        transparency = 1.0f;

        name = newName;
        activated = false;
        
        maxSPD = maximumSpeed;
        speed = 0.0f;
        acce = 0.0f;
        acceleration = newAcceleration;
        accelerating = false;
        reverse = false;

        steeringDir = STRAIGHT;
        kartDir = vec3(pos.x, pos.y, pos.z + 1.0f);
        turningSPD = 0.05;
        roll = 0.0f;
    };
    void getUserInput(GLFWwindow* window) {
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            theta.y += turningSPD;
            roll -= turningSPD / 1.15;
            steeringDir = LEFT;
            kartDir = vec3(
                1.0f * sin(radians(theta.y)), 
                0.0f, 
                1.0f * cos(radians(theta.y))
            );
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            theta.y -= turningSPD;
            roll += turningSPD / 1.15;
            steeringDir = RIGHT;
            kartDir = vec3(
                1.0f * sin(radians(theta.y)),
                0.0f,
                1.0f * cos(radians(theta.y))
            );
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            acce = acceleration;
            accelerating = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            acce = -acceleration;
            reverse = true;
        }
        
    };
    void update() {

        // Steering/Rolling
        if (roll > 0.0f || steeringDir==LEFT) {
            roll -= turningSPD * 2.5;
        }
        if (roll < 0.0f || steeringDir==RIGHT) {
            roll += turningSPD * 2.5;
        }
        if (roll > 55.f) {
            roll = 55.f;
        }
        if (roll < -55.0f) {
            roll = -55.0f;
        }
        theta.z = roll;
        steeringDir = STRAIGHT;
        
        //Basic Acceleration Deceleration Movement
        if (speed > maxSPD) {
            speed = maxSPD;
        }
        
        speed += acce;
        if (activated) {
            pos += kartDir * speed;
        }
        acce = 0;


        // Deceleration
        if (!accelerating) {
            speed -= acceleration*0.8;
        }
        if (speed <= 0.0f&&!reverse) {
            speed = 0.0f;
        }
        if (speed <= -maxSPD / 2) {
            speed = -maxSPD / 2;
        }

        accelerating = false;
        reverse = false;
    }
    vec3 getDir() {
        return kartDir;
    }
};

class PerspectiveCamera : public Camera {
private:
    double mouseX, mouseY, prevMouseX, prevMouseY, scrollX, scrollY;
    float sensitivity;
    float thetaX, thetaY;
    float distanceFromFocus;
    PlayerKart* parent;
    bool POV_3;
    double POV_Cooldown;
    double prevTime;
public:
    PerspectiveCamera(float newWindowWidth, float newWindowHeight) {
        windowWidth = newWindowWidth;
        windowHeight = newWindowHeight;

        distanceFromFocus = 2.0f;

        sensitivity = 0.05;

        POV_Cooldown = 0.5;
        prevTime = glfwGetTime();
        POV_3 = true;
        scrollX = 0.0f;
        scrollY = 0.0f;
        mouseX = 0.0f;
        mouseY = 0.0f;
        prevMouseX = 0.0f;
        prevMouseY = 0.0f;
        thetaX = 180.0f; // To have the camera face the back of the kart instead
        thetaY = 0.0f;
        
        worldUp = normalize(vec3(0.f, 1.f, 0.f)); //Pointing upward

        pos = vec3(0.0f, 0.0f, 1.0f);
        cameraGaze = vec3(0.0f, 0.0f, 0.0f); // Gaze is where the camera is looking. In this case 0,0,0 is the center of the Model we are looking at

        viewMatrix = lookAt(pos, cameraGaze, worldUp);

        projectionMatrix = perspective(
            radians(60.f - pos.z),      //This is your FOV
            windowHeight / windowWidth, //Aspect ratio
            0.1f,                       //z-Near, should never be <= 0
            100.f                       //z-Far   
        );
    }
    void attachParent(PlayerKart* newParent) {
        parent = newParent;
    }
    void getInputs(GLFWwindow* window) {
        float zoomSpeed = 1.0f;
        //Mouse Inputs
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (mouseX != prevMouseX) {
            thetaX += (sensitivity * (prevMouseX - mouseX));
            prevMouseX = mouseX;
        }
        if (mouseY != prevMouseY) {
            thetaY += (sensitivity * (prevMouseY - mouseY));
            prevMouseY = mouseY;
            if (thetaY >= 89.0f) {
                thetaY = 89.0f;
            }
            if (thetaY <= -89.0f) {
                thetaY = -89.0f;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            if (glfwGetTime() - prevTime >= POV_Cooldown) {
                POV_3 = !POV_3;
                prevTime = glfwGetTime();
            }
        }
        
    }

    void update(float newWindowWidth, float newWindowHeight) {
        windowWidth = newWindowWidth;
        windowHeight = newWindowHeight;

        if (POV_3 == true) {
            //Update the tracking position of the camera to the position of its parent, the kart
            if (parent != nullptr) {
                cameraGaze.x = parent->getPos().x;
                cameraGaze.y = parent->getPos().y;
                cameraGaze.z = parent->getPos().z;
            }
            //  3rd person camera movement (Thin Matrix, 2024)
            float groundDist = distanceFromFocus * cos(radians(thetaY));
            pos = vec3(
                cameraGaze.x + (groundDist * sin(radians(thetaX))),
                cameraGaze.y + (-distanceFromFocus * sin(radians(thetaY))),
                cameraGaze.z + (groundDist * cos(radians(thetaX)))
            );
        }
        else{
            if (parent != nullptr) {
                pos = vec3(parent->getPos().x, parent->getPos().y + 0.15f, parent->getPos().z);
                cameraGaze = vec3(pos + parent->getDir());
            }
        }


        projectionMatrix = perspective(
            radians(80.0f),      //This is your FOV
            windowHeight / windowWidth, //Aspect ratio
            0.1f,                       //z-Near, should never be <= 0
            10000.f                       //z-Far   
        );
        viewMatrix = lookAt(pos, cameraGaze, worldUp);
    }
    void setZoom(float newZoom) {
        distanceFromFocus = newZoom;
    }
};

class Skybox {
private:
    const char* sky_v;
    const char* sky_f;
    GLuint sky_vertexShader;
    GLuint sky_fragShader;
    GLuint sky_shaderProg;
    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    unsigned int skyboxTex;
    mat4 sky_view;

public:
    Skybox(string skyboxShaderV, string skyboxShaderF, string dayNight) {
        sky_shaderProg = glCreateProgram();
        fstream sky_vertSrc(skyboxShaderV);
        stringstream sky_vertBuff;
        sky_vertBuff << sky_vertSrc.rdbuf();
        string sky_vertS = sky_vertBuff.str();
        sky_v = sky_vertS.c_str();

        fstream sky_fragSrc(skyboxShaderF);
        stringstream sky_fragBuff;
        sky_fragBuff << sky_fragSrc.rdbuf();
        string sky_fragS = sky_fragBuff.str();
        sky_f = sky_fragS.c_str();

        GLuint sky_vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(sky_vertexShader, 1, &sky_v, NULL);
        glCompileShader(sky_vertexShader);

        GLuint sky_fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(sky_fragShader, 1, &sky_f, NULL);
        glCompileShader(sky_fragShader);

        glAttachShader(sky_shaderProg, sky_vertexShader);
        glAttachShader(sky_shaderProg, sky_fragShader);
        glLinkProgram(sky_shaderProg);

        float skyboxVertices[]{
            -1.f, -1.f, 1.f, //0
            1.f, -1.f, 1.f,  //1
            1.f, -1.f, -1.f, //2
            -1.f, -1.f, -1.f,//3
            -1.f, 1.f, 1.f,  //4
            1.f, 1.f, 1.f,   //5
            1.f, 1.f, -1.f,  //6
            -1.f, 1.f, -1.f  //7
        };

        //Skybox Indices
        unsigned int skyboxIndices[]{
            1,2,6,
            6,5,1,

            0,4,7,
            7,3,0,

            4,5,6,
            6,7,4,

            0,3,2,
            2,1,0,

            0,1,5,
            5,4,0,

            3,7,6,
            6,2,3
        };

        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);

        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

        glGenBuffers(1, &skyboxEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 36, &skyboxIndices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        string facesSkybox[6];
        if (dayNight == "day") {
            facesSkybox[0] = "Skybox/Morning/right.png";
            facesSkybox[1] = "Skybox/Morning/left.png";
            facesSkybox[2] = "Skybox/Morning/top.png";
            facesSkybox[3] = "Skybox/Morning/bottom.png";
            facesSkybox[4] = "Skybox/Morning/front.png";
            facesSkybox[5] = "Skybox/Morning/back.png";

        }

        else if (dayNight == "evening") {
            facesSkybox[0] = "Skybox/Night/right.png";
            facesSkybox[1] = "Skybox/Night/left.png";
            facesSkybox[2] = "Skybox/Night/top.png";
            facesSkybox[3] = "Skybox/Night/bottom.png";
            facesSkybox[4] = "Skybox/Night/front.png";
            facesSkybox[5] = "Skybox/Night/back.png";

        }

        glGenTextures(1, &skyboxTex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

        // Alleviates the quality of the texture 200px -> 2000px
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Alleviates the quality of the texture 2000px -> 200px
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        //UV == Tex
        //ST == Face of the 3D Model
        //RST== Cubemap

        //Makes sure the texture is stretched to the edge
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        for (unsigned int i = 0; i < 6; i++) {
            int w, h, skyCChannel;
            stbi_set_flip_vertically_on_load(false);
            unsigned char* data = stbi_load(
                facesSkybox[i].c_str(),
                &w,
                &h,
                &skyCChannel,
                0
            );
            if (data) {
                if (skyCChannel==3){
                    glTexImage2D(
                        GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                        0,
                        GL_RGB,
                        w,
                        h,
                        0,
                        GL_RGB,
                        GL_UNSIGNED_BYTE,
                        data
                    );
                }
                if (skyCChannel == 4) {
                    glTexImage2D(
                        GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                        0,
                        GL_RGBA,
                        w,
                        h,
                        0,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        data
                    );
                }
            }
            stbi_image_free(data);
        }
        stbi_set_flip_vertically_on_load(true);
    }
    ~Skybox() {
        glDeleteShader(sky_vertexShader);
        glDeleteShader(sky_fragShader);
        glDeleteVertexArrays(1, &skyboxVAO);
        glDeleteBuffers(1, &skyboxVBO);
        glDeleteBuffers(1, &skyboxEBO);
    }
    void draw(PerspectiveCamera perspectiveCamera) {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glUseProgram(sky_shaderProg);

        sky_view = mat4(1.f);
        sky_view = mat4( //Retain orientation
            mat3(perspectiveCamera.getViewMatrix()) //Removes the position components
        );

        unsigned int sky_projectionLoc = glGetUniformLocation(sky_shaderProg, "projection");
        glUniformMatrix4fv(sky_projectionLoc, 1, GL_FALSE, value_ptr(perspectiveCamera.getProjectionMatrix()));

        unsigned int sky_viewLoc = glGetUniformLocation(sky_shaderProg, "view");
        glUniformMatrix4fv(sky_viewLoc, 1, GL_FALSE, value_ptr(sky_view));

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
};

class TrafficLight:public Model3D{
private:
    bool startSequence, endSequence;
    double sequenceTime, startTime;
    float glow, rotateSPD;
    bool greenLight;
    double redTime, yellowTime, greenTime;
    PointLight* childPointLight;
    float half;
    float r;
    float g;
    float b;
public:
    TrafficLight(VAO* newModelVao, Texture* newTexture, Shader* newShader, PointLight* newChildPointLight) {
        modelVAO = newModelVao;
        texture = newTexture;
        modelShader = newShader;
        childPointLight = newChildPointLight;
        identity_matrix = mat4(1.0f);
        transparency = 1.0f;

        childPointLight->setRGB(vec3(10,10,10));
        childPointLight->setLumens(4000.f);
        redTime = 100.0;
        yellowTime = 150.0;
        greenTime = 250.0;
        sequenceTime = 0.0f;
        startTime = 0.0f;
        startSequence = false;
        endSequence = false;
        greenLight = false;
        glow = 0.0f;
        rotateSPD = 0.15f;

        half = 254/2;
        r = 0;
        g = 0;
        b = 0;
    }
    void setRedTime(double newRedTime) {
        redTime = newRedTime;
    }
    void setYellowTime(double newYellowTime) {
        yellowTime = newYellowTime;
    }
    void setGreenTime(double newGreenTime) {
        greenTime = newGreenTime;
    }
    void start() {
        startSequence = true;
        startTime = glfwGetTime();
    }
    void end() {
        endSequence = true;
        rotateSPD=0.01;
        childPointLight->setLumens(75000.0f *glow);
        childPointLight->setRGB(vec3(r, g, b));
    }
    bool getGreenLight() {
        return greenLight;
    }
    bool getStart() {
        return startSequence;
    }
    void update(double currentTime) {
        childPointLight->setPosX(pos.x);
        childPointLight->setPosY(pos.y);
        childPointLight->setPosZ(pos.z);

        glow = 1.25+cos(currentTime*rotateSPD);

        r = half + (half * sin(currentTime * 1.25));
        g = half + (half * cos(currentTime * 1.35));
        b = half + (half * sin(currentTime * 1.45));


        if (!greenLight) {
            size.x = (half + (b*0.25f)) * 0.35;
            size.y = (half + (g*0.085f)) * 0.35;
            size.z = (half + (r*0.25f)) * 0.35;
        }

        theta.x += rotateSPD *0.25f;
        theta.y += rotateSPD;
        theta.z += rotateSPD *0.005;

        if (startSequence) {
            sequenceTime = currentTime - startTime;
            if (sequenceTime <= redTime) {
                rotateSPD+=0.0001f;
                childPointLight->setLumens(40000.0f*glow);
                childPointLight->setRGB(vec3(255, 0, 0));
            }
            else if (sequenceTime<=redTime + yellowTime) {
                rotateSPD+=0.00075f;
                childPointLight->setLumens(65000.0f*glow);
                childPointLight->setRGB(vec3(255, 200, 0));
            }
            else if (sequenceTime <=redTime + yellowTime + greenTime) {
                rotateSPD=0;
                setSize(pos.y * 0.3f);
                theta = vec3(0, 0, 0);
                childPointLight->setLumens(75000.0f*glow);
                childPointLight->setRGB(vec3(0, 255, 0));
                greenLight = true;
            }
            else {
                end();
            }

        }
    }
};

class FinishLine : public Model3D {
private:
    int rank = 0;
    bool playerFinished = false;
public:
    FinishLine(VAO* newModelVao, Texture* newTexture, Shader* newShader) {
        modelVAO = newModelVao;
        texture = newTexture;
        modelShader = newShader;
        identity_matrix = mat4(1.0f);
        transparency = 1.0f;
        rank = 0;
        playerFinished = false;
    }
    bool CollisionCheck(Model3D* entity) {
        if ((entity->getPos().z + entity->getScale().z)>=pos.z) {
            Kart* kart=dynamic_cast<Kart*>(entity);
            if (kart->getActivation()) {
                PlayerKart* player = dynamic_cast<PlayerKart*>(entity);
                if (player == nullptr) {
                    kart->toggleActivation();
                    kart->setEndTime(glfwGetTime());
                    rank++;
                    cout << "RANK: " << rank << " ";
                    kart->printTime();
                }
                else if (!playerFinished){
                    playerFinished = true;
                    kart->setEndTime(glfwGetTime());
                    rank++;
                    cout << "RANK: " << rank << " ";
                    kart->printTime();
                }
            }
            return true;
        }
        return false;
    }
};

int main(void)
{
    GLFWwindow* window;
    if (!glfwInit()) return -1;

    bool countdown1, countdown2, countdown3, gameEnd;
    countdown1 = countdown2 = countdown3 = gameEnd = false;
    double startCountdownTime = 0.0;

    float windowWidth = 700.f;
    float windowHeight = 700.f;

    bool playerFinished = false;
    bool ghost1Finished = false;
    bool ghost2Finished = false;

    window = glfwCreateWindow(700, 700, "GDGRAP1-MP | Chen-Elomina | Karting | ESC to close program", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowPos(window, 960 - (windowWidth/2), 540 - (windowHeight / 2));

    gladLoadGL();

    /* Screen Space is usually:
        From 0 to screen width
        From 0 to screen height
    */
    //min x //min y, //max x // max y
    glViewport(0, 0, windowWidth, windowHeight);

    /* ====================================================== INITIALIZATION ====================================================== */
    glfwSetKeyCallback(window, getUserInput);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // CAMERA STUFF
    PerspectiveCamera perspectiveCam(windowWidth, windowHeight);

    // Create Shaders
    Shader* objectShader = new Shader("Shaders/objectShaderV.vert", "Shaders/objectShaderF.frag");
    Shader* solidColorShader = new Shader("Shaders/solidColorShaderV.vert", "Shaders/solidColorShaderF.frag");
    Shader* landmarkShader = new Shader("Shaders/NormalMap.vert", "Shaders/NormalMap.frag");

    // Sky Box
    Skybox night("Shaders/skybox.vert", "Shaders/skybox.frag", "evening");
    Skybox morning("Shaders/skybox.vert", "Shaders/skybox.frag", "day");

    // Create a VAOs
    VAO* planeVAO = new VAO("3D/plane.obj");
    VAO* spaceCarVAO = new VAO("3D/space_car_centered.obj");
    VAO* artifactVAO = new VAO("3D/artifact.obj");
    VAO* ballVAO = new VAO("3D/ball.obj");

    //Create Textures
    Texture* planeTex = new Texture("3D/mercury.jpg",2);
    Texture* spaceCarTex = new Texture("3D/spaceCarTexture.png",3);
    Texture* artifactTex = new Texture("3D/artifact.png",4);

    //Normal Map
    NormalMapTexture* earthTex = new NormalMapTexture("3D/earth_normal.png", 5, "3D/earth.png", 6);
    NormalMapTexture* meteoriteTex = new NormalMapTexture("3D/meteorite_normal.png", 7, "3D/meteorite.png", 8);

    // Create 3D Models.

    //Plane / Ground
    Model3D plane(planeVAO, planeTex, objectShader);
    plane.setTransparency(1.0);
    plane.setSize(750.0f);
    plane.setThetaZ(-90.0f);
    plane.setThetaX(90.f);
    plane.setPosY(-0.25f);

    // Finish Line
    FinishLine finishLine(planeVAO, planeTex, solidColorShader);
    finishLine.setPosY(1.85f);
    finishLine.setPosZ(400.0f);
    finishLine.setScaleX(40.0f);
    finishLine.setScaleY(0.1f);

    //landmarks
    NormalMapModel earth(ballVAO, earthTex, landmarkShader);
    NormalMapModel meteorite(ballVAO,meteoriteTex,landmarkShader);

    meteorite.setSize(1.f);
    meteorite.setPosZ(300.f);
    meteorite.setPosX(120.f);

    earth.setSize(0.5f);
    earth.setPosZ(75);
    earth.setPosX(-40.f);


    // Karts
    PlayerKart playerSpaceCar(spaceCarVAO, spaceCarTex, objectShader, "Player", 0.035f, 0.00005f);
    playerSpaceCar.setPosY(2.0f);
    playerSpaceCar.setPosZ(-3.0f);
    playerSpaceCar.setSize(0.25f);
    Kart ghost1(spaceCarVAO, spaceCarTex, objectShader, "Turtle", 0.04f, 0.00006f); // Faster
    ghost1.setTransparency(0.1f);
    ghost1.setSize(0.25f);
    ghost1.setPosY(2.0f);
    ghost1.setPosZ(-2.8f);
    ghost1.setPosX(2.25f);
    Kart ghost2(spaceCarVAO, spaceCarTex, objectShader, "Hare", 0.033f, 0.000045f); // Slower
    ghost2.setTransparency(0.1f);
    ghost2.setSize(0.25f);
    ghost2.setPosY(2.0f);
    ghost2.setPosZ(-2.0f);
    ghost2.setPosX(-2.25f);

    //LIGHT STUFF
    PointLight pointLight;
    PointLight landmarkLight;
    landmarkLight.setLumens(500.f);

    TrafficLight trafficLight(artifactVAO, artifactTex, objectShader, &pointLight);
    trafficLight.setTransparency(1.0f);
    trafficLight.setSize(30.f);
    trafficLight.setPosY(200.0f);
    trafficLight.setPosZ(400.0f);
    trafficLight.setRedTime(2.5);
    trafficLight.setYellowTime(4.0);
    trafficLight.setGreenTime(3.5);

    DirectionLight directionLight(vec3(4.0f, 5.0f, 3.0f));
    directionLight.setPosX(0.0f);
    directionLight.setPosY(-5.0f);
    directionLight.setPosZ(0.0f);

    //Set the Kart as the parent of Camera
    perspectiveCam.attachParent(&playerSpaceCar);

    /* =========================== GL DEPTH AND GL BLEND =========================== */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); //Enable Blend

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);


    while (!glfwWindowShouldClose(window))
    {
        /* =========================== UPDATES AND INPUTS =========================== */

        if (!trafficLight.getStart() && glfwGetTime() > 6.0) {
            trafficLight.start();
        }

        //Get User Input
        perspectiveCam.getInputs(window);
        playerSpaceCar.getUserInput(window);

        //Update
        if (stopCars == false) {
            ghost1.setAcceleration(0.00006f);
            ghost2.setAcceleration(0.00004f);
        }
        else {
            ghost1.setAcceleration(0.0f);
            ghost2.setAcceleration(0.0f);
            ghost1.setSpeed(0.0f);
            ghost2.setSpeed(0.0f);
        }

        ghost1.update();
        ghost2.update();

        playerSpaceCar.update();

        perspectiveCam.setZoom(perspectiveCameraZoom);
        perspectiveCam.update(windowWidth, windowHeight);

        trafficLight.update(glfwGetTime());
        if (trafficLight.getGreenLight()&&!raceStarted) {
            playerSpaceCar.toggleActivation();
            ghost1.toggleActivation();
            ghost2.toggleActivation();
            raceStarted = true;
            stopCars = false;
        }

        earth.update();
        meteorite.update();

        playerFinished = finishLine.CollisionCheck(&playerSpaceCar);
        ghost1Finished = finishLine.CollisionCheck(&ghost1);
        ghost2Finished = finishLine.CollisionCheck(&ghost2);

        /* =========================== RENDER =========================== */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //Draw the models
        if (day) {
            morning.draw(perspectiveCam);
            vec3 cottonCandyPink(242, 153, 205);
            directionLight.setRGB(cottonCandyPink);
            directionLight.setLumens(3.0f);
        }else{
            night.draw(perspectiveCam);
            vec3 mikuTeal(9, 179, 130);
            directionLight.setRGB(mikuTeal);
            directionLight.setLumens(1.25f);
        }

        //If all karts past finish line
        if (playerFinished && ghost1Finished && ghost2Finished) {
            
            if (!gameEnd) {
                cout << endl <<"Thank You For Playing!" << endl <<endl;
                cout << "Game Will Now Close in..." << endl;
                gameEnd = true;
                startCountdownTime = glfwGetTime();  // Start countdown
            }

            double currentTime = glfwGetTime();  // Get the current time
            double elapsedTime = currentTime - startCountdownTime;  // Time elapsed since game ended

            if (elapsedTime >= 1.0 && elapsedTime < 2.0) {
                if (!countdown1) {
                    cout << "3..." << endl;
                    countdown1 = true;
                }
            }
            else if (elapsedTime >= 2.0 && elapsedTime < 3.0) {
                if (!countdown2) {
                    cout << "2..." << endl;
                    countdown2 = true;
                }
            }
            else if (elapsedTime >= 3.0 && elapsedTime < 4.0) {
                if (!countdown3) {
                    cout << "1..." << endl;
                    countdown3 = true;
                }
            }
            else if (elapsedTime >= 4.0) {
                cout << "0" << endl;
                glfwSetWindowShouldClose(window, GL_TRUE);  // Close the window after the countdown
            }
        }

        plane.draw(perspectiveCam, pointLight, directionLight);
        finishLine.draw(perspectiveCam, pointLight, directionLight);
        
        trafficLight.draw(perspectiveCam, pointLight, directionLight);
        playerSpaceCar.draw(perspectiveCam, pointLight, directionLight);
        ghost1.draw(perspectiveCam, pointLight, directionLight);
        ghost2.draw(perspectiveCam, pointLight, directionLight);
        meteorite.draw(perspectiveCam, landmarkLight, directionLight);
        earth.draw(perspectiveCam, landmarkLight, directionLight);

        /*jupiter.draw(perspectiveCam, pointLight, directionLight);
        mars.draw(perspectiveCam, pointLight, directionLight);*/

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    /* =========================== CLEAN UP =========================== */
    //Delete Shaders
    delete objectShader;
    delete solidColorShader;
    delete landmarkShader;


    //Delete VAOs
    delete planeVAO;
    delete spaceCarVAO;
    delete artifactVAO;
    delete ballVAO;

    //Delete Textures
    delete artifactTex;
    delete spaceCarTex;
    delete planeTex;
    delete earthTex;
    delete meteoriteTex;

    glfwTerminate();
    return 0;
};