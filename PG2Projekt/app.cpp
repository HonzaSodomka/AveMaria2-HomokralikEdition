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
    fov(DEFAULT_FOV),
    fountain(nullptr),
    particleModel(nullptr)
{
    // Pozice světla uprostřed mapy (15x15 / 2)
    pointLightPosition = glm::vec3(7.5f, 2.0f, 7.5f);
    // Nastavení poèáteèní pozice kamery
    camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
}

App::~App() {
    // Úklid
    shader.clear();
    if (triangle) {
        delete triangle;
        triangle = nullptr;
    }

    // Uvolnění modelu slunce
    if (sunModel) {
        delete sunModel;
        sunModel = nullptr;
    }

    // Uvolnění modelů barevných světel
    if (redLightModel) {
        delete redLightModel;
        redLightModel = nullptr;
    }

    if (blueLightModel) {
        delete blueLightModel;
        blueLightModel = nullptr;
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
    glDeleteVertexArrays(1, &lampVAO);

    // Uvolnění systému částic fontány
    if (fountain) {
        delete fountain;
        fountain = nullptr;
    }

    // Uvolnění modelu částic
    if (particleModel) {
        delete particleModel;
        particleModel = nullptr;
    }
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

    // Explicitní nastavení view matice
    if (shader.getID() != 0) {
        shader.activate();
        shader.setUniform("uV_m", camera.GetViewMatrix());
    }

    // Inicializace světla (malá kostka)
    float lampVertices[] = {
        // pozice (malá kostka)
        -0.1f, -0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f,  0.1f, -0.1f,
         0.1f,  0.1f, -0.1f,
        -0.1f,  0.1f, -0.1f,
        -0.1f, -0.1f, -0.1f,

        -0.1f, -0.1f,  0.1f,
         0.1f, -0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,
        -0.1f, -0.1f,  0.1f,

        -0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f, -0.1f,
        -0.1f, -0.1f, -0.1f,
        -0.1f, -0.1f, -0.1f,
        -0.1f, -0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,

         0.1f,  0.1f,  0.1f,
         0.1f,  0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f, -0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,

        -0.1f, -0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f, -0.1f,  0.1f,
         0.1f, -0.1f,  0.1f,
        -0.1f, -0.1f,  0.1f,
        -0.1f, -0.1f, -0.1f,

        -0.1f,  0.1f, -0.1f,
         0.1f,  0.1f, -0.1f,
         0.1f,  0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f, -0.1f
    };

    // Vytvoření VAO a VBO pro lampu
    GLuint lampVBO;
    glGenVertexArrays(1, &lampVAO);
    glGenBuffers(1, &lampVBO);

    glBindVertexArray(lampVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW);

    // Nastavení atributů
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void App::init_assets() {
    // Naètení shaderù
    try {
        std::cout << "Loading shaders..." << std::endl;
        shader = ShaderProgram("resources/shaders/tex.vert", "resources/shaders/tex.frag");
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

    // Nastavení pozice slunce/světla - jako poslední krok
    try {
        std::cout << "Setting up sun light..." << std::endl;
        createSunModel();
        std::cout << "Sun light set up successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Sun light setup error: " << e.what() << std::endl;
        throw;
    }

    // Přidání barevných světel
    try {
        std::cout << "Creating colored lights..." << std::endl;
        createColoredLights();
        std::cout << "Colored lights created successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Colored lights creation error: " << e.what() << std::endl;
        throw;
    }

    // Vytvoření fontány
    try {
        std::cout << "Creating fountain..." << std::endl;
        createFountain();
        std::cout << "Fountain created successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Fountain creation error: " << e.what() << std::endl;
        throw;
    }
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
        // Vytvoøení modelu králíka
        Model* bunny = new Model("resources/models/bunny_tri_vnt.obj", shader);

        // Nastavení textury a barvy s prùhledností
        bunny->meshes[0].texture_id = bunnyTexture;
        bunny->meshes[0].diffuse_material = bunny_colors[i];

        // Nastavení pozice a velikosti
        bunny->origin = bunny_positions[i];
        bunny->scale = glm::vec3(0.5f, 0.5f, 0.5f);

        // Oznaèení jako transparentní objekt
        bunny->transparent = true;

        // Pøidání do vektoru transparentních králíkù
        transparent_bunnies.push_back(bunny);
    }
}

GLuint App::textureInit(const std::filesystem::path& filepath) {
    std::cout << "Naèítám texturu: " << filepath << std::endl;
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
    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateTextureMipmap(ID);

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
    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_height(1, map.rows - 2);
    std::uniform_int_distribution<int> uniform_width(1, map.cols - 2);
    std::uniform_int_distribution<int> uniform_block(0, 15);

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
    } while (getmap(map, start_position.x, start_position.y) == '#');

    // gen end different from start, inside maze (excluding outer walls) 
    do {
        end_position.x = uniform_width(e1);
        end_position.y = uniform_height(e1);
    } while (start_position == end_position || getmap(map, end_position.x, end_position.y) == '#');
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
    camera.Position.x = (start_position.x) + 1.0f;
    camera.Position.z = (start_position.y) + 1.0f;
    camera.Position.y = 2.5f;
}

void App::createMazeModel() {
    // Naètení atlasu textur
    cv::Mat atlas = cv::imread("resources/textures/tex_256.png", cv::IMREAD_UNCHANGED);
    if (atlas.empty()) {
        throw std::runtime_error("Nelze naèíst atlas textur!");
    }

    // Atlas je 16x16 textur
    const int texCount = 16;
    const int tileSize = atlas.cols / texCount;

    // Uložení textur
    auto saveTextureFromAtlas = [&](int row, int col, const std::string& filename) {
        int x = col * tileSize;
        int y = row * tileSize;
        cv::Mat tileMat = atlas(cv::Rect(x, y, tileSize, tileSize));
        cv::imwrite("resources/textures/" + filename, tileMat);
        };

    // Uložení textur
    saveTextureFromAtlas(1, 1, "floor.png");
    saveTextureFromAtlas(3, 2, "wall.png");

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
            Model* floor = new Model("resources/models/cube.obj", shader);
            floor->meshes[0].texture_id = floorTexture;
            floor->origin = glm::vec3(i, 0.0f, j);
            floor->scale = glm::vec3(1.0f, 1.0f, 1.0f);
            maze_walls.push_back(floor);
        }
    }

    // **Vytvoøení zdí (z `maze_map`)**
    for (int j = 0; j < 15; j++) {
        for (int i = 0; i < 15; i++) {
            if (getmap(maze_map, i, j) == '#') {
                Model* wall = new Model("resources/models/cube.obj", shader);
                wall->meshes[0].texture_id = wallTexture;
                wall->origin = glm::vec3(i, 1.0f, j);
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
    shader.activate();

    // Vypíšeme pozici světla pro debugging
    std::cout << "Light position: "
        << pointLightPosition[0] << ", "
        << pointLightPosition[1] << ", "
        << pointLightPosition[2] << std::endl;

    // Nastavení světel
    // Původní světlo (nyní jako index 0)
    PointLight mainLight(
        pointLightPosition,           // stejná pozice jako dřív
        glm::vec3(1.0f, 1.0f, 0.0f),  // žlutá barva
        1.0f, 0.09f, 0.032f           // parametry útlumu
    );
    // Hlavní světlo (žluté)
    mainLight.SetPosition(pointLightPosition); // aktualizace pozice hlavního světla
    mainLight.SetUniforms(shader, 0);

    // Červené a modré světlo (statické, neměnné)
    redLightModel->origin = redLight.GetPosition();
    blueLightModel->origin = blueLight.GetPosition();

    // Červené světlo (nyní jako index 1)
    redLight.SetUniforms(shader, 1);

    // Modré světlo (nyní jako index 2)
    blueLight.SetUniforms(shader, 2);

    // Nastavení pozice kamery pro výpočet spekulárních odlesků
    shader.setUniform("viewPos", camera.Position);

    // Inicializace projekèní a pohledové matice
    update_projection_matrix();
    shader.setUniform("uV_m", camera.GetViewMatrix());

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

        shader.activate();

        // Aktualizace světel v každém snímku
        // Hlavní světlo (žluté)
        mainLight.SetPosition(pointLightPosition); // aktualizace pozice hlavního světla
        mainLight.SetUniforms(shader, 0);

        // Červené a modré světlo (statické, neměnné)
        redLight.SetUniforms(shader, 1);
        blueLight.SetUniforms(shader, 2);

        // Aktualizace pozice kamery pro výpočet odlesků
        shader.setUniform("viewPos", camera.Position);

        // Získání aktuálních pozic světel
        glm::vec3 currentRedLightPos = redLight.GetPosition();
        glm::vec3 currentBlueLightPos = blueLight.GetPosition();

        // Explicitní aktualizace pozic modelů světel
        redLightModel->origin = currentRedLightPos;
        blueLightModel->origin = currentBlueLightPos;

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

        // Vypočtení nové pozice
        glm::vec3 newPosition = camera.Position + direction;

        // Kontrola kolize před provedením pohybu
        if (!checkCollision(newPosition)) {
            camera.Move(direction);
        }
        else {
            // Zkusíme pohyb po jednotlivých osách
            glm::vec3 xMove = camera.Position + glm::vec3(direction.x, 0.0f, 0.0f);
            glm::vec3 yMove = camera.Position + glm::vec3(0.0f, direction.y, 0.0f);
            glm::vec3 zMove = camera.Position + glm::vec3(0.0f, 0.0f, direction.z);

            // Pokud je pohyb platný alespoň v jednom směru
            if (!checkCollision(xMove)) {
                camera.Move(glm::vec3(direction.x, 0.0f, 0.0f));
            }
            if (!checkCollision(yMove)) {
                camera.Move(glm::vec3(0.0f, direction.y, 0.0f));
            }
            if (!checkCollision(zMove)) {
                camera.Move(glm::vec3(0.0f, 0.0f, direction.z));
            }
        }

        // Aktualizace pohledové matice
        shader.setUniform("uV_m", camera.GetViewMatrix());

        // Aktualizace systému částic fontány
        if (fountain) {
            fountain->Update(deltaTime);
        }

        // Vyèištìní obrazovky
        glClearColor(0.2f, 0.2f, 0.3f, 1.0f); // Tmavě modrá obloha
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Implementace Painter's algoritmu pro transparentní objekty
        std::vector<Model*> transparent_objects;

        // 1. NEJPRVE VYKRESLÍME VŠECHNY NEPRÙHLEDNÉ OBJEKTY

        // Vykreslení bludištì (neprùhledné objekty)
        for (auto& wall : maze_walls) {
            // Nastavení model matice v shaderu
            shader.setUniform("uM_m", wall->getModelMatrix());
            // Vykreslení modelu
            wall->draw();
        }

        // Vykreslení slunce
        if (sunModel) {
            // Zajistit, že pozice modelu slunce přesně odpovídá pozici světla
            sunModel->origin = pointLightPosition;

            // Nastavení model matice pro slunce
            shader.setUniform("uM_m", sunModel->getModelMatrix());
            // Nastavení barvy slunce (jasná žlutá)
            shader.setUniform("u_diffuse_color", sunModel->meshes[0].diffuse_material);
            // Vykreslení modelu slunce
            sunModel->draw();
        }

        // 2. PØIPRAVÍME SI SEZNAM TRANSPARENTNÍCH OBJEKTÙ

        // Pøidání transparentních králíkù do seznamu
        for (auto& bunny : transparent_bunnies) {
            transparent_objects.push_back(bunny);
        }

        // Přidání modelů barevných světel do seznamu transparentních objektů
        transparent_objects.push_back(redLightModel);
        transparent_objects.push_back(blueLightModel);

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
            shader.setUniform("uM_m", model->getModelMatrix());
            // Nastavení diffuse materiálu (vèetnì alpha)
            shader.setUniform("u_diffuse_color", model->meshes[0].diffuse_material);
            // Vykreslení modelu
            model->draw();
        }

        // Vykreslení fontány
        if (fountain) {
            fountain->Draw();
        }

        // 6. OBNOVENÍ PÙVODNÍHO STAVU OPENGL
        glDepthMask(GL_TRUE);  // Povolit zápis do depth bufferu
        glDisable(GL_BLEND);   // Vypnout blending

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

    // Nastavení uniform v shaderu
    if (shader.getID() != 0) {
        shader.activate();
        shader.setUniform("uP_m", projection_matrix);
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

            // Ovládání pozice světla pomocí šipek
        case GLFW_KEY_UP:
            app->pointLightPosition.y += 0.1f;
            // Aktualizace pozice objektu slunce
            if (app->sunModel) {
                app->sunModel->origin = app->pointLightPosition;
            }
            break;

        case GLFW_KEY_DOWN:
            app->pointLightPosition.y -= 0.1f;
            // Aktualizace pozice objektu slunce
            if (app->sunModel) {
                app->sunModel->origin = app->pointLightPosition;
            }
            break;

        case GLFW_KEY_LEFT:
            app->pointLightPosition.x -= 0.1f;
            // Aktualizace pozice objektu slunce
            if (app->sunModel) {
                app->sunModel->origin = app->pointLightPosition;
            }
            break;

        case GLFW_KEY_RIGHT:
            app->pointLightPosition.x += 0.1f;
            // Aktualizace pozice objektu slunce
            if (app->sunModel) {
                app->sunModel->origin = app->pointLightPosition;
            }
            break;

        case GLFW_KEY_PAGE_UP:
            app->pointLightPosition.z -= 0.1f;
            // Aktualizace pozice objektu slunce
            if (app->sunModel) {
                app->sunModel->origin = app->pointLightPosition;
            }
            break;

        case GLFW_KEY_PAGE_DOWN:
            app->pointLightPosition.z += 0.1f;
            // Aktualizace pozice objektu slunce
            if (app->sunModel) {
                app->sunModel->origin = app->pointLightPosition;
            }
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
    double yoffset = app->lastY - ypos;

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

void App::createSunModel() {
    // Nastavení pozice světla nad středem mapy
    pointLightPosition = glm::vec3(7.5f, 8.0f, 7.5f);

    // Vytvoření viditelného objektu reprezentujícího slunce
    sunModel = new Model("resources/models/cube.obj", shader);

    // Nastavení jasně žluté barvy pro slunce
    sunModel->meshes[0].diffuse_material = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

    // Nastavení PŘESNĚ STEJNÉ pozice jako má světlo
    sunModel->origin = pointLightPosition;
    sunModel->scale = glm::vec3(0.5f);

    std::cout << "Inicializace slunce - pozice světla: ("
        << pointLightPosition.x << ", "
        << pointLightPosition.y << ", "
        << pointLightPosition.z << ")" << std::endl;
}

// Metoda pro vytvoření barevných světel s explicitním nastavením pozic
void App::createColoredLights() {
    // Explicitně definované pozice světel
    glm::vec3 redLightPos = glm::vec3(1.0f, 4.0f, 1.0f);
    glm::vec3 blueLightPos = glm::vec3(13.0f, 4.0f, 13.0f);

    // Barvy světel
    glm::vec3 redLightColor = glm::vec3(1.0f, 0.2f, 0.2f);
    glm::vec3 blueLightColor = glm::vec3(0.2f, 0.2f, 1.0f);

    // Inicializace červeného světla
    redLight = PointLight(
        redLightPos,         // pozice
        redLightColor,       // barva (červená)
        1.0f,                // konstanta útlumu
        0.09f,               // lineární útlum
        0.032f               // kvadratický útlum
    );

    // Vytvoření modelu pro červené světlo
    redLightModel = new Model("resources/models/cube.obj", shader);
    // Nastavení PŘESNĚ STEJNÉ pozice jako světlo
    redLightModel->origin = redLightPos;  // Použití stejné proměnné pro obě pozice
    redLightModel->scale = glm::vec3(0.4f);
    redLightModel->meshes[0].diffuse_material = glm::vec4(redLightColor, 0.5f);
    redLightModel->transparent = true;

    // Inicializace modrého světla
    blueLight = PointLight(
        blueLightPos,        // pozice
        blueLightColor,      // barva (modrá)
        1.0f,                // konstanta útlumu
        0.09f,               // lineární útlum
        0.032f               // kvadratický útlum
    );

    // Vytvoření modelu pro modré světlo
    blueLightModel = new Model("resources/models/cube.obj", shader);
    // Nastavení PŘESNĚ STEJNÉ pozice jako světlo
    blueLightModel->origin = blueLightPos;  // Použití stejné proměnné pro obě pozice
    blueLightModel->scale = glm::vec3(0.4f);
    blueLightModel->meshes[0].diffuse_material = glm::vec4(blueLightColor, 0.5f);
    blueLightModel->transparent = true;

    // Výpis pozic do konzole pro debugging
    std::cout << "Červené světlo - PointLight pozice: ("
        << redLight.GetPosition().x << ", " << redLight.GetPosition().y << ", " << redLight.GetPosition().z
        << "), Model pozice: ("
        << redLightModel->origin.x << ", " << redLightModel->origin.y << ", " << redLightModel->origin.z << ")" << std::endl;

    std::cout << "Modré světlo - PointLight pozice: ("
        << blueLight.GetPosition().x << ", " << blueLight.GetPosition().y << ", " << blueLight.GetPosition().z
        << "), Model pozice: ("
        << blueLightModel->origin.x << ", " << blueLightModel->origin.y << ", " << blueLightModel->origin.z << ")" << std::endl;
}

void App::createFountain() {
    // Vytvoření modelu pro částice (použijeme jednoduchou kostku)
    particleModel = new Model("resources/models/cube.obj", shader);

    // Umístění fontány do středu bludiště
    glm::vec3 fountainPosition = glm::vec3(7.5f, 0.1f, 7.5f);

    // Vytvoření systému částic pro fontánu
    fountain = new ParticleSystem(particleModel, shader, fountainPosition, 0.3f);

    std::cout << "Fountain initialized at position (" <<
        fountainPosition.x << ", " <<
        fountainPosition.y << ", " <<
        fountainPosition.z << ")" << std::endl;
}

bool App::checkCollision(const glm::vec3& position, float radius) {
    // Kontrola kolize s podlahou - použití přesné Y souřadnice
    if (position.y < 0.3f + radius) { // 0.1f je jistá tolerance nad podlahou
        return true; // Kolize s podlahou
    }

    // Kontrola kolize se zdmi v bludišti
    for (auto& wall : maze_walls) {
        // Zkontroluj jen stěny (ne podlahu)
        if (wall->origin.y > 0.5f) {
            // Jednoduchá AABB kolize (axis-aligned bounding box)
            // Získání hranic kostky zdi
            glm::vec3 wallMin = wall->origin - wall->scale * 0.5f;
            glm::vec3 wallMax = wall->origin + wall->scale * 0.5f;

            // Rozšíření hranic o poloměr hráče
            wallMin -= glm::vec3(radius);
            wallMax += glm::vec3(radius);

            // Kontrola kolize
            if (position.x >= wallMin.x && position.x <= wallMax.x &&
                position.y >= wallMin.y && position.y <= wallMax.y &&
                position.z >= wallMin.z && position.z <= wallMax.z) {
                return true; // Kolize se zdí
            }
        }
    }

    // Kontrola kolize s králíky
    for (auto& bunny : transparent_bunnies) {
        // Získání hranic králíka - použijeme zmenšený box, protože model králíka není přesná kostka
        float bunnyScale = std::max(bunny->scale.x, std::max(bunny->scale.y, bunny->scale.z));
        glm::vec3 bunnyMin = bunny->origin - glm::vec3(bunnyScale * 0.4f); // Zmenšený box
        glm::vec3 bunnyMax = bunny->origin + glm::vec3(bunnyScale * 0.4f);

        // Rozšíření hranic o poloměr hráče
        bunnyMin -= glm::vec3(radius);
        bunnyMax += glm::vec3(radius);

        // Kontrola kolize
        if (position.x >= bunnyMin.x && position.x <= bunnyMax.x &&
            position.y >= bunnyMin.y && position.y <= bunnyMax.y &&
            position.z >= bunnyMin.z && position.z <= bunnyMax.z) {
            return true; // Kolize s králíkem
        }
    }

    return false; // Žádná kolize
}