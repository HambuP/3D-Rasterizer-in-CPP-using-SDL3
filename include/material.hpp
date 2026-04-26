#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdint>

struct Col {
    int r, g, b;
};

class Material {
public:

    std::string nombre;
    std::vector <Col> texture;
    uint32_t width, height;

    explicit Material(const std::string &nombre, const std::string &mtl, const std::string &base) : nombre(nombre) {

        std::string direccion; //inicializamos la direccion de la imagen del material

        std::ifstream file(mtl);

        if (!file) {
            std::cout << "ERROR: no se pudo abrir mtl: " << mtl << "\n";
            return;
        }
        if (file) {

            std::string line;
            std::string material;

            while(std::getline(file,line)) {//vamos linea por linea

                std::istringstream iss(line);
                std::string word;

                iss >> word;

                if (word == "newmtl") {

                    iss >> word;
                    material = word;

                }

                if (word == "map_Kd") {

                    iss >> word;

                    if (nombre == material) {

                        direccion = word;

                    }
                }

            }
        }

        file.close();

        std::ifstream textura(base + direccion,std::ifstream::binary);//abrimos la textura en modo binario, ya que es bmp

        if (!textura) {
            std::cout << "ERROR: no se pudo abrir textura: " << direccion << "\n";
            return;
        }

        if (textura) { //si la textura se abre

            textura.seekg(0,textura.end); //ponemos "el cursor" al final del textura
            unsigned int length = textura.tellg(); //guardamos en una variable la longitud, osea, en que posición estamos en este caso
            textura.seekg(0,textura.beg); //volvemos al inicio

            auto buffer = new char[length]; //creamos el buffer donde vamos a guardar la info, debe tener claro, la misma longitud

            textura.read(buffer,length); //leemos y guardamos en el buffer

            if (textura) { //verificamos rapidamente si se puede leer todo el textura, buena práctica para debuggear y identificar errores
                std::cout << "all characters read successfully \n";
            } else {
                std::cout << "error only " << textura.gcount() << " could be read";
            }
            uint32_t start = //sabemos por el signature de los header de los bmp que se localizan ahí estas infos, ahora, para evitar
                                //cualquier error por el hecho de que las cosas son char* usamos el static cast uint8t,
                static_cast<uint8_t>(buffer[10]) |
                (static_cast<uint8_t>(buffer[11]) << 8) |
                (static_cast<uint8_t>(buffer[12]) << 16) |
                (static_cast<uint8_t>(buffer[13]) << 24);

            width =
                static_cast<uint8_t>(buffer[18]) |
                (static_cast<uint8_t>(buffer[19]) << 8) |
                (static_cast<uint8_t>(buffer[20]) << 16) |
                (static_cast<uint8_t>(buffer[21]) << 24);

            height =
                static_cast<uint8_t>(buffer[22]) |
                (static_cast<uint8_t>(buffer[23]) << 8) |
                (static_cast<uint8_t>(buffer[24]) << 16) |
                (static_cast<uint8_t>(buffer[25]) << 24);

            int padding = (4-((width*3)%4))%4; //significa, cuanto le falta a la linea de bits para que pueda divirse por 4
            std::vector<Col> colores(width*height,{0,0,0});

            std::cout << "start: " << start << "\n";
            std::cout << "width: " << width << "\n";
            std::cout << "height: " << height << "\n";
            std::cout << "padding: " << padding << "\n";

            for (int j= height-1; j >= 0; j--){
                for (int i = 0; i < width*3; i = i + 3){

                    //los archivos bmp usan orden bgr por convención, en vez del epico y conocido rgb
                    colores[(height-1-j)*width + i/3].b = static_cast<unsigned int>(static_cast<unsigned char>(buffer[j*(width*3+padding) + i + start])); //b
                    colores[(height-1-j)*width + i/3].g = static_cast<unsigned int>(static_cast<unsigned char>(buffer[j*(width*3+padding) + i + start + 1])); //g
                    colores[(height-1-j)*width + i/3].r = static_cast<unsigned int>(static_cast<unsigned char>(buffer[j*(width*3+padding) + i + start + 2])); //r

                }

            }

            delete[] buffer; //borramos el buffer, buena practica para liberar memoria, pero claro, si quieremos editarlo, toca hacer eso antes de esto
            texture = colores;//asignamos los colores a la textura

        }

    textura.close(); //cosa para liberar memoria y terminar el codigo epicamente


    }


};