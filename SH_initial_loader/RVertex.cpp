


#include "RVertex.h"





RVertex::RVertex(){}
RVertex::~RVertex(){}


RVertex::RVertex(const RVertex & r)
{
	int i, size = sizeof(RVertex);
	char *rhs = (char *)&r, *lhs = (char *)this;

	for(i=0;i < size; i++)
		lhs[i] = rhs[i];
}




RVertex & RVertex::operator=(const RVertex & r)
{
	if(&r != this)
	{
		int i, size = sizeof(RVertex);
		char *rhs = (char *)&r, *lhs = (char *)this;

		for(i=0;i < size; i++)
			lhs[i] = rhs[i];
	}
	return *this;
}



void RVertex::ComputeTangentSpaceLightVector(vertex & l, int lightType)
{





	/*

	vertex position,   
		normal,   
		sTangent, 
		tTangent,    
        tsLightVec; 
	texcoord tc[MAX_TEXCOORDS_PER_VERTEX];
	*/
}