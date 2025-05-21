/***********************************************************************************
 *  NOTES:                                                                         *
 * This header contains some of the classes and defs for mesh data for texturing,  *
 * bump mapping, and per-pixel lighting.  Some of it was moved from <typedefs.h>   *
 * so I can get rid of that hap-hazard file...  Anyways, it' pretty much a bunch   *
 * of containers, with a few operations that can't be stored (or shouldn't) such   *
 * as tangent space vectors, ect.                                                  *
 ***********************************************************************************/



#ifndef RVERTEX_H_
#define RVERTEX_H_

#include <windows.h>
#include "vertex.h"
#include "typedefs.h"

enum TexCoordTypes{TEXC_BASE =0,/* TEXC_DECAL, TEXC_TSPACE_LIGHT,*/ TEXC_COUNT};     //PLACE DECAL IN OWN "DECAL TREE"
//enum TexUnitTypes {TEX_DIFF =0, TEX_BUMP, TEX_SPEC, TEX_HALF, TEX_DECAL, TEX_CUBE, TEX_COUNT};

#define MAX_TEXCOORDS_PER_VERTEX    TEXC_COUNT
#define MAX_TEXTURES_PER_PRIMITIVE  TEXC_COUNT  
#define MAX_SHADERS_PER_PRIMITIVE  MAX_TEXCOORDS_PER_VERTEX

//--------------------------------------------------------------------/
//--  texcoord class:   -holds the texture coordinates.               /
//--------------------------------------------------------------------/
class texcoord{
public:
	float s, t;

	texcoord(){}
	texcoord(vert_type S,vert_type T ):s(S),t(T){}
	~texcoord(){}

	texcoord & operator=(const texcoord &rhs)
	{
		if(this!=&rhs)
		{
			s = rhs.s;
			t = rhs.t;
		}
		return *this;
	}
};

//----------------------------------------------------------------------------------/
//-- RVertex Class:  -holds a "renderable vertex", containing info needed for     --/
//--     complete rendering in an environment w/ per-pixel light, and textures.   --/
//--   -Base:  RVertex contains:  position, normal, texture coords for the Max    --/
//--           number of textures, the tangent and bi-normal, and tangent space   --/
//--           light vector, possibly pointing to the most intense light in a     --/
//--           scene.                                                             --/
//--       Possible Extensions: a base color, associated weights,                 --/
//----------------------------------------------------------------------------------/


class RVertex{
public:
	RVertex();
	RVertex(const RVertex & r);
	~RVertex();

	RVertex & operator=(const RVertex & r);

	void ComputeTangentSpaceLightVector(vertex & l, int lightType);  //l is either the position of the light, or direction
	void ComputeTangentVectors();                                    //Computes tangent vectors - Done One time!!


	vertex position,   //Position of the RVertex in world coords
		normal,        //Vertex normal (at the vertex, not the face)
		sTangent,      //The tangent 
		tTangent,      //The bi-normal
        tsLightVec;    //The Light Vector, in tangent space
	texcoord tc;       //[MAX_TEXCOORDS_PER_VERTEX]; //Texcoords for each veetex;
};
#endif
