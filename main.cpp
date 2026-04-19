#include <SDL3/SDL.h>

#include <algorithm>
#include <vector>
#include <iostream>
#include <cmath>

#include "include/math.hpp"

constexpr int WIDTH = 800; //usamos constexpr en vez del simple const esto lo evalua en compile time y no en runtime, así que el programa reemplaza el valor antes de ejecutarlo y no tener que buscarlo mientras corre
constexpr int HEIGHT = 600; //el programa. Es lo más eficiente para variables de este tipo. El programa no busca el valor en memoria, pues ya está escrito

void Rasterize(Vec3 const &pA, Vec3 const &pB, Vec3 const &pC, std::vector<Uint32> &framebuffer, SDL_Texture* texture, std::vector<float>& zbuffer, Color const &color) {
    //definir bounding box

    const int minx = static_cast<int>(std::min(pA.x, std::min(pB.x, pC.x))); // el static cast es para convertir de float a int
    const int miny = static_cast<int>(std::min(pA.y, std::min(pB.y, pC.y)));
    const int maxx = static_cast<int>(std::max(pA.x, std::max(pB.x, pC.x)));
    const int maxy = static_cast<int>(std::max(pA.y, std::max(pB.y, pC.y)));
    //para todos esos pixeleles, usando edge function para determinar si estan o no, pues ponerlos

    const Vec2 pA_2d = {pA.x, pA.y};
    const Vec2 pB_2d = {pB.x, pB.y};
    const Vec2 pC_2d = {pC.x, pC.y};

    const Vec2 ladoAB = pB_2d - pA_2d;
    const Vec2 ladoBC = pC_2d - pB_2d;
    const Vec2 ladoCA = pA_2d - pC_2d;

    for (int i = minx; i <= maxx; i++) {
        for (int j = miny; j <= maxy; j++) {

            const float w1 = pA.z;
            const float w2 = pB.z;
            const float w3 = pC.z;

            Vec2 p = {static_cast<float>(i), static_cast<float>(j)};

            //definir si esta o no esta en triangulo

            float const edge1 = edge_function(p - pA_2d,ladoAB);
            float const edge2 = edge_function(p - pB_2d,ladoBC);
            float const edge3 = edge_function(p - pC_2d,ladoCA);

            if ((edge1 >= 0 && edge2 >= 0 && edge3 >= 0)||(edge1 <= 0 && edge2 <= 0 && edge3 < 0)) {
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

    while (running) {
        while (SDL_PollEvent(&event)) { //el sdl_pollevent saca un evento de la cola de eventos de SDL y lo pone en la variable event
            if (event.type == SDL_EVENT_QUIT) running = false; //si cierras la ventana, running se vuelve falso
        }

        Rasterize({200, 100, 0.1f}, {600, 300, 0.8f}, {200, 500, 0.1f}, framebuffer, texture, zbuffer, {255, 0, 0});
        Rasterize({600, 100, 0.9f}, {200, 300, 0.3f}, {600, 500, 0.1f}, framebuffer, texture, zbuffer, {0, 0, 255});

        SDL_RenderTexture(renderer, texture, nullptr, nullptr); // esto dibuja la textura en el renderer, con nullptr para el source y el destination, lo que significa que se va a dibujar toda la textura en toda la ventana
        SDL_RenderPresent(renderer); // esto presenta el frame actual en pantalla
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer); // en c++ toca liberar la memoria manualmente, así se hace
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
