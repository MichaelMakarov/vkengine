#pragma once

#include <vector>

template <typename Vertex, typename Index>
class Mesh {
    std::vector<Vertex> vertices_;
    std::vector<Index> indices_;

  public:
    Mesh() = default;

    Mesh(std::vector<Vertex> &&vertices, std::vector<Index> &&indices)
        : vertices_(std::move(vertices))
        , indices_(std::move(indices)) {
    }

    Vertex const *get_vertex_data() const {
        return vertices_.data();
    }

    size_t get_vertex_data_size() const {
        return vertices_.size() * sizeof(Vertex);
    }

    size_t get_vertex_count() const {
        return vertices_.size();
    }

    Vertex &get_vertex(size_t i) {
        return vertices_[i];
    }

    Vertex const &get_vertex(size_t i) const {
        return vertices_[i];
    }

    Index const *get_index_data() const {
        return indices_.data();
    }

    size_t get_index_data_size() const {
        return indices_.size() * sizeof(Index);
    }

    size_t get_index_count() const {
        return indices_.size();
    }

    Index &get_index(size_t i) {
        return indices_[i];
    }

    Index const &get_index(size_t i) const {
        return indices_[i];
    }
};