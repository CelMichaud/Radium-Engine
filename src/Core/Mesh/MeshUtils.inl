#include "MeshUtils.hpp"

namespace Ra { namespace Core
{
    namespace MeshUtils
    {
        inline float getTriangleArea(const TriangleMesh& mesh, TriangleIdx triIdx)
        {
            std::array<Vector3, 3> v;
            getTriangleVertices(mesh, triIdx, v);

            const Vector3 edge0 = v[1] - v[0];
            const Vector3 edge1 = v[2] - v[0];

            return (edge0.cross(edge1)).norm() / 2.f;
        }

        inline Vector3 getTriangleNormal(const TriangleMesh& mesh, TriangleIdx triIdx)
        {
            std::array<Vector3, 3> v;
            getTriangleVertices(mesh, triIdx, v);

            const Vector3 edge0 = v[1] - v[0];
            const Vector3 edge1 = v[2] - v[0];
            return edge0.cross(edge1).normalized();
        }

        inline void getTriangleVertices(const TriangleMesh& mesh, TriangleIdx triIdx,
                                        std::array<Vector3, 3>& verticesOut)
        {
            const Triangle& tri = mesh.m_triangles[triIdx];
            for (uint i = 0; i < 3; ++i)
            {
                verticesOut[i] = mesh.m_vertices[tri[i]];
            }
        }

        inline Aabb getAabb(const TriangleMesh& mesh)
        {
            const Vector3 min = mesh.m_vertices.getMap().rowwise().minCoeff();
            const Vector3 max = mesh.m_vertices.getMap().rowwise().maxCoeff();
            return Aabb(min, max);
        }

        template<uint U, uint V>
        TriangleMesh makeParametricSphere(Scalar radius)
        {
            const uint slices = U;
            const uint stacks = V;
            TriangleMesh result;
            for (uint u = 0; u < slices; ++u)
            {
                const Scalar theta = Scalar(2 * u) * Scalar(M_PI) / Scalar(slices);
                for (uint v = 1; v < stacks; ++v)
                {
                    // Regular vertices on the sphere.
                    const Scalar phi = Scalar(v) * Scalar(M_PI) / Scalar(stacks);
                    result.m_vertices.push_back(Vector3(
                            radius * std::cos(theta) * std::sin(phi),
                            radius * std::sin(theta) * std::sin(phi),
                            radius * std::cos(phi)
                    ));
                    // Regular triangles
                    if (v > 1 && v < stacks)
                    {
                        const uint nextSlice = ((u + 1) % slices) * (stacks - 1);
                        const uint baseSlice = u * (stacks - 1);
                        result.m_triangles.push_back(
                                Triangle(baseSlice + v - 2, baseSlice + v - 1, nextSlice + v - 1));
                        result.m_triangles.push_back(
                                Triangle(baseSlice + v - 2, nextSlice + v - 1, nextSlice + v - 2));
                    }
                }
            }

            // Add the pole vertices.
            uint northPoleIdx = result.m_vertices.size();
            result.m_vertices.push_back(Vector3(0, 0, radius));
            uint southPoleIdx = result.m_vertices.size();
            result.m_vertices.push_back(Vector3(0, 0, -radius));

            // Add the polar caps triangles.
            for (uint u = 0; u < slices; ++u)
            {
                const uint nextSlice = ((u + 1) % slices) * (stacks - 1);
                const uint baseSlice = u * (stacks - 1);
                result.m_triangles.push_back(Triangle(northPoleIdx, baseSlice, nextSlice));
                result.m_triangles.push_back(
                        Triangle(southPoleIdx, nextSlice + stacks - 2, baseSlice + stacks - 2));
            }

            // Compute normals.
            for (const auto v : result.m_vertices)
            {
                const Vector3 n = v.normalized();
                result.m_normals.push_back(n);
            }

            checkConsistency(result);
            return result;
        }


    }
}}