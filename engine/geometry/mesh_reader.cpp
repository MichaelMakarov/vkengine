#include "mesh_reader.hpp"

#include "utility/error.hpp"

#include <fstream>
#include <unordered_map>

namespace {

    struct Node {
        size_t point;
        size_t texture;
        size_t normal;

        friend std::istream &operator>>(std::istream &is, Node &node) {
            char sep;
            return is >> node.point >> sep >> node.texture >> sep >> node.normal;
        }
    };

    struct Face {
        Node nodes[3];

        friend std::istream &operator>>(std::istream &is, Face &face) {
            return is >> face.nodes[2] >> face.nodes[1] >> face.nodes[0];
        }
    };

    size_t get_counter_index(std::istream &is) {
        if (is.eof()) {
            raise_error("Unexpected EOF.");
        }
        char symbol;
        is.read(&symbol, 1);
        switch (symbol) {
        case ' ':
            // coordinates
            return 0;
        case 't':
            // texture
            return 1;
        case 'n':
            // normal
            return 2;
        default:
            raise_error("Unexpected symbol {} after v", symbol);
            return 3;
        }
    }

    void read_line(std::istream &is) {
        char symbol;
        while (is.read(&symbol, 1) && symbol != '\n') {
            // loop untill the line ends
        }
    }

} // namespace

std::istream &operator>>(std::istream &is, glm::vec3 &v) {
    return is >> v.x >> v.y >> v.z;
}

std::istream &operator>>(std::istream &is, glm::vec2 &v) {
    return is >> v.x >> v.y;
}

Mesh<Vertex, uint32_t> MeshReader::read_blender(std::string_view filename) {
    std::ifstream ifs{filename.data()};
    if (!ifs.is_open()) {
        raise_error("Failed to open file {}", filename);
    }

    size_t counters[4]{};
    while (!ifs.eof()) {
        char symbol;
        ifs.read(&symbol, 1);
        switch (symbol) {
        case 'v':
            // vertex begins
            counters[get_counter_index(ifs)]++;
            break;
        case 'f':
            // face begins
            counters[3]++;
            break;
        case '#':
            // comment begins
            break;
        default:
            raise_error("Unexpected symbol in the begining of the line");
            break;
        }
        read_line(ifs);
    }
    ifs.clear();
    ifs.seekg(0, ifs.beg);

    std::vector<glm::vec3> points(counters[0]);
    std::vector<glm::vec2> textures(counters[1]);
    std::vector<glm::vec3> normals(counters[2]);
    std::vector<Face> faces(counters[3]);
    size_t point_index = 0;
    size_t texture_index = 0;
    size_t normal_index = 0;
    size_t face_index = 0;
    while (!ifs.eof()) {
        char symbol;
        ifs.read(&symbol, 1);
        switch (symbol) {
        case 'v':
            switch (get_counter_index(ifs)) {
            case 0:
                ifs >> points[point_index];
                points[point_index].z = -points[point_index].z;
                point_index++;
                break;
            case 1:
                ifs >> textures[texture_index++];
                break;
            case 2:
                ifs >> normals[normal_index];
                normals[normal_index].z = -normals[normal_index].z;
                normal_index++;
                break;
            }
        case 'f':
            ifs >> faces[face_index++];
            break;
        }
        read_line(ifs);
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(faces.size() * 3);
    indices.reserve(vertices.size());
    for (auto const &face : faces) {
        for (auto const &node : face.nodes) {
            Vertex vertex;
            vertex.point = points[node.point];
            vertex.texture = textures[node.texture];
            vertex.normal = normals[node.normal];
            vertices.push_back(vertex);
        }
    }

    return Mesh<Vertex, uint32_t>();
}
