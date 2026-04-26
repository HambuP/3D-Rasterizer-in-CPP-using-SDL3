#include <SDL3/SDL.h>
#include <ctime>
#include "include/render.hpp"

constexpr int WIDTH = 800; //usamos constexpr en vez del simple const esto lo evalua en compile time y no en runtime, así que el programa reemplaza el valor antes de ejecutarlo y no tener que buscarlo mientras corre
constexpr int HEIGHT = 600; //el programa. Es lo más eficiente para variables de este tipo. El programa no busca el valor en memoria, pues ya está escrito

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

    Render renderizador(renderer,WIDTH,HEIGHT); //incializamos el render con la clase

    Mesh cubo_minecraft;
    cubo_minecraft.load_obj("C:/Users/forer/CLionProjects/3D-Rasterizer-in-CPP-using-SDL3/obj/cubo_mine/sin_nombre.obj"); //cargamos el cubo de minecraft

    cubo_minecraft.transforms.translation.z = 5;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) { //el sdl_pollevent saca un evento de la cola de eventos de SDL y lo pone en la variable event
            if (event.type == SDL_EVENT_QUIT) running = false; //si cierras la ventana, running se vuelve falso
        }

        cubo_minecraft.transforms.rotation.x = sinf(clock() * 0.0001f) * 5.f; //usamos el clock para hacer que el cubo rote, el clock devuelve el tiempo en milisegundos desde que se inició el programa, y lo multiplicamos por 0.001 para tenerlo en segundos, y luego por 0.5 para que la rotación sea más lenta
        cubo_minecraft.transforms.rotation.y  = cosf(clock() * 0.0001f) * 5.f;
        cubo_minecraft.transforms.rotation.z  = sinf(clock() * 0.0001f) * 5.f;
        //cubo_minecraft.transforms.scale = 1.0f * cosf(clock() * 0.001f); //podemos usar el tiempo para hacer que el cubo escale también, pero por ahora lo dejamos fijo

        SDL_RenderClear(renderer); // esto limpia el renderer, para que no se queden los pixeles del frame anterior

        renderizador.render_obj(cubo_minecraft); //renderizamos el cubo, esto va a llenar el framebuffer con los pixeles que corresponden al cubo en su posición actual

        SDL_RenderPresent(renderer); // esto presenta el frame actual en pantalla
    }

    SDL_DestroyRenderer(renderer); // en c++ toca liberar la memoria manualmente, así se hace
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
