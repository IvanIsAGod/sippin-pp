struct mstudiobbox_t
{
	int					bone;
	int					group;				// intersection group
	Vector				bbmin;				// bounding box
	Vector				bbmax;
	int					szhitboxnameindex;	// offset to the name of the hitbox.
	int					unused[3];
	float				radius;
	int					unused2[4];

	const char* pszHitboxName()
	{
		if (szhitboxnameindex == 0)
			return "";

		return ((char*)this) + szhitboxnameindex;
	}

	mstudiobbox_t() {}

private:
	mstudiobbox_t(const mstudiobbox_t& vOther);
};

struct mstudiobone_t
{
	int					sznameindex;
	inline char * const pszName(void) const { return ((char *)this) + sznameindex; }
	int		 			parent;		// parent bone
	int					bonecontroller[6];	// bone controller index, -1 == none

											// default values
	Vector				pos;
	Quaternion			quat;
	Vector			rot;
	// compression scale
	Vector				posscale;
	Vector				rotscale;

	VMatrix			poseToBone;
	Quaternion			qAlignment;
	int					flags;
	int					proctype;
	int					procindex;		// procedural rule
	mutable int			physicsbone;	// index into physically simulated bone
	inline void *pProcedure() const { if (procindex == 0) return NULL; else return  (void *)(((byte *)this) + procindex); };
	int					surfacepropidx;	// index into string tablefor property name
	inline char * const pszSurfaceProp(void) const { return ((char *)this) + surfacepropidx; }
	int					contents;		// See BSPFlags.h for the contents flags

	int					unused[8];		// remove as appropriate

	mstudiobone_t() {}
private:
	// No copy constructors allowed
	mstudiobone_t(const mstudiobone_t& vOther);
};

struct mstudioseqdesc_t
{
	int	szlabelindex;
	inline char * const pszLabel(void) const { return ((char *)this) + szlabelindex; }

	int	szactivitynameindex;
	inline char * const pszActivityName(void) const { return ((char *)this) + szactivitynameindex; }

	int	flags;		// looping/non-looping flags

	int	activity;	// initialized at loadtime to game DLL values
	int	actweight;

	int	numevents;
	int	eventindex;
	inline void *pEvent(int i) const { return (((byte *)this) + eventindex) + i; };

	Vector	bbmin;		// per sequence bounding box
	Vector	bbmax;

	//-------------------------------------------------------------------------
	// Purpose: returns a model animation from the sequence group size and
	//          blend index
	// Note: this replaces GetAnimValue() that was previously in bone_setup
	// Note: this also acts as a SetAnimValue() as it returns a reference to
	//       the anim value in question
	//-------------------------------------------------------------------------
	inline unsigned short& pAnimValue(int nIndex0, int nIndex1) const
	{
		// Clamp indexes
		if (nIndex0 >= groupsize[0])
			nIndex0 = groupsize[0] - 1;

		if (nIndex1 >= groupsize[1])
			nIndex1 = groupsize[1] - 1;

		return *pBlend(nIndex1 * groupsize[0] + nIndex0);
	}

	int	numblends;

	int blendindex;
	inline unsigned short *pBlend(int i) const { return (unsigned short *)(((byte *)this) + blendindex) + i; };

	int seqgroup; // sequence group for demand loading

	int	groupsize[2];
	int	paramindex[2];	// X, Y, Z, XR, YR, ZR
	float	paramstart[2];	// local (0..1) starting value
	float	paramend[2];	// local (0..1) ending value
	int	paramparent;

	float	fadeintime;	// ideal cross fate in time (0.2 default)
	float	fadeouttime;	// ideal cross fade out time (0.2 default)

	int	entrynode;	// transition node at entry
	int	exitnode;	// transition node at exit
	int	nodeflags;	// transition rules

	float	entryphase;	// used to match entry gait
	float	exitphase;	// used to match exit gait

	float	lastframe;	// frame that should generation EndOfSequence

	int	nextseq;	// auto advancing sequences
	int	pose;		// index of delta animation between end and nextseq

	int	numikrules;

	int	numautolayers;
	int	autolayerindex;
	inline void *pAutolayer(int i) const { return (((byte *)this) + autolayerindex) + i; };

	int	weightlistindex;
	float	*pBoneweight(int i) const { return ((float *)(((byte *)this) + weightlistindex) + i); };
	float	weight(int i) const { return *(pBoneweight(i)); };

	int	posekeyindex;
	float				*pPoseKey(int iParam, int iAnim) const { return (float *)(((byte *)this) + posekeyindex) + iParam * groupsize[0] + iAnim; }
	float				poseKey(int iParam, int iAnim) const { return *(pPoseKey(iParam, iAnim)); }

	int	numiklocks;
	int	iklockindex;
	inline void *pIKLock(int i) const { return (((byte *)this) + iklockindex) + i; };

	// Key values
	int	keyvalueindex;
	int	keyvaluesize;
	inline const char * KeyValueText(void) const { return keyvaluesize != 0 ? ((char *)this) + keyvalueindex : NULL; }

	int	unused[3];		// remove/add as appropriate
};

struct mstudiohitboxset_t
{
	int					sznameindex;
	inline char * const	pszName(void) const { return ((char *)this) + sznameindex; }
	int					numhitboxes;
	int					hitboxindex;
	inline mstudiobbox_t *pHitbox(int i) const { return (mstudiobbox_t *)(((byte *)this) + hitboxindex) + i; };
	inline mstudiobbox_t* GetHitbox(int i) const
	{
		return (mstudiobbox_t*)(((DWORD)this) + hitboxindex) + i;
	};
};

struct studiohdr_t
{
	int	id;
	int	version;

	long checksum;	// this has to be the same in the phy and vtx files to load!

	char name[64];
	int	length;

	Vector	eyeposition;	// ideal eye position

	Vector	illumposition;	// illumination center

	Vector	hull_min;	// ideal movement hull size
	Vector	hull_max;

	Vector	view_bbmin;	// clipping bounding box
	Vector	view_bbmax;

	int	flags;

	int	numbones;	// bones
	int	boneindex;
	inline mstudiobone_t *pBone(int i) const { return (mstudiobone_t *)(((byte *)this) + boneindex) + i; };

	int	numbonecontrollers;	// bone controllers
	int	bonecontrollerindex;
	inline void *pBonecontroller(int i) const { return (((byte *)this) + bonecontrollerindex) + i; };

	int	numhitboxsets;
	int	hitboxsetindex;

	// Look up hitbox set by index

	mstudiohitboxset_t  *pHitboxSet(int i) const
	{
		return (mstudiohitboxset_t *)(((byte *)this) + hitboxsetindex) + i;
	};

	// Calls through to hitbox to determine size of specified set
	inline mstudiobbox_t *pHitbox(int i, int set) const
	{
		mstudiohitboxset_t *s = pHitboxSet(set);

		if (!s)
			return NULL;

		return s->pHitbox(i);
	};

	// Calls through to set to get hitbox count for set
	inline int  iHitboxCount(int set) const
	{
		mstudiohitboxset_t const *s = pHitboxSet(set);
		if (!s)
			return 0;

		return s->numhitboxes;
	};

	int	numanim;	// animations/poses
	int	animdescindex;	// animation descriptions
	inline void *pAnimdesc(int i) const { return (((byte *)this) + animdescindex) + i; };

	int 	numanimgroup;
	int 	animgroupindex;
	inline  void *pAnimGroup(int i) const { return (((byte *)this) + animgroupindex) + i; };

	int 	numbonedesc;
	int 	bonedescindex;
	inline  void *pBoneDesc(int i) const { return (((byte *)this) + bonedescindex) + i; };

	int	numseq;		// sequences
	int	seqindex;
	inline mstudioseqdesc_t *pSeqdesc(int i) const { if (i < 0 || i >= numseq) i = 0; return (mstudioseqdesc_t *)(((byte *)this) + seqindex) + i; };
	int	sequencesindexed;	// initialization flag - have the sequences been indexed?

	int	numseqgroups;		// demand loaded sequences
	int	seqgroupindex;
	inline  void *pSeqgroup(int i) const { return (((byte *)this) + seqgroupindex) + i; };

	int	numtextures;		// raw textures
	int	textureindex;
	inline void *pTexture(int i) const { return (((byte *)this) + textureindex) + i; };

	int	numcdtextures;		// raw textures search paths
	int	cdtextureindex;
	inline char			*pCdtexture(int i) const { return (((char *)this) + *((int *)(((byte *)this) + cdtextureindex) + i)); };

	int	numskinref;		// replaceable textures tables
	int	numskinfamilies;
	int	skinindex;
	inline short		*pSkinref(int i) const { return (short *)(((byte *)this) + skinindex) + i; };

	int	numbodyparts;
	int	bodypartindex;
	inline void	*pBodypart(int i) const { return (((byte *)this) + bodypartindex) + i; };

	int	numattachments;		// queryable attachable points
	int	attachmentindex;
	inline void	*pAttachment(int i) const { return (((byte *)this) + attachmentindex) + i; };

	int	numtransitions;		// animation node to animation node transition graph
	int	transitionindex;
	inline byte	*pTransition(int i) const { return (byte *)(((byte *)this) + transitionindex) + i; };

	int	numflexdesc;
	int	flexdescindex;
	inline void *pFlexdesc(int i) const { return (((byte *)this) + flexdescindex) + i; };

	int	numflexcontrollers;
	int	flexcontrollerindex;
	inline void *pFlexcontroller(int i) const { return (((byte *)this) + flexcontrollerindex) + i; };

	int	numflexrules;
	int	flexruleindex;
	inline void *pFlexRule(int i) const { return (((byte *)this) + flexruleindex) + i; };

	int	numikchains;
	int	ikchainindex;
	inline void *pIKChain(int i) const { return (((byte *)this) + ikchainindex) + i; };

	int	nummouths;
	int	mouthindex;
	inline void *pMouth(int i) const { return (((byte *)this) + mouthindex) + i; };

	int	numposeparameters;
	int	poseparamindex;
	inline void *pPoseParameter(int i) const { return (((byte *)this) + poseparamindex) + i; };

	int	surfacepropindex;
	inline char * const pszSurfaceProp(void) const { return ((char *)this) + surfacepropindex; }

	// Key values
	int	keyvalueindex;
	int	keyvaluesize;
	inline const char * KeyValueText(void) const { return keyvaluesize != 0 ? ((char *)this) + keyvalueindex : NULL; }

	int	numikautoplaylocks;
	int	ikautoplaylockindex;
	inline void *pIKAutoplayLock(int i) const { return (((byte *)this) + ikautoplaylockindex) + i; };

	float mass;		// The collision model mass that jay wanted
	int	contents;
	int	unused[9];	// remove as appropriate
};
struct model_t;
//0x3F00FB33

class CModelInfo
{
public:
	studiohdr_t* GetStudioModel(const model_t* Model)
	{
		if (!this)
			return nullptr;

		typedef studiohdr_t*(__thiscall* Fn)(void*, const model_t*);
		return CallVFunction<Fn>(this, 32)(this, Model);

	}
	void GetModelRenderBounds(const model_t *model, Vector& mins, Vector& maxs)
	{
		typedef void(__thiscall* Fn)(void*, const model_t*, Vector&, Vector&);
		return CallVFunction<Fn>(this, 6)(this, model, mins, maxs);
	}
	int GetModelIndex(const char* ModelName)
	{
		typedef int(__thiscall* OriginalFn)(PVOID, const char*);
		return CallVFunction<OriginalFn>(this, 2)(this, ModelName);
	}
	const char* GetModelName(const model_t *model)
	{
		typedef const char* (__thiscall* OriginalFn)(PVOID, const model_t*);
		return CallVFunction<OriginalFn>(this, 3)(this, model);
	}
	std::string GetModelNameString(const model_t* pModel)
	{
		typedef const char*(__thiscall* OriginalFn)(PVOID, const model_t*);
		return std::string(CallVFunction< OriginalFn >(this, 3)(this, pModel));
	}
	void GetModelMaterials(const model_t *model, int count, IMaterial** ppMaterial)
	{
		typedef void(__thiscall* OriginalFn)(PVOID, const model_t*, int, IMaterial**);
		CallVFunction<OriginalFn>(this, 17)(this, model, count, ppMaterial);
	}/*
	 void SetMaterialVarFlag(MaterialVarFlags_t flag, bool on)
	 {
	 typedef void(__thiscall* OriginalFn)(PVOID, MaterialVarFlags_t, bool);
	 CallVFunction<OriginalFn>(this, 29)(this, flag, on);
	 }
	 void SetColorModulation(float const* blend)
	 {
	 typedef void(__thiscall* OriginalFn)(PVOID, float const*);
	 return CallVFunction<OriginalFn>(this, 6)(this, blend);
	 }*/
};