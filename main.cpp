#include <SDL3/SDL.h>

#include <algorithm>
#include <vector>
#include <iostream>
#include <cmath>
#include <complex>

#include <ctime>
#include <cstdlib>
#include <random>

#include "SDL3/SDL_render.h"
#include "include/math.hpp"

constexpr int WIDTH = 800; //usamos constexpr en vez del simple const esto lo evalua en compile time y no en runtime, así que el programa reemplaza el valor antes de ejecutarlo y no tener que buscarlo mientras corre
constexpr int HEIGHT = 600; //el programa. Es lo más eficiente para variables de este tipo. El programa no busca el valor en memoria, pues ya está escrito

struct Face {
    int v1,v2,v3,col;
};

void Rasterize(Vec3 const &pA, Vec3 const &pB, Vec3 const &pC, std::vector<Uint32> &framebuffer, SDL_Texture* texture, std::vector<float>& zbuffer, int const &col) {
    //definir bounding box


    std::vector<Color> colors = {
        {255, 0, 0},
        {0, 255, 0},
        {0, 0, 255},
        {255, 255, 0},
        {255, 0, 255},
        {0, 255, 255},
        {128, 128, 128},
        {255, 128, 0},
        {128, 0, 255},
        {0, 128, 255},
        {255,0,125},
        {100,100,255}

    };

    Color color = colors[col];

    int minx = static_cast<int>(std::min(pA.x, std::min(pB.x, pC.x))); // el static cast es para convertir de float a int
    int miny = static_cast<int>(std::min(pA.y, std::min(pB.y, pC.y)));
    int maxx = static_cast<int>(std::max(pA.x, std::max(pB.x, pC.x)));
    int maxy = static_cast<int>(std::max(pA.y, std::max(pB.y, pC.y)));

    minx = std::max(minx, 0); //asegurarse de mantenerse en pantalla of course
    miny = std::max(miny, 0);
    maxx = std::min(maxx, WIDTH - 1);
    maxy = std::min(maxy, HEIGHT - 1);

    //para todos esos pixeleles, usando edge function para determinar si estan o no, pues ponerlos

    const Vec2 pA_2d = {pA.x, pA.y};
    const Vec2 pB_2d = {pB.x, pB.y};
    const Vec2 pC_2d = {pC.x, pC.y};

    const Vec2 ladoAB = pB_2d - pA_2d;
    const Vec2 ladoBC = pC_2d - pB_2d;
    const Vec2 ladoCA = pA_2d - pC_2d;

    const float w1 = pA.z;
    const float w2 = pB.z;
    const float w3 = pC.z;

    for (int i = minx; i <= maxx; i++) {
        for (int j = miny; j <= maxy; j++) {

            Vec2 p = {static_cast<float>(i), static_cast<float>(j)};

            //definir si esta o no esta en triangulo

            float const edge1 = edge_function(p - pA_2d,ladoAB);
            float const edge2 = edge_function(p - pB_2d,ladoBC);
            float const edge3 = edge_function(p - pC_2d,ladoCA);

            if ((edge1 >= 0 && edge2 >= 0 && edge3 >= 0)||(edge1 <= 0 && edge2 <= 0 && edge3 <= 0)) {
                //pintar pixel

                float const area_total = edge1 + edge2 + edge3;

                float const e1_norm = edge1 / area_total;
                float const e2_norm = edge2 / area_total;
                float const e3_norm = edge3 / area_total;

                float const depth = e1_norm*w3 + e2_norm*w1 + e3_norm*w2; //interpolamos la profundidad

                if (depth < zbuffer[j * WIDTH + i]) {
                    zbuffer[j * WIDTH + i] = depth;
                } else {
                    continue; //si el pixel que queremos pintar es más profundo que el que ya está en el zbuffer, no lo pintamos
                }



                const Uint32 colorin = (255 << 24) | (color.r << 16) | (color.g << 8) | color.b; //esto es bit manipulation Uint tiene 32 bits, 8 para A,R,G,B, lo que hacemos es mandar 255 con << que es un shift left, osea, esos 8 bits,
                                                            //se van a mover 24 posiciones a la izquierda, osea, van a quedar en la parte de A, luego r se mueve 16 posiciones, g se mueve 8 posiciones
                                                            //y b se queda en la parte de B, luego hacemos un OR binario que es cada palito para juntar todito en un solo Uint32

                framebuffer[j * WIDTH + i] = colorin; //esto es para pintar el pixel, el framebuffer es un array de pixeles, y cada pixel es un Uint32, que es un entero de 32 bits, y cada bit representa un canal de color (RGBA)
            }

        }
    }
    SDL_UpdateTexture(texture, nullptr, framebuffer.data(), WIDTH * sizeof(Uint32));

}

void Render(const std::vector<Vec3> &vertices, const std::vector<Face> &faces, const float &rotx, const float &roty, const float &rotz, const float &scale, const Vec3 &move, const Vec3 &eye, const Vec3 &center
            , std::vector<Uint32> &framebuffer, SDL_Texture* texture, std::vector<float>& zbuffer) {

    const Mat4 MVP = projection_matrix(90,static_cast<float>(WIDTH)/HEIGHT,0.1,100.0) * lookAt_matrix(eye, center) * move_matrix(move.x, move.y, move.z) * rotz_matrix(rotz)
                * roty_matrix(roty) * rotx_matrix(rotx) * scale_matrix(scale); //la epica y superpoderosa matriz MVP, que transforma cada vertice
    //a clip space mandando su profundidad con w listo para transformar a NDC y luego a screen space
    //otro detallito es que para hacer division entre dos enteros, toca transformar uno a float antes de hacerlo

    for (const auto& face : faces) {

        Vec3 ver1 = vertices[face.v1];
        Vec3 ver2 = vertices[face.v2];
        Vec3 ver3 = vertices[face.v3];

        //auto& [ver1,ver2,ver3] = face; //esto es una forma de desempaquetar la estructura de face
        //es mucho más limpio y épico

        Vec4 ver1_clip = MVP * Vec4{ver1.x, ver1.y, ver1.z, 1.0f}; //transformamos cada vertice a clip space
        Vec4 ver2_clip = MVP * Vec4{ver2.x, ver2.y, ver2.z, 1.0f};
        Vec4 ver3_clip = MVP * Vec4{ver3.x, ver3.y, ver3.z, 1.0f};

        Vec3 ndc1 = {ver1_clip.x / ver1_clip.w, ver1_clip.y / ver1_clip.w, ver1_clip.z / ver1_clip.w}; //transformamos a NDC dividiendo por w
        Vec3 ndc2 = {ver2_clip.x / ver2_clip.w, ver2_clip.y / ver2_clip.w, ver2_clip.z / ver2_clip.w};
        Vec3 ndc3 = {ver3_clip.x / ver3_clip.w, ver3_clip.y / ver3_clip.w, ver3_clip.z / ver3_clip.w};

        Vec2 real1 = {(1 + ndc1.x) * (WIDTH -1)/ 2, (1 - ndc1.y) * (HEIGHT -1)/ 2}; //transformamos a screen space, el +1 es para tener el x entre 0 y 2 y pues por eso toca dividir por 2
        Vec2 real2 = {(1 + ndc2.x) * (WIDTH -1)/ 2, (1 - ndc2.y) * (HEIGHT -1)/ 2}; //además, restamos por 1 ya que obviamente la pantalla no va a hasta height sino hasta height-1
        Vec2 real3 = {(1 + ndc3.x) * (WIDTH -1)/ 2, (1 - ndc3.y) * (HEIGHT -1)/ 2}; //y en y restamos ya que en el ndc 1 es arriba, pero en la screen space 0 es arriba

        Vec3 p1 = {real1.x, real1.y, ndc1.z}; //y pues ya tenemos los vertices listos para ser rasterizados, con su profundidad en z
        Vec3 p2 = {real2.x, real2.y, ndc2.z};
        Vec3 p3 = {real3.x, real3.y, ndc3.z};

        Rasterize(p1, p2, p3, framebuffer, texture, zbuffer, face.col); //y pues ya tenemos los vertices listos para ser rasterizados

    }
}

int main(int argc, char* argv[]) { // aquí el argc(numero de argumentos) y el char* argv (array con strings de argumentos)
                                    //son los argumentos que le mandamos al programa y SDL3 por convención lo requiere, aunque no los vayamos
                                    //a usar, pero es necesario para que el programa funcione correctamente


    SDL_Init(SDL_INIT_VIDEO); // Entonces esto es para inicializar SDL

    SDL_Window* window = SDL_CreateWindow("Rasterizer",WIDTH,HEIGHT,0); // creamos el datatype de window y con el pointer esto nos va a llevar al window que va a estar en SDL_CreateWindow
                                                    // SDL_CreateWindow crea la ventana, pero debido a la complexidad que usa SDL lo maneja internamente, esta función, nos
                                                    // devuelve la direccion en donde se creo la ventana y esa se guarda en window

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr); // SDL_CreateRenderer es para crear el renderer, que es el encargado de dibujar en la ventana,
                                                                                        //y le pasamos la ventana que acabamos de crear, un nullptr para el driver
                                                                                        //(que es el que se encarga de manejar la renderización) con nullptr elige el mejor, que traduce todito al lenguaje que usa Windows10

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,WIDTH,HEIGHT); //creamos la textura, que es lo que nos va a servir para pintar en el renderer

    std::vector<Uint32> framebuffer(WIDTH * HEIGHT, 0); //creamos un framebuffer, que es un array de pixeles, con el mismo tamaño que la ventana, y lo inicializamos con 0

    std::vector<float> zbuffer(WIDTH * HEIGHT, HUGE_VALF); //creamos un zbuffer, que es un array de floats, y lo inicializamos con el valor más grande posible

    bool running = true;
    SDL_Event event;

    std::vector<Vec3> vertices = {
        {-0.5f,0.5f,0.5f},
        {0.5f,0.5f,0.5f},
        {0.5f,0.5f,-0.5f},
        {-0.5f,0.5f,-0.5f},
        {-0.5f,-0.5f,0.5f},
        {0.5f,-0.5f,0.5f},
        {0.5f,-0.5f,-0.5f},
        {-0.5f,-0.5f,-0.5f}
    };

    std::vector<Face> faces = {
        {0,1,2,0},
        {0,2,3,1},
        {4,5,6,2},
        {4,6,7,3},
        {0,1,5,4},
        {0,5,4,5},
        {2,3,7,6},
        {2,7,6,7},
        {1,2,6,8},
        {1,6,5,9},
        {0,3,7,10},
        {0,7,4,11}
    };

    while (running) {
        while (SDL_PollEvent(&event)) { //el sdl_pollevent saca un evento de la cola de eventos de SDL y lo pone en la variable event
            if (event.type == SDL_EVENT_QUIT) running = false; //si cierras la ventana, running se vuelve falso
        }

        float rx = sinf(clock() * 0.0001f) * 5.f; //usamos el clock para hacer que el cubo rote, el clock devuelve el tiempo en milisegundos desde que se inició el programa, y lo multiplicamos por 0.001 para tenerlo en segundos, y luego por 0.5 para que la rotación sea más lenta
        float ry = cosf(clock() * 0.0001f) * 5.f;
        float rz = sinf(clock() * 0.0001f) * 5.f;
        float scale = 1.0f * cosf(clock() * 0.001f); //podemos usar el tiempo para hacer que el cubo escale también, pero por ahora lo dejamos fijo


        std::fill(framebuffer.begin(), framebuffer.end(), 0);//reiniciamos el framebuffer cada frame para no tener los pixeles del frame anterior, esto es importante para que no se queden los pixeles pintados de un frame a otro
        std::fill(zbuffer.begin(), zbuffer.end(), HUGE_VALF);//reiniciamos tambien el zbuffer por la misma razon

        Render(vertices, faces, rx, ry, rz,
            scale, {0, 0, 2}, {0, 0, -2},
            {0, 0, 0},
            framebuffer, texture, zbuffer);

        //Rasterize({200, 100, 0.1f}, {600, 300, 0.8f}, {200, 500, 0.1f}, framebuffer, texture, zbuffer, {255, 0, 0});
        //Rasterize({600, 100, 0.9f}, {200, 300, 0.3f}, {600, 500, 0.1f}, framebuffer, texture, zbuffer, {0, 0, 255});

        SDL_RenderClear(renderer); // esto limpia el renderer, para que no se queden los pixeles del frame anterior
        SDL_RenderTexture(renderer, texture, nullptr, nullptr); // esto dibuja la textura en el renderer, con nullptr para el source y el destination, lo que significa que se va a dibujar toda la textura en toda la ventana
        SDL_RenderPresent(renderer); // esto presenta el frame actual en pantalla
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer); // en c++ toca liberar la memoria manualmente, así se hace
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
