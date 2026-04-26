#pragma once
#include <vector>
#include "material.hpp"
#include "math.hpp"
#include "fstream"
#include "sstream"
#include <filesystem>

struct Face {

    std::vector<int> v_indices; // indices de los vertices que forman la cara
    std::vector<int> n_indices; // indices de las normales de la cara
    std::vector<int> uv_indices; // indices de las coordenadas de textura de la cara

    std::string name; // nombre del material que se va a usar para esta cara

};

class Mesh {

public:
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec2> uvs;

    std::vector<Face> faces;
    std::vector<Material> materials;

    void load_obj(std::string filename) { //el parser de obj

        std::ifstream objeto(filename); //abrimos el archivo
        std::string base = std::filesystem::path(filename).parent_path().string() + "/"; //esto es para obtener la ruta del archivo,
                                                            //para luego usarla para cargar el mtl, que se encuentra en la misma carpeta

        if (objeto) { // si el archivo esta abierto
            std::string line;

            while(std::getline(objeto,line)){ //vamos linea por linea

                std::istringstream iss(line); //inicializamos el istringstream como la primera linea

                std::string material; //inicializamos el material
                std::string word; //inicializamos el word, que es una variable que va a guardar e iss
                iss >> word; //inicializamos las primera "palabra" que es simplemente el primer texto antes del primer espacio a word

                if (word == "mtllib"){ //si al leer, nos encontramos esto pues tenemos el nombre del mtl

                    iss >> word;
                    std::string mtl = word;

                }

                if (word == "v"){ //si al leer, nos encontramos una "v" rellenamos la lista de vertices

                    iss >> word; //word se convierte en la siguiente "palabra" que es simplemente el siguiente string entre espacios
                    float v1 = std::stof(word); //lo convertimos a float, cuidado con usar static cast, de string a float no existe
                    iss >> word;
                    float v2 = std::stof(word);
                    iss >> word;
                    float v3 = std::stof(word);

                    vertices.push_back({v1,v2,v3}); //hacemos el append a la lista

                }

                if (word == "vn"){//si al leer, nos encontramos una "vn" rellenamos la lista de normales

                    iss >> word;
                    float v1 = std::stof(word);
                    iss >> word;
                    float v2 = std::stof(word);
                    iss >> word;
                    float v3 = std::stof(word);

                    normals.push_back({v1,v2,v3});

                }

                if (word == "vt"){//si al leer, nos encontramos una "vt" rellenamos la lista de coordenadas de textura

                    iss >> word;
                    float v1 = std::stof(word);
                    iss >> word;
                    float v2 = std::stof(word);

                    uvs.push_back({v1,v2});
                }

                if (word == "usemtl"){
                    std::string mtl;

                    iss >> word;
                    material = word;

                    materials.push_back(Material(material,base + mtl)); //creamos el material y lo añadimos a la lista de materiales
                }

                if (word == "f"){
                    Face face;
                    face.name = material; //nombramos el face con el material que le corresponde

                    std::string token;
                    while (iss >> token) { //mandamos cada indice a la lista de face que le corresponde
                        std::istringstream iss2(token);
                        std::string index;

                        std::getline(iss2, index, '/');
                        face.v_indices.push_back(std::stoi(index));

                        std::getline(iss2, index, '/');
                        face.n_indices.push_back(std::stoi(index));

                        std::getline(iss2, index, '/');
                        face.uv_indices.push_back(std::stoi(index));

                    }
                }

            }

        }
        objeto.close();

    }
};
