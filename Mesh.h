//
//  Mesh.h
//  OpenGL_test
//
//  Created by 菊池祐作 on 2024/08/08.
//

#ifndef Mesh_h
#define Mesh_h

#include <vector>
#include <map>
#include <Eigen/Core>

class Mesh
{
    std::vector<Eigen::Vector3f> V;
    std::vector<Eigen::Vector3i> F;
    std::vector<Eigen::Vector3f> normalV;

public:
    void reedOBJ(std::string const& filename)
    {
        std::ifstream ifs(filename);
        if (ifs.fail())
        {
            std::cerr << "Failed to open file." << "\n";
            std::exit(1);
        }
        std::string line;
        while (std::getline(ifs, line)){
            if (line.empty() || line[0] == '#')
            {
                // Do nothing
            }
            else if (line[0] == 'v' && line[1] != 'n')
            {
                Eigen::Vector3f v;
                std::istringstream string_in{line.substr(1)};
                string_in >> v(0) >> v(1) >> v(2);
                V.push_back(v);
            }
            else if (line[0] == 'v' && line[1] == 'n')
            {
                Eigen::Vector3f vn;
                std::istringstream string_in{line.substr(2)};
                string_in >> vn(0) >> vn(1) >> vn(2);
                normalV.push_back(vn);
            }
            else if (line[0] == 'f')
            {
                Eigen::Vector3i f;
                std::replace(line.begin(), line.end(), '/', ' ');
                std::istringstream string_in{line.substr(1)};
                string_in >> f(0) >> f(0) >> f(1) >> f(1) >> f(2) >> f(2);
                f -= Eigen::Vector3i{1, 1, 1};
                F.push_back(f);
            }
        }

        normalizeMesh();
    }

    GLuint getVertexSize()
    {
        return (GLuint)V.size();
    }

    GLuint getIndexSize()
    {
        return (GLuint)F.size() * 3;
    }

    void convertMeshData(Object::Vertex *Vertices, GLuint *Indices)
    {
        for (int i = 0; i < V.size(); i++)
        {
            Object::Vertex temp = {V[i](0), V[i](1), V[i](2), normalV[i](0), normalV[i](1), normalV[i](2)};
            Vertices[i] = temp;
        }

        for (int i = 0; i < F.size(); i++)
        {
            Indices[3*i] = F[i](0);
            Indices[3*i+1] = F[i](1);
            Indices[3*i+2] = F[i](2);
        }
    }

    void exportOBJ(std::string name)
    {
        std::ofstream of;
        name.erase(name.length()-4);
        std::string filename = name + "_normalized.obj";
        of.open(filename, std::ios::out);
        for(auto& v : V)
        {
            of << "v " << v(0) << " " << v(1) << " " << v(2) << std::endl;
        }
        for (auto& vn : normalV)
        {
            of << "vn " << vn(0) << " " << vn(1) << " " << vn(2) << std::endl;
        }
        for(auto& f : F)
        {
            of << "f "
            << f(0)+1 << "//" << f(0)+1 << " "
            << f(1)+1 << "//" << f(1)+1 << " "
            << f(2)+1 << "//" << f(2)+1 << std::endl;
        }
        of.close();
    }

private:
    struct Sphere
    {
        Eigen::Vector3f center;
        float radius;
    };

    Sphere update_min_bounding_sphere(Eigen::Vector3f const& c_old, float const& r_old, Eigen::Vector3f const& pi)
    {
        Sphere s;
        Eigen::Vector3f c_new;
        float r_new;
        float d = (pi - c_old).norm() - r_old;
        if ( (pi - c_old).norm() > r_old )
        {
            c_new = c_old + (d / 2.0) * ( (pi - c_old) / (pi - c_old).norm() );
            r_new = r_old + d / 2.0;
        }
        else
        {
            c_new = c_old;
            r_new = r_old;
        }
        s = {c_new, r_new};
        return s;
    }

    Sphere min_bounding_sphere()
    {
        Sphere s;
        Eigen::Vector3f initial_pa;
        Eigen::Vector3f initial_pb;

        // initial sphere
        float max_dist = (V[1] - V[0]).norm();
        for (int i = 0; i < V.size(); i++)
        {
            float dist = (V[i] - V[0]).norm();
            if (dist > max_dist)
            {
                max_dist = dist;
                initial_pa = V[i];
            }
        }
        max_dist = (V[0] - initial_pa).norm();
        for (int i = 0; i < V.size(); i++)
        {
            float dist = (V[i] - initial_pa).norm();
            if (dist > max_dist)
            {
                max_dist = dist;
                initial_pb = V[i];
            }
        }
        s.center = (initial_pa + initial_pb) / 2.0;
        s.radius = (initial_pa - initial_pb).norm() / 2.0;

        // updaate sphere
        for (int i=0; i<V.size(); i++)
        {
            s = update_min_bounding_sphere(s.center, s.radius, V[i]);
        }

        return s;
    }

    void normalizeMesh()
    {
        Sphere s = min_bounding_sphere();
        for (auto& v : V)
        {
            v = (v - s.center) / s.radius;
        }

        for (auto& vn : normalV)
        {
            vn.normalize();
        }
    }
};

#endif /* Mesh_h */
