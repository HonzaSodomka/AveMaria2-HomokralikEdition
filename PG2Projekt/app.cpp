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
    // Nastaven� po��te�n� pozice kamery
    camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
}

App::~App() {
    // �klid
    shader.clear();
    if (triangle) {
        delete triangle;
        triangle = nullptr;
    }

    // Uvoln�n� model� bludi�t�
    for (auto& wall : maze_walls) {
        delete wall;
    }
    maze_walls.clear();

    // Uvoln�n� transparentn�ch kr�l�k�
    for (auto& bunny : transparent_bunnies) {
        delete bunny;
    }
    transparent_bunnies.clear();
}

bool App::init(GLFWwindow* win) {
    window = win;

    // Ujist�me se, �e FOV m� spr�vnou hodnotu
    fov = DEFAULT_FOV;

    if (!GLEW_ARB_direct_state_access) {
        std::cerr << "No DSA :-(" << std::endl;
        return false;
    }

    // Nastaven� ukazatele na t��du App pro callbacky
    glfwSetWindowUserPointer(window, this);

    // Nastaven� callback�
    glfwSetFramebufferSizeCallback(window, fbsize_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Zak�z�n� zobrazen� kurzoru a jeho omezen� na okno
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Povolen� Z-bufferu pro spr�vn� vykreslov�n� 3D model�
    glEnable(GL_DEPTH_TEST);

    // Nastaven� pro pr�hlednost
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Z�sk�n� velikosti framebufferu
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Na�ten� assets
    init_assets();

    // Prvn� aktualizace projek�n� matice
    update_projection_matrix();

    // Explicitn� nastaven� view matice
    if (shader.getID() != 0) {
        shader.activate();
        shader.setUniform("uV_m", camera.GetViewMatrix());
    }

    return true;
}

void App::init_assets() {
    // Na�ten� shader�
    try {
        std::cout << "Loading shaders..." << std::endl;
        shader = ShaderProgram("resources/shaders/tex.vert", "resources/shaders/tex.frag");
        std::cout << "Shaders loaded successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Shader loading error: " << e.what() << std::endl;
        throw;
    }

    // Vytvo�en� bludi�t�
    try {
        std::cout << "Creating maze..." << std::endl;
        createMazeModel();
        std::cout << "Maze created successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Maze creation error: " << e.what() << std::endl;
        throw;
    }

    // Vytvo�en� transparentn�ch kr�l�k�
    try {
        std::cout << "Creating transparent bunnies..." << std::endl;
        createTransparentBunnies();
        std::cout << "Transparent bunnies created successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Transparent bunnies creation error: " << e.what() << std::endl;
        throw;
    }
}

void App::createTransparentBunnies() {
    // Na�ten� textury pro kr�l�ky
    GLuint bunnyTexture = textureInit("resources/textures/kralik.jpg");

    // Pozice pro kr�l�ky v bludi�ti
    std::vector<glm::vec3> bunny_positions = {
        glm::vec3(0.0f, 5.0f, 3.0f),   // Prvn� kr�l�k
        glm::vec3(20.0f, 5.0f, 7.0f),   // Druh� kr�l�k
        glm::vec3(40.0f, 5.0f, 5.0f)   // T�et� kr�l�k
    };

    // Barvy pro kr�l�ky (RGBA, kde A je pr�hlednost) - NI��� HODNOTY ALPHA
    std::vector<glm::vec4> bunny_colors = {
        glm::vec4(1.0f, 0.3f, 0.3f, 0.8f),   // �erven� v�ce pr�hledn� 
        glm::vec4(0.3f, 1.0f, 0.3f, 0.6f),   // Zelen� je�t� v�ce pr�hledn�
        glm::vec4(0.3f, 0.3f, 1.0f, 0.4f)    // Modr� nejv�ce pr�hledn�
    };

    // Vytvo�en� t�� transparentn�ch kr�l�k�
    for (int i = 0; i < 3; i++) {
        // Vytvo�en� modelu kr�l�ka
        Model* bunny = new Model("resources/models/bunny_tri_vnt.obj", shader);

        // Nastaven� textury a barvy s pr�hlednost�
        bunny->meshes[0].texture_id = bunnyTexture;
        bunny->meshes[0].diffuse_material = bunny_colors[i];

        // Nastaven� pozice a velikosti
        bunny->origin = bunny_positions[i];
        bunny->scale = glm::vec3(0.5f, 0.5f, 0.5f); // Zmen��me kr�l�ky

        // Ozna�en� jako transparentn� objekt
        bunny->transparent = true;

        // P�id�n� do vektoru transparentn�ch kr�l�k�
        transparent_bunnies.push_back(bunny);
    }
}

GLuint App::textureInit(const std::filesystem::path& filepath) {
    std::cout << "Na��t�m texturu: " << filepath << std::endl;  // Debug v�pis
    // Pou�ij std::filesystem::path spr�vn�
    std::string pathString = filepath.string();
    cv::Mat image = cv::imread(pathString, cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        throw std::runtime_error("Nelze na��st texturu ze souboru: " + pathString);
    }
    return gen_tex(image);
}

GLuint App::gen_tex(cv::Mat& image) {
    GLuint ID = 0;

    // Generov�n� ID OpenGL textury
    glCreateTextures(GL_TEXTURE_2D, 1, &ID);

    // Nastaven� podle po�tu kan�l� v obr�zku
    switch (image.channels()) {
    case 3:
        // Vytvo�en� a inicializace datov�ho prostoru pro texturu - immutable form�t
        glTextureStorage2D(ID, 1, GL_RGB8, image.cols, image.rows);
        // Nahr�n� dat do textury
        glTextureSubImage2D(ID, 0, 0, 0, image.cols, image.rows, GL_BGR, GL_UNSIGNED_BYTE, image.data);
        break;
    case 4:
        glTextureStorage2D(ID, 1, GL_RGBA8, image.cols, image.rows);
        glTextureSubImage2D(ID, 0, 0, 0, image.cols, image.rows, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
        break;
    default:
        throw std::runtime_error("Nepodporovan� po�et kan�l� v textu�e: " + std::to_string(image.channels()));
    }

    // Nastaven� filtrov�n� textury
    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // biline�rn� zv�t�ov�n�
    glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // triline�rn� zmen�ov�n�
    glGenerateTextureMipmap(ID);  // Generov�n� mipmap

    // Nastaven� opakov�n� textury
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
    camera.Position.y = 0.5f; // V��ka o��
}

void App::createMazeModel() {
    // Na�ten� atlasu textur
    cv::Mat atlas = cv::imread("resources/textures/tex_256.png", cv::IMREAD_UNCHANGED);
    if (atlas.empty()) {
        throw std::runtime_error("Nelze na��st atlas textur!");
    }

    // Atlas je 16x16 textur
    const int texCount = 16; // Po�et textur v ka�d�m sm�ru
    const int tileSize = atlas.cols / texCount; // Velikost jedn� textury v pixelech

    // Ulo�en� textur
    auto saveTextureFromAtlas = [&](int row, int col, const std::string& filename) {
        int x = col * tileSize;
        int y = row * tileSize;
        cv::Mat tileMat = atlas(cv::Rect(x, y, tileSize, tileSize));
        cv::imwrite("resources/textures/" + filename, tileMat);
        };

    // Ulo�en� textur
    saveTextureFromAtlas(1, 1, "floor.png"); // Podlaha
    saveTextureFromAtlas(3, 2, "wall.png");  // Zdi

    // Na�ten� textur
    wall_textures.clear();
    GLuint floorTexture = textureInit("resources/textures/floor.png");
    GLuint wallTexture = textureInit("resources/textures/wall.png");
    wall_textures.push_back(floorTexture);
    wall_textures.push_back(wallTexture);

    // Vytvo�en� bludi�t�
    maze_map = cv::Mat(15, 15, CV_8U);
    genLabyrinth(maze_map);

    // **Vytvo�en� podlahy (15x15)**
    for (int j = 0; j < 15; j++) {
        for (int i = 0; i < 15; i++) {
            Model* floor = new Model("resources/models/cube.obj", shader);
            floor->meshes[0].texture_id = floorTexture;
            floor->origin = glm::vec3(i, 0.0f, j); // Spodn� vrstva
            floor->scale = glm::vec3(1.0f, 1.0f, 1.0f);
            maze_walls.push_back(floor);
        }
    }

    // **Vytvo�en� zd� (z `maze_map`)**
    for (int j = 0; j < 15; j++) {
        for (int i = 0; i < 15; i++) {
            if (getmap(maze_map, i, j) == '#') { // Pokud je tam ze�
                Model* wall = new Model("resources/models/cube.obj", shader);
                wall->meshes[0].texture_id = wallTexture;
                wall->origin = glm::vec3(i, 1.0f, j); // Um�st�n� nad podlahu
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

    // Ujist�me se, �e FOV m� spr�vnou hodnotu
    if (fov <= 0.0f) {
        fov = DEFAULT_FOV;
    }

    // Aktivace shader programu
    shader.activate();

    // Inicializace projek�n� a pohledov� matice
    update_projection_matrix();
    shader.setUniform("uV_m", camera.GetViewMatrix());

    // Prom�nn� pro m��en� FPS a deltaTime
    double lastTime = glfwGetTime();
    double lastFrameTime = lastTime;
    int frameCount = 0;
    std::string title = "OpenGL Maze Demo";

    // Hlavn� smy�ka
    while (!glfwWindowShouldClose(window)) {
        // V�po�et deltaTime
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;

        // M��en� FPS
        frameCount++;
        if (currentTime - lastTime >= 1.0) {
            std::string fpsTitle = title + " | FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(window, fpsTitle.c_str());
            frameCount = 0;
            lastTime = currentTime;
        }

        // Zpracov�n� vstupu z kl�vesnice pro pohyb kamery
        glm::vec3 direction = camera.ProcessKeyboard(window, deltaTime);
        camera.Move(direction);

        // Aktualizace pohledov� matice
        shader.setUniform("uV_m", camera.GetViewMatrix());

        // Vy�i�t�n� obrazovky
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Implementace Painter's algoritmu pro transparentn� objekty
        std::vector<Model*> transparent_objects;

        // 1. NEJPRVE VYKRESL�ME V�ECHNY NEPR�HLEDN� OBJEKTY
        // Vykreslen� bludi�t� (nepr�hledn� objekty)
        for (auto& wall : maze_walls) {
            // Nastaven� model matice v shaderu
            shader.setUniform("uM_m", wall->getModelMatrix());
            // Vykreslen� modelu
            wall->draw();
        }

        // 2. P�IPRAV�ME SI SEZNAM TRANSPARENTN�CH OBJEKT�
        // P�id�n� transparentn�ch kr�l�k� do seznamu
        for (auto& bunny : transparent_bunnies) {
            transparent_objects.push_back(bunny);
        }

        // 3. SE�AD�ME TRANSPARENTN� OBJEKTY OD NEJVZD�LEN�J��HO K NEJBLI���MU
        std::sort(transparent_objects.begin(), transparent_objects.end(),
            [this](Model* a, Model* b) {
                // Z�sk�n� pozice objekt�
                glm::vec3 pos_a = a->origin;
                glm::vec3 pos_b = b->origin;

                // V�po�et vzd�lenosti od kamery
                float dist_a = glm::distance(camera.Position, pos_a);
                float dist_b = glm::distance(camera.Position, pos_b);

                // �azen� od nejvzd�len�j��ho k nejbli���mu (v�t�� > men��)
                return dist_a > dist_b;
            });

        // 4. NASTAVEN� OPENGL PRO TRANSPARENTN� OBJEKTY
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Zak�zat z�pis do depth bufferu

        // 5. VYKRESLEN� TRANSPARENTN�CH OBJEKT�
        for (auto* model : transparent_objects) {
            // Nastaven� model matice v shaderu
            shader.setUniform("uM_m", model->getModelMatrix());
            // Nastaven� diffuse materi�lu (v�etn� alpha)
            shader.setUniform("u_diffuse_color", model->meshes[0].diffuse_material);
            // Vykreslen� modelu
            model->draw();
        }

        // 6. OBNOVEN� P�VODN�HO STAVU OPENGL
        glDepthMask(GL_TRUE);  // Povolit z�pis do depth bufferu
        glDisable(GL_BLEND);   // Vypnout blending

        // V�m�na buffer� a zpracov�n� ud�lost�
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return true;
}

void App::update_projection_matrix() {
    if (height < 1)
        height = 1;   // Prevence d�len� nulou

    // Kontrola, zda fov m� platnou hodnotu
    if (fov <= 0.0f) {
        fov = DEFAULT_FOV;
    }

    float ratio = static_cast<float>(width) / height;

    projection_matrix = glm::perspective(
        glm::radians(fov),   // Vertik�ln� zorn� pole ve stupn�ch, p�evedeno na radi�ny
        ratio,               // Pom�r stran okna
        0.1f,                // Near clipping plane
        20000.0f             // Far clipping plane
    );

    // Nastaven� uniform v shaderu
    if (shader.getID() != 0) {
        shader.activate();
        shader.setUniform("uP_m", projection_matrix);
    }
}

void App::fbsize_callback(GLFWwindow* window, int width, int height) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->width = width;
    app->height = height;

    // Nastaven� viewportu
    glViewport(0, 0, width, height);

    // Aktualizace projek�n� matice
    app->update_projection_matrix();
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->fov += -5.0f * static_cast<float>(yoffset);  // Zoom in/out
    app->fov = glm::clamp(app->fov, 20.0f, 170.0f);  // Omezen� FOV na rozumn� hodnoty
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
    double yoffset = app->lastY - ypos; // P�evr�ceno, proto�e y-sou�adnice jdou od shora dol�

    app->lastX = xpos;
    app->lastY = ypos;

    app->camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        // Reset FOV na v�choz� hodnotu
        app->fov = app->DEFAULT_FOV;
        app->update_projection_matrix();
        std::cout << "Zoom reset to default" << std::endl;
    }
}