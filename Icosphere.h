#pragma once
#include <glm\glm.hpp>
#include <unordered_map>

class Icosphere
{
public:	

	struct MeshGeometry3D
	{
		std::list<glm::vec3> positions;
		std::list<int> triangleIndices;

		void clear()
		{
			positions.clear();
			triangleIndices.clear();
		}
	};

	Icosphere(void);
	~Icosphere(void);

	MeshGeometry3D Icosphere::Create(int recursionLevel);

private:	

    struct TriangleIndices
    {
        int v1;
        int v2;
        int v3;

        TriangleIndices(int v1, int v2, int v3)
        {
            this->v1 = v1;
            this->v2 = v2;
            this->v3 = v3;
        }
    };

	int addVertex(glm::vec3 p);
	int getMiddlePoint(int p1, int p2);

	MeshGeometry3D geometry;
    int index;
    std::unordered_map<int64_t, int> middlePointIndexCache;
};

