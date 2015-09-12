#pragma once
#include <glm\glm.hpp>
#include <unordered_map>

class Icosphere
{
public:	
	Icosphere(void);
	~Icosphere(void);

	void Icosphere::create(int recursionLevel);

	std::vector<glm::vec3> getVertices(void);
	std::vector<unsigned int> getIndices(void);

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

	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	
    int index;
    std::unordered_map<int64_t, int> middlePointIndexCache;
};

