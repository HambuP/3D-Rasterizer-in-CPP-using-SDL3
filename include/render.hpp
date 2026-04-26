#pragma once
#include "math.hpp"
#include <algorithm>
#include <vector>
#include <SDL3/SDL.h>
#include "mesh.hpp"
#include <complex>
#include "material.hpp"

class Render {
public:

    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int WIDTH, HEIGHT;
    std::vector<Uint32> framebuffer; //creamos un framebuffer, que es un array de pixeles, con el mismo tamaño que la ventana, y lo inicializamos con 0
    std::vector<float> zbuffer; //creamos un zbuffer, que es un array de floats, y lo inicializamos con el valor más grande posible

    Render(SDL_Renderer* renderer, const int width, const int height) : renderer(renderer), WIDTH(width), HEIGHT(height),
                                                framebuffer(width * height, 0), zbuffer(width * height, HUGE_VALF) {

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                            WIDTH,HEIGHT); //creamos la textura, que es lo que nos va a servir para pintar en el renderer

    }

    void Rasterize(Vec3 const &pA, Vec3 const &pB, Vec3 const &pC, Vec2 const &uvA, Vec2 const &uvB, Vec2 const &uvC, std::vector<Col> const &colores, const
        uint32_t &tex_width, const uint32_t &tex_height) {
        //definir bounding box

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

                float const edge1 = edge_function(p - pA_2d,ladoAB); //usamos para interpolar el punto c
                float const edge2 = edge_function(p - pB_2d,ladoBC); //usamos para interpolar el punto a
                float const edge3 = edge_function(p - pC_2d,ladoCA); //usamos para interpolar el punto b

                if ((edge1 >= 0 && edge2 >= 0 && edge3 >= 0)||(edge1 <= 0 && edge2 <= 0 && edge3 <= 0)) {
                    //pintar pixel

                    float const area_total = edge1 + edge2 + edge3;

                    float const e1_norm = edge1 / area_total;
                    float const e2_norm = edge2 / area_total;
                    float const e3_norm = edge3 / area_total;

                    float depth = e1_norm*(1/w3) + e2_norm*(1/w1) + e3_norm*(1/w2); //interpolamos la profundidad con correcta profundidad
                    depth = 1/depth;

                    Vec2 uv_coor = uvC*e1_norm*(1/w3) + uvA*e2_norm*(1/w1) + uvB*e3_norm*(1/w2); //interpolamos las coordenadas uv con interpolacion correcta por profundidad

                    uv_coor = uv_coor* depth; //volvemos a multiplicar por depth para que nos de bien

                    if (depth < zbuffer[j * WIDTH + i]) {
                        zbuffer[j * WIDTH + i] = depth;
                    } else {
                        continue; //si el pixel que queremos pintar es más profundo que el que ya está en el zbuffer, no lo pintamos
                    }

                    int pix_x = static_cast<int>(uv_coor.x * (tex_width-1)); //calculamos la coordenada x del pixel en la textura, multiplicando la coordenada uv por el ancho de la textura
                    int pix_y = static_cast<int>((1-uv_coor.y) * (tex_height-1)); //calculamos la coordenada y del pixel en la textura, multiplicando la coordenada uv por el alto de la textura,
                                                                                //  y restando por 1 porque en las coordenadas uv, 0 es abajo, pero en la textura, 0 es arriba

                    Col color = colores[pix_y * tex_width + pix_x]; //obtenemos el color del pixel en la textura, usando las coordenadas que acabamos de calcular


                    const Uint32 colorin = (255 << 24) | (color.r << 16) | (color.g << 8) | color.b; //esto es bit manipulation Uint tiene 32 bits, 8 para A,R,G,B, lo que hacemos es mandar 255 con << que es un shift left, osea, esos 8 bits,
                                                                //se van a mover 24 posiciones a la izquierda, osea, van a quedar en la parte de A, luego r se mueve 16 posiciones, g se mueve 8 posiciones
                                                                //y b se queda en la parte de B, luego hacemos un OR binario que es cada palito para juntar todito en un solo Uint32

                    framebuffer[j * WIDTH + i] = colorin; //esto es para pintar el pixel, el framebuffer es un array de pixeles, y cada pixel es un Uint32, que es un entero de 32 bits, y cada bit representa un canal de color (RGBA)
                }

            }

        }



    }

    void render_obj(Mesh const &mesh) {

        std::cout << "materiales: " << mesh.materials.size() << "\n";
        for (auto& mat : mesh.materials) {
            std::cout << "material: " << mat.nombre << "\n";
            std::cout << "textura width: " << mat.width << "\n";
            std::cout << "textura height: " << mat.height << "\n";
            std::cout << "pixeles: " << mat.texture.size() << "\n";
        }

        std::fill(framebuffer.begin(), framebuffer.end(), 0);//reiniciamos el framebuffer cada frame para no tener los pixeles del frame anterior, esto es importante para que no se queden los pixeles pintados de un frame a otro
        std::fill(zbuffer.begin(), zbuffer.end(), HUGE_VALF);//reiniciamos tambien el zbuffer por la misma razon

        const Vec3 eye = {0,0,3}; //posicion de la camara
        const Vec3 center = {0,0,0}; //a donde esta viendo la camara

        Vec3 move = mesh.transforms.translation; //obtenemos la traslacion del mesh
        const float rotx = mesh.transforms.rotation.x; //obtenemos la rotacion en x del mesh
        const float roty = mesh.transforms.rotation.y; //obtenemos la rotacion en y del mesh
        const float rotz = mesh.transforms.rotation.z; //obtenemos la rotacion en z del mesh
        const float scale = mesh.transforms.scale; //obtenemos la escala del mesh

        const std::vector<Face> faces = mesh.faces; //obtenemos las caras
        const std::vector<Vec3> vertices = mesh.vertices; //obtenemos los vertices
        const std::vector<Vec2> uvs = mesh.uvs; //obtenemos los uvs

        const Mat4 MVP = projection_matrix(90,static_cast<float>(WIDTH)/HEIGHT,0.1,100.0) * lookAt_matrix(eye, center) * move_matrix(move.x, move.y, move.z) * rotz_matrix(rotz)
                        * roty_matrix(roty) * rotx_matrix(rotx) * scale_matrix(scale); //la epica y superpoderosa matriz MVP, que transforma cada vertice
            //a clip space mandando su profundidad con w listo para transformar a NDC y luego a screen space
            //otro detallito es que para hacer division entre dos enteros, toca transformar uno a float antes de hacerlo

        std::cout << "numero de caras: " << faces.size() << "\n";

        for (const auto& face : faces) {

            std::cout << "entramos al loop de las caras del mesh: \n";

            const std::string nom_mat = face.name; //obtenemos el nombre del material
            Material mat = mesh.get_material(nom_mat); //obtenemos el material usando el nombre

            for (int i=1; i < face.v_indices.size()-1;i++){ //esto es para hacer triangulación de caras con más de 3 vertices, si la cara tiene 4 vertices, va a hacer un triangulo con los vertices 0,1,2 y otro triangulo con los vertices 0,2,3

                Vec2 uv1 = uvs[face.uv_indices[0]-1]; //el menos uno es porque los indices en .obj comienzan en uno
                Vec2 uv2 = uvs[face.uv_indices[i]-1];
                Vec2 uv3 = uvs[face.uv_indices[i+1]-1];

                Vec3 ver1 = vertices[face.v_indices[0]-1];
                Vec3 ver2 = vertices[face.v_indices[i]-1];
                Vec3 ver3 = vertices[face.v_indices[i+1]-1];

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

                Vec3 p1 = {real1.x, real1.y, ver1_clip.w}; //y pues ya tenemos los vertices listos para ser rasterizados, con su profundidad en z
                Vec3 p2 = {real2.x, real2.y, ver2_clip.w}; //y pues mando el w que teníamos antes, porque es el que se usa para interpolar por profundidad
                Vec3 p3 = {real3.x, real3.y, ver3_clip.w};// no mandamos ndc.z porque este no contiene informacion de profundidad lineal real, que es la que necesitamos

                std::cout << "p1: " << p1.x << "\n";
                std::cout << "p2: " << p2.x << "\n";
                std::cout << "p3: " << p3.x << "\n";

                Rasterize(p1, p2, p3, uv1, uv2, uv3,mat.texture,mat.width,mat.height); //y pues ya tenemos los vertices listos para ser rasterizados

            }
        }
        int nonzero = 0;
        for (auto& p : framebuffer) if (p != 0) nonzero++;
        std::cout << "pixeles pintados: " << nonzero << "\n";

        SDL_UpdateTexture(texture, nullptr, framebuffer.data(), WIDTH * sizeof(Uint32)); //esto actualiza la textura con la info del framebuffer
        SDL_RenderTexture(renderer, texture, nullptr, nullptr); // esto dibuja la textura en el renderer, con nullptr para el source y el destination, lo que significa que se va a dibujar toda la textura en toda la ventana
        std::cout << "Llegamos al uptadetexture y render texture gucci \n";

    }

    ~Render() {
        SDL_DestroyTexture(texture); //destruimos la textura al terminar
    }

};