#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <glew.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include <vector>

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "SDL2_image.lib")
#pragma comment(lib, "opengl32.lib")

int t = 0;

float v[] = 
{
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

float randBetween(float a, float b) {
    float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float range = b - a;
    return a + random * range;
}

unsigned int indices[] = 
{  
        0, 1, 3,  
        1, 2, 3   
};

struct Obj
{
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotate;
    float angle;
    glm::vec3 color;
    glm::vec3 velocity;
};
std::vector<Obj> cubes;
Obj camera;

glm::vec2 mousePos;
bool enableMouseMove = false;

unsigned int vbo, vao, shaderProgram, texture1, texture2;

float deltaTime = 0.0f;


const char* vShader = R"( 
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
gl_Position = projection * view * model * vec4(aPos, 1.0);
TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}
)";

const char* fShader = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 color;

void main()
{
FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2) * vec4(color, 1.0);
}

)";

bool checkCollision3D(const Obj& obj1, const Obj& obj2) {
    if (obj1.position.x + obj1.scale.x <= obj2.position.x) return false;
    if (obj1.position.x >= obj2.position.x + obj2.scale.x) return false;
    if (obj1.position.y + obj1.scale.y <= obj2.position.y) return false;
    if (obj1.position.y >= obj2.position.y + obj2.scale.y) return false;
    if (obj1.position.z + obj1.scale.z <= obj2.position.z) return false;
    if (obj1.position.z >= obj2.position.z + obj2.scale.z) return false;

    return true;
}


void initShaderCube(void)
{
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShader, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader: " << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShader, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Fragment shader: " << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Shader program: " << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void initBufferCube(void)
{
    for (int i = -500; i < 500; i += 60)
    {       
        for (int k = -500; k < 500; k += 60)
        {
            Obj obj;
            obj.position = glm::vec3(i, -500.0f, k);
            obj.scale = glm::vec3(20.0f, 20.0f, 20.0f);
            obj.rotate = glm::vec3(randBetween(0.0f, 1.0f), randBetween(0.0f, 1.0f), randBetween(0.0f, 1.0f));
            obj.color = glm::vec3(randBetween(0.0f, 1.0f),
                randBetween(0.0f, 1.0f), randBetween(0.0f, 1.0f));
            obj.angle = 0.0f;
            obj.velocity = glm::vec3(0, 0, 0);

            cubes.push_back(obj);
        }
    }


    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initTextureCube(void)
{
    //texture 1
    SDL_Surface* surface = IMG_Load("cube1.png");
    if (!surface) {
        std::cerr << "Failed to load image: " << IMG_GetError() << std::endl;
        system("pause");
    }

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLint format = (surface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D,0,format,surface->w,surface->h,0,format,
        GL_UNSIGNED_BYTE,surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    SDL_FreeSurface(surface);
    //---

    //texture 2
    SDL_Surface* surface2 = IMG_Load("cube2.png");
    if (!surface2) {
        std::cerr << "Failed to load image: " << IMG_GetError() << std::endl;
        system("pause");
    }

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLint format2 = (surface2->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, surface2->w, surface2->h, 0, format, 
        GL_UNSIGNED_BYTE, surface2->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    SDL_FreeSurface(surface2);
    //---

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);
}

void drawBufferCube(SDL_Window* window)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glUseProgram(shaderProgram);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 5000.0f);
    view = glm::rotate(view, camera.rotate.x, glm::vec3(1, 0, 0));
    view = glm::rotate(view, camera.rotate.y, glm::vec3(0, 1, 0));
    view = glm::translate(view, glm::vec3(-camera.position.x, -camera.position.y, camera.position.z));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
   
    glBindVertexArray(vao);


    for (unsigned int i = 0; i < cubes.size(); ++i)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubes[i].position);
        model = glm::rotate(model, glm::radians(cubes[i].angle), cubes[i].rotate);
        model = glm::scale(model, cubes[i].scale);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &cubes[i].color[0]);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        if (cubes[i].position.y >= -500)
        {
            glm::vec3 lastPos = cubes[i].position;
            bool isCollide = false;

            cubes[i].position.y += cubes[i].velocity.y * deltaTime;
            cubes[i].velocity.y += -(9.8f) * deltaTime;

            for (unsigned int j = 0; j < cubes.size(); ++j)
            {
                if (i != j)
                {
                    if (checkCollision3D(cubes[i], cubes[j]))
                        isCollide = true;
                }
            }

            if (isCollide)
                cubes[i].position = lastPos;
                
        }

    }
}



void Start(void)
{
   camera.position = glm::vec3(-132.346f, -476.343f, -491.194f);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);

    initShaderCube();
    initBufferCube();
    initTextureCube();
}

void Update(SDL_Window* window)
{
    if (t % 10 == 0)
    {
        Obj obj;
        obj.position = glm::vec3(randBetween(-500.0f, 500.0f), randBetween(0.0f, 500.0f), 
            randBetween(-500.0f, 500.0f));
        obj.scale = glm::vec3(20.0f, 20.0f, 20.0f);
        obj.rotate = glm::vec3(randBetween(0.0f, 1.0f), randBetween(0.0f, 1.0f), randBetween(0.0f, 1.0f));
        obj.color = glm::vec3(randBetween(0.0f, 1.0f),
            randBetween(0.0f, 1.0f), randBetween(0.0f, 1.0f));
        obj.angle = 0.0f;
        obj.velocity = glm::vec3(0, 0, 0);

        cubes.push_back(obj);
    }

    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_W]) 
    {
        camera.position.z += 40.0f * deltaTime * cos(camera.rotate.x) * cos(camera.rotate.y);
        camera.position.y -= 40.0f * deltaTime * sin(camera.rotate.x);
        camera.position.x += 40.0f * deltaTime * cos(camera.rotate.x) * sin(camera.rotate.y);
    }
    if (state[SDL_SCANCODE_S])
    {
        camera.position.z -= 40.0f * deltaTime * cos(camera.rotate.x) * cos(camera.rotate.y);
        camera.position.y += 40.0f * deltaTime * sin(camera.rotate.x);
        camera.position.x -= 40.0f * deltaTime * cos(camera.rotate.x) * sin(camera.rotate.y);
    }
    
    if (state[SDL_SCANCODE_SPACE])
    {
        std::cout << camera.position.x << "\t" << camera.position.y << "\t" << camera.position.z << std::endl;
    }

    drawBufferCube(window);

    t += 1;
}

void End(void)
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);
}



int main()
{
    srand(static_cast<unsigned int>(time(NULL)));

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL_Init: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init: " << IMG_GetError() << std::endl;
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_Window* window = SDL_CreateWindow("LBM",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        700,
        500,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_BORDERLESS);

    if (!window) 
    {
        std::cerr << "SDL_CreateWindow: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        std::cerr << "SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "glewInit" << std::endl;
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_GL_SetSwapInterval(1);
    SDL_Event events;

    Start();

    Uint32 lastTime = SDL_GetTicks();
   
    bool stop = false;
    while (!stop)
    {
        while (SDL_PollEvent(&events))
        {
            if (events.type == SDL_QUIT)
            {
                stop = true;
            }

            if (events.type == SDL_KEYDOWN)
            {
                if (events.key.keysym.sym == SDLK_0)
                {
                    glEnable(GL_MULTISAMPLE);
                    std::cout << "enable" << std::endl;
                }
                if (events.key.keysym.sym == SDLK_1)
                {
                    glDisable(GL_MULTISAMPLE);
                    std::cout << "disable" << std::endl;
                }
            }

            if (events.type == SDL_MOUSEBUTTONDOWN)
            {
                enableMouseMove = true;
            }

            if (events.type == SDL_MOUSEBUTTONUP)
            {
                enableMouseMove = false;
            }

            if (events.type == SDL_MOUSEMOTION)
            {
                
                if (enableMouseMove)
                {
                    float x = static_cast<float>(events.motion.x);
                    float y = static_cast<float>(events.motion.y);

                    float diff_x = x - mousePos.x;
                    float diff_y = y - mousePos.y;
                    float angle = atan2(diff_y, diff_x);

                    float speed = 1.0f;
                    camera.rotate.y += deltaTime * speed * cos(angle);
                    camera.rotate.x += deltaTime * speed * sin(angle);

                    mousePos.x = x;
                    mousePos.y = y;
                }
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        Update(window);

        SDL_GL_SwapWindow(window);
    }

    End();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;

}
