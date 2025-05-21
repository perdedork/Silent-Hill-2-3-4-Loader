#ifndef _BASETYPES_H_
#define _BASETYPES_H_

#include "RVertex.h"
#include "PerPixelLighting.h"
#include "typedefs.h"
#include "plane.h"

using namespace std;

//-- Forward Class Declarations --/
class light;
class IShadow;
class IRenderMgr;
class Camera;
class IObjectMesh;
class lightAttrib;
//class Material;

typedef enum e_lightType{ L_SPOT = 0, L_DIRECTIONAL = 1, L_POINT = 2, L_AREA = 3, L_COUNT};
typedef enum e_areaLight{ AREA_NONE = 0, AREA_CIRCLE = 1, AREA_RECT = 2, AREA_COUNT};

/*
typedef struct {
	vertex lDir;
	float cosAng, 
	linearAtten,			//side view linear attenuation    | If the light is pointed along the
	quadAtten,				//side view quadratic attenuation | x-axis, side view is up and down (y),
	spotAtten;				//head-on view attenuation        | and forward and rear (z). Head on is (x)
}spotData_tag; 

typedef struct {
	vertex lDir;
	float intensity;            //this is the intensity of the light (ie. overcast, sunny, ect )
}directionalData_tag;

typedef struct {            
	float linearAtten,
		  quadAtten;
}pointData_tag;

typedef struct {            
	float linearAtten,      //Atten coming out from the light, (think a square,flourescent light)  |Cross  >---<    light from corners    
		  quadAtten;        //The light breaks very hard along the edges(except cross light).      |Light  |a b|    shines out side opp-
	vertex lDir;            //Tangent vector to the surface.                                       |       *****    osite to the corner.
	vertex upDir;           //a vector used to form a basis to define orientation of surface area  |      b*"""*a    Think Displacement
	float  cosAng;          //this is the cos(angle) that the light comes out each side  _|/       |    bbb*"""*aaa   mappint.
	union areaDim_tag{
		float w, h;         //Width and height of a RECTANGLE
		float rad, aspect;  //Radius and Aspect Ratio of a CIRCLE or ELIPSE
		//############ ADD IN OTHER AREA LIGHT STUFF HERE
	}areaDim;
}areaData_tag;

typedef union lightTypeAttribs_tag{  
	//--  Union Member:: SPOT LIGHT  --/
		spotData_tag spotData;
	//--  Union Member:: DIRECTIONAL LIGHT  --/
		directionalData_tag directionalData;
	//--  Union Member:: POINT LIGHT  --/
		pointData_tag pointData;
	//--  Union Member:: AREA LIGHT  --/
		areaData_tag areaData;		
	}lightTypeAttribs_tag;     //End of lightTypeAttribs union
*/

class lightAttrib{
public:
	lightAttrib(){}
	~lightAttrib(){}
	//lightAttrib(const lightAttrib &l){int x;for(x = 0; x < sizeof(lightAttrib);x++)this[x]=(&l)[x];}

	e_lightType type;
	vertex      lPos;
	vertex		lDir;

	float lightViewAngle, aspectRatio;
	float linearAtten, quadAtten;
	//lightTypeAttribs_tag lightTypeAttribs;

   //--  Union for the Different Light Types' Attributes  --/
	

};
//############## Place in a "sprite" class where models have bones.
//  int mWeightsPerVert;//Number of weights associated to a vertex
//NOTE: We will define scenes based on obj format, and use the radiosity
//   program along with the tristrip generator to optimize and create light
//   maps similar to my own format.  Characters will use something like 3ds 
//   formats.  ########################################################


//######NOTE: EVENTUALLY, Place Factory classes in their own file.

//#########PLACE PRIMITIVE DATA IN THEIR OWN CLASS


typedef struct{    //24 bit RGB color
  unsigned char r,g,b;
}rgb;

typedef struct{    //32 bit RGB color
  unsigned char r,g,b,a;
}rgba;

typedef struct{    //OpenGL color
  float r,g,b,a;
}rgbaf;

typedef struct{    //Floating point RGB type
  float r,g,b;     //RGB of color
}rgbf;

typedef struct{
  int indexT1,indexT2;  //indicies for the tri's 
  int lowVert,highVert; //Vert indicies, lower index #, and higher index # 
}indexedEdge;


typedef struct{
	plane triPlane;     //The geometric plane of triangle (which includes the normal)
	int index[3];       //Triangle verticies
	bool isCap;         //TRUE if this tri is part of shadow vol cap, false otherwise
}triangle;


typedef struct{
	vertex centerPos;
	rgbaf  reflectColor;
}reflectSetup;

//############# RENDERING STUFF ############### 
// move this to another place eventually,since
// it deals w/ context
class texRenderTarget
{
public:
	texRenderTarget( ){ setNullAll( ); }
	texRenderTarget( const texRenderTarget & rhs ){ operator=( rhs ); }
	~texRenderTarget( ){ deleteAllData( ); }
	texRenderTarget & operator=( const texRenderTarget & rhs )
	{ 
		if( & rhs != this )
		{ 
			texName = rhs.texName; 
			hPBuffer = rhs.hPBuffer;
			hdc = rhs.hdc;
			hglrc = rhs.hglrc;
			w = rhs.w;
			h = rhs.h;
			flags = rhs.flags;
			extra = rhs.extra;
		}
		return *this;
	}

	void setNullAll( )
	{
		extra = NULL;
	}

	void deleteAllData( )
	{
		setNullAll( );
	}


	string      texName;  //Texture Name, for script driven stuff
    HPBUFFERARB hPBuffer; //Handle to texture context
    HDC         hdc;      //Device context
    HGLRC       hglrc;    //OpenGL rendering context.
    int         w;        //Width of render texture/cube face
    int         h;        //Height of render texture/cube face
	GLuint      texID;    //Texture handle for the pbuffer
	int         flags;    //Flags for this render target
	void *		extra;
};

//-- Collidable class interface --/
class ICollidable{
public:

	~ICollidable(){}
	void setHit(){}
	bool isHit(){return false;}
	void getHitVector(vertex *dir){}
	eBoundingShape getShape(){return mBoundingShape;}
	void setShape(eBoundingShape shape){mBoundingShape=shape;}

	enum eBoundingShape mBoundingShape;
};





//-- Shadow class interface --/

class IShadow{
public:
	~IShadow(){}
	void render(){}
	void restore(){}
	bool isInView(){return false;}
	void recompute(vertex & from,vertex & at){}
};


//-- Renderable class interface --/
class IRenderable{
public:
	~IRenderable(){}
	void render(){}
	void read(FILE *f);
	void write(FILE *f);
	void setRenderer(IRenderMgr *rM){mRenderer=rM;}      
	bool isInView(){return false;}                       //Checks to see if object is in view
	void setEdges();                                     //Sets the edge connectivity


	IRenderMgr   *mRenderer;
	IShadow      *pDirLightShadow;   //Pointer normally set to NULL, except in cases of directional light, where all IInstance 's shadows point to this
	indexedEdge  *edges;             //Edges, indexed with the triangles, and the low/high verticies
    int           eCnt;              //Number of indexed edges
};



//-- IInstance                                                --/
//--  This is a seperate instance of a repeated object.  This --/
//-   is used in cases of multiple objects of the same thing, --/
class IInstance: public ICollidable{
public:
	IInstance();
	~IInstance();

	//void        runPhysics();  ...hmm... what to do?

	DWORD       flags;         //16 flags for various uses - type of IRenderable, type of shadow,
	matrix3x3   orient;        //Orientation matrix (Orthonormal)
	vertex      position;      //Position of object
	bool        hasMoved;      //Object has moved since it was last rendered

	triangle        *tris;      //The triangles for the mesh
	int              tCnt;      //Triangle Count


	IRenderable *renderable;   //The RVertex/Material/Geometric data for this instance.  Instances of same type of obj point to same renderable data
	IShadow     *shadow;       //Shadow data for this instance
};







//-- IObjectMesh                                                --/
//--   This type is for static objects, or simple movement type --/
//--   objects.  It isn't so much for terrain/level, but objs   --/
//--   populating a level (like a tree, car, pressure cooker)   --/
//--   This isn't for animated objects like people, that have   --/
//--   Vertex blending.
//--   ObjectMeshes have:  Mesh and orientation dat             --/
//--             Oriented Bounding Boxes (AABB, affected by the --/
//--                            orientation dat)                --/

class IObjectMesh: public IRenderable{
public:
	IObjectMesh();
	~IObjectMesh();

    int  createTriangleArray(IInstance *ptrInst); //Creates the triangle array and it's data, and returns the length of it
	void computeTrianglePlanes(IInstance *ptrInst);    //Computes new planes for triangles (if they have changed)

	/*---------------Vertex Data----------------*/

	RVertex         *vData;  //All vertex data - position, pervertex normal, per-vert binormal, tangent, and texcoord
	int              vCnt;   //RVertex Count

	/*-----------Triangle/Mesh Data------------*/
	unsigned int    *iData;     //Index Data
	int              iCnt;      //Index Count

	/*------Texture/Material/Shader Data-------*/
	int              rMat;      //ref to Material of Renderable type
 
	/*---------Shadow/Connectivity Info--------*/
	IShadow      *shadow;       //Ptr to the Shadow Base, either map or vol
  
	AABB          box;          //Bounding box for obj   -> to transform to world: wMin= orient * min + position, wMax= orient * max + position
};





//-- Primitive enumerated type --/
enum e_primitive{
  P_POINT=0,         P_LINE,  P_LINESTRIP,
  P_POLYGON,     P_TRISTRIP,    P_INDEXED};



//Thoughts on Rendering -- while pushing things into the renderqueue, put them
//  into appropriate places based upon the render attributes, first by textures
//  and shaders, then by state changes, or whatever takes the most time, then
//  render based upon groupings.  Need to incorporate shadow volumes into this
//  as well.


//-- IPrimitiveSet class interface --/
//-- NOTE: No shadow data within the class - that is handled by the DataMgr for the whole scene
  class IPrimitiveSet: IRenderable{
public:

  /*---------------Vertex Data----------------*/

  RVertex         *vData;  //All vertex data - position, pervertex normal, per-vert binormal, tangent, and texcoord
  int              vCnt;   //RVertex Count

  /*-----------Triangle/Mesh Data------------*/
  unsigned int    *iData;     //Index Data
  int              iCnt;      //Index Count
  triangle        *tris;      //The triangles for the mesh
  int              tCnt;      //Triangle Count


  /*------Texture/Material/Shader Data-------*/
  int              rMat;      //ref to Material of Renderable type
 
  
  void read(FILE *f);
  void write(FILE *f);
};





class EdgeTree{
public:
	EdgeTree();
	~EdgeTree();

	enum { EDGE_X_SPLIT = 0, EDGE_Y_SPLIT = 1, EDGE_Z_SPLIT = 2, EDGE_W_SPLIT = 3, EDGE_COUNT_SPLIT,
			EDGE_LEAF_SPLIT = EDGE_COUNT_SPLIT};

	typedef struct tag_EdgeTreeNode{
		int splitDir;                     //The Plane Direction of the Split
		vertex4f minLocal,				  //Minimum of all edges stored at this node
				 minChildren,             //Minumum of all edges stored in ALL of the children of this node
				 maxLocal,				  //Maximum of all edges stored at this node
				 maxChildren;             //Maximum of all edges stored in ALL of the children of this node
		vector<int> *edges;               //Vector of the edges at this node, where edge spans the computedCenter
		tag_EdgeTreeNode *lowSide,        // edges where plane[splitDir] < computedCenter
						 *highSide;       // edges where plane[splitDir] > computedCenter
	}EdgeTreeNode;
	typedef vector<int> * intVecPtr;

	//int  insertEdge( indexedEdge *edge );                 //Header function to insert an edge
	//int  insertEdge( indexedEdge *edge, edgeTreeNode *node, int splitDir

	void deleteTree();                                       //Deletes whole tree
	void deleteTree( EdgeTreeNode *node );                   //Recursive deleteTree function
	int createEdgeTree( IObjectMesh *r, triangle *t);
	int createEdgeTree( IObjectMesh *r, triangle *t,  EdgeTreeNode *node,
						vertex4f *min, vertex4f *max, vector<int> &edges, int splitDir);
	int recomputeTree( IObjectMesh *mesh, IInstance *objInstance );      //Recreates 

	void searchEdgeTree( lightAttrib *l );                         //Base function to search through the edgeTree
	void searchEdgeTree( lightAttrib *l, EdgeTreeNode *node );     //Recursive function to search edgeTree
						 
	int  getEdgeIndexes( intVecPtr  **edges );               //Returns the # of edge, & list created in search.
	bool isCritical( vertex & lDir, vertex & lPos,
					 vertex4f &min, vertex4f &max);          //Checks to see if edge is perpendicular

	EdgeTreeNode *root;
	intVecPtr *potentialEdges;
	int potEdgeCount;
	int nodeCount;


};


//-- Shadow Volumeclass interface --/

class ShadowVol:public IShadow{
public:
  ShadowVol();
  ~ShadowVol();
  void init(char *texName);
  void render();
  void restore();
  void setInView();
  bool isInView();
  bool isCamInShadow( Camera *cam );
  void recompute(vertex & from,vertex & at);

  EdgeTree *eTree;          //EdgeTree for this shadow - using the indicies of the basic model, and the orientation of the instance
};


class ShadowMap: public IShadow{
public:
	ShadowMap();
	~ShadowMap();
	void init(char *texName, int _flags, lightAttrib *pLA );
	void render();//vertex &from,vertex &at);
	void restore();
	bool isInView();
	void computeLightProj();
	void recompute(vertex & from,vertex & at);
	void compileMatrix(matrix & modelViewInv, matrix & modelProj);
	void loadTexMVInvMat(int startOffset, GLenum target = GL_VERTEX_PROGRAM_ARB);
	void loadCamMVMat(int startOffset, GLenum target = GL_VERTEX_PROGRAM_ARB);
	void loadShadowMapTex(int texUnit, GLenum type = GL_TEXTURE_2D);
	void releaseShadowMapTex(int texUnit, GLenum type = GL_TEXTURE_2D);
	void BindShadowMapShaders( void );
	void FreeShadowMapShaders( void );
	void CheckLost(char *place);
	void setAttribs( lightAttrib *pLA );

	//GLfloat lightMV[16];
	matrix	lightMV;
	GLfloat lightProj[16];
	matrix	texMatrix;
	matrix	texMatrixInv;
	matrix	texMVInvMatrix;
	matrix	texMVInvMatrixA;
	matrix	texMVInvMatrixB;
	int		flags;
	GLuint  texID;
	GLuint  vProg,                //Vertex program for this light type, and for shadow maps
		    fProg;                //Fragment program for the light type
	int		texNum;				  //Texture number in the renderTargets TexMgr Arrray
	int		shadowRes;			  //Resolutioon of the shadow map texture
	

	lightAttrib *pLight;

};








//-- IRenderMgr class interface --/
//--  This is the main rendering class.  In the future, both DirectX and
//--  OpenGL are derived from it, but for now, it is just openGL.  Only one
//--  instance of this class exists, and derived classes should be static.
class IRenderMgr{
public:
	~IRenderMgr(){}
	bool createContext(int type, long renderFlags){return false;}
	void render(){}
	bool init(char *s){return false;}  //Startup whatever needs to be started
	void shutdown(){}                  //Used to switch contexts during execution
};



//-- Scene rendering override function --/
//-- DESC: Renders scene as an init    --/
//--   step for shadow maps.  Uses an  --/
//--   Octtree and simple fustrum.     --/
void gRenderSceneForDepth(vertex &from, vertex &at);

//-- Collidable class factory functions --/
int CollideClassCreate(int type,ICollidable **ptrPtrICollidable);

//-- Shadow class factory functions --/
int ShadowClassCreate(int type,IShadow **shadow,light &l);




#endif
