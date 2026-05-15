#pragma once
#include <vector>
#include "material.hpp"
#include "math.hpp"
#include "fstream"
#include "sstream"
#include <filesystem>

enum class ShadingMode { //un enum, define un tipo de variable que puede tomar un conjunto de valores limitado
                        //lo usamos porque compararlos es mucho más simple porque cada elemento es un int
    PHONG,
    TOON
};

struct Face {

    std::vector<int> v_indices; // indices de los vertices que forman la cara
    std::vector<int> n_indices; // indices de las normales de la cara
    std::vector<int> uv_indices; // indices de las coordenadas de textura de la cara

    std::string name; // nombre del material que se va a usar para esta cara

};

struct Transform {

    Vec3 rotation = {0,0,0};
    Vec3 translation = {0,0,0};
    float scale = 1;

};

class Mesh {

public:

    ShadingMode shading_mode = ShadingMode::PHONG; //shading mode por defecto en phong

    //listas de los vertices, normales, uvs y las respectivas caras
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec2> uvs;
    std::vector<Face> faces;

    //lista de materiales del objeto y de sus transformaciones
    std::vector<Material> materials;
    Transform transforms;

    void load_obj(std::string const &filename) { //el parser de obj

        std::cout<<"load obj filename : "<<filename<<std::endl;
        std::ifstream objeto(filename); //abrimos el archivo

        if (!objeto) {
            std::cout << "ERROR: no se pudo abrir el archivo\n";
            return;
        }

        std::cout << "archivo abierto correctamente\n";
        std::string base = std::filesystem::path(filename).parent_path().string() + "/"; //esto es para obtener la ruta del archivo,
                                                            //para luego usarla para cargar el mtl, que se encuentra en la misma carpeta
        std::string mtl;

        if (objeto) { // si el archivo esta abierto

            std::string line;
            std::string material; //inicializamos el material

            while(std::getline(objeto,line)){ //vamos linea por linea

                std::istringstream iss(line); //inicializamos el istringstream como la primera linea

                std::string word; //inicializamos el word, que es una variable que va a guardar e iss
                iss >> word; //le mandamos a word un token de iss

                if (word == "mtllib"){ //establecemos el nombre del archivo mtl

                    iss >> word;
                    mtl = word;

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

                if (word == "vn"){ //si al leer, nos encontramos una "vn" rellenamos la lista de normales

                    iss >> word;
                    float v1 = std::stof(word);
                    iss >> word;
                    float v2 = std::stof(word);
                    iss >> word;
                    float v3 = std::stof(word);

                    normals.push_back({v1,v2,v3});

                }

                if (word == "vt"){ //si al leer, nos encontramos una "vt" rellenamos la lista de coordenadas de textura

                    iss >> word;
                    float v1 = std::stof(word);
                    iss >> word;
                    float v2 = std::stof(word);

                    uvs.push_back({v1,v2});
                }

                if (word == "usemtl"){ //si llegamos acá, tenemos una textura, así que creamos su material

                    iss >> word;
                    material = word;

                    materials.push_back(Material(material,base + mtl,base)); //creamos el material y lo añadimos a la lista de materiales
                }

                if (word == "f"){

                    Face face;
                    face.name = material; //nombramos el face con el material que le corresponde

                    std::string token;

                    while (iss >> token) { //mandamos cada indice a la lista de face que le corresponde

                        std::istringstream iss2(token); //cada seccion x/x/x se convierte en un nuevo token
                        std::string index;

                        std::getline(iss2, index, '/'); //sacamos el primer indice
                        face.v_indices.push_back(std::stoi(index));

                        std::getline(iss2, index, '/'); //sacamos el segundo
                        face.uv_indices.push_back(std::stoi(index));

                        std::getline(iss2, index); //sacamos el tercero
                        face.n_indices.push_back(std::stoi(index));
                    }

                    faces.push_back(face); //mandamos la cara a la lista de caras
                }
            }
        }

        objeto.close(); //cerramos el archivo
    }

    const Material* get_material(const std::string &nombre) const {

        for (const auto& material : materials) { //buscar el material que tenga el mismo nombre

                if (material.nombre == nombre) {
                    return &material; //importante mandar por referencia, no queremos copiar el material en cada frame
                }

            }
            return nullptr; // Si no se encuentra el material, se devuelve un material vacío
    }

};
