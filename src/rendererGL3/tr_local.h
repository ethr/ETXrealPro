/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef TR_LOCAL_H
#define TR_LOCAL_H

#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../qcommon/qcommon.h"
#include "../renderer/tr_public.h"

#if 0
#if !defined(USE_D3D10)
#define USE_D3D10
#endif
#endif

#if defined(USE_D3D10)
#include <d3d10.h>
#include <d3dx10.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_thread.h>
#else
#include "qgl.h"
#endif


#if defined(__cplusplus)
extern "C" {
#endif

#if 1
#define GL_INDEX_TYPE		GL_UNSIGNED_INT
typedef unsigned int glIndex_t;
#else
#define GL_INDEX_TYPE		GL_UNSIGNED_SHORT
typedef unsigned short glIndex_t;
#endif

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// everything that is needed by the backend needs
// to be double buffered to allow it to run in
// parallel on a dual cpu machine
#define	SMP_FRAMES		2

#define	MAX_SHADERS				(1 << 12)
#define SHADERS_MASK			(MAX_SHADERS -1)

#define MAX_SHADER_TABLES		1024
#define MAX_SHADER_STAGES		16

//#define MAX_SHADER_STATES 2048
#define MAX_STATES_PER_SHADER 32
#define MAX_STATE_NAME 32

#define MAX_OCCLUSION_QUERIES	4096

#define	MAX_FBOS				64

#define MAX_VISCOUNTS			5
#define MAX_VIEWS				10

#define MAX_SHADOWMAPS			5

#if !defined(USE_D3D10)
//#define VOLUMETRIC_LIGHTING 1
#endif

#define DEBUG_OPTIMIZEVERTICES 0
#define CALC_REDUNDANT_SHADOWVERTS 0

#define OFFSCREEN_PREPASS_LIGHTING 1

//#define DEFERRED_SHADING_Z_PREPASS 1

typedef enum
{
	DS_DISABLED,				// traditional Doom 3 style rendering
	DS_STANDARD,				// deferred rendering like in Stalker
	DS_PREPASS_LIGHTING			// light pre pass rendering like in Cry Engine 3
} deferredShading_t;

typedef enum
{
	SHADOWING_BLOB,
	SHADOWING_PLANAR,
	SHADOWING_STENCIL,
	SHADOWING_VSM16,
	SHADOWING_VSM32,
	SHADOWING_ESM
} shadowingMode_t;

#define DS_STANDARD_ENABLED() ((r_deferredShading->integer == DS_STANDARD && glConfig2.maxColorAttachments >= 4 && glConfig2.drawBuffersAvailable && glConfig2.maxDrawBuffers >= 4 && glConfig2.framebufferPackedDepthStencilAvailable && glConfig.driverType != GLDRV_MESA))

#if defined(OFFSCREEN_PREPASS_LIGHTING)
#define DS_PREPASS_LIGHTING_ENABLED() ((r_deferredShading->integer == DS_PREPASS_LIGHTING && glConfig2.maxColorAttachments >= 2 && glConfig2.drawBuffersAvailable && glConfig2.maxDrawBuffers >= 2 && glConfig2.framebufferPackedDepthStencilAvailable && glConfig.driverType != GLDRV_MESA))
#else
#define DS_PREPASS_LIGHTING_ENABLED() ((r_deferredShading->integer == DS_PREPASS_LIGHTING))
#endif

#define HDR_ENABLED() ((r_hdrRendering->integer && glConfig2.textureFloatAvailable && glConfig2.framebufferObjectAvailable && glConfig2.framebufferBlitAvailable && glConfig.driverType != GLDRV_MESA))

#define REF_CUBEMAP_SIZE	128
#define REF_CUBEMAP_STORE_SIZE	1024
#define REF_CUBEMAP_STORE_SIDE	(REF_CUBEMAP_STORE_SIZE/REF_CUBEMAP_SIZE)

typedef enum
{
	CULL_IN,					// completely unclipped
	CULL_CLIP,					// clipped by one or more planes
	CULL_OUT,					// completely outside the clipping planes
} cullResult_t;

typedef struct screenRect_s
{
	int             coords[4];
	struct screenRect_s *next;
} screenRect_t;

typedef enum
{
	FRUSTUM_LEFT,
	FRUSTUM_RIGHT,
	FRUSTUM_BOTTOM,
	FRUSTUM_TOP,
	FRUSTUM_NEAR,
	FRUSTUM_FAR,
	FRUSTUM_PLANES = 5,
	FRUSTUM_CLIPALL = 1 | 2 | 4 | 8 | 16	//| 32
} frustumBits_t;

typedef cplane_t frustum_t[6];

enum
{
	CUBESIDE_PX = (1 << 0),
	CUBESIDE_PY = (1 << 1),
	CUBESIDE_PZ = (1 << 2),
	CUBESIDE_NX = (1 << 3),
	CUBESIDE_NY = (1 << 4),
	CUBESIDE_NZ = (1 << 5),
	CUBESIDE_CLIPALL = 1 | 2 | 4 | 8 | 16 | 32
};



typedef struct link_s
{
	void            *data;
	int				numElements;	// only used by sentinels
	struct link_s	*prev, *next;
} link_t;

static ID_INLINE void InitLink(link_t *l, void *data)
{
	l->data = data;
	l->prev = l->next = l;
}

static ID_INLINE void ClearLink(link_t *l)
{
	l->data = NULL;
	l->prev = l->next = l;
}

static ID_INLINE void RemoveLink(link_t *l)
{
	l->next->prev = l->prev;
	l->prev->next = l->next;

	l->prev = l->next = NULL;
}
/*
static ID_INLINE void InsertLinkBefore(link_t *l, link_t *sentinel)
{
	l->next = sentinel;
	l->prev = sentinel->prev;

	l->prev->next = l;
	l->next->prev = l;
}
*/

static ID_INLINE void InsertLink(link_t *l, link_t *sentinel)
{
	l->next = sentinel->next;
	l->prev = sentinel;

	l->next->prev = l;
	l->prev->next = l;
}



static ID_INLINE qboolean StackEmpty(link_t *l)
{
	// GCC shit: cannot convert 'bool' to 'qboolean' in return
	return l->next == l ? qtrue : qfalse;
}

static ID_INLINE link_t* StackTop(link_t *l)
{
	return l->next;
}

static ID_INLINE void StackPush(link_t *sentinel, void *data)
{
	link_t *l;

	l = (link_t *) Com_Allocate(sizeof(*l));
	InitLink(l, data);

	InsertLink(l, sentinel);
}

static ID_INLINE void* StackPop(link_t *l)
{
	link_t* top;
	void *data;

	if(l->next == l)
		return NULL;

	top = l->next;

#if 1
	RemoveLink(top);
#else
	top->next->prev = top->prev;
	top->prev->next = top->next;

	top->prev = top->next = NULL;
#endif

	data = top->data;
	Com_Dealloc(top);

	return data;
}


static ID_INLINE void QueueInit(link_t *l)
{
	l->data = NULL;
	l->numElements = 0;
	l->prev = l->next = l;
}

static ID_INLINE int QueueSize(link_t *l)
{
	return l->numElements;
}

static ID_INLINE qboolean QueueEmpty(link_t *l)
{
	return l->prev == l ? qtrue : qfalse;
}

static ID_INLINE void EnQueue(link_t *sentinel, void *data)
{
	link_t *l;

	l = (link_t *) Com_Allocate(sizeof(*l));
	InitLink(l, data);

	InsertLink(l, sentinel);

	sentinel->numElements++;
}

/*
static ID_INLINE void EnQueue2(link_t *sentinel, void *data, void *(*mallocFunc)(size_t __size))
{
	link_t *l;

	l = mallocFunc(sizeof(*l));
	InitLink(l, data);

	InsertLink(l, sentinel);

	sentinel->numElements++;
}
*/

static ID_INLINE void* DeQueue(link_t *l)
{
	link_t* tail;
	void *data;

	tail = l->prev;

#if 1
	RemoveLink(tail);
#else
	tail->next->prev = tail->prev;
	tail->prev->next = tail->next;

	tail->prev = tail->next = NULL;
#endif

	data = tail->data;
	Com_Dealloc(tail);

	l->numElements--;

	return data;
}

static ID_INLINE link_t* QueueFront(link_t *l)
{
	return l->prev;
}


// a trRefLight_t has all the information passed in by
// the client game, as well as some locally derived info
typedef struct trRefLight_s
{
	// public from client game
	refLight_t      l;

	// local
	qboolean        isStatic;	// loaded from the BSP entities lump
	qboolean        noRadiosity;	// this is a pure realtime light that was not considered by XMap2
	qboolean        additive;	// texture detail is lost tho when the lightmap is dark
	vec3_t          origin;		// l.origin + rotated l.center
	vec3_t          transformed;	// origin in local coordinate system
	vec3_t			direction;	// for directional lights (sun)

	matrix_t        transformMatrix;	// light to world
	matrix_t        viewMatrix;			// object to light
	matrix_t        projectionMatrix;	// light frustum

	float			falloffLength;

	matrix_t		shadowMatrices[MAX_SHADOWMAPS];
	matrix_t		shadowMatricesBiased[MAX_SHADOWMAPS];
	matrix_t        attenuationMatrix;	// attenuation * (light view * entity transform)
	matrix_t        attenuationMatrix2;	// attenuation * tcMod matrices

	cullResult_t    cull;
	vec3_t          localBounds[2];
	vec3_t          worldBounds[2];
	float           sphereRadius;	// calculated from localBounds

	int8_t          shadowLOD;	// Level of Detail for shadow mapping

	// GL_EXT_depth_bounds_test
	float           depthNear;
	float           depthFar;
	qboolean        noDepthBoundsTest;

	qboolean        noOcclusionQueries;
	uint32_t        occlusionQueryObject;
	uint32_t        occlusionQuerySamples;
	link_t			multiQuery;				// CHC++: list of all nodes that are used by the same occlusion query

	frustum_t       frustum;
	vec4_t			localFrustum[6];
	struct VBO_s   *frustumVBO;
	struct IBO_s   *frustumIBO;
	uint16_t        frustumIndexes;
	uint16_t       	frustumVerts;

	screenRect_t    scissor;

	struct shader_s *shader;

	struct interactionCache_s *firstInteractionCache;	// only used by static lights
	struct interactionCache_s *lastInteractionCache;	// only used by static lights

	struct interactionVBO_s *firstInteractionVBO;	// only used by static lights
	struct interactionVBO_s *lastInteractionVBO;	// only used by static lights

	struct interaction_s *firstInteraction;
	struct interaction_s *lastInteraction;

	uint16_t        numInteractions;	// total interactions
	uint16_t        numShadowOnlyInteractions;
	uint16_t        numLightOnlyInteractions;
	qboolean        noSort;		// don't sort interactions by material

	link_t			leafs;

	int             visCounts[MAX_VISCOUNTS];	// node needs to be traversed if current
	//struct bspNode_s **leafs;
	//int             numLeafs;
} trRefLight_t;


// a trRefEntity_t has all the information passed in by
// the client game, as well as some locally derived info
typedef struct
{
	// public from client game
	refEntity_t     e;

	// local
	float           axisLength;	// compensate for non-normalized axis
	qboolean        lightingCalculated;
	vec3_t          lightDir;	// normalized direction towards light
	vec3_t          ambientLight;	// color normalized to 0-1
	vec3_t          directedLight;
	qboolean        needZFail;

	cullResult_t    cull;
	vec3_t          localBounds[2];
	vec3_t          worldBounds[2];	// only set when not completely culled. use them for light interactions
	vec3_t          worldCorners[8];
} trRefEntity_t;

typedef struct
{
	vec3_t          origin;		// in world coordinates
	vec3_t          axis[3];	// orientation in world
	vec3_t          viewOrigin;	// viewParms->or.origin in local coordinates
	matrix_t        transformMatrix;	// transforms object to world: either used by camera, model or light
	matrix_t        viewMatrix;	// affine inverse of transform matrix to transform other objects into this space
	matrix_t        viewMatrix2;	// without quake2opengl conversion
	matrix_t        modelViewMatrix;	// only used by models, camera viewMatrix * transformMatrix
} orientationr_t;

// useful helper struct
typedef struct vertexHash_s
{
	vec3_t				xyz;
	void			    *data;

	struct vertexHash_s	*next;
} vertexHash_t;

enum
{
	IF_NONE,
	IF_NOPICMIP = BIT(0),
	IF_NOCOMPRESSION = BIT(1),
	IF_INTENSITY = BIT(2),
	IF_ALPHA = BIT(3),
	IF_NORMALMAP = BIT(4),
	IF_RGBA16F = BIT(5),
	IF_RGBA32F = BIT(6),
	IF_LA16F = BIT(7),
	IF_LA32F = BIT(8),
	IF_ALPHA16F = BIT(9),
	IF_ALPHA32F = BIT(10),
	IF_DEPTH16 = BIT(11),
	IF_DEPTH24 = BIT(12),
	IF_DEPTH32 = BIT(13),
	IF_PACKED_DEPTH24_STENCIL8 = BIT(14),
	IF_LIGHTMAP = BIT(15),
	IF_RGBA16 = BIT(16),
	IF_RGBE = BIT(17),
	IF_ALPHATEST = BIT(18),
	IF_DISPLACEMAP = BIT(19)
};

typedef enum
{
	FT_DEFAULT,
	FT_LINEAR,
	FT_NEAREST
} filterType_t;

typedef enum
{
	WT_REPEAT,
	WT_CLAMP,					// don't repeat the texture for texture coords outside [0, 1]
	WT_EDGE_CLAMP,
	WT_ZERO_CLAMP,				// guarantee 0,0,0,255 edge for projected textures
	WT_ALPHA_ZERO_CLAMP			// guarante 0 alpha edge for projected textures
} wrapType_t;

typedef struct image_s
{
	char            name[1024];	// formerly MAX_QPATH, game path, including extension
	// can contain stuff like this now:
	// addnormals ( textures/base_floor/stetile4_local.tga ,
	// heightmap ( textures/base_floor/stetile4_bmp.tga , 4 ) )
#if defined(USE_D3D10)
	// TODO
#else
	GLenum          type;
	GLuint          texnum;		// gl texture binding
#endif
	uint16_t        width, height;	// source image
	uint16_t        uploadWidth, uploadHeight;	// after power of two and picmip but not including clamp to MAX_TEXTURE_SIZE

	int             frameUsed;	// for texture usage in frame statistics

	uint32_t        internalFormat;

	uint32_t        bits;
	filterType_t    filterType;
	wrapType_t      wrapType;	// GL_CLAMP or GL_REPEAT

	struct image_s *next;
} image_t;

typedef struct FBO_s
{
	char            name[MAX_QPATH];

	int             index;

	uint32_t        frameBuffer;

	uint32_t        colorBuffers[16];
	int             colorFormat;

	uint32_t        depthBuffer;
	int             depthFormat;

	uint32_t        stencilBuffer;
	int             stencilFormat;

	uint32_t        packedDepthStencilBuffer;
	int             packedDepthStencilFormat;

	int             width;
	int             height;
} FBO_t;

typedef enum
{
	VBO_USAGE_STATIC,
	VBO_USAGE_DYNAMIC
} vboUsage_t;

typedef struct VBO_s
{
	char            name[MAX_QPATH];

	uint32_t        vertexesVBO;
	int             vertexesSize;	// amount of memory data allocated for all vertices in bytes
	uint32_t        ofsXYZ;
	uint32_t        ofsTexCoords;
	uint32_t        ofsLightCoords;
	uint32_t        ofsTangents;
	uint32_t        ofsBinormals;
	uint32_t        ofsNormals;
	uint32_t        ofsColors;
	uint32_t		ofsPaintColors;		// for advanced terrain blending
	uint32_t		ofsLightDirections;
	uint32_t        ofsBoneIndexes;
	uint32_t        ofsBoneWeights;

	int             attribs;
} VBO_t;

typedef struct IBO_s
{
	char            name[MAX_QPATH];

	uint32_t        indexesVBO;
	int             indexesSize;	// amount of memory data allocated for all triangles in bytes
//  uint32_t        ofsIndexes;
} IBO_t;

//===============================================================================

typedef enum
{
	SS_BAD,
	SS_PORTAL,					// mirrors, portals, viewscreens
	SS_ENVIRONMENT,				// sky box
	SS_OPAQUE,					// opaque

	SS_DECAL,					// scorch marks, etc.
	SS_SEE_THROUGH,				// ladders, grates, grills that may have small blended edges
	// in addition to alpha test
	SS_BANNER,

	SS_FOG,

	SS_UNDERWATER,				// for items that should be drawn in front of the water plane
	SS_WATER,

	SS_FAR,
	SS_MEDIUM,
	SS_CLOSE,

	SS_BLEND0,					// regular transparency and filters
	SS_BLEND1,					// generally only used for additive type effects
	SS_BLEND2,
	SS_BLEND3,

	SS_BLEND6,

	SS_ALMOST_NEAREST,			// gun smoke puffs

	SS_NEAREST,					// blood blobs
	SS_POST_PROCESS
} shaderSort_t;

typedef struct shaderTable_s
{
	char            name[MAX_QPATH];

	uint16_t        index;

	qboolean        clamp;
	qboolean        snap;

	float          *values;
	uint16_t        numValues;

	struct shaderTable_s *next;
} shaderTable_t;

typedef enum
{
	GF_NONE,

	GF_SIN,
	GF_SQUARE,
	GF_TRIANGLE,
	GF_SAWTOOTH,
	GF_INVERSE_SAWTOOTH,

	GF_NOISE
} genFunc_t;

typedef enum
{
	DEFORM_NONE,
	DEFORM_WAVE,
	DEFORM_NORMALS,
	DEFORM_BULGE,
	DEFORM_MOVE,
	DEFORM_PROJECTION_SHADOW,
	DEFORM_AUTOSPRITE,
	DEFORM_AUTOSPRITE2,
	DEFORM_TEXT0,
	DEFORM_TEXT1,
	DEFORM_TEXT2,
	DEFORM_TEXT3,
	DEFORM_TEXT4,
	DEFORM_TEXT5,
	DEFORM_TEXT6,
	DEFORM_TEXT7,
	DEFORM_SPRITE,
	DEFORM_FLARE
} deform_t;

// deformVertexes types that can be handled by the GPU
typedef enum
{
	// do not edit: same as genFunc_t

	DGEN_NONE,
	DGEN_WAVE_SIN,
	DGEN_WAVE_SQUARE,
	DGEN_WAVE_TRIANGLE,
	DGEN_WAVE_SAWTOOTH,
	DGEN_WAVE_INVERSE_SAWTOOTH,
	DGEN_WAVE_NOISE,

	// do not edit until this line

	DGEN_BULGE,
	DGEN_MOVE
} deformGen_t;

typedef enum
{
	DEFORM_TYPE_NONE,
	DEFORM_TYPE_CPU,
	DEFORM_TYPE_GPU,
} deformType_t;

typedef enum
{
	AGEN_IDENTITY,
	AGEN_ENTITY,
	AGEN_ONE_MINUS_ENTITY,
	AGEN_VERTEX,
	AGEN_ONE_MINUS_VERTEX,
	AGEN_WAVEFORM,
	AGEN_CONST,
	AGEN_CUSTOM
} alphaGen_t;

typedef enum
{
	CGEN_BAD,
	CGEN_IDENTITY_LIGHTING,		// tr.identityLight
	CGEN_IDENTITY,				// always (1,1,1,1)
	CGEN_ENTITY,				// grabbed from entity's modulate field
	CGEN_ONE_MINUS_ENTITY,		// grabbed from 1 - entity.modulate
	CGEN_VERTEX,				// tess.colors
	CGEN_ONE_MINUS_VERTEX,
	CGEN_WAVEFORM,				// programmatically generated
	CGEN_CONST,					// fixed color
	CGEN_CUSTOM_RGB,			// like fixed color but generated dynamically, single arithmetic expression
	CGEN_CUSTOM_RGBs,			// multiple expressions
} colorGen_t;

typedef enum
{
	ATEST_NONE,
	ATEST_GT_0,
	ATEST_LT_128,
	ATEST_GE_128
} alphaTest_t;

typedef enum
{
	OP_BAD,
	// logic operators
	OP_LAND,
	OP_LOR,
	OP_GE,
	OP_LE,
	OP_LEQ,
	OP_LNE,
	// arithmetic operators
	OP_ADD,
	OP_SUB,
	OP_DIV,
	OP_MOD,
	OP_MUL,
	OP_NEG,
	// logic operators
	OP_LT,
	OP_GT,
	// embracements
	OP_LPAREN,
	OP_RPAREN,
	OP_LBRACKET,
	OP_RBRACKET,
	// constants or variables
	OP_NUM,
	OP_TIME,
	OP_PARM0,
	OP_PARM1,
	OP_PARM2,
	OP_PARM3,
	OP_PARM4,
	OP_PARM5,
	OP_PARM6,
	OP_PARM7,
	OP_PARM8,
	OP_PARM9,
	OP_PARM10,
	OP_PARM11,
	OP_GLOBAL0,
	OP_GLOBAL1,
	OP_GLOBAL2,
	OP_GLOBAL3,
	OP_GLOBAL4,
	OP_GLOBAL5,
	OP_GLOBAL6,
	OP_GLOBAL7,
	OP_FRAGMENTSHADERS,
	OP_FRAMEBUFFEROBJECTS,
	OP_SOUND,
	OP_DISTANCE,
	// table access
	OP_TABLE
} opcode_t;

typedef struct
{
	const char     *s;
	opcode_t        type;
} opstring_t;

typedef struct
{
	opcode_t        type;
	float           value;
} expOperation_t;

#define MAX_EXPRESSION_OPS	32
typedef struct
{
	expOperation_t  ops[MAX_EXPRESSION_OPS];
	uint8_t         numOps;

	qboolean        active;		// no parsing problems
} expression_t;

typedef struct
{
	genFunc_t       func;

	float           base;
	float           amplitude;
	float           phase;
	float           frequency;
} waveForm_t;

#define TR_MAX_TEXMODS 4

typedef enum
{
	TMOD_NONE,
	TMOD_TRANSFORM,
	TMOD_TURBULENT,
	TMOD_SCROLL,
	TMOD_SCALE,
	TMOD_STRETCH,
	TMOD_ROTATE,
	TMOD_ENTITY_TRANSLATE,

	TMOD_SCROLL2,
	TMOD_SCALE2,
	TMOD_CENTERSCALE,
	TMOD_SHEAR,
	TMOD_ROTATE2
} texMod_t;

#define	MAX_SHADER_DEFORMS	3
typedef struct
{
	deform_t        deformation;	// vertex coordinate modification type

	vec3_t          moveVector;
	waveForm_t      deformationWave;
	float           deformationSpread;

	float           bulgeWidth;
	float           bulgeHeight;
	float           bulgeSpeed;

	float           flareSize;
} deformStage_t;

typedef struct
{
	texMod_t        type;

	// used for TMOD_TURBULENT and TMOD_STRETCH
	waveForm_t      wave;

	// used for TMOD_TRANSFORM
	matrix_t        matrix;		// s' = s * m[0][0] + t * m[1][0] + trans[0]
	// t' = s * m[0][1] + t * m[0][1] + trans[1]

	// used for TMOD_SCALE
	float           scale[2];	// s *= scale[0]
	// t *= scale[1]

	// used for TMOD_SCROLL
	float           scroll[2];	// s' = s + scroll[0] * time
	// t' = t + scroll[1] * time

	// + = clockwise
	// - = counterclockwise
	float           rotateSpeed;

	// used by everything else
	expression_t    sExp;
	expression_t    tExp;
	expression_t    rExp;

} texModInfo_t;


#define	MAX_IMAGE_ANIMATIONS	16

enum
{
	TB_COLORMAP = 0,
	TB_DIFFUSEMAP = 0,
	TB_NORMALMAP,
	TB_SPECULARMAP,
	MAX_TEXTURE_BUNDLES = 3
};

typedef struct
{
	uint8_t			numImages;
	float           imageAnimationSpeed;
	image_t        *image[MAX_IMAGE_ANIMATIONS];

	uint8_t         numTexMods;
	texModInfo_t   *texMods;

	int             videoMapHandle;
	qboolean        isVideoMap;
} textureBundle_t;

typedef enum
{
	// material shader stage types
	ST_COLORMAP,				// vanilla Q3A style shader treatening
	ST_DIFFUSEMAP,
	ST_NORMALMAP,
	ST_SPECULARMAP,
	ST_REFLECTIONMAP,			// cubeMap based reflection
	ST_REFRACTIONMAP,
	ST_DISPERSIONMAP,
	ST_SKYBOXMAP,
	ST_SCREENMAP,				// 2d offscreen or portal rendering
	ST_PORTALMAP,
	ST_HEATHAZEMAP,				// heatHaze post process effect
	ST_LIQUIDMAP,

#if defined(COMPAT_ET)
	ST_LIGHTMAP,
#endif

	ST_COLLAPSE_lighting_DB,	// diffusemap + bumpmap
	ST_COLLAPSE_lighting_DBS,	// diffusemap + bumpmap + specularmap
	ST_COLLAPSE_reflection_CB,	// color cubemap + bumpmap

	// light shader stage types
	ST_ATTENUATIONMAP_XY,
	ST_ATTENUATIONMAP_Z
} stageType_t;

typedef enum
{
	COLLAPSE_none,
	COLLAPSE_genericMulti,
	COLLAPSE_lighting_DB,
	COLLAPSE_lighting_DBS,
	COLLAPSE_reflection_CB
} collapseType_t;

typedef struct
{
	stageType_t     type;

	qboolean        active;

	textureBundle_t bundle[MAX_TEXTURE_BUNDLES];

	expression_t    ifExp;

	waveForm_t      rgbWave;
	colorGen_t      rgbGen;
	expression_t    rgbExp;
	expression_t    redExp;
	expression_t    greenExp;
	expression_t    blueExp;

	waveForm_t      alphaWave;
	alphaGen_t      alphaGen;
	expression_t    alphaExp;

	expression_t    alphaTestExp;

	qboolean		tcGen_Environment;

	byte            constantColor[4];	// for CGEN_CONST and AGEN_CONST

	uint32_t        stateBits;	// GLS_xxxx mask

	qboolean        overrideNoPicMip;	// for images that must always be full resolution
	qboolean        overrideFilterType;	// for console fonts, 2D elements, etc.
	filterType_t    filterType;
	qboolean        overrideWrapType;
	wrapType_t      wrapType;

	qboolean        uncompressed;
	qboolean        highQuality;
	qboolean        forceHighQuality;

	qboolean        privatePolygonOffset;	// set for decals and other items that must be offset
	float           privatePolygonOffsetValue;

	expression_t    refractionIndexExp;

	expression_t    fresnelPowerExp;
	expression_t    fresnelScaleExp;
	expression_t    fresnelBiasExp;

	expression_t	normalScaleExp;

	expression_t    etaExp;
	expression_t    etaDeltaExp;

	expression_t    fogDensityExp;

	expression_t    depthScaleExp;

	expression_t    deformMagnitudeExp;

	expression_t    blurMagnitudeExp;

	expression_t    wrapAroundLightingExp;
} shaderStage_t;

struct shaderCommands_s;

typedef enum
{
	CT_FRONT_SIDED,
	CT_BACK_SIDED,
	CT_TWO_SIDED
} cullType_t;

typedef struct
{
	float           cloudHeight;
	image_t        *outerbox, *innerbox;
} skyParms_t;

typedef struct
{
	vec3_t          color;
	float           density;
} fogParms_t;

typedef enum
{
	SHADER_2D,					// surface material: shader is for 2D rendering
	SHADER_3D_DYNAMIC,			// surface material: shader is for cGen diffuseLighting lighting
	SHADER_3D_STATIC,			// surface material: pre-lit triangle models
	SHADER_LIGHT				// light material: attenuation
} shaderType_t;

typedef struct shader_s
{
	char            name[MAX_QPATH];	// game path, including extension
	shaderType_t    type;

	int             index;		// this shader == tr.shaders[index]
	int             sortedIndex;	// this shader == tr.sortedShaders[sortedIndex]

	float           sort;		// lower numbered shaders draw before higher numbered

	qboolean        defaultShader;	// we want to return index 0 if the shader failed to
	// load for some reason, but R_FindShader should
	// still keep a name allocated for it, so if
	// something calls RE_RegisterShader again with
	// the same name, we don't try looking for it again

	qboolean        explicitlyDefined;	// found in a .shader file
	qboolean        createdByGuide;	// created using a shader .guide template

	int             surfaceFlags;	// if explicitlyDefined, this will have SURF_* flags
	int             contentFlags;

	qboolean        entityMergable;	// merge across entites optimizable (smoke, blood)
	qboolean        alphaTest;		// helps merging shadowmap generating surfaces

	qboolean		fogVolume;		// surface encapsulates a fog volume
	fogParms_t		fogParms;

	qboolean		parallax;		// material has normalmaps suited for parallax mapping

	qboolean        noShadows;
	qboolean        fogLight;
	qboolean        blendLight;
	qboolean        ambientLight;
	qboolean        volumetricLight;
	qboolean        translucent;
	qboolean        forceOpaque;
	qboolean        isSky;
	skyParms_t      sky;

	float           portalRange;	// distance to fog out at
	qboolean        isPortal;

	collapseType_t  collapseType;
	int             collapseTextureEnv;	// 0, GL_MODULATE, GL_ADD (FIXME: put in stage)

	cullType_t      cullType;	// CT_FRONT_SIDED, CT_BACK_SIDED, or CT_TWO_SIDED
	qboolean        polygonOffset;	// set for decals and other items that must be offset
	float           polygonOffsetValue;

	qboolean		uncompressed;
	qboolean        noPicMip;	// for images that must always be full resolution
	filterType_t    filterType;	// for console fonts, 2D elements, etc.
	wrapType_t      wrapType;

	// spectrums are used for "invisible writing" that can only be illuminated by a light of matching spectrum
	qboolean        spectrum;
	int             spectrumValue;

	qboolean        interactLight;	// this shader can interact with light shaders

	uint8_t         numDeforms;
	deformStage_t   deforms[MAX_SHADER_DEFORMS];

	uint8_t         numStages;
	shaderStage_t  *stages[MAX_SHADER_STAGES];

	int             numStates;	// if non-zero this is a state shader
	struct shader_s *currentShader;	// current state if this is a state shader
	struct shader_s *parentShader;	// current state if this is a state shader
	int             currentState;	// current state index for cycle purposes
	long            expireTime;	// time in milliseconds this expires

	struct shader_s *remappedShader;	// current shader this one is remapped too

	int             shaderStates[MAX_STATES_PER_SHADER];	// index to valid shader states

	struct shader_s *next;
} shader_t;

static ID_INLINE qboolean ShaderRequiresCPUDeforms(const shader_t * shader)
{
	if(shader->numDeforms)
	{
		const deformStage_t *ds = &shader->deforms[0];

		switch (ds->deformation)
		{
			case DEFORM_WAVE:
			case DEFORM_BULGE:
				return qfalse;

			default:
				return qtrue;
		}
	}

	return qfalse;
}

typedef struct shaderState_s
{
	char            shaderName[MAX_QPATH];	// name of shader this state belongs to
	char            name[MAX_STATE_NAME];	// name of this state
	char            stateShader[MAX_QPATH];	// shader this name invokes
	int             cycleTime;	// time this cycle lasts, <= 0 is forever
	shader_t       *shader;
} shaderState_t;

#if 0
enum
{
	ATTR_INDEX_POSITION = 0,
	ATTR_INDEX_TEXCOORD0 = 8,
	ATTR_INDEX_TEXCOORD1 = 9,
//  ATTR_INDEX_TEXCOORD2 = 10,
//  ATTR_INDEX_TEXCOORD3 = 11,
	ATTR_INDEX_TANGENT = 12,
	ATTR_INDEX_BINORMAL = 13,
	ATTR_INDEX_NORMAL = 14,
	ATTR_INDEX_COLOR = 15,
	ATTR_INDEX_LIGHTCOLOR = 16,
	ATTR_INDEX_LIGHTDIRECTION = 17,
	ATTR_INDEX_BONE_INDEXES = 10,
	ATTR_INDEX_BONE_WEIGHTS = 11,
};
#else
enum
{
	ATTR_INDEX_POSITION = 0,
	ATTR_INDEX_TEXCOORD0 = 1,
	ATTR_INDEX_TEXCOORD1 = 2,
	ATTR_INDEX_TANGENT = 3,
	ATTR_INDEX_BINORMAL = 4,
	ATTR_INDEX_NORMAL = 5,
	ATTR_INDEX_COLOR = 6,
	ATTR_INDEX_PAINTCOLOR = 7,
	ATTR_INDEX_LIGHTDIRECTION = 8,
	ATTR_INDEX_BONE_INDEXES = 9,
	ATTR_INDEX_BONE_WEIGHTS = 10,
};
#endif

// *INDENT-OFF*
enum
{
	GLS_SRCBLEND_ZERO					= (1 << 0),
	GLS_SRCBLEND_ONE					= (1 << 1),
	GLS_SRCBLEND_DST_COLOR				= (1 << 2),
	GLS_SRCBLEND_ONE_MINUS_DST_COLOR	= (1 << 3),
	GLS_SRCBLEND_SRC_ALPHA				= (1 << 4),
	GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA	= (1 << 5),
	GLS_SRCBLEND_DST_ALPHA				= (1 << 6),
	GLS_SRCBLEND_ONE_MINUS_DST_ALPHA	= (1 << 7),
	GLS_SRCBLEND_ALPHA_SATURATE			= (1 << 8),

	GLS_SRCBLEND_BITS					= GLS_SRCBLEND_ZERO
											| GLS_SRCBLEND_ONE
											| GLS_SRCBLEND_DST_COLOR
											| GLS_SRCBLEND_ONE_MINUS_DST_COLOR
											| GLS_SRCBLEND_SRC_ALPHA
											| GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA
											| GLS_SRCBLEND_DST_ALPHA
											| GLS_SRCBLEND_ONE_MINUS_DST_ALPHA
											| GLS_SRCBLEND_ALPHA_SATURATE,

	GLS_DSTBLEND_ZERO					= (1 << 9),
	GLS_DSTBLEND_ONE					= (1 << 10),
	GLS_DSTBLEND_SRC_COLOR				= (1 << 11),
	GLS_DSTBLEND_ONE_MINUS_SRC_COLOR	= (1 << 12),
	GLS_DSTBLEND_SRC_ALPHA				= (1 << 13),
	GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA	= (1 << 14),
	GLS_DSTBLEND_DST_ALPHA				= (1 << 15),
	GLS_DSTBLEND_ONE_MINUS_DST_ALPHA	= (1 << 16),

	GLS_DSTBLEND_BITS					= GLS_DSTBLEND_ZERO
											| GLS_DSTBLEND_ONE
											| GLS_DSTBLEND_SRC_COLOR
											| GLS_DSTBLEND_ONE_MINUS_SRC_COLOR
											| GLS_DSTBLEND_SRC_ALPHA
											| GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA
											| GLS_DSTBLEND_DST_ALPHA
											| GLS_DSTBLEND_ONE_MINUS_DST_ALPHA,

	GLS_DEPTHMASK_TRUE					= (1 << 17),

	GLS_POLYMODE_LINE					= (1 << 18),

	GLS_DEPTHTEST_DISABLE				= (1 << 19),

	GLS_DEPTHFUNC_LESS					= (1 << 20),
	GLS_DEPTHFUNC_EQUAL					= (1 << 21),

	GLS_DEPTHFUNC_BITS					= GLS_DEPTHFUNC_LESS
											| GLS_DEPTHFUNC_EQUAL,

	GLS_ATEST_GT_0						= (1 << 22),
	GLS_ATEST_LT_128					= (1 << 23),
	GLS_ATEST_GE_128					= (1 << 24),
//	GLS_ATEST_GE_CUSTOM					= (1 << 25),

	GLS_ATEST_BITS						= GLS_ATEST_GT_0
											| GLS_ATEST_LT_128
											| GLS_ATEST_GE_128,
//											| GLS_ATEST_GT_CUSTOM,

	GLS_REDMASK_FALSE					= (1 << 26),
	GLS_GREENMASK_FALSE					= (1 << 27),
	GLS_BLUEMASK_FALSE					= (1 << 28),
	GLS_ALPHAMASK_FALSE					= (1 << 29),

	GLS_COLORMASK_BITS					= GLS_REDMASK_FALSE
											| GLS_GREENMASK_FALSE
											| GLS_BLUEMASK_FALSE
											| GLS_ALPHAMASK_FALSE,

	GLS_STENCILTEST_ENABLE				= (1 << 30),

	GLS_DEFAULT							= GLS_DEPTHMASK_TRUE
};
// *INDENT-ON*

enum
{
	ATTR_POSITION = BIT(0),
	ATTR_TEXCOORD = BIT(1),
	ATTR_LIGHTCOORD = BIT(2),
	ATTR_TANGENT = BIT(3),
	ATTR_BINORMAL = BIT(4),
	ATTR_NORMAL = BIT(5),
	ATTR_COLOR = BIT(6),
	ATTR_PAINTCOLOR = BIT(7),
	ATTR_LIGHTDIRECTION = BIT(8),
	ATTR_BONE_INDEXES = BIT(9),
	ATTR_BONE_WEIGHTS = BIT(10),

	ATTR_DEFAULT = ATTR_POSITION,
	ATTR_BITS =	ATTR_POSITION |
				ATTR_TEXCOORD |
				ATTR_LIGHTCOORD |
				ATTR_TANGENT |
				ATTR_BINORMAL |
				ATTR_NORMAL |
				ATTR_COLOR |
				ATTR_PAINTCOLOR |
				ATTR_LIGHTDIRECTION |
				ATTR_BONE_INDEXES |
				ATTR_BONE_WEIGHTS
};

// Tr3B - shaderProgram_t represents a pair of one
// GLSL vertex and one GLSL fragment shader
#if !defined(USE_D3D10)
typedef struct shaderProgram_s
{
	char            name[MAX_QPATH];

	GLhandleARB     program;
	uint32_t        attribs;	// vertex array attributes

	// uniform parameters
	GLint           u_ColorMap;
	GLint           u_CurrentMap;
	GLint           u_ContrastMap;
	GLint           u_DiffuseMap;
	GLint           u_NormalMap;
	GLint           u_SpecularMap;
	GLint           u_LightMap;
	GLint           u_DeluxeMap;
	GLint           u_DepthMap;
	GLint           u_DepthMapBack;
	GLint           u_DepthMapFront;
	GLint           u_PortalMap;
	GLint           u_AttenuationMapXY;
	GLint           u_AttenuationMapZ;
	GLint           u_ShadowMap;
	GLint           u_ShadowMap0;
	GLint           u_ShadowMap1;
	GLint           u_ShadowMap2;
	GLint           u_ShadowMap3;
	GLint           u_ShadowMap4;

	GLint           u_GrainMap;
	GLint           u_VignetteMap;

	GLint           u_ColorTextureMatrix;
	matrix_t		t_ColorTextureMatrix;

	GLint           u_DiffuseTextureMatrix;
	matrix_t		t_DiffuseTextureMatrix;

	GLint           u_NormalTextureMatrix;
	matrix_t		t_NormalTextureMatrix;

	GLint           u_SpecularTextureMatrix;
	matrix_t		t_SpecularTextureMatrix;

	GLint           u_AlphaTest;
	alphaTest_t		t_AlphaTest;

	GLint           u_ViewOrigin;
	vec3_t			t_ViewOrigin;

	GLint           u_TCGen_Environment;
	qboolean		t_TCGen_Environment;

	GLint           u_DeformGen;
	deformGen_t		t_DeformGen;

	GLint           u_DeformWave;
	vec4_t			t_DeformWave;

	GLint           u_DeformBulge;
	vec3_t			t_DeformBulge;

	GLint           u_DeformSpread;
	float			t_DeformSpread;

	GLint           u_ColorGen;
	colorGen_t		t_ColorGen;

	GLint           u_AlphaGen;
	alphaGen_t		t_AlphaGen;

	GLint           u_Color;
	vec4_t			t_Color;

	GLint           u_AmbientColor;
	vec3_t			t_AmbientColor;

	GLint           u_LightDir;
	vec3_t			t_LightDir;

	GLint           u_LightOrigin;
	vec3_t			t_LightOrigin;

	GLint           u_LightColor;
	vec3_t			t_LightColor;

	GLint           u_LightRadius;
	float			t_LightRadius;

	GLint           u_LightParallel;
	qboolean		t_LightParallel;

	GLint           u_LightScale;
	float			t_LightScale;

	GLint           u_LightWrapAround;
	float			t_LightWrapAround;

	GLint           u_LightAttenuationMatrix;
	matrix_t		t_LightAttenuationMatrix;

	GLint           u_LightFrustum;
	vec4_t			t_LightFrustum;


	GLint           u_ShadowMatrix;
	matrix_t		t_ShadowMatrix;

	GLint           u_ShadowCompare;
	qboolean		t_ShadowCompare;

	GLint           u_ShadowTexelSize;
	float			t_ShadowTexelSize;

	GLint           u_ShadowBlur;
	float			t_ShadowBlur;

	GLint			u_ShadowParallelSplitDistances;
	vec4_t			t_ShadowParallelSplitDistances;

	GLint           u_RefractionIndex;
	float			t_RefractionIndex;

	GLint           u_FresnelPower;
	GLint           u_FresnelScale;
	GLint           u_FresnelBias;

	GLint			u_NormalScale;

	GLint           u_EtaRatio;

	GLint           u_FogDensity;
	GLint           u_FogColor;

	GLint           u_SSAOJitter;
	GLint           u_SSAORadius;

	GLint			u_ParallaxMapping;
	qboolean		t_ParallaxMapping;

	GLint           u_DepthScale;
	float			t_DepthScale;


	GLint			u_PortalClipping;
	qboolean		t_PortalClipping;

	GLint			u_PortalPlane;
	vec4_t			t_PortalPlane;

	GLint           u_PortalRange;
	float			t_PortalRange;


	GLint			u_HDRKey;
	GLint			u_HDRAverageLuminance;
	GLint			u_HDRMaxLuminance;

	GLint           u_DeformMagnitude;
	GLint           u_BlurMagnitude;


	GLint           u_ModelMatrix;	// model -> world
	matrix_t		t_ModelMatrix;

	GLint           u_ViewMatrix;	// world -> camera
	matrix_t		t_ViewMatrix;

	GLint           u_ModelViewMatrix;	// model -> camera
	matrix_t		t_ModelViewMatrix;

	GLint           u_ModelViewMatrixTranspose;
	matrix_t		t_ModelViewMatrixTranspose;

	GLint           u_ProjectionMatrix;
	matrix_t		t_ProjectionMatrix;

	GLint           u_ProjectionMatrixTranspose;
	matrix_t		t_ProjectionMatrixTranspose;

	GLint           u_ModelViewProjectionMatrix;
	matrix_t		t_ModelViewProjectionMatrix;

	GLint           u_UnprojectMatrix;
	matrix_t		t_UnprojectMatrix;

	GLint           u_VertexSkinning;
	qboolean		t_VertexSkinning;

	GLint           u_BoneMatrix;

	GLint           u_Time;
	float			t_Time;
} shaderProgram_t;

//
// Tr3B: these are fire wall functions to avoid expensive redundant glUniform* calls
#define USE_UNIFORM_FIREWALL 1
#define LOG_GLSL_UNIFORMS 1

#if defined(LOG_GLSL_UNIFORMS)
extern cvar_t  *r_logFile;		// number of frames to emit GL logs
void            GLimp_LogComment(char *comment);
#endif

// *INDENT-OFF*

static ID_INLINE void GLSL_SetUniform_ColorTextureMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_ColorTextureMatrix, m))
		return;

	MatrixCopy(m, program->t_ColorTextureMatrix);
#endif

	qglUniformMatrix4fvARB(program->u_ColorTextureMatrix, 1, GL_FALSE, m);
}

static ID_INLINE void GLSL_SetUniform_DiffuseTextureMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_DiffuseTextureMatrix, m))
		return;

	MatrixCopy(m, program->t_DiffuseTextureMatrix);
#endif

	qglUniformMatrix4fvARB(program->u_DiffuseTextureMatrix, 1, GL_FALSE, m);
}

static ID_INLINE void GLSL_SetUniform_NormalTextureMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_NormalTextureMatrix, m))
		return;

	MatrixCopy(m, program->t_NormalTextureMatrix);
#endif

	qglUniformMatrix4fvARB(program->u_NormalTextureMatrix, 1, GL_FALSE, m);
}

static ID_INLINE void GLSL_SetUniform_SpecularTextureMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_SpecularTextureMatrix, m))
		return;

	MatrixCopy(m, program->t_SpecularTextureMatrix);
#endif

	qglUniformMatrix4fvARB(program->u_SpecularTextureMatrix, 1, GL_FALSE, m);
}


void            GLimp_LogComment(char *comment);

static ID_INLINE void GLSL_SetUniform_AlphaTest(shaderProgram_t * program, uint32_t stateBits)
{
	alphaTest_t			value;

	switch (stateBits & GLS_ATEST_BITS)
	{
		case GLS_ATEST_GT_0:
			value = ATEST_GT_0;
			break;

		case GLS_ATEST_LT_128:
			value = ATEST_LT_128;
			break;

		case GLS_ATEST_GE_128:
			value = ATEST_GE_128;
			break;

		default:
			value = ATEST_NONE;
			break;
	}

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- GLSL_SetUniformAlphaTest( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_AlphaTest == value)
		return;

	program->t_AlphaTest = value;
#endif

	qglUniform1iARB(program->u_AlphaTest, value);
}

static ID_INLINE void GLSL_SetUniform_ViewOrigin(shaderProgram_t * program, const vec3_t v)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(VectorCompare(program->t_ViewOrigin, v))
		return;

	VectorCopy(v, program->t_ViewOrigin);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ViewOrigin( program = %s, viewOrigin = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	qglUniform3fARB(program->u_ViewOrigin, v[0], v[1], v[2]);
}

static ID_INLINE void GLSL_SetUniform_TCGen_Environment(shaderProgram_t * program, qboolean value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_TCGen_Environment == value)
		return;

	program->t_TCGen_Environment = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_TCGen_Environment( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	qglUniform1iARB(program->u_TCGen_Environment, value);
}

static ID_INLINE void GLSL_SetUniform_DeformGen(shaderProgram_t * program, deformGen_t value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_DeformGen == value)
		return;

	program->t_DeformGen = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_DeformGen( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	qglUniform1iARB(program->u_DeformGen, value);
}

static ID_INLINE void GLSL_SetUniform_DeformWave(shaderProgram_t * program, const waveForm_t * wf)
{
	vec4_t v;

	Vector4Set(v, wf->base, wf->amplitude, wf->phase, wf->frequency);

#if defined(USE_UNIFORM_FIREWALL)
	if(Vector4Compare(program->t_DeformWave, v))
		return;

	Vector4Copy(v, program->t_DeformWave);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_DeformWave( program = %s, wave form = ( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2], v[3]));
	}
#endif

	qglUniform4fARB(program->u_DeformWave, v[0], v[1], v[2], v[3]);
}

static ID_INLINE void GLSL_SetUniform_DeformBulge(shaderProgram_t * program, const deformStage_t * ds)
{
	vec3_t v;

	VectorSet(v, ds->bulgeWidth, ds->bulgeHeight, ds->bulgeSpeed);

#if defined(USE_UNIFORM_FIREWALL)
	if(VectorCompare(program->t_DeformBulge, v))
		return;

	VectorCopy(v, program->t_DeformBulge);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_DeformBulge( program = %s, bulge = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	qglUniform3fARB(program->u_DeformBulge, v[0], v[1], v[2]);
}

static ID_INLINE void GLSL_SetUniform_DeformSpread(shaderProgram_t * program, float value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_DeformSpread == value)
		return;

	program->t_DeformSpread = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_DeformSpread( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	qglUniform1fARB(program->u_DeformSpread, value);
}

static ID_INLINE void GLSL_SetUniform_ColorGen(shaderProgram_t * program, colorGen_t value)
{
#if 0
	float			floatValue;

	switch (value)
	{
		case CGEN_VERTEX:
			floatValue = 1.0f;
			break;

		case CGEN_ONE_MINUS_VERTEX:
			floatValue = -1.0f;
			break;

		default:
			floatValue = 0.0f;
			break;
	}

#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_ColorGen == floatValue)
		return;

	program->t_ColorGen = floatValue;
#endif

	qglUniform1fARB(program->u_ColorGen, floatValue);
#else
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_ColorGen == value)
		return;

	program->t_ColorGen = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ColorGen( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	qglUniform1iARB(program->u_ColorGen, value);
#endif
}

static ID_INLINE void GLSL_SetUniform_AlphaGen(shaderProgram_t * program, alphaGen_t value)
{
#if 0
	float			floatValue;

	switch (value)
	{
		case AGEN_VERTEX:
			floatValue = 1.0f;
			break;

		case AGEN_ONE_MINUS_VERTEX:
			floatValue = -1.0f;
			break;

		default:
			floatValue = 0.0f;
			break;
	}

#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_AlphaGen == floatValue)
		return;

	program->t_AlphaGen = floatValue;
#endif

	qglUniform1fARB(program->u_AlphaGen, floatValue);

#else
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_AlphaGen == value)
		return;

	program->t_AlphaGen = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_AlphaGen( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	qglUniform1iARB(program->u_AlphaGen, value);
#endif
}

static ID_INLINE void GLSL_SetUniform_Color(shaderProgram_t * program, const vec4_t v)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(Vector4Compare(program->t_Color, v))
		return;

	Vector4Copy(v, program->t_Color);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_Color( program = %s, color = ( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2], v[3]));
	}
#endif

	qglUniform4fARB(program->u_Color, v[0], v[1], v[2], v[3]);
}

static ID_INLINE void GLSL_SetUniform_AmbientColor(shaderProgram_t * program, const vec3_t v)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(VectorCompare(program->t_AmbientColor, v))
		return;

	VectorCopy(v, program->t_AmbientColor);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_AmbientColor( program = %s, color = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	qglUniform3fARB(program->u_AmbientColor, v[0], v[1], v[2]);
}

static ID_INLINE void GLSL_SetUniform_LightDir(shaderProgram_t * program, const vec3_t v)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(VectorCompare(program->t_LightDir, v))
		return;

	VectorCopy(v, program->t_LightDir);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightDir( program = %s, direction = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	qglUniform3fARB(program->u_LightDir, v[0], v[1], v[2]);
}

static ID_INLINE void GLSL_SetUniform_LightOrigin(shaderProgram_t * program, const vec3_t v)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(VectorCompare(program->t_LightOrigin, v))
		return;

	VectorCopy(v, program->t_LightOrigin);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightOrigin( program = %s, origin = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	qglUniform3fARB(program->u_LightOrigin, v[0], v[1], v[2]);
}

static ID_INLINE void GLSL_SetUniform_LightColor(shaderProgram_t * program, const vec3_t v)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(VectorCompare(program->t_LightColor, v))
		return;

	VectorCopy(v, program->t_LightColor);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightColor( program = %s, color = ( %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2]));
	}
#endif

	qglUniform3fARB(program->u_LightColor, v[0], v[1], v[2]);
}

static ID_INLINE void GLSL_SetUniform_LightRadius(shaderProgram_t * program, float value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_LightRadius == value)
		return;

	program->t_LightRadius = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightRadius( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	qglUniform1fARB(program->u_LightRadius, value);
}

static ID_INLINE void GLSL_SetUniform_LightParallel(shaderProgram_t * program, qboolean value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_LightParallel == value)
		return;

	program->t_LightParallel = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightParallel( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	qglUniform1iARB(program->u_LightParallel, value);
}

static ID_INLINE void GLSL_SetUniform_LightScale(shaderProgram_t * program, float value)
{
#if 0
	if(DS_PREPASS_LIGHTING_ENABLED())
	{
		value -= (r_lightScale->value -1);
		value = Q_max(value, 0);
	}
#endif

#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_LightScale == value)
		return;

	program->t_LightScale = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightScale( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	qglUniform1fARB(program->u_LightScale, value);
}

static ID_INLINE void GLSL_SetUniform_LightWrapAround(shaderProgram_t * program, float value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_LightWrapAround == value)
		return;

	program->t_LightWrapAround = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_LightWrapAround( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	qglUniform1fARB(program->u_LightWrapAround, value);
}

static ID_INLINE void GLSL_SetUniform_LightAttenuationMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_LightAttenuationMatrix, m))
		return;

	MatrixCopy(m, program->t_LightAttenuationMatrix);
#endif

	qglUniformMatrix4fvARB(program->u_LightAttenuationMatrix, 1, GL_FALSE, m);
}

/*
static ID_INLINE void GLSL_SetUniform_LightFrustum(shaderProgram_t * program, vec4_t lightFrustum[6])
{
	vec4_t          lightFrustum[6];
#if 1
	if(memcmp(program->t_LightFrustum, m))
		return;
#endif

	qglUniform4fvARB(tr.deferredLightingShader_DBS_omni.u_LightFrustum, 6, &lightFrustum[0][0]);
}
*/


static ID_INLINE void GLSL_SetUniform_ShadowMatrix(shaderProgram_t * program, matrix_t m[MAX_SHADOWMAPS])
{
/*
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_ShadowMatrix[index], m))
		return;

	MatrixCopy(m, program->t_ShadowMatrix[index]);
#endif
*/

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		int			i;

		for(i = 0; i < MAX_SHADOWMAPS; i++)
		{
			GLimp_LogComment(va("--- GLSL_SetUniform_ShadowMatrix( program = %s, "
							"matrix(%i) = \n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
							program->name,
							i,
							m[i][0], m[i][4], m[i][8], m[i][12],
							m[i][1], m[i][5], m[i][9], m[i][13],
							m[i][2], m[i][6], m[i][10], m[i][14],
							m[i][3], m[i][7], m[i][11], m[i][15]));
		}
	}
#endif

	qglUniformMatrix4fvARB(program->u_ShadowMatrix, MAX_SHADOWMAPS, GL_FALSE, &m[0][0]);
}

static ID_INLINE void GLSL_SetUniform_ShadowCompare(shaderProgram_t * program, qboolean value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_ShadowCompare == value)
		return;

	program->t_ShadowCompare = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ShadowCompare( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	qglUniform1iARB(program->u_ShadowCompare, value);
}

static ID_INLINE void GLSL_SetUniform_ShadowTexelSize(shaderProgram_t * program, float value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_ShadowTexelSize == value)
		return;

	program->t_ShadowTexelSize = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ShadowTexelSize( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	qglUniform1fARB(program->u_ShadowTexelSize, value);
}

static ID_INLINE void GLSL_SetUniform_ShadowBlur(shaderProgram_t * program, float value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_ShadowBlur == value)
		return;

	program->t_ShadowBlur = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ShadowBlur( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	qglUniform1fARB(program->u_ShadowBlur, value);
}

static ID_INLINE void GLSL_SetUniform_ShadowParallelSplitDistances(shaderProgram_t * program, const vec4_t v)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(Vector4Compare(program->t_ShadowParallelSplitDistances, v))
		return;

	Vector4Copy(v, program->t_ShadowParallelSplitDistances);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ShadowParallelSplitDistances( program = %s, distances = ( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2], v[3]));
	}
#endif

	qglUniform4fARB(program->u_ShadowParallelSplitDistances, v[0], v[1], v[2], v[3]);
}

static ID_INLINE void GLSL_SetUniform_RefractionIndex(shaderProgram_t * program, float value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_RefractionIndex == value)
		return;

	program->t_RefractionIndex = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_RefractionIndex( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	qglUniform1fARB(program->u_RefractionIndex, value);
}

static ID_INLINE void GLSL_SetUniform_ParallaxMapping(shaderProgram_t * program, qboolean value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_ParallaxMapping == value)
		return;

	program->t_ParallaxMapping = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ParallaxMapping( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	qglUniform1iARB(program->u_ParallaxMapping, value);
}

static ID_INLINE void GLSL_SetUniform_DepthScale(shaderProgram_t * program, float value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_DepthScale == value)
		return;

	program->t_DepthScale = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_DepthScale( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	qglUniform1fARB(program->u_DepthScale, value);
}

static ID_INLINE void GLSL_SetUniform_PortalClipping(shaderProgram_t * program, qboolean value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_PortalClipping == value)
		return;

	program->t_PortalClipping = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_PortalClipping( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	qglUniform1iARB(program->u_PortalClipping, value);
}

static ID_INLINE void GLSL_SetUniform_PortalPlane(shaderProgram_t * program, const vec4_t v)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(Vector4Compare(program->t_PortalPlane, v))
		return;

	VectorCopy(v, program->t_PortalPlane);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_PortalPlane( program = %s, plane = ( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n", program->name, v[0], v[1], v[2], v[3]));
	}
#endif

	qglUniform4fARB(program->u_PortalPlane, v[0], v[1], v[2], v[3]);
}

static ID_INLINE void GLSL_SetUniform_PortalRange(shaderProgram_t * program, float value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_PortalRange == value)
		return;

	program->t_PortalRange = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_PortalRange( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	qglUniform1fARB(program->u_PortalRange, value);
}

static ID_INLINE void GLSL_SetUniform_ModelMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_ModelMatrix, m))
		return;

	MatrixCopy(m, program->t_ModelMatrix);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ModelMatrix( program = %s, "
							"matrix = \n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
							program->name,
							m[0], m[4], m[8], m[12],
							m[1], m[5], m[9], m[13],
							m[2], m[6], m[10], m[14],
							m[3], m[7], m[11], m[15]));
	}
#endif

	qglUniformMatrix4fvARB(program->u_ModelMatrix, 1, GL_FALSE, m);
}

static ID_INLINE void GLSL_SetUniform_ViewMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_ViewMatrix, m))
		return;

	MatrixCopy(m, program->t_ViewMatrix);
#endif

	qglUniformMatrix4fvARB(program->u_ViewMatrix, 1, GL_FALSE, m);
}

static ID_INLINE void GLSL_SetUniform_ModelViewMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_ModelViewMatrix, m))
		return;

	MatrixCopy(m, program->t_ModelViewMatrix);
#endif

	qglUniformMatrix4fvARB(program->u_ModelViewMatrix, 1, GL_FALSE, m);
}

static ID_INLINE void GLSL_SetUniform_ModelViewMatrixTranspose(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_ModelViewMatrixTranspose, m))
		return;

	MatrixCopy(m, program->t_ModelViewMatrixTranspose);
#endif

	qglUniformMatrix4fvARB(program->u_ModelViewMatrixTranspose, 1, GL_TRUE, m);
}

static ID_INLINE void GLSL_SetUniform_ProjectionMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_ProjectionMatrix, m))
		return;

	MatrixCopy(m, program->t_ProjectionMatrix);
#endif

	qglUniformMatrix4fvARB(program->u_ProjectionMatrix, 1, GL_FALSE, m);
}

static ID_INLINE void GLSL_SetUniform_ProjectionMatrixTranspose(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_ProjectionMatrixTranspose, m))
		return;

	MatrixCopy(m, program->t_ProjectionMatrixTranspose);
#endif

	qglUniformMatrix4fvARB(program->u_ProjectionMatrixTranspose, 1, GL_TRUE, m);
}

static ID_INLINE void GLSL_SetUniform_ModelViewProjectionMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_ModelViewProjectionMatrix, m))
		return;

	MatrixCopy(m, program->t_ModelViewProjectionMatrix);
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_ModelViewProjectionMatrix( program = %s, "
							"matrix = \n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f )\n"
							"( %5.3f, %5.3f, %5.3f, %5.3f ) ) ---\n",
							program->name,
							m[0], m[4], m[8], m[12],
							m[1], m[5], m[9], m[13],
							m[2], m[6], m[10], m[14],
							m[3], m[7], m[11], m[15]));
	}
#endif

	qglUniformMatrix4fvARB(program->u_ModelViewProjectionMatrix, 1, GL_FALSE, m);
}

static ID_INLINE void GLSL_SetUniform_UnprojectMatrix(shaderProgram_t * program, const matrix_t m)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(MatrixCompare(program->t_UnprojectMatrix, m))
		return;

	MatrixCopy(m, program->t_UnprojectMatrix);
#endif

	qglUniformMatrix4fvARB(program->u_UnprojectMatrix, 1, GL_FALSE, m);
}

static ID_INLINE void GLSL_SetUniform_VertexSkinning(shaderProgram_t * program, qboolean value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_VertexSkinning == value)
		return;

	program->t_VertexSkinning = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_VertexSkinning( program = %s, value = %i ) ---\n", program->name, value));
	}
#endif

	qglUniform1iARB(program->u_VertexSkinning, value);
}

static ID_INLINE void GLSL_SetUniform_Time(shaderProgram_t * program, float value)
{
#if defined(USE_UNIFORM_FIREWALL)
	if(program->t_Time == value)
		return;

	program->t_Time = value;
#endif

#if defined(LOG_GLSL_UNIFORMS)
	if(r_logFile->integer)
	{
		GLimp_LogComment(va("--- GLSL_SetUniform_Time( program = %s, value = %f ) ---\n", program->name, value));
	}
#endif

	qglUniform1fARB(program->u_Time, value);
}

// *INDENT-ON*

#endif // #if !defined(USE_D3D10)










// trRefdef_t holds everything that comes in refdef_t,
// as well as the locally generated scene information
typedef struct
{
	int             x, y, width, height;
	float           fov_x, fov_y;
	vec3_t          vieworg;
	vec3_t          viewaxis[3];	// transformation matrix

	int             time;		// time in milliseconds for shader effects and other time dependent rendering issues
	int             rdflags;	// RDF_NOWORLDMODEL, etc

	// 1 bits will prevent the associated area from rendering at all
	byte            areamask[MAX_MAP_AREA_BYTES];
	qboolean        areamaskModified;	// qtrue if areamask changed since last scene

	float           floatTime;	// tr.refdef.time / 1000.0

	// text messages for deform text shaders
	char            text[MAX_RENDER_STRINGS][MAX_RENDER_STRING_LENGTH];

	int             numEntities;
	trRefEntity_t  *entities;

	int             numLights;
	trRefLight_t   *lights;

	int             numPolys;
	struct srfPoly_s *polys;

	int             numDrawSurfs;
	struct drawSurf_s *drawSurfs;

	int             numInteractions;
	struct interaction_s *interactions;

	byte           *pixelTarget;		//set this to Non Null to copy to a buffer after scene rendering
	int             pixelTargetWidth;
	int             pixelTargetHeight;
} trRefdef_t;


//=================================================================================

// ydnar: decal projection
typedef struct decalProjector_s
{
	shader_t       *shader;
	byte            color[4];
	int             fadeStartTime, fadeEndTime;
	vec3_t          mins, maxs;
	vec3_t          center;
	float           radius, radius2;
	qboolean        omnidirectional;
	int             numPlanes;	// either 5 or 6, for quad or triangle projectors
	vec4_t          planes[6];
	vec4_t          texMat[3][2];
}
decalProjector_t;

typedef struct corona_s
{
	vec3_t          origin;
	vec3_t          color;		// range from 0.0 to 1.0, should be color normalized
	vec3_t          transformed;	// origin in local coordinate system
	float           scale;		// uses r_flaresize as the baseline (1.0)
	int             id;
	qboolean        visible;	// still send the corona request, even if not visible, for proper fading
} corona_t;


//=================================================================================


// skins allow models to be retextured without modifying the model file
typedef struct
{
	char            name[MAX_QPATH];
	shader_t       *shader;
} skinSurface_t;

//----(SA) modified
#define MAX_PART_MODELS 5

typedef struct
{
	char            type[MAX_QPATH];	// md3_lower, md3_lbelt, md3_rbelt, etc.
	char            model[MAX_QPATH];	// lower.md3, belt1.md3, etc.
	int             hash;
} skinModel_t;

typedef struct skin_s
{
	char            name[MAX_QPATH];	// game path, including extension
	int             numSurfaces;
	int             numModels;
	skinSurface_t  *surfaces[MD3_MAX_SURFACES];
	skinModel_t    *models[MAX_PART_MODELS];
} skin_t;

//----(SA) end


//=================================================================================

typedef struct
{
	orientationr_t  orientation;
	orientationr_t  world;

	vec3_t          pvsOrigin;	// may be different than or.origin for portals

	qboolean        isPortal;	// true if this view is through a portal
	qboolean        isMirror;	// the portal is a mirror, invert the face culling

	int             frameSceneNum;	// copied from tr.frameSceneNum
	int             frameCount;	// copied from tr.frameCount
	int				viewCount; // copied from tr.viewCount

	cplane_t        portalPlane;	// clip anything behind this if mirroring
	int             viewportX, viewportY, viewportWidth, viewportHeight;
	vec4_t          viewportVerts[4];	// for immediate 2D quad rendering

	float           fovX, fovY;
	matrix_t        projectionMatrix;
	matrix_t        unprojectionMatrix;	// transform pixel window space -> world space

	float			parallelSplitDistances[MAX_SHADOWMAPS + 1];	// distances in camera space

	frustum_t       frustums[MAX_SHADOWMAPS + 1];	// first frustum is the default one with complete zNear - zFar range
													// and the other ones are for PSSM

	vec3_t          visBounds[2];
	float           zNear;
	float           zFar;

	int             numDrawSurfs;
	struct drawSurf_s *drawSurfs;

	int             numInteractions;
	struct interaction_s *interactions;
} viewParms_t;


/*
==============================================================================

SURFACES

==============================================================================
*/

// any changes in surfaceType must be mirrored in rb_surfaceTable[]
typedef enum
{
	SF_BAD,
	SF_SKIP,					// ignore
	SF_FACE,
	SF_GRID,
	SF_TRIANGLES,
	SF_POLY,
	SF_MDV,

	SF_MDM,

#if defined(USE_REFENTITY_ANIMATIONSYSTEM)
	SF_MD5,
#endif

	SF_FLARE,
	SF_ENTITY,					// beams, rails, lightning, etc that can be determined by entity
	SF_VBO_MESH,

#if defined(USE_REFENTITY_ANIMATIONSYSTEM)
	SF_VBO_MD5MESH,
#endif

	SF_VBO_SHADOW_VOLUME,

	SF_NUM_SURFACE_TYPES,
	SF_MAX = 0x7fffffff			// ensures that sizeof( surfaceType_t ) == sizeof( int )
} surfaceType_t;

typedef struct drawSurf_s
{
	trRefEntity_t  *entity;
	uint32_t        shaderNum;
	int16_t         lightmapNum;

	surfaceType_t  *surface;	// any of surface*_t
} drawSurf_t;

typedef enum
{
	IA_DEFAULT,					// lighting and shadowing
	IA_SHADOWONLY,
	IA_LIGHTONLY
} interactionType_t;

// an interactionCache is a node between a light and a precached world surface
typedef struct interactionCache_s
{
	interactionType_t type;

	struct bspSurface_s *surface;

	byte            cubeSideBits;
	qboolean        redundant;
	qboolean		mergedIntoVBO;

	struct interactionCache_s *next;
} interactionCache_t;

typedef struct interactionVBO_s
{
	interactionType_t type;

	byte            cubeSideBits;

	struct shader_s *shader;
	struct srfVBOMesh_s *vboLightMesh;
	struct srfVBOMesh_s *vboShadowMesh;
	struct srfVBOShadowVolume_s *vboShadowVolume;	// only if cg_shadows 3

	struct interactionVBO_s *next;
} interactionVBO_t;

// an interaction is a node between a light and any surface
typedef struct interaction_s
{
	interactionType_t type;

	trRefLight_t   *light;

	trRefEntity_t  *entity;
	surfaceType_t  *surface;	// any of surface*_t
	shader_t       *surfaceShader;

	byte            cubeSideBits;

	int16_t         scissorX, scissorY, scissorWidth, scissorHeight;

	float           depthNear;	// for GL_EXT_depth_bounds_test
	float           depthFar;
	qboolean        noDepthBoundsTest;

	uint32_t        occlusionQuerySamples;	// visible fragment count
	qboolean        noOcclusionQueries;

	struct interaction_s *next;
} interaction_t;

typedef struct
{
	int             numDegenerated;	// number of bad triangles
	qboolean        degenerated[SHADER_MAX_TRIANGLES];

	qboolean        facing[SHADER_MAX_TRIANGLES];
	int             numFacing;	// number of triangles facing the light origin

	int             numIndexes;
	int             indexes[SHADER_MAX_INDEXES];
} shadowState_t;

extern shadowState_t shadowState;


#define	MAX_FACE_POINTS		64

#define	MAX_PATCH_SIZE		64	// max dimensions of a patch mesh in map file
#define	MAX_GRID_SIZE		65	// max dimensions of a grid mesh in memory

// when cgame directly specifies a polygon, it becomes a srfPoly_t
// as soon as it is called
typedef struct srfPoly_s
{
	surfaceType_t   surfaceType;
	qhandle_t       hShader;
	int16_t         numVerts;
	polyVert_t     *verts;
} srfPoly_t;

// ydnar: decals
#define MAX_DECAL_VERTS         10	// worst case is triangle clipped by 6 planes
#define MAX_WORLD_DECALS        1024
#define MAX_ENTITY_DECALS       128
typedef struct srfDecal_s
{
	surfaceType_t   surfaceType;
	int             numVerts;
	polyVert_t      verts[MAX_DECAL_VERTS];
}
srfDecal_t;

typedef struct srfFlare_s
{
	surfaceType_t   surfaceType;
	vec3_t          origin;
	vec3_t          normal;
	vec3_t          color;
} srfFlare_t;

typedef struct
{
	vec3_t          xyz;
	vec2_t          st;
	vec2_t          lightmap;
	vec3_t          tangent;
	vec3_t          binormal;
	vec3_t          normal;
	vec4_t          paintColor;
	vec4_t          lightColor;
	vec3_t          lightDirection;

#if DEBUG_OPTIMIZEVERTICES
	unsigned int    id;
#endif
} srfVert_t;

typedef struct
{
	int             indexes[3];
	int             neighbors[3];
	vec4_t          plane;
	qboolean        facingLight;
	qboolean        degenerated;
} srfTriangle_t;

typedef struct srfGridMesh_s
{
	surfaceType_t   surfaceType;

	// culling information
	vec3_t          meshBounds[2];
	vec3_t          localOrigin;
	float           meshRadius;

	// lod information, which may be different
	// than the culling information to allow for
	// groups of curves that LOD as a unit
	vec3_t          lodOrigin;
	float           lodRadius;
	int             lodFixed;
	int             lodStitched;

	// triangle definitions
	int             width, height;
	float          *widthLodError;
	float          *heightLodError;

	int             numTriangles;
	srfTriangle_t  *triangles;

	int             numVerts;
	srfVert_t      *verts;

	// BSP VBO offsets
	int             firstVert;
	int             firstTriangle;

	// static render data
	VBO_t          *vbo;		// points to bsp model VBO
	IBO_t          *ibo;
} srfGridMesh_t;

typedef struct
{
	surfaceType_t   surfaceType;

	// culling information
	cplane_t        plane;
	vec3_t          bounds[2];

	// triangle definitions
	int             numTriangles;
	srfTriangle_t  *triangles;

	int             numVerts;
	srfVert_t      *verts;

	// BSP VBO offsets
	int             firstVert;
	int             firstTriangle;

	// static render data
	VBO_t          *vbo;		// points to bsp model VBO
	IBO_t          *ibo;
} srfSurfaceFace_t;


// misc_models in maps are turned into direct geometry by xmap
typedef struct
{
	surfaceType_t   surfaceType;

	// culling information
	vec3_t          bounds[2];

	// triangle definitions
	int             numTriangles;
	srfTriangle_t  *triangles;

	int             numVerts;
	srfVert_t      *verts;

	// BSP VBO offsets
	int             firstVert;
	int             firstTriangle;

	// static render data
	VBO_t          *vbo;		// points to bsp model VBO
	IBO_t          *ibo;
} srfTriangles_t;

typedef struct srfVBOMesh_s
{
	surfaceType_t   surfaceType;

	struct shader_s *shader;	// FIXME move this to somewhere else
	int             lightmapNum;	// FIXME get rid of this by merging all lightmaps at level load

	// culling information
	vec3_t          bounds[2];

	// backEnd stats
	int             numIndexes;
	int             numVerts;

	// static render data
	VBO_t          *vbo;
	IBO_t          *ibo;
} srfVBOMesh_t;

typedef struct srfVBOMD5Mesh_s
{
	surfaceType_t   surfaceType;

	struct md5Model_s *md5Model;
	struct shader_s *shader;	// FIXME move this to somewhere else

	int				skinIndex;

	int				numBoneRemap;
	int				boneRemap[MAX_BONES];
	int				boneRemapInverse[MAX_BONES];

	// backEnd stats
	int             numIndexes;
	int             numVerts;

	// static render data
	VBO_t          *vbo;
	IBO_t          *ibo;
} srfVBOMD5Mesh_t;

typedef struct srfVBOShadowVolume_s
{
	surfaceType_t   surfaceType;

	// backEnd stats
	int             numIndexes;
	int             numVerts;

	// static render data
	VBO_t          *vbo;
	IBO_t          *ibo;
} srfVBOShadowVolume_t;


extern void     (*rb_surfaceTable[SF_NUM_SURFACE_TYPES]) (void *);

/*
==============================================================================
BRUSH MODELS - in memory representation
==============================================================================
*/
typedef struct bspSurface_s
{
	int             viewCount;	// if == tr.viewCount, already added
	int             lightCount;
	struct shader_s *shader;
	int16_t         lightmapNum;	// -1 = no lightmap

	surfaceType_t  *data;		// any of srf*_t
} bspSurface_t;

// ydnar: bsp model decal surfaces
typedef struct decal_s
{
	bspSurface_t   *parent;
	shader_t       *shader;
	float           fadeStartTime, fadeEndTime;
	int             fogIndex;
	int             numVerts;
	polyVert_t      verts[MAX_DECAL_VERTS];
}
decal_t;


#define	CONTENTS_NODE		-1
typedef struct bspNode_s
{
	// common with leaf and node
	int             contents;	// -1 for nodes, to differentiate from leafs
	int             visCounts[MAX_VISCOUNTS];	// node needs to be traversed if current
	int             lightCount;
	vec3_t          mins, maxs;	// for bounding box culling
	vec3_t			origin;		// center of the bounding box
	struct bspNode_s *parent;

	qboolean		visible[MAX_VIEWS];
	int				lastVisited[MAX_VIEWS];
	int				lastQueried[MAX_VIEWS];
	qboolean		issueOcclusionQuery[MAX_VIEWS];

	link_t			visChain;			// updated every visit
	link_t			occlusionQuery;		// updated every visit
	link_t			occlusionQuery2;	// updated every visit
	link_t			multiQuery;			// CHC++: list of all nodes that are used by the same occlusion query

	VBO_t          *volumeVBO;
	IBO_t          *volumeIBO;
	int				volumeVerts;
	int				volumeIndexes;

#if 1//!defined(USE_D3D10)
	uint32_t        occlusionQueryObjects[MAX_VIEWS];
	int             occlusionQuerySamples[MAX_VIEWS];	// visible fragment count
	int             occlusionQueryNumbers[MAX_VIEWS];	// for debugging
#endif

	// node specific
	cplane_t       *plane;
	struct bspNode_s *children[2];

	// leaf specific
	int             cluster;
	int             area;
	qboolean		sameAABBAsParent;

	int             numMarkSurfaces;
	bspSurface_t  **markSurfaces;
} bspNode_t;

typedef struct
{
	int             numMarkSurfaces;
	bspSurface_t  **markSurfaces;

	vec3_t			origin;		// used for cubemaps
} bspCluster_t;

/*
typedef struct
{
	int             numMarkSurfaces;
	bspSurface_t  **markSurfaces;

	int             numVBOSurfaces;
	srfVBOMesh_t  **vboSurfaces;
} bspArea_t;

typedef struct
{
	int             areas[2];

	vec3_t          points[4];
} bspAreaPortal_t;
*/

typedef struct
{
	vec3_t          bounds[2];	// for culling

	uint32_t        numSurfaces;
	bspSurface_t   *firstSurface;

	uint32_t        numVBOSurfaces;
	srfVBOMesh_t  **vboSurfaces;
} bspModel_t;

typedef struct
{
	vec4_t          ambient;
	vec4_t          directed;
	byte            latLong[2];
} bspGridPoint_t;

typedef struct
{
	char            name[MAX_QPATH];	// ie: maps/tim_dm2.bsp
	char            baseName[MAX_QPATH];	// ie: tim_dm2

	uint32_t        dataSize;

	int             numShaders;
	dshader_t      *shaders;

	int             numModels;
	bspModel_t     *models;

	int             numplanes;
	cplane_t       *planes;

	int             numnodes;	// includes leafs
	int             numDecisionNodes;
	bspNode_t      *nodes;

	int             numVerts;
	srfVert_t      *verts;
	int             redundantVertsCalculationNeeded;
	int            *redundantLightVerts;	// util to optimize IBOs
	int            *redundantShadowVerts;
	int            *redundantShadowAlphaTestVerts;
	VBO_t          *vbo;

	int             numTriangles;
	srfTriangle_t  *triangles;

//  int             numAreas;
//  bspArea_t      *areas;

//  int             numAreaPortals;
//  bspAreaPortal_t *areaPortals;

	int             numWorldSurfaces;

	int             numSurfaces;
	bspSurface_t   *surfaces;

	int             numMarkSurfaces;
	bspSurface_t  **markSurfaces;

	vec3_t          lightGridOrigin;
	vec3_t          lightGridSize;
	vec3_t          lightGridInverseSize;
	int             lightGridBounds[3];
	bspGridPoint_t *lightGridData;

	int             numLights;
	trRefLight_t   *lights;

	int             numInteractions;
	interactionCache_t **interactions;

	int             numClusters;
	bspCluster_t   *clusters;
	int             clusterBytes;
	const byte     *vis;		// may be passed in by CM_LoadMap to save space
	byte           *novis;		// clusterBytes of 0xff

	int             numClusterVBOSurfaces[MAX_VISCOUNTS];
	growList_t      clusterVBOSurfaces[MAX_VISCOUNTS];	// updated every time when changing the view cluster

	char           *entityString;
	char           *entityParsePoint;
} world_t;



/*
==============================================================================
MDV MODELS - meta format for vertex animation models like .md2, .md3, .mdc
==============================================================================
*/
typedef struct
{
	float           bounds[2][3];
	float           localOrigin[3];
	float           radius;
} mdvFrame_t;

typedef struct
{
	char            name[MAX_QPATH];	// tag name
	float           origin[3];
	float           axis[3][3];
} mdvTag_t;

typedef struct
{
	short           xyz[3];
} mdvVertex_t;

typedef struct
{
	float           st[2];
} mdvSt_t;

typedef struct
{
	surfaceType_t   surfaceType;

	char            name[MAX_QPATH];	// polyset name

	shader_t       *shader;

	int             numVerts;
	mdvVertex_t    *verts;
	mdvSt_t        *st;

	int             numTriangles;
	srfTriangle_t  *triangles;

	struct mdvModel_s *model;
} mdvSurface_t;

typedef struct mdvModel_s
{
	int             numFrames;
	mdvFrame_t     *frames;

	int             numTags;
	mdvTag_t       *tags;

	int             numSurfaces;
	mdvSurface_t   *surfaces;

	int             numVBOSurfaces;
	srfVBOMesh_t  **vboSurfaces;

	int             numSkins;
} mdvModel_t;


/*
==============================================================================
MD5 MODELS - in memory representation
==============================================================================
*/
#define MD5_IDENTSTRING     "MD5Version"
#define MD5_VERSION			10

typedef struct
{
	uint8_t         boneIndex;	// these are indexes into the boneReferences,
	float           boneWeight;	// not the global per-frame bone list
	vec3_t          offset;
} md5Weight_t;

typedef struct
{
	vec3_t          position;
	vec2_t          texCoords;
	vec3_t          tangent;
	vec3_t          binormal;
	vec3_t          normal;

	uint32_t        firstWeight;
	uint16_t        numWeights;
	md5Weight_t   **weights;
} md5Vertex_t;

/*
typedef struct
{
	int             indexes[3];
	int             neighbors[3];
} md5Triangle_t;
*/

typedef struct
{
	surfaceType_t   surfaceType;

//  char            name[MAX_QPATH];    // polyset name
	char            shader[MAX_QPATH];
	int             shaderIndex;	// for in-game use

	uint32_t        numVerts;
	md5Vertex_t    *verts;

	uint32_t        numTriangles;
	srfTriangle_t  *triangles;

	uint32_t        numWeights;
	md5Weight_t    *weights;

	struct md5Model_s *model;
} md5Surface_t;

typedef struct
{
	char            name[MAX_QPATH];
	int8_t          parentIndex;	// parent index (-1 if root)
	vec3_t          origin;
	quat_t          rotation;
	matrix_t        inverseTransform;	// full inverse for tangent space transformation
} md5Bone_t;

typedef struct md5Model_s
{
	uint16_t        numBones;
	md5Bone_t      *bones;

	uint16_t        numSurfaces;
	md5Surface_t   *surfaces;

	uint16_t        numVBOSurfaces;
	srfVBOMD5Mesh_t **vboSurfaces;

	vec3_t          bounds[2];
} md5Model_t;


typedef enum
{
	AT_BAD,
	AT_MD5,
	AT_PSA
} animType_t;

enum
{
	COMPONENT_BIT_TX = 1 << 0,
	COMPONENT_BIT_TY = 1 << 1,
	COMPONENT_BIT_TZ = 1 << 2,
	COMPONENT_BIT_QX = 1 << 3,
	COMPONENT_BIT_QY = 1 << 4,
	COMPONENT_BIT_QZ = 1 << 5
};

typedef struct
{
	char            name[MAX_QPATH];
	int8_t          parentIndex;

	uint8_t         componentsBits;	// e.g. (COMPONENT_BIT_TX | COMPONENT_BIT_TY | COMPONENT_BIT_TZ)
	uint16_t        componentsOffset;

	vec3_t          baseOrigin;
	quat_t          baseQuat;
} md5Channel_t;

typedef struct
{
	vec3_t          bounds[2];	// bounds of all surfaces of all LOD's for this frame
	float          *components;	// numAnimatedComponents many
} md5Frame_t;

typedef struct md5Animation_s
{
	uint16_t        numFrames;
	md5Frame_t     *frames;

	uint8_t         numChannels;	// same as numBones in model
	md5Channel_t   *channels;

	int16_t        frameRate;

	uint32_t        numAnimatedComponents;
} md5Animation_t;


typedef struct
{
	axAnimationInfo_t info;

	int				  numBones;
	axReferenceBone_t *bones;

	int				  numKeys;
	axAnimationKey_t *keys;
} psaAnimation_t;

typedef struct
{
	char            name[MAX_QPATH];	// game path, including extension
	animType_t      type;
	int             index;		// anim = tr.animations[anim->index]

	md5Animation_t *md5;
	psaAnimation_t *psa;

} skelAnimation_t;


//======================================================================

typedef enum
{
	MOD_BAD,
	MOD_BSP,
	MOD_MESH,
	MOD_MDM,
	MOD_MDX,
	MOD_MD5
} modtype_t;

typedef struct model_s
{
	char            name[MAX_QPATH];
	modtype_t       type;
	int             index;		// model = tr.models[model->index]

	int             dataSize;	// just for listing purposes
	bspModel_t     *bsp;		// only if type == MOD_BSP
	mdvModel_t     *mdv[MD3_MAX_LODS];	// only if type == MOD_MESH
	mdmHeader_t    *mdm;		// only if type == MOD_MDM
	mdxHeader_t    *mdx;		// only if type == MOD_MDX
	md5Model_t     *md5;		// only if type == MOD_MD5

	int             numLods;
} model_t;

void            R_ModelInit(void);
model_t        *R_GetModelByHandle(qhandle_t hModel);

int             RE_LerpTag(orientation_t * tag, const refEntity_t * refent, const char *tagNameIn, int startIndex);

int             RE_BoneIndex(qhandle_t hModel, const char *boneName);


void            R_ModelBounds(qhandle_t handle, vec3_t mins, vec3_t maxs);

void            R_Modellist_f(void);

//====================================================
extern refimport_t ri;

#define	MAX_MOD_KNOWN			1024
#define	MAX_ANIMATIONFILES		4096

#define	MAX_LIGHTMAPS			256
#define	MAX_SKINS				1024

#define	MAX_DRAWSURFS			0x10000
#define	DRAWSURF_MASK			(MAX_DRAWSURFS-1)

#define MAX_INTERACTIONS		MAX_DRAWSURFS*8
#define INTERACTION_MASK		(MAX_INTERACTIONS-1)

extern int      gl_filter_min, gl_filter_max;

/*
** performanceCounters_t
*/
typedef struct
{
	int             c_sphere_cull_patch_in, c_sphere_cull_patch_clip, c_sphere_cull_patch_out;
	int             c_box_cull_patch_in, c_box_cull_patch_clip, c_box_cull_patch_out;
	int             c_sphere_cull_mdx_in, c_sphere_cull_mdx_clip, c_sphere_cull_mdx_out;
	int             c_box_cull_mdx_in, c_box_cull_mdx_clip, c_box_cull_mdx_out;
	int             c_box_cull_md5_in, c_box_cull_md5_clip, c_box_cull_md5_out;
	int             c_box_cull_light_in, c_box_cull_light_clip, c_box_cull_light_out;
	int             c_pvs_cull_light_out;

	int             c_pyramidTests;
	int             c_pyramid_cull_ent_in, c_pyramid_cull_ent_clip, c_pyramid_cull_ent_out;

	int				c_nodes;
	int             c_leafs;

	int             c_slights;
	int             c_slightSurfaces;
	int             c_slightInteractions;

	int             c_dlights;
	int             c_dlightSurfaces;
	int             c_dlightSurfacesCulled;
	int             c_dlightInteractions;

	int             c_depthBoundsTests, c_depthBoundsTestsRejected;

	int				c_occlusionQueries;
	int				c_occlusionQueriesMulti;
	int				c_occlusionQueriesSaved;
	int             c_CHCTime;
} frontEndCounters_t;

#define FUNCTABLE_SIZE		1024
#define FUNCTABLE_SIZE2		10
#define FUNCTABLE_MASK		(FUNCTABLE_SIZE-1)


#define MAX_GLSTACK			5

// the renderer front end should never modify glState_t or dxGlobals_t
#if defined(USE_D3D10)
typedef struct
{
	D3D10_DRIVER_TYPE       driverType;// = D3D10_DRIVER_TYPE_NULL;
	ID3D10Device*           d3dDevice;
	IDXGISwapChain*         swapChain;
	ID3D10RenderTargetView* renderTargetView;

	ID3D10Effect*			genericEffect;
	ID3D10EffectTechnique*	genericTechnique;

	ID3D10InputLayout*      vertexLayout;
	ID3D10Buffer*           vertexBuffer;
}
dxGlobals_t;
#else
typedef struct
{
	int				blendSrc, blendDst;
	float			clearColorRed, clearColorGreen, clearColorBlue, clearColorAlpha;
	double			clearDepth;
	int				clearStencil;
	int				colorMaskRed, colorMaskGreen, colorMaskBlue, colorMaskAlpha;
	int				cullFace;
	int				depthFunc;
	int				depthMask;
	int				drawBuffer;
	int				frontFace;
	int				polygonFace, polygonMode;
	int             scissorX, scissorY, scissorWidth, scissorHeight;
	int             viewportX, viewportY, viewportWidth, viewportHeight;

	int             currenttextures[32];
	int             currenttmu;
//  matrix_t        textureMatrix[32];

	int             stackIndex;
//  matrix_t        modelMatrix[MAX_GLSTACK];
//  matrix_t        viewMatrix[MAX_GLSTACK];
	matrix_t        modelViewMatrix[MAX_GLSTACK];
	matrix_t        projectionMatrix[MAX_GLSTACK];
	matrix_t        modelViewProjectionMatrix[MAX_GLSTACK];

	qboolean        finishCalled;
	int             faceCulling;	// FIXME redundant cullFace
	uint32_t        glStateBits;
	uint32_t		vertexAttribsState;
	uint32_t		vertexAttribPointersSet;
	shaderProgram_t *currentProgram;
	FBO_t          *currentFBO;
	VBO_t          *currentVBO;
	IBO_t          *currentIBO;
} glstate_t;
#endif // !defined(USE_D3D10)

typedef struct
{
	int             c_views;
	int             c_portals;
	int             c_batches;
	int             c_surfaces;
	int             c_vertexes;
	int             c_indexes;
	int             c_drawElements;
	float           c_overDraw;
	int             c_vboVertexBuffers;
	int             c_vboIndexBuffers;
	int             c_vboVertexes;
	int             c_vboIndexes;

	int             c_flareAdds;
	int             c_flareTests;
	int             c_flareRenders;

	int             c_occlusionQueries;
	int				c_occlusionQueriesMulti;
	int				c_occlusionQueriesSaved;
	int             c_occlusionQueriesAvailable;
	int             c_occlusionQueriesLightsCulled;
	int             c_occlusionQueriesLeafsCulled;
	int             c_occlusionQueriesInteractionsCulled;
	int             c_occlusionQueriesResponseTime;
	int             c_occlusionQueriesFetchTime;

	int             c_forwardAmbientTime;
	int             c_forwardLightingTime;
	int             c_forwardTranslucentTime;

	int             c_deferredGBufferTime;
	int             c_deferredLightingTime;

	int             msec;		// total msec for backend run
} backEndCounters_t;

// all state modified by the back end is seperated
// from the front end state
typedef struct
{
	int             smpFrame;
	trRefdef_t      refdef;
	viewParms_t     viewParms;
	orientationr_t  orientation;
	backEndCounters_t pc;
	qboolean        isHyperspace;
	trRefEntity_t  *currentEntity;
	trRefLight_t   *currentLight;	// only used when lighting interactions
	qboolean        skyRenderedThisView;	// flag for drawing sun

	float			hdrAverageLuminance;
	float			hdrMaxLuminance;
	float			hdrTime;

	qboolean        projection2D;	// if qtrue, drawstretchpic doesn't need to change modes
	vec4_t          color2D;
	qboolean        vertexes2D;	// shader needs to be finished
	trRefEntity_t   entity2D;	// currentEntity will point at this when doing 2D rendering
} backEndState_t;

typedef struct
{
	vec3_t          origin;
	image_t        *cubemap;
} cubemapProbe_t;


/*
** trGlobals_t
**
** Most renderer globals are defined here.
** backend functions should never modify any of these fields,
** but may read fields that aren't dynamically modified
** by the frontend.
*/
typedef struct
{
	qboolean        registered;	// cleared at shutdown, set at beginRegistration

	int             visIndex;
	int             visClusters[MAX_VISCOUNTS];
	int             visCounts[MAX_VISCOUNTS];	// incremented every time a new vis cluster is entered

	link_t			traversalStack;
	link_t			occlusionQueryQueue;
	link_t			occlusionQueryList;

	int             frameCount;	// incremented every frame
	int             sceneCount;	// incremented every scene
	int             viewCount;	// incremented every view (twice a scene if portaled)
	int				viewCountNoReset;
	int             lightCount;	// incremented every time a dlight traverses the world
	// and every R_MarkFragments call

	int             smpFrame;	// toggles from 0 to 1 every endFrame

	int             frameSceneNum;	// zeroed at RE_BeginFrame

	qboolean        worldMapLoaded;
	qboolean        worldDeluxeMapping;
	qboolean		worldHDR_RGBE;
	world_t        *world;

	const byte     *externalVisData;	// from RE_SetWorldVisData, shared with CM_Load

	image_t        *defaultImage;
	image_t        *scratchImage[32];
	image_t        *quadraticImage;
	image_t        *whiteImage;	// full of 0xff
	image_t        *blackImage;	// full of 0x0
	image_t        *flatImage;	// use this as default normalmap
	image_t        *noFalloffImage;
	image_t        *attenuationXYImage;
	image_t        *blackCubeImage;
	image_t        *autoCubeImage;			// special pointer to the nearest cubemap probe

	image_t        *contrastRenderFBOImage;
	image_t        *bloomRenderFBOImage[2];
	image_t        *currentRenderImage;
	image_t        *depthRenderImage;
	image_t        *portalRenderImage;

	image_t        *deferredDiffuseFBOImage;
	image_t        *deferredNormalFBOImage;
	image_t        *deferredSpecularFBOImage;
	image_t        *deferredRenderFBOImage;
	image_t        *lightRenderFBOImage;
	image_t        *occlusionRenderFBOImage;
	image_t        *depthToColorBackFacesFBOImage;
	image_t        *depthToColorFrontFacesFBOImage;
	image_t        *downScaleFBOImage_quarter;
	image_t        *downScaleFBOImage_64x64;
//	image_t        *downScaleFBOImage_16x16;
//	image_t        *downScaleFBOImage_4x4;
//	image_t        *downScaleFBOImage_1x1;
	image_t        *shadowMapFBOImage[MAX_SHADOWMAPS];
	image_t        *shadowCubeFBOImage[MAX_SHADOWMAPS];

	// external images
	image_t        *charsetImage;
	image_t        *grainImage;
	image_t        *vignetteImage;

	// framebuffer objects
	FBO_t          *geometricRenderFBO;		// is the G-Buffer for deferred shading
	FBO_t          *lightRenderFBO;			// is the light buffer which contains all light properties of the light pre pass
	FBO_t          *deferredRenderFBO;		// is used by HDR rendering and deferred shading
	FBO_t          *portalRenderFBO;		// holds a copy of the last currentRender that was rendered into a FBO
	FBO_t          *occlusionRenderFBO;		// used for overlapping visibility determination
	FBO_t          *downScaleFBO_quarter;
	FBO_t          *downScaleFBO_64x64;
//	FBO_t          *downScaleFBO_16x16;
//	FBO_t          *downScaleFBO_4x4;
//	FBO_t          *downScaleFBO_1x1;
	FBO_t          *contrastRenderFBO;
	FBO_t          *bloomRenderFBO[2];
	FBO_t          *shadowMapFBO[MAX_SHADOWMAPS];

	// internal shaders
	shader_t       *defaultShader;
	shader_t       *defaultPointLightShader;
	shader_t       *defaultProjectedLightShader;
	shader_t       *defaultDynamicLightShader;

	// external shaders
	shader_t       *projectionShadowShader;
	shader_t       *flareShader;
	shader_t       *sunShader;

	growList_t      lightmaps;
	growList_t      deluxemaps;

	image_t        *fatLightmap;
	int				fatLightmapSize;
	int				fatLightmapStep;

	// render entities
	trRefEntity_t  *currentEntity;
	trRefEntity_t   worldEntity;	// point currentEntity at this when rendering world
	model_t        *currentModel;

	// render lights
	trRefLight_t   *currentLight;

	//
	// GPU shader programs
	//
#if !defined(USE_D3D10)
	// Q3A standard simple vertex color rendering
	shaderProgram_t genericSingleShader;

	// simple vertex color shading for entities
	shaderProgram_t vertexLightingShader_DBS_entity;

	// simple vertex color shading for the world
	shaderProgram_t vertexLightingShader_DBS_world;

	// standard light mapping
	shaderProgram_t lightMappingShader;

	// directional light mapping
	shaderProgram_t deluxeMappingShader;

	// deferred Geometric-Buffer processing
	shaderProgram_t geometricFillShader_DBS;

	// deferred lighting
	shaderProgram_t deferredLightingShader_DBS_omni;
	shaderProgram_t deferredLightingShader_DBS_proj;
	shaderProgram_t deferredLightingShader_DBS_directional;

	// black depth fill rendering with textures
	shaderProgram_t depthFillShader;

	// colored depth test rendering with textures into gl_FragData[1]
	shaderProgram_t depthTestShader;

	// depth to color encoding
	shaderProgram_t depthToColorShader;

	// stencil shadow volume extrusion
	shaderProgram_t shadowExtrudeShader;

	// shadowmap distance compression
	shaderProgram_t shadowFillShader;

	// Doom3 style omni-directional multi-pass lighting
	shaderProgram_t forwardLightingShader_DBS_omni;
	shaderProgram_t forwardLightingShader_DBS_proj;
	shaderProgram_t forwardLightingShader_DBS_directional;

	// forward shading using the pre pass light buffer
	shaderProgram_t forwardLightingShader_DBS_post;

#ifdef VOLUMETRIC_LIGHTING
	// volumetric lighting
	shaderProgram_t lightVolumeShader_omni;
#endif

	// UT3 style player shadowing
	shaderProgram_t deferredShadowingShader_proj;

	// environment mapping effects
	shaderProgram_t reflectionShader_C;
	shaderProgram_t reflectionShader_CB;
	shaderProgram_t refractionShader_C;
	shaderProgram_t dispersionShader_C;
	shaderProgram_t skyBoxShader;

	// post process effects
	shaderProgram_t heatHazeShader;
	shaderProgram_t bloomShader;
	shaderProgram_t contrastShader;
	shaderProgram_t blurXShader;
	shaderProgram_t blurYShader;
	shaderProgram_t rotoscopeShader;
	shaderProgram_t cameraEffectsShader;
	shaderProgram_t screenShader;
	shaderProgram_t portalShader;
	shaderProgram_t liquidShader;
	shaderProgram_t uniformFogShader;
	shaderProgram_t volumetricFogShader;
#ifdef EXPERIMENTAL
	shaderProgram_t screenSpaceAmbientOcclusionShader;
#endif
#ifdef EXPERIMENTAL
	shaderProgram_t depthOfFieldShader;
#endif
	shaderProgram_t toneMappingShader;
	shaderProgram_t debugShadowMapShader;

#endif // !defined(USE_D3D10)

	// -----------------------------------------

	viewParms_t     viewParms;

	float           identityLight;	// 1.0 / ( 1 << overbrightBits )
	int             overbrightBits;	// r_overbrightBits->integer, but set to 0 if no hw gamma

	orientationr_t  orientation;			// for current entity

	trRefdef_t      refdef;

	vec3_t          sunLight;	// from the sky shader for this level
	vec3_t          sunDirection;

	vec3_t          fogColor;
	float           fogDensity;

	frontEndCounters_t pc;
	int             frontEndMsec;	// not in pc due to clearing issue

	//
	// put large tables at the end, so most elements will be
	// within the +/32K indexed range on risc processors
	//
	model_t        *models[MAX_MOD_KNOWN];
	int             numModels;

	int             numAnimations;
	skelAnimation_t *animations[MAX_ANIMATIONFILES];

	growList_t      images;

	int             numFBOs;
	FBO_t          *fbos[MAX_FBOS];

	growList_t      vbos;
	growList_t      ibos;

	byte           *cubeTemp[6];	// 6 textures for cubemap storage
	growList_t		cubeProbes;		// all cubemaps in a linear growing list
	vertexHash_t  **cubeHashTable;	// hash table for faster access

	// shader indexes from other modules will be looked up in tr.shaders[]
	// shader indexes from drawsurfs will be looked up in sortedShaders[]
	// lower indexed sortedShaders must be rendered first (opaque surfaces before translucent)
	int             numShaders;
	shader_t       *shaders[MAX_SHADERS];
	shader_t       *sortedShaders[MAX_SHADERS];

	int             numSkins;
	skin_t         *skins[MAX_SKINS];

	int             numTables;
	shaderTable_t  *shaderTables[MAX_SHADER_TABLES];

	float           sinTable[FUNCTABLE_SIZE];
	float           squareTable[FUNCTABLE_SIZE];
	float           triangleTable[FUNCTABLE_SIZE];
	float           sawToothTable[FUNCTABLE_SIZE];
	float           inverseSawToothTable[FUNCTABLE_SIZE];

	uint32_t        occlusionQueryObjects[MAX_OCCLUSION_QUERIES];
	int				numUsedOcclusionQueryObjects;
} trGlobals_t;

extern const matrix_t quakeToOpenGLMatrix;
extern const matrix_t openGLToQuakeMatrix;
extern const matrix_t quakeToD3DMatrix;
extern const matrix_t flipZMatrix;
extern int      shadowMapResolutions[5];

extern backEndState_t backEnd;
extern trGlobals_t tr;
extern glconfig_t glConfig;		// outside of TR since it shouldn't be cleared during ref re-init
extern glconfig2_t glConfig2;

#if defined(USE_D3D10)
extern dxGlobals_t dx;
#else
extern glstate_t glState;		// outside of TR since it shouldn't be cleared during ref re-init
#endif

extern float    displayAspect;	// FIXME


//
// cvars
//
extern cvar_t  *r_flares;		// light flares
extern cvar_t  *r_flareSize;
extern cvar_t  *r_flareFade;

extern cvar_t  *r_railWidth;
extern cvar_t  *r_railCoreWidth;
extern cvar_t  *r_railSegmentLength;

extern cvar_t  *r_ignore;		// used for debugging anything
extern cvar_t  *r_verbose;		// used for verbose debug spew

extern cvar_t  *r_znear;		// near Z clip plane
extern cvar_t  *r_zfar;

extern cvar_t  *r_stencilbits;	// number of desired stencil bits
extern cvar_t  *r_depthbits;	// number of desired depth bits
extern cvar_t  *r_colorbits;	// number of desired color bits, only relevant for fullscreen
extern cvar_t  *r_stereo;		// desired pixelformat stereo flag

extern cvar_t  *r_measureOverdraw;	// enables stencil buffer overdraw measurement

extern cvar_t  *r_lodbias;		// push/pull LOD transitions
extern cvar_t  *r_lodscale;

extern cvar_t  *r_forceFog;
extern cvar_t  *r_noFog;

extern cvar_t  *r_forceAmbient;
extern cvar_t  *r_ambientScale;
extern cvar_t  *r_lightScale;
extern cvar_t  *r_debugLight;

extern cvar_t  *r_inGameVideo;	// controls whether in game video should be draw
extern cvar_t  *r_fastsky;		// controls whether sky should be cleared or drawn
extern cvar_t  *r_drawSun;		// controls drawing of sun quad
extern cvar_t  *r_noDynamicLighting;	// dynamic lights enabled/disabled
extern cvar_t  *r_noStaticLighting;	// dynamic lights enabled/disabled
extern cvar_t  *r_dynamicLightsCastShadows;
extern cvar_t  *r_precomputedLighting;
extern cvar_t  *r_vertexLighting;
extern cvar_t  *r_compressDiffuseMaps;
extern cvar_t  *r_compressSpecularMaps;
extern cvar_t  *r_compressNormalMaps;
extern cvar_t  *r_heatHazeFix;
extern cvar_t  *r_noMarksOnTrisurfs;

extern cvar_t  *r_norefresh;	// bypasses the ref rendering
extern cvar_t  *r_drawentities;	// disable/enable entity rendering
extern cvar_t  *r_drawworld;	// disable/enable world rendering
extern cvar_t  *r_drawpolies;	// disable/enable world rendering
extern cvar_t  *r_speeds;		// various levels of information display
extern cvar_t  *r_novis;		// disable/enable usage of PVS
extern cvar_t  *r_nocull;
extern cvar_t  *r_facePlaneCull;	// enables culling of planar surfaces with back side test
extern cvar_t  *r_nocurves;
extern cvar_t  *r_nobatching;
extern cvar_t  *r_noLightScissors;
extern cvar_t  *r_noLightVisCull;
extern cvar_t  *r_noInteractionSort;
extern cvar_t  *r_showcluster;

extern cvar_t  *r_mode;			// video mode
extern cvar_t  *r_fullscreen;
extern cvar_t  *r_gamma;
extern cvar_t  *r_displayRefresh;	// optional display refresh option
extern cvar_t  *r_ignorehwgamma;	// overrides hardware gamma capabilities

extern cvar_t  *r_ext_texture_compression;	// these control use of specific extensions
extern cvar_t  *r_ext_occlusion_query;
extern cvar_t  *r_ext_texture_non_power_of_two;
extern cvar_t  *r_ext_draw_buffers;
extern cvar_t  *r_ext_vertex_array_object;
extern cvar_t  *r_ext_half_float_pixel;
extern cvar_t  *r_ext_texture_float;
extern cvar_t  *r_ext_stencil_wrap;
extern cvar_t  *r_ext_texture_filter_anisotropic;
extern cvar_t  *r_ext_stencil_two_side;
extern cvar_t  *r_ext_separate_stencil;
extern cvar_t  *r_ext_depth_bounds_test;
extern cvar_t  *r_ext_framebuffer_object;
extern cvar_t  *r_ext_packed_depth_stencil;
extern cvar_t  *r_ext_framebuffer_blit;
extern cvar_t  *r_extx_framebuffer_mixed_formats;
extern cvar_t  *r_ext_generate_mipmap;

extern cvar_t  *r_nobind;		// turns off binding to appropriate textures
extern cvar_t  *r_collapseStages;
extern cvar_t  *r_singleShader;	// make most world faces use default shader
extern cvar_t  *r_roundImagesDown;
extern cvar_t  *r_colorMipLevels;	// development aid to see texture mip usage
extern cvar_t  *r_picmip;		// controls picmip values
extern cvar_t  *r_finish;
extern cvar_t  *r_drawBuffer;
extern cvar_t  *r_glDriver;
extern cvar_t  *r_swapInterval;
extern cvar_t  *r_textureMode;
extern cvar_t  *r_offsetFactor;
extern cvar_t  *r_offsetUnits;
extern cvar_t  *r_forceSpecular;
extern cvar_t  *r_specularExponent;
extern cvar_t  *r_specularScale;
extern cvar_t  *r_normalScale;
extern cvar_t  *r_normalMapping;
extern cvar_t  *r_wrapAroundLighting;
extern cvar_t  *r_halfLambertLighting;

extern cvar_t  *r_uiFullScreen;	// ui is running fullscreen

extern cvar_t  *r_logFile;		// number of frames to emit GL logs

extern cvar_t  *r_clear;		// force screen clear every frame

extern cvar_t  *r_shadows;		// controls shadows: 0 = none, 1 = blur, 2 = black planar projection,

								// 3 = stencil shadow volumes
								// 4 = shadow mapping
extern cvar_t  *r_softShadows;
extern cvar_t  *r_shadowBlur;
extern cvar_t  *r_shadowMapQuality;
extern cvar_t  *r_shadowMapSizeUltra;
extern cvar_t  *r_shadowMapSizeVery;
extern cvar_t  *r_shadowMapSizeHigh;
extern cvar_t  *r_shadowMapSizeMedium;
extern cvar_t  *r_shadowMapSizeLow;
extern cvar_t  *r_shadowOffsetFactor;
extern cvar_t  *r_shadowOffsetUnits;
extern cvar_t  *r_shadowLodBias;
extern cvar_t  *r_shadowLodScale;
extern cvar_t  *r_noShadowPyramids;
extern cvar_t  *r_cullShadowPyramidFaces;
extern cvar_t  *r_cullShadowPyramidCurves;
extern cvar_t  *r_cullShadowPyramidTriangles;
extern cvar_t  *r_debugShadowMaps;
extern cvar_t  *r_noShadowFrustums;
extern cvar_t  *r_noLightFrustums;
extern cvar_t  *r_shadowMapLuminanceAlpha;
extern cvar_t  *r_shadowMapLinearFilter;
extern cvar_t  *r_lightBleedReduction;
extern cvar_t  *r_overDarkeningFactor;
extern cvar_t  *r_shadowMapDepthScale;
extern cvar_t  *r_parallelShadowSplits;
extern cvar_t  *r_parallelShadowSplitWeight;
extern cvar_t  *r_lightSpacePerspectiveWarping;

extern cvar_t  *r_intensity;

extern cvar_t  *r_lockpvs;
extern cvar_t  *r_noportals;
extern cvar_t  *r_portalOnly;

extern cvar_t  *r_subdivisions;
extern cvar_t  *r_stitchCurves;

extern cvar_t  *r_smp;
extern cvar_t  *r_showSmp;
extern cvar_t  *r_skipBackEnd;
extern cvar_t  *r_skipLightBuffer;

extern cvar_t  *r_ignoreGLErrors;

extern cvar_t  *r_overBrightBits;
extern cvar_t  *r_mapOverBrightBits;

extern cvar_t  *r_debugSurface;
extern cvar_t  *r_simpleMipMaps;

extern cvar_t  *r_showImages;
extern cvar_t  *r_debugSort;

extern cvar_t  *r_printShaders;

extern cvar_t  *r_maxPolys;
extern cvar_t  *r_maxPolyVerts;

extern cvar_t  *r_showTris;		// enables wireframe rendering of the world
extern cvar_t  *r_showSky;		// forces sky in front of all surfaces
extern cvar_t  *r_showShadowVolumes;
extern cvar_t  *r_showShadowLod;
extern cvar_t  *r_showShadowMaps;
extern cvar_t  *r_showSkeleton;
extern cvar_t  *r_showEntityTransforms;
extern cvar_t  *r_showLightTransforms;
extern cvar_t  *r_showLightInteractions;
extern cvar_t  *r_showLightScissors;
extern cvar_t  *r_showLightBatches;
extern cvar_t  *r_showOcclusionQueries;
extern cvar_t  *r_showBatches;
extern cvar_t  *r_showLightMaps;	// render lightmaps only
extern cvar_t  *r_showDeluxeMaps;
extern cvar_t  *r_showAreaPortals;
extern cvar_t  *r_showCubeProbes;
extern cvar_t  *r_showBspNodes;
extern cvar_t  *r_showParallelShadowSplits;

extern cvar_t  *r_showDeferredDiffuse;
extern cvar_t  *r_showDeferredNormal;
extern cvar_t  *r_showDeferredSpecular;
extern cvar_t  *r_showDeferredPosition;
extern cvar_t  *r_showDeferredRender;
extern cvar_t  *r_showDeferredLight;

extern cvar_t  *r_vboFaces;
extern cvar_t  *r_vboCurves;
extern cvar_t  *r_vboTriangles;
extern cvar_t  *r_vboShadows;
extern cvar_t  *r_vboLighting;
extern cvar_t  *r_vboDynamicLighting;
extern cvar_t  *r_vboModels;
extern cvar_t  *r_vboOptimizeVertices;
extern cvar_t  *r_vboVertexSkinning;
extern cvar_t  *r_vboSmoothNormals;

extern cvar_t  *r_mergeClusterSurfaces;
extern cvar_t  *r_mergeClusterFaces;
extern cvar_t  *r_mergeClusterCurves;
extern cvar_t  *r_mergeClusterTriangles;

extern cvar_t  *r_deferredShading;
extern cvar_t  *r_parallaxMapping;
extern cvar_t  *r_parallaxDepthScale;

extern cvar_t  *r_dynamicBspOcclusionCulling;
extern cvar_t  *r_chcMaxPrevInvisNodesBatchSize;
extern cvar_t  *r_chcMaxVisibleFrames;
extern cvar_t  *r_chcVisibilityThreshold;

extern cvar_t  *r_hdrRendering;
extern cvar_t  *r_hdrMinLuminance;
extern cvar_t  *r_hdrMaxLuminance;
extern cvar_t  *r_hdrKey;
extern cvar_t  *r_hdrContrastThreshold;
extern cvar_t  *r_hdrContrastOffset;
extern cvar_t  *r_hdrLightmap;
extern cvar_t  *r_hdrLightmapExposure;
extern cvar_t  *r_hdrLightmapGamma;
extern cvar_t  *r_hdrLightmapCompensate;
extern cvar_t  *r_hdrToneMappingOperator;
extern cvar_t  *r_hdrGamma;

#ifdef EXPERIMENTAL
extern cvar_t  *r_screenSpaceAmbientOcclusion;
#endif
#ifdef EXPERIMENTAL
extern cvar_t  *r_depthOfField;
#endif

extern cvar_t  *r_reflectionMapping;
extern cvar_t  *r_highQualityNormalMapping;

extern cvar_t  *r_bloom;
extern cvar_t  *r_bloomBlur;
extern cvar_t  *r_bloomPasses;
extern cvar_t  *r_rotoscope;
extern cvar_t  *r_cameraPostFX;

//====================================================================

#define IMAGE_FILE_HASH_SIZE      4096
extern image_t *r_imageHashTable[IMAGE_FILE_HASH_SIZE];

extern long		GenerateImageHashValue(const char *fname);

float           R_NoiseGet4f(float x, float y, float z, float t);
void            R_NoiseInit(void);

void            R_SwapBuffers(int);

void            R_RenderView(viewParms_t * parms);

void            R_AddMDVSurfaces(trRefEntity_t * e);
void            R_AddMDVInteractions(trRefEntity_t * e, trRefLight_t * light);
void            R_AddNullModelSurfaces(trRefEntity_t * e);
void            R_AddBeamSurfaces(trRefEntity_t * e);
void            R_AddRailSurfaces(trRefEntity_t * e, qboolean isUnderwater);
void            R_AddLightningBoltSurfaces(trRefEntity_t * e);

void            R_AddPolygonSurfaces(void);

void            R_AddDrawSurf(surfaceType_t * surface, shader_t * shader, int lightmapNum);


void            R_LocalNormalToWorld(const vec3_t local, vec3_t world);
void            R_LocalPointToWorld(const vec3_t local, vec3_t world);

int             R_CullLocalBox(vec3_t bounds[2]);
int             R_CullLocalPointAndRadius(vec3_t origin, float radius);
int             R_CullPointAndRadius(vec3_t origin, float radius);

void            R_SetupEntityWorldBounds(trRefEntity_t * ent);

void            R_RotateEntityForViewParms(const trRefEntity_t * ent, const viewParms_t * viewParms, orientationr_t * orien);
void            R_RotateEntityForLight(const trRefEntity_t * ent, const trRefLight_t * light, orientationr_t * orien);
void            R_RotateLightForViewParms(const trRefLight_t * ent, const viewParms_t * viewParms, orientationr_t * orien);

void            R_SetupFrustum2(frustum_t frustum, const matrix_t modelViewProjectionMatrix);

qboolean        R_CompareVert(srfVert_t * v1, srfVert_t * v2, qboolean checkst);
void            R_CalcNormalForTriangle(vec3_t normal, const vec3_t v0, const vec3_t v1, const vec3_t v2);

void            R_CalcTangentsForTriangle(vec3_t tangent, vec3_t binormal,
							   const vec3_t v0, const vec3_t v1, const vec3_t v2,
							   const vec2_t t0, const vec2_t t1, const vec2_t t2);

void            R_CalcTangentsForTriangle2(vec3_t tangent, vec3_t binormal,
							   const vec3_t v0, const vec3_t v1, const vec3_t v2,
							   const vec2_t t0, const vec2_t t1, const vec2_t t2);

void            R_CalcTangentSpace(vec3_t tangent, vec3_t binormal, vec3_t normal,
								   const vec3_t v0, const vec3_t v1, const vec3_t v2,
								   const vec2_t t0, const vec2_t t1, const vec2_t t2);

void            R_CalcTangentSpaceFast(vec3_t tangent, vec3_t binormal, vec3_t normal,
								   const vec3_t v0, const vec3_t v1, const vec3_t v2,
								   const vec2_t t0, const vec2_t t1, const vec2_t t2);

void            R_CalcTBN(vec3_t tangent, vec3_t binormal, vec3_t normal,
								   const vec3_t v0, const vec3_t v1, const vec3_t v2,
								   const vec2_t t0, const vec2_t t1, const vec2_t t2);

qboolean        R_CalcTangentVectors(srfVert_t * dv[3]);

void            R_CalcSurfaceTriangleNeighbors(int numTriangles, srfTriangle_t * triangles);
void            R_CalcSurfaceTrianglePlanes(int numTriangles, srfTriangle_t * triangles, srfVert_t * verts);

float           R_CalcFov(float fovX, float width, float height);

// Tr3B - visualisation tools to help debugging the renderer frontend
void            R_DebugAxis(const vec3_t origin, const matrix_t transformMatrix);
void            R_DebugBoundingBox(const vec3_t origin, const vec3_t mins, const vec3_t maxs, vec4_t color);
void			R_DebugPolygon(int color, int numPoints, float *points);
void			R_DebugText(const vec3_t org, float r, float g, float b, const char *text, qboolean neverOcclude);




/*
====================================================================

OpenGL WRAPPERS, tr_backend.c

====================================================================
*/
#if !defined(USE_D3D10)
void            GL_Bind(image_t * image);
void			GL_BindNearestCubeMap(const vec3_t xyz);
void			GL_Unbind();
void            BindAnimatedImage(textureBundle_t * bundle);
void            GL_TextureFilter(image_t * image, filterType_t filterType);
void            GL_BindProgram(shaderProgram_t * program);
void            GL_BindNullProgram(void);
void            GL_SetDefaultState(void);
void            GL_SelectTexture(int unit);
void            GL_TextureMode(const char *string);


void			GL_BlendFunc(GLenum sfactor, GLenum dfactor);
void			GL_ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void			GL_ClearDepth(GLclampd depth);
void			GL_ClearStencil(GLint s);
void			GL_ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void			GL_CullFace(GLenum mode);
void			GL_DepthFunc(GLenum func);
void			GL_DepthMask(GLboolean flag);
void			GL_DrawBuffer(GLenum mode);
void			GL_FrontFace(GLenum mode);
void            GL_LoadModelViewMatrix(const matrix_t m);
void            GL_LoadProjectionMatrix(const matrix_t m);
void            GL_PushMatrix();
void            GL_PopMatrix();
void			GL_PolygonMode(GLenum face, GLenum mode);
void			GL_Scissor(GLint x, GLint y, GLsizei width, GLsizei height);
void			GL_Viewport(GLint x, GLint y, GLsizei width, GLsizei height);

void            GL_CheckErrors_(const char *filename, int line);

#define         GL_CheckErrors()	GL_CheckErrors_(__FILE__, __LINE__)

void            GL_State(uint32_t stateVector);
void            GL_VertexAttribsState(uint32_t stateBits);
void			GL_VertexAttribPointers(uint32_t attribBits);
void            GL_Cull(int cullType);
#endif // !defined(USE_D3D10)



/*
====================================================================



====================================================================
*/

void            RE_StretchRaw(int x, int y, int w, int h, int cols, int rows, const byte * data, int client, qboolean dirty);
void            RE_UploadCinematic(int w, int h, int cols, int rows, const byte * data, int client, qboolean dirty);

void            RE_BeginFrame(stereoFrame_t stereoFrame);
void            RE_BeginRegistration(glconfig_t * glconfig);
void            RE_LoadWorldMap(const char *mapname);
void            RE_SetWorldVisData(const byte * vis);
qhandle_t       RE_RegisterModel(const char *name);
qhandle_t       RE_RegisterSkin(const char *name);
void            RE_Shutdown(qboolean destroyWindow);

//----(SA)
qboolean        RE_GetSkinModel(qhandle_t skinid, const char *type, char *name);
qhandle_t       RE_GetShaderFromModel(qhandle_t modelid, int surfnum, int withlightmap);	//----(SA)
//----(SA) end

qboolean        R_GetEntityToken(char *buffer, int size);

model_t        *R_AllocModel(void);


void            R_Init(void);


qboolean        R_GetModeInfo(int *width, int *height, float *windowAspect, int mode);

void            R_SetColorMappings(void);
void            R_GammaCorrect(byte * buffer, int bufSize);

void            R_ImageList_f(void);
void            R_SkinList_f(void);

void            R_SubImageCpy(byte *dest, size_t destx, size_t desty, size_t destw, size_t desth, byte *src, size_t srcw, size_t srch, size_t bytes, qboolean in);

// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=516
const void     *RB_TakeScreenshotCmd(const void *data);


void            R_InitSkins(void);
skin_t         *R_GetSkinByHandle(qhandle_t hSkin);

void            R_DeleteSurfaceVBOs();


/*
====================================================================

IMAGES, tr_image.c

====================================================================
*/
void            R_InitImages(void);
void            R_ShutdownImages(void);
int             R_SumOfUsedImages(void);

image_t        *R_FindImageFile(const char *name, int bits, filterType_t filterType, wrapType_t wrapType);
image_t        *R_FindCubeImage(const char *name, int bits, filterType_t filterType, wrapType_t wrapType);

image_t        *R_CreateImage(const char *name, const byte * pic, int width, int height, int bits, filterType_t filterType,
							  wrapType_t wrapType);

image_t        *R_CreateCubeImage(const char *name,
								  const byte * pic[6],
								  int width, int height, int bits, filterType_t filterType, wrapType_t wrapType);

image_t        *R_AllocImage(const char *name, qboolean linkIntoHashTable);
void			R_UploadImage(const byte ** dataArray, int numData, image_t * image);

int				RE_GetTextureId(const char *name);



/*
====================================================================

SHADERS, tr_shader.c

====================================================================
*/
qhandle_t       RE_RegisterShader(const char *name);
qhandle_t       RE_RegisterShaderNoMip(const char *name);
qhandle_t       RE_RegisterShaderLightAttenuation(const char *name);
qhandle_t       RE_RegisterShaderFromImage(const char *name, image_t * image, qboolean mipRawImage);
qboolean		RE_LoadDynamicShader(const char *shadername, const char *shadertext);

shader_t       *R_FindShader(const char *name, shaderType_t type, qboolean mipRawImage);
shader_t       *R_GetShaderByHandle(qhandle_t hShader);
shader_t       *R_GetShaderByState(int index, long *cycleTime);
shader_t       *R_FindShaderByName(const char *name);
void            R_InitShaders(void);
void            R_ShaderList_f(void);
void            R_ShaderExp_f(void);
void            R_RemapShader(const char *oldShader, const char *newShader, const char *timeOffset);

/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void            GLimp_Init(void);
void            GLimp_Shutdown(void);
void            GLimp_EndFrame(void);

qboolean        GLimp_SpawnRenderThread(void (*function) (void));
void			GLimp_ShutdownRenderThread(void);
void           *GLimp_RendererSleep(void);
void            GLimp_FrontEndSleep(void);
void            GLimp_WakeRenderer(void *data);

void            GLimp_LogComment(char *comment);


// NOTE TTimo linux works with float gamma value, not the gamma table
//   the params won't be used, getting the r_gamma cvar directly
void            GLimp_SetGamma(unsigned char red[256], unsigned char green[256], unsigned char blue[256]);


/*
====================================================================

TESSELATOR/SHADER DECLARATIONS

====================================================================
*/

typedef byte    color4ub_t[4];

typedef struct stageVars
{
	vec4_t          color;
	qboolean        texMatricesChanged[MAX_TEXTURE_BUNDLES];
	matrix_t        texMatrices[MAX_TEXTURE_BUNDLES];
} stageVars_t;

typedef struct shaderCommands_s
{
	vec4_t          xyz[SHADER_MAX_VERTEXES];
	vec4_t          texCoords[SHADER_MAX_VERTEXES];
	vec4_t          lightCoords[SHADER_MAX_VERTEXES];
	vec4_t          tangents[SHADER_MAX_VERTEXES];
	vec4_t          binormals[SHADER_MAX_VERTEXES];
	vec4_t          normals[SHADER_MAX_VERTEXES];
	vec4_t          colors[SHADER_MAX_VERTEXES];
	vec4_t          paintColors[SHADER_MAX_VERTEXES];		// for advanced terrain blending
	vec4_t          lightDirections[SHADER_MAX_VERTEXES];
	vec4_t          boneIndexes[SHADER_MAX_VERTEXES];
	vec4_t          boneWeights[SHADER_MAX_VERTEXES];

	glIndex_t       indexes[SHADER_MAX_INDEXES];

	VBO_t          *vbo;
	IBO_t          *ibo;

	stageVars_t     svars;

	shader_t       *surfaceShader;
	shader_t       *lightShader;

	qboolean        skipTangentSpaces;
	qboolean        shadowVolume;
	int16_t         lightmapNum;

	uint32_t        numIndexes;
	uint32_t        numVertexes;

	qboolean        vboVertexSkinning;
	matrix_t        boneMatrices[MAX_BONES];

	// info extracted from current shader or backend mode
	void            (*stageIteratorFunc) ();
	void            (*stageIteratorFunc2) ();

	int             numSurfaceStages;
	shaderStage_t **surfaceStages;
} shaderCommands_t;

extern shaderCommands_t tess;

#if !defined(USE_D3D10)
void            GLSL_InitGPUShaders();
void            GLSL_ShutdownGPUShaders();
#endif

// *INDENT-OFF*
void            Tess_Begin(	void (*stageIteratorFunc)(),
							shader_t * surfaceShader, shader_t * lightShader,
							qboolean skipTangentSpaces,
							qboolean shadowVolume,
							int lightmapNum);
// *INDENT-ON*
void            Tess_End(void);
void            Tess_DrawElements();
void            Tess_CheckOverflow(int verts, int indexes);

void            Tess_ComputeColor(shaderStage_t * pStage);

void            Tess_StageIteratorGeneric();
void            Tess_StageIteratorGBuffer();
void            Tess_StageIteratorDepthFill();
void            Tess_StageIteratorShadowFill();
void            Tess_StageIteratorStencilShadowVolume();
void            Tess_StageIteratorStencilLighting();
void            Tess_StageIteratorLighting();
void            Tess_StageIteratorSky();

void            Tess_AddQuadStamp(vec3_t origin, vec3_t left, vec3_t up, const vec4_t color);
void            Tess_AddQuadStampExt(vec3_t origin, vec3_t left, vec3_t up, const vec4_t color, float s1, float t1, float s2, float t2);
void            Tess_AddQuadStampExt2(vec4_t quadVerts[4], const vec4_t color, float s1, float t1, float s2, float t2, qboolean calcNormals);
void            Tess_AddQuadStamp2(vec4_t quadVerts[4], const vec4_t color);
void            Tess_AddQuadStamp2WithNormals(vec4_t quadVerts[4], const vec4_t color);

/*
Add a polyhedron that is composed of four triangular faces

@param tretraVerts[0..2] are the ground vertices, tretaVerts[3] is the pyramid offset
*/
void            Tess_AddTetrahedron(vec4_t tetraVerts[4], vec4_t const color);

void			Tess_AddCube(const vec3_t position, const vec3_t minSize, const vec3_t maxSize, const vec4_t color);
void			Tess_AddCubeNormals(const vec3_t position, const vec3_t minSize, const vec3_t maxSize, const vec4_t color);

void            Tess_InstantQuad(vec4_t quadVerts[4]);
void            Tess_UpdateVBOs(uint32_t attribBits);

void            RB_ShowImages(void);


/*
============================================================

WORLD MAP, tr_world.c

============================================================
*/

void            R_AddBSPModelSurfaces(trRefEntity_t * e);
void            R_AddWorldSurfaces(void);
qboolean        R_inPVS(const vec3_t p1, const vec3_t p2);

void            R_AddWorldInteractions(trRefLight_t * light);
void            R_AddPrecachedWorldInteractions(trRefLight_t * light);
void            R_ShutdownVBOs();

/*
============================================================

FLARES, tr_flares.c

============================================================
*/

void            R_ClearFlares(void);

void            RB_AddFlare(void *surface, vec3_t point, vec3_t color, vec3_t normal);
void            RB_AddLightFlares(void);
void            RB_RenderFlares(void);

/*
============================================================

LIGHTS, tr_light.c

============================================================
*/

void            R_AddBrushModelInteractions(trRefEntity_t * ent, trRefLight_t * light);
void            R_SetupEntityLighting(const trRefdef_t * refdef, trRefEntity_t * ent);
int             R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir);

void            R_SetupLightOrigin(trRefLight_t * light);
void            R_SetupLightLocalBounds(trRefLight_t * light);
void            R_SetupLightWorldBounds(trRefLight_t * light);

void            R_SetupLightView(trRefLight_t * light);
void            R_SetupLightFrustum(trRefLight_t * light);
void            R_SetupLightProjection(trRefLight_t * light);

qboolean        R_AddLightInteraction(trRefLight_t * light, surfaceType_t * surface, shader_t * surfaceShader, byte cubeSideBits,
									  interactionType_t iaType);

void            R_SortInteractions(trRefLight_t * light);

void            R_SetupLightScissor(trRefLight_t * light);
void            R_SetupLightDepthBounds(trRefLight_t * light);
void            R_SetupLightLOD(trRefLight_t * light);

void            R_SetupLightShader(trRefLight_t * light);

byte            R_CalcLightCubeSideBits(trRefLight_t * light, vec3_t worldBounds[2]);

int             R_CullLightPoint(trRefLight_t * light, const vec3_t p);
int             R_CullLightTriangle(trRefLight_t * light, vec3_t verts[3]);
int             R_CullLightWorldBounds(trRefLight_t * light, vec3_t worldBounds[2]);

void            R_ComputeFinalAttenuation(shaderStage_t * pStage, trRefLight_t * light);

/*
============================================================

FOG, tr_fog.c

============================================================
*/

void			RE_SetFog(int fogvar, int var1, int var2, float r, float g, float b, float density);
void			RE_SetGlobalFog(qboolean restore, int duration, float r, float g, float b, float depthForOpaque);




/*
============================================================

SHADOWS, tr_shadows.c

============================================================
*/

void            RB_ProjectionShadowDeform();

/*
============================================================

SKIES

============================================================
*/

void            R_InitSkyTexCoords(float cloudLayerHeight);
void            RB_DrawSun(void);

/*
============================================================

CURVE TESSELATION, tr_curve.c

============================================================
*/

srfGridMesh_t  *R_SubdividePatchToGrid(int width, int height, srfVert_t points[MAX_PATCH_SIZE * MAX_PATCH_SIZE]);
srfGridMesh_t  *R_GridInsertColumn(srfGridMesh_t * grid, int column, int row, vec3_t point, float loderror);
srfGridMesh_t  *R_GridInsertRow(srfGridMesh_t * grid, int row, int column, vec3_t point, float loderror);
void            R_FreeSurfaceGridMesh(srfGridMesh_t * grid);

/*
============================================================

MARKERS, POLYGON PROJECTION ON WORLD POLYGONS, tr_marks.c

============================================================
*/

int             R_MarkFragments(int numPoints, const vec3_t * points, const vec3_t projection,
								int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t * fragmentBuffer);


/*
============================================================

FRAME BUFFER OBJECTS, tr_fbo.c

============================================================
*/
qboolean        R_CheckFBO(const FBO_t * fbo);

FBO_t          *R_CreateFBO(const char *name, int width, int height);

void            R_CreateFBOColorBuffer(FBO_t * fbo, int format, int index);
void            R_CreateFBODepthBuffer(FBO_t * fbo, int format);
void            R_CreateFBOStencilBuffer(FBO_t * fbo, int format);

void            R_AttachFBOTexture1D(int texId, int attachmentIndex);
void            R_AttachFBOTexture2D(int target, int texId, int attachmentIndex);
void            R_AttachFBOTexture3D(int texId, int attachmentIndex, int zOffset);
void            R_AttachFBOTextureDepth(int texId);

void            R_BindFBO(FBO_t * fbo);
void            R_BindNullFBO(void);

void            R_InitFBOs(void);
void            R_ShutdownFBOs(void);
void            R_FBOList_f(void);


/*
============================================================

VERTEX BUFFER OBJECTS, tr_vbo.c

============================================================
*/
VBO_t          *R_CreateVBO(const char *name, byte * vertexes, int vertexesSize, vboUsage_t usage);
VBO_t          *R_CreateVBO2(const char *name, int numVertexes, srfVert_t * vertexes, uint32_t stateBits, vboUsage_t usage);

IBO_t          *R_CreateIBO(const char *name, byte * indexes, int indexesSize, vboUsage_t usage);
IBO_t          *R_CreateIBO2(const char *name, int numTriangles, srfTriangle_t * triangles, vboUsage_t usage);

void            R_BindVBO(VBO_t * vbo);
void            R_BindNullVBO(void);

void            R_BindIBO(IBO_t * ibo);
void            R_BindNullIBO(void);

void            R_InitVBOs(void);
void            R_ShutdownVBOs(void);
void            R_VBOList_f(void);


/*
============================================================

DECALS - ydnar, tr_decals.c

============================================================
*/

void            RE_ProjectDecal(qhandle_t hShader, int numPoints, vec3_t * points, vec4_t projection, vec4_t color, int lifeTime,
								int fadeTime);
void            RE_ClearDecals(void);

void            R_AddModelShadow(refEntity_t * ent);

void            R_TransformDecalProjector(decalProjector_t * in, vec3_t axis[3], vec3_t origin, decalProjector_t * out);
qboolean        R_TestDecalBoundingBox(decalProjector_t * dp, vec3_t mins, vec3_t maxs);
qboolean        R_TestDecalBoundingSphere(decalProjector_t * dp, vec3_t center, float radius2);

void            R_ProjectDecalOntoSurface(decalProjector_t * dp, bspSurface_t * surf, bspModel_t * bmodel);

void            R_AddDecalSurface(decal_t * decal);
void            R_AddDecalSurfaces(bspModel_t * bmodel);
void            R_CullDecalProjectors(void);


/*
============================================================

SCENE GENERATION, tr_scene.c

============================================================
*/

void            R_ToggleSmpFrame(void);

void            RE_ClearScene(void);
void            RE_AddRefEntityToScene(const refEntity_t * ent);
void            RE_AddRefLightToScene(const refLight_t * light);
void            RE_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t * verts);
void            RE_AddPolysToScene(qhandle_t hShader, int numVerts, const polyVert_t * verts, int numPolys);
void            RE_AddPolyBufferToScene(polyBuffer_t * pPolyBuffer);
void			RE_AddDynamicLightToScene(const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags);
void            RE_AddCoronaToScene(const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible);
void            RE_RenderScene(const refdef_t * fd);
void            RE_SaveViewParms();
void            RE_RestoreViewParms();

/*
=============================================================

ANIMATED MODELS

=============================================================
*/

void            R_InitAnimations(void);

#if defined(USE_REFENTITY_ANIMATIONSYSTEM)
qhandle_t       RE_RegisterAnimation(const char *name);
#endif

skelAnimation_t *R_GetAnimationByHandle(qhandle_t hAnim);
void            R_AnimationList_f(void);

void            R_AddMD5Surfaces(trRefEntity_t * ent);
void            R_AddMD5Interactions(trRefEntity_t * ent, trRefLight_t * light);

#if defined(USE_REFENTITY_ANIMATIONSYSTEM)
int				RE_CheckSkeleton(refSkeleton_t * skel, qhandle_t hModel, qhandle_t hAnim);
int             RE_BuildSkeleton(refSkeleton_t * skel, qhandle_t anim, int startFrame, int endFrame, float frac,
								 qboolean clearOrigin);
int             RE_BlendSkeleton(refSkeleton_t * skel, const refSkeleton_t * blend, float frac);
int             RE_AnimNumFrames(qhandle_t hAnim);
int             RE_AnimFrameRate(qhandle_t hAnim);
#endif

/*
=============================================================

ANIMATED MODELS WOLFENSTEIN

=============================================================
*/

/*
void            R_MakeAnimModel(model_t * model);
void            R_AddAnimSurfaces(trRefEntity_t * ent);
void            RB_SurfaceAnim(mdsSurface_t * surfType);
int             R_GetBoneTag(orientation_t * outTag, mdsHeader_t * mds, int startTagIndex, const refEntity_t * refent,
							 const char *tagName);
							 */

/*
=============================================================

ANIMATED MODELS WOLF:ET  MDM/MDX

=============================================================
*/

void            R_MDM_AddAnimSurfaces(trRefEntity_t * ent);
void            Tess_MDM_SurfaceAnim(mdmSurface_t * surfType);
int             R_MDM_GetBoneTag(orientation_t * outTag, mdmHeader_t * mdm, int startTagIndex, const refEntity_t * refent,
								 const char *tagName);

/*
=============================================================
=============================================================
*/

void            R_TransformWorldToClip(const vec3_t src, const float *cameraViewMatrix,
									   const float *projectionMatrix, vec4_t eye, vec4_t dst);
void            R_TransformModelToClip(const vec3_t src, const float *modelViewMatrix,
									   const float *projectionMatrix, vec4_t eye, vec4_t dst);
void            R_TransformClipToWindow(const vec4_t clip, const viewParms_t * view, vec4_t normalized, vec4_t window);
float           R_ProjectRadius(float r, vec3_t location);


void            Tess_DeformGeometry(void);

float           RB_EvalWaveForm(const waveForm_t * wf);
float           RB_EvalWaveFormClamped(const waveForm_t * wf);
float           RB_EvalExpression(const expression_t * exp, float defaultValue);

void            RB_CalcTexMatrix(const textureBundle_t * bundle, matrix_t matrix);


/*
=============================================================

RENDERER BACK END FUNCTIONS

=============================================================
*/

void            RB_RenderThread(void);
void            RB_ExecuteRenderCommands(const void *data);

/*
=============================================================

RENDERER BACK END COMMAND QUEUE

=============================================================
*/

#define	MAX_RENDER_COMMANDS	(0x40000 * 8) // Tr3B: was 0x40000

typedef struct
{
	byte            cmds[MAX_RENDER_COMMANDS];
	int             used;
} renderCommandList_t;

typedef struct
{
	int             commandId;
	float           color[4];
} setColorCommand_t;

typedef struct
{
	int             commandId;
	int             buffer;
} drawBufferCommand_t;

typedef struct
{
	int             commandId;
	image_t        *image;
	int             width;
	int             height;
	void           *data;
} subImageCommand_t;

typedef struct
{
	int             commandId;
} swapBuffersCommand_t;

typedef struct
{
	int             commandId;
	int             buffer;
} endFrameCommand_t;

typedef struct
{
	int             commandId;
	shader_t       *shader;
	float           x, y;
	float           w, h;
	float           s1, t1;
	float           s2, t2;

	byte            gradientColor[4];	// color values 0-255
	int             gradientType;	//----(SA)  added
	float           angle;		// NERVE - SMF
} stretchPicCommand_t;

typedef struct
{
	int             commandId;
	polyVert_t     *verts;
	int             numverts;
	shader_t       *shader;
} poly2dCommand_t;

typedef struct
{
	int             commandId;
	trRefdef_t      refdef;
	viewParms_t     viewParms;
} drawViewCommand_t;

typedef struct
{
	int             commandId;
	image_t        *image;
	int             x;
	int             y;
	int             w;
	int             h;
} renderToTextureCommand_t;


typedef enum
{
	SSF_TGA,
	SSF_JPEG,
	SSF_PNG
} ssFormat_t;

typedef struct
{
	int             commandId;
	int             x;
	int             y;
	int             width;
	int             height;
	char           *fileName;
	ssFormat_t      format;
} screenshotCommand_t;

typedef struct
{
	int             commandId;
	int             width;
	int             height;
	byte           *captureBuffer;
	byte           *encodeBuffer;
	qboolean        motionJpeg;
} videoFrameCommand_t;

typedef struct
{
	int             commandId;
} renderFinishCommand_t;

typedef enum
{
	RC_END_OF_LIST,
	RC_SET_COLOR,
	RC_STRETCH_PIC,
	RC_2DPOLYS,
	RC_ROTATED_PIC,
	RC_STRETCH_PIC_GRADIENT,	// (SA) added
	RC_DRAW_VIEW,
	RC_DRAW_BUFFER,
	RC_SWAP_BUFFERS,
	RC_SCREENSHOT,
	RC_VIDEOFRAME,
	RC_RENDERTOTEXTURE,			//bani
	RC_FINISH					//bani
} renderCommand_t;


// all of the information needed by the back end must be
// contained in a backEndData_t.  This entire structure is
// duplicated so the front and back end can run in parallel
// on an SMP machine
typedef struct
{
	drawSurf_t      drawSurfs[MAX_DRAWSURFS];
	interaction_t   interactions[MAX_INTERACTIONS];

	trRefLight_t    lights[MAX_REF_LIGHTS];
	trRefEntity_t   entities[MAX_REF_ENTITIES];

	srfPoly_t      *polys;		//[MAX_POLYS];
	polyVert_t     *polyVerts;	//[MAX_POLYVERTS];

	renderCommandList_t commands;
} backEndData_t;

extern backEndData_t *backEndData[SMP_FRAMES];	// the second one may not be allocated

extern volatile renderCommandList_t *renderCommandList;

extern volatile qboolean renderThreadActive;


void           *R_GetCommandBuffer(int bytes);
void            RB_ExecuteRenderCommands(const void *data);

void            R_InitCommandBuffers(void);
void            R_ShutdownCommandBuffers(void);

void            R_SyncRenderThread(void);

void            R_AddDrawViewCmd(void);

void            RE_SetColor(const float *rgba);
void            RE_StretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader);
void            RE_RotatedPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, float angle);	// NERVE - SMF
void            RE_StretchPicGradient(float x, float y, float w, float h,
									  float s1, float t1, float s2, float t2, qhandle_t hShader, const float *gradientColor,
									  int gradientType);
void            RE_2DPolyies(polyVert_t * verts, int numverts, qhandle_t hShader);



void            RE_BeginFrame(stereoFrame_t stereoFrame);
void            RE_EndFrame(int *frontEndMsec, int *backEndMsec);


void			LoadTGA(const char *name, byte ** pic, int *width, int *height, byte alphaByte);

void            LoadJPG(const char *filename, unsigned char **pic, int *width, int *height, byte alphaByte);
void            SaveJPG(char *filename, int quality, int image_width, int image_height, unsigned char *image_buffer);
int             SaveJPGToBuffer(byte * buffer, int quality, int image_width, int image_height, byte * image_buffer);

void			LoadPNG(const char *name, byte ** pic, int *width, int *height, byte alphaByte);
void            SavePNG(const char *name, const byte * pic, int width, int height, int numBytes, qboolean flip);

// video stuff
const void     *RB_TakeVideoFrameCmd(const void *data);
void            RE_TakeVideoFrame(int width, int height, byte * captureBuffer, byte * encodeBuffer, qboolean motionJpeg);

// cubemap reflections stuff
void            R_BuildCubeMaps(void);

void            FreeVertexHashTable(vertexHash_t ** hashTable);

// font stuff
void            R_InitFreeType();
void            R_DoneFreeType();
void            RE_RegisterFont(const char *fontName, int pointSize, fontInfo_t * font);

// bani
void			RE_RenderToTexture(int textureid, int x, int y, int w, int h);
void			RE_Finish(void);


#if defined(__cplusplus)
}
#endif

#endif							// TR_LOCAL_H