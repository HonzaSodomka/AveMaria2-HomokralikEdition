#include <iostream>
#include <string>
#include <cmath>
#include <random>
#include <algorithm>
// OpenGL includes
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GLFW/glfw3.h>
// GLM includes
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "app.hpp"

App::App() :
    lastX(400.0),
    lastY(300.0),
    firstMouse(true),
    fov(DEFAULT_FOV)
{
    // Nastavení poèáteèní pozice kamery
    camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));

    // Inicializace smìrového svìtla
    dirLight.direction = glm::vec3(0.0f, -1.0f, -1.0f); // Smìr - dolù a dopøedu
    dirLight.ambient = glm::vec3(0.2f, 0.2f, 0.2f);    // Ambient složka
    dirLight.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);    // Diffuse složka
    dirLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);   // Specular složka
}

App::~App() {
    // Úklid
    shader.clear();
    lightingShader.clear();

    if (triangle) {
        delete triangle;
        triangle = nullptr;
    }

    // Uvolnìní modelu slunce
    if (sunModel) {
        delete sunModel;
        sunModel = nullptr;
    }

    // Uvolnìní modelù bludištì
    for (auto& wall : maze_walls) {
        delete wall;
    }
    maze_walls.clear();

    // Uvolnìní transparentních králíkù
    for (auto& bunny : transparent_bunnies) {
        delete bunny;
    }
    transparent_bunnies.clear();
}

bool App::init(GLFWwindow* win) {
    window = win;

    // Ujistíme se, že FOV má správnou hodnotu
    fov = DEFAULT_FOV;

    if (!GLEW_ARB_direct_state_access) {
        std::cerr << "No DSA :-(" << std::endl;
        return false;
    }

    // Nastavení ukazatele na tøídu App pro callbacky
    glfwSetWindowUserPointer(window, this);

    // Nastavení callbackù
    glfwSetFramebufferSizeCallback(window, fbsize_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Zakázání zobrazení kurzoru a jeho omezení na okno
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Povolení Z-bufferu pro správné vykreslování 3D modelù
    glEnable(GL_DEPTH_TEST);

    // Nastavení pro prùhlednost
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Získání velikosti framebufferu
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Naètení assets
    init_assets();

    // První aktualizace projekèní matice
    update_projection_matrix();

    // Inicializace osvìtlení
    initLighting();

    // Explicitní nastavení view matice
    if (shader.getID() != 0) {
        shader.activate();
        shader.setUniform("uV_m", camera.GetViewMatrix());
    }

    if (lightingShader.getID() != 0) {
        lightingShader.activate();
        lightingShader.setUniform("uV_m", camera.GetViewMatrix());
        lightingShader.setUniform("viewPos", camera.Position);
    }

    return true;
}

void App::init_assets() {
    // Naètení shaderù
    try {
        std::cout << "Loading shaders..." << std::endl;
        shader = ShaderProgram("resources/shaders/tex.vert", "resources/shaders/tex.frag");

        // Naètení shaderù pro osvìtlení
        lightingShader = ShaderProgram("resources/shaders/directional.vert", "resources/shaders/directional.frag");

        std::cout << "Shaders loaded successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Shader loading error: " << e.what() << std::endl;
        throw;
    }

    // Vytvoøení bludištì
    try {
        std::cout << "Creating maze..." << std::endl;
        createMazeModel();
        std::cout << "Maze created successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Maze creation error: " << e.what() << std::endl;
        throw;
    }

    // Vytvoøení transparentních králíkù
    try {
        std::cout << "Creating transparent bunnies..." << std::endl;
        createTransparentBunnies();
        std::cout << "Transparent bunnies created successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Transparent bunnies creation error: " << e.what() << std::endl;
        throw;
    }

    // Vytvoøení modelu slunce
    try {
        std::cout << "Creating sun model..." << std::endl;
        createSunModel();
        std::cout << "Sun model created successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Sun model creation error: " << e.what() << std::endl;
        throw;
    }
}

// Nová metoda pro inicializaci osvìtlení
void App::initLighting() {
    // Nastavení light uniforms pro osvìtlení
    lightingShader.activate();

    // Nastavení smìrového svìtla
    lightingShader.setUniform("lightDir", dirLight.direction);
    lightingShader.setUniform("lightAmbient", dirLight.ambient);
    lightingShader.setUniform("lightDiffuse", dirLight.diffuse);
    lightingShader.setUniform("lightSpecular", dirLight.specular);

    // Výchozí materiál
    lightingShader.setUniform("ambientMaterial", glm::vec3(0.2f, 0.2f, 0.2f));
    lightingShader.setUniform("diffuseMaterial", glm::vec3(1.0f, 1.0f, 1.0f));
    lightingShader.setUniform("specularMaterial", glm::vec3(1.0f, 1.0f, 1.0f));
    lightingShader.setUniform("shininess", 32.0f);

    // Deaktivace shaderu
    lightingShader.deactivate();
}


void App::setupLightingUniforms() {
    lightingShader.activate();

    // Nastavení smìrového svìtla
    lightingShader.setUniform("lightDir", dirLight.direction);
    lightingShader.setUniform("lightAmbient", dirLight.ambient);
    lightingShader.setUniform("lightDiffuse", dirLight.diffuse);
    lightingShader.setUniform("lightSpecular", dirLight.specular);

    // Nastavení pozice kamery pro spekulární výpoèty (v world space)
    lightingShader.setUniform("viewPos", camera.Position);

    lightingShader.deactivate();
}

void App::createTransparentBunnies() {
    // Naètení textury pro králíky
    GLuint bunnyTexture = textureInit("resources/textures/kralik.jpg");

    // Pozice pro králíky v bludišti
    std::vector<glm::vec3> bunny_positions = {
        glm::vec3(0.0f, 5.0f, 3.0f),   // První králík
        glm::vec3(20.0f, 5.0f, 7.0f),   // Druhý králík
        glm::vec3(40.0f, 5.0f, 5.0f)   // Tøetí králík
    };

    // Barvy pro králíky (RGBA, kde A je prùhlednost) - NIŽŠÍ HODNOTY ALPHA
    std::vector<glm::vec4> bunny_colors = {
        glm::vec4(1.0f, 0.3f, 0.3f, 0.8f),   // Èervený více prùhledný 
        glm::vec4(0.3f, 1.0f, 0.3f, 0.6f),   // Zelený ještì více prùhledný
        glm::vec4(0.3f, 0.3f, 1.0f, 0.4f)    // Modrý nejvíce prùhledný
    };

    // Vytvoøení tøí transparentních králíkù
    for (int i = 0; i < 3; i++) {
        // Vytvoøení modelu králíka - nyní použijeme lightingShader
        Model* bunny = new Model("resources/models/bunny_tri_vnt.obj", lightingShader);

        // Nastavení textury a barvy s prùhledností
        bunny->meshes[0].texture_id = bunnyTexture;
        bunny->meshes[0].diffuse_material = bunny_colors[i];

        // Nastavení pozice a velikosti
        bunny->origin = bunny_positions[i];
        bunny->scale = glm::vec3(0.5f, 0.5f, 0.5f); // Zmenšíme králíky

        // Oznaèení jako transparentní objekt
        bunny->transparent = true;

        // Pøidání do vektoru transparentních králíkù
        transparent_bunnies.push_back(bunny);
    }
}

// Metoda pro vytvoøení modelu slunce
void App::createSunModel() {
    // Vytvoøení modelu slunce (koule) - použijeme pùvodní shader bez osvìtlení, aby slunce vždy svítilo
    sunModel = new Model("resources/models/sphere.obj", shader);

    // Vytvoøení textury slunce (žlutá koule)
    cv::Mat sunTexture(64, 64, CV_8UC3, cv::Scalar(255, 255, 0)); // Žlutá barva

    // Uložení textury
    cv::imwrite("resources/textures/sun.png", sunTexture);

    // Naètení textury pro slunce
    GLuint sunTextureID = textureInit("resources/textures/sun.png");

    // Nastavení textury a materiálu
    sunModel->meshes[0].texture_id = sunTextureID;
    sunModel->meshes[0].diffuse_material = glm::vec4(1.0f, 1.0f, 0, 1.0f); // Žlutá barva

    // Nastavení velikosti
    sunModel->scale = glm::vec3(2.0f, 2.0f, 2.0f);

    // Výchozí pozice slunce (bude aktualizována v updateLighting)
    sunModel->origin = glm::vec3(0.0f, 30.0f, 0.0f);
}

void App::updateLighting(float deltaTime) {
    // Pomalá rotace smìru svìtla pro simulaci pohybu slunce
    static float angle = 0.0f;
    angle += 0.1f * deltaTime; // Rychlost rotace

    // Aktualizace smìru svìtla (v world space)
    dirLight.direction.x = sin(angle);
    dirLight.direction.y = -1.0f;
    dirLight.direction.z = cos(angle);

    // Normalizace smìru
    dirLight.direction = glm::normalize(dirLight.direction);

    // Pevný støed bludištì - nezávislý na kameøe
    const glm::vec3 mazeCenter(7.5f, 0.0f, 7.5f); // Pro bludištì 15x15

    // Aktualizace pozice modelu slunce (fixní trajektorie kolem støedu bludištì)
    const float sunDistance = 50.0f;
    const float sunHeight = 20.0f;

    // Umístìní slunce kolem støedu bludištì
    sunModel->origin.x = mazeCenter.x - dirLight.direction.x * sunDistance;
    sunModel->origin.y = sunHeight;
    sunModel->origin.z = mazeCenter.z - dirLight.direction.z * sunDistance;

    // Nastavení uniforms pro osvìtlení
    setupLightingUniforms();
}

GLuint App::textureInit(const std::filesystem::path& filepath) {
    std::cout << "Naèítám texturu: " << filepath << std::endl;  // Debug výpis
    // Použij std::filesystem::path správnì
    std::string pathString = filepath.string();
    cv::Mat image = cv::imread(pathString, cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        throw std::runtime_error("Nelze naèíst texturu ze souboru: " + pathString);
    }
    return gen_tex(image);
}

GLuint App::gen_tex(cv::Mat& image) {
    GLuint ID = 0;

    // Generování ID OpenGL textury
    glCreateTextures(GL_TEXTURE_2D, 1, &ID);

    // Nastavení podle poètu kanálù v obrázku
    switch (image.channels()) {
    case 3:
        // Vytvoøení a inicializace datového prostoru pro texturu - immutable formát
        glTextureStorage2D(ID, 1, GL_RGB8, image.cols, image.rows);
        // Nahrání dat do textury
        glTextureSubImage2D(ID, 0, 0, 0, image.cols, image.rows, GL_BGR, GL_UNSIGNED_BYTE, image.data);
        break;
    case 4:
        glTextureStorage2D(ID, 1, GL_RGBA8, image.cols, image.rows);
        glTextureSubImage2D(ID, 0, 0, 0, image.cols, image.rows, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
        break;
    default:
        throw std::runtime_error("Nepodporovaný poèet kanálù v textuøe: " + std::to_string(image.channels()));
    }

    // Nastavení filtrování textury
    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // bilineární zvìtšování
    glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // trilineární zmenšování
    glGenerateTextureMipmap(ID);  // Generování mipmap

    // Nastavení opakování textury
    glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return ID;
}

uchar App::getmap(cv::Mat& map, int x, int y) {
    x = std::clamp(x, 0, map.cols - 1);
    y = std::clamp(y, 0, map.rows - 1);

    // at(row,col)!!!
    return map.at<uchar>(y, x);
}

void App::genLabyrinth(cv::Mat& map) {
    cv::Point2i start_position, end_position;

    // C++ random numbers
    std::random_device r; // Seed with a real random value, if available
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_height(1, map.rows - 2); // uniform distribution between int..int
    std::uniform_int_distribution<int> uniform_width(1, map.cols - 2);
    std::uniform_int_distribution<int> uniform_block(0, 15); // how often are walls generated: 0=wall, anything else=empty

    // inner maze 
    for (int j = 0; j < map.rows; j++) {
        for (int i = 0; i < map.cols; i++) {
            switch (uniform_block(e1))
            {
            case 0:
                map.at<uchar>(cv::Point(i, j)) = '#';
                break;
            default:
                map.at<uchar>(cv::Point(i, j)) = '.';
                break;
            }
        }
    }

    // walls
    for (int i = 0; i < map.cols; i++) {
        map.at<uchar>(cv::Point(i, 0)) = '#';
        map.at<uchar>(cv::Point(i, map.rows - 1)) = '#';
    }
    for (int j = 0; j < map.rows; j++) {
        map.at<uchar>(cv::Point(0, j)) = '#';
        map.at<uchar>(cv::Point(map.cols - 1, j)) = '#';
    }

    // gen start_position inside maze (excluding walls)
    do {
        start_position.x = uniform_width(e1);
        start_position.y = uniform_height(e1);
    } while (getmap(map, start_position.x, start_position.y) == '#'); // check wall

    // gen end different from start, inside maze (excluding outer walls) 
    do {
        end_position.x = uniform_width(e1);
        end_position.y = uniform_height(e1);
    } while (start_position == end_position || getmap(map, end_position.x, end_position.y) == '#'); // check overlap and wall
    map.at<uchar>(cv::Point(end_position.x, end_position.y)) = 'e';

    std::cout << "Start: " << start_position << std::endl;
    std::cout << "End: " << end_position << std::endl;

    // print map
    for (int j = 0; j < map.rows; j++) {
        for (int i = 0; i < map.cols; i++) {
            if ((i == start_position.x) && (j == start_position.y))
                std::cout << 'X';
            else
                std::cout << getmap(map, i, j);
        }
        std::cout << std::endl;
    }

    // set player position in 3D space (transform X-Y in map to XYZ in GL)
    camera.Position.x = (start_position.x) + 0.5f;
    camera.Position.z = (start_position.y) + 0.5f;
    camera.Position.y = 0.5f; // Výška oèí
}

void App::createMazeModel() {
    // Naètení atlasu textur
    cv::Mat atlas = cv::imread("resources/textures/tex_256.png", cv::IMREAD_UNCHANGED);
    if (atlas.empty()) {
        throw std::runtime_error("Nelze naèíst atlas textur!");
    }

    // Atlas je 16x16 textur
    const int texCount = 16; // Poèet textur v každém smìru
    const int tileSize = atlas.cols / texCount; // Velikost jedné textury v pixelech

    // Uložení textur
    auto saveTextureFromAtlas = [&](int row, int col, const std::string& filename) {
        int x = col * tileSize;
        int y = row * tileSize;
        cv::Mat tileMat = atlas(cv::Rect(x, y, tileSize, tileSize));
        cv::imwrite("resources/textures/" + filename, tileMat);
        };

    // Uložení textur
    saveTextureFromAtlas(1, 1, "floor.png"); // Podlaha
    saveTextureFromAtlas(3, 2, "wall.png");  // Zdi

    // Naètení textur
    wall_textures.clear();
    GLuint floorTexture = textureInit("resources/textures/floor.png");
    GLuint wallTexture = textureInit("resources/textures/wall.png");
    wall_textures.push_back(floorTexture);
    wall_textures.push_back(wallTexture);

    // Vytvoøení bludištì
    maze_map = cv::Mat(15, 15, CV_8U);
    genLabyrinth(maze_map);

    // **Vytvoøení podlahy (15x15)**
    for (int j = 0; j < 15; j++) {
        for (int i = 0; i < 15; i++) {
            // Použití lightingShader místo pùvodního shader
            Model* floor = new Model("resources/models/cube.obj", lightingShader);
            floor->meshes[0].texture_id = floorTexture;
            floor->origin = glm::vec3(i, 0.0f, j); // Spodní vrstva
            floor->scale = glm::vec3(1.0f, 1.0f, 1.0f);
            maze_walls.push_back(floor);
        }
    }

    // **Vytvoøení zdí (z `maze_map`)**
    for (int j = 0; j < 15; j++) {
        for (int i = 0; i < 15; i++) {
            if (getmap(maze_map, i, j) == '#') { // Pokud je tam zeï
                // Použití lightingShader místo pùvodního shader
                Model* wall = new Model("resources/models/cube.obj", lightingShader);
                wall->meshes[0].texture_id = wallTexture;
                wall->origin = glm::vec3(i, 1.0f, j); // Umístìní nad podlahu
                wall->scale = glm::vec3(1.0f, 1.0f, 1.0f);
                maze_walls.push_back(wall);
            }
        }
    }
}

bool App::run() {
    if (!window) {
        std::cerr << "No active GLFW window!" << std::endl;
        return false;
    }

    // Ujistíme se, že FOV má správnou hodnotu
    if (fov <= 0.0f) {
        fov = DEFAULT_FOV;
    }

    // Aktivace shader programu
    lightingShader.activate();

    // Inicializace projekèní a pohledové matice
    update_projection_matrix();
    lightingShader.setUniform("uV_m", camera.GetViewMatrix());
    lightingShader.setUniform("viewPos", camera.Position);

    // Promìnné pro mìøení FPS a deltaTime
    double lastTime = glfwGetTime();
    double lastFrameTime = lastTime;
    int frameCount = 0;
    std::string title = "OpenGL Maze Demo";

    // Hlavní smyèka
    while (!glfwWindowShouldClose(window)) {
        // Výpoèet deltaTime
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;

        // Mìøení FPS
        frameCount++;
        if (currentTime - lastTime >= 1.0) {
            std::string fpsTitle = title + " | FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(window, fpsTitle.c_str());
            frameCount = 0;
            lastTime = currentTime;
        }

        // Zpracování vstupu z klávesnice pro pohyb kamery
        glm::vec3 direction = camera.ProcessKeyboard(window, deltaTime);
        camera.Move(direction);

        // Aktualizace pohledové matice a pozice kamery
        lightingShader.activate();
        lightingShader.setUniform("uV_m", camera.GetViewMatrix());
        lightingShader.setUniform("viewPos", camera.Position);

        // Aktualizace osvìtlení
        updateLighting(deltaTime);

        // Vyèištìní obrazovky
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Implementace Painter's algoritmu pro transparentní objekty
        std::vector<Model*> transparent_objects;

        // 1. NEJPRVE VYKRESLÍME VŠECHNY NEPRÙHLEDNÉ OBJEKTY
        // Vykreslení bludištì (neprùhledné objekty)
        for (auto& wall : maze_walls) {
            // Nastavení model matice v shaderu
            lightingShader.setUniform("uM_m", wall->getModelMatrix());
            lightingShader.setUniform("transparent", false);
            // Vykreslení modelu
            wall->draw();
        }

        // 2. PØIPRAVÍME SI SEZNAM TRANSPARENTNÍCH OBJEKTÙ
        // Pøidání transparentních králíkù do seznamu
        for (auto& bunny : transparent_bunnies) {
            transparent_objects.push_back(bunny);
        }

        // 3. SEØADÍME TRANSPARENTNÍ OBJEKTY OD NEJVZDÁLENÌJŠÍHO K NEJBLIŽŠÍMU
        std::sort(transparent_objects.begin(), transparent_objects.end(),
            [this](Model* a, Model* b) {
                // Získání pozice objektù
                glm::vec3 pos_a = a->origin;
                glm::vec3 pos_b = b->origin;

                // Výpoèet vzdálenosti od kamery
                float dist_a = glm::distance(camera.Position, pos_a);
                float dist_b = glm::distance(camera.Position, pos_b);

                // Øazení od nejvzdálenìjšího k nejbližšímu (vìtší > menší)
                return dist_a > dist_b;
            });

        // 4. NASTAVENÍ OPENGL PRO TRANSPARENTNÍ OBJEKTY
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Zakázat zápis do depth bufferu

        // 5. VYKRESLENÍ TRANSPARENTNÍCH OBJEKTÙ
        for (auto* model : transparent_objects) {
            // Nastavení model matice v shaderu
            lightingShader.setUniform("uM_m", model->getModelMatrix());
            // Nastavení diffuse materiálu (vèetnì alpha)
            lightingShader.setUniform("u_diffuse_color", model->meshes[0].diffuse_material);
            lightingShader.setUniform("transparent", true);
            // Vykreslení modelu
            model->draw();
        }

        // 6. OBNOVENÍ PÙVODNÍHO STAVU OPENGL
        glDepthMask(GL_TRUE);  // Povolit zápis do depth bufferu
        glDisable(GL_BLEND);   // Vypnout blending

        // 7. VYKRESLENÍ SLUNCE (používáme pùvodní shader, aby bylo jasnì viditelné)
        if (sunModel) {
            shader.activate();
            shader.setUniform("uP_m", projection_matrix);
            shader.setUniform("uV_m", camera.GetViewMatrix());
            shader.setUniform("uM_m", sunModel->getModelMatrix());
            shader.setUniform("u_diffuse_color", glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Jasnì žlutá barva
            sunModel->draw();
        }

        // Výmìna bufferù a zpracování událostí
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return true;
}

void App::update_projection_matrix() {
    if (height < 1)
        height = 1;   // Prevence dìlení nulou

    // Kontrola, zda fov má platnou hodnotu
    if (fov <= 0.0f) {
        fov = DEFAULT_FOV;
    }

    float ratio = static_cast<float>(width) / height;

    projection_matrix = glm::perspective(
        glm::radians(fov),   // Vertikální zorné pole ve stupních, pøevedeno na radiány
        ratio,               // Pomìr stran okna
        0.1f,                // Near clipping plane
        20000.0f             // Far clipping plane
    );

    // Nastavení uniform v shaderech
    if (shader.getID() != 0) {
        shader.activate();
        shader.setUniform("uP_m", projection_matrix);
    }

    if (lightingShader.getID() != 0) {
        lightingShader.activate();
        lightingShader.setUniform("uP_m", projection_matrix);
    }
}

void App::fbsize_callback(GLFWwindow* window, int width, int height) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->width = width;
    app->height = height;

    // Nastavení viewportu
    glViewport(0, 0, width, height);

    // Aktualizace projekèní matice
    app->update_projection_matrix();
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->fov += -5.0f * static_cast<float>(yoffset);  // Zoom in/out
    app->fov = glm::clamp(app->fov, 20.0f, 170.0f);  // Omezení FOV na rozumné hodnoty
    app->update_projection_matrix();
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
    }
}

void App::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (app->firstMouse) {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }

    double xoffset = xpos - app->lastX;
    double yoffset = app->lastY - ypos; // Pøevráceno, protože y-souøadnice jdou od shora dolù

    app->lastX = xpos;
    app->lastY = ypos;

    app->camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        // Reset FOV na výchozí hodnotu
        app->fov = app->DEFAULT_FOV;
        app->update_projection_matrix();
        std::cout << "Zoom reset to default" << std::endl;
    }
}