#define PHYSICS_MULTIPLAYER_AUTODETECT	0	// use multiplayer physics mode as defined in model prop data

#define PHYSICS_MULTIPLAYER_SOLID		1	// soild, pushes player away 

#define PHYSICS_MULTIPLAYER_NON_SOLID	2	// nonsolid, but pushed by player

#define PHYSICS_MULTIPLAYER_CLIENTSIDE	3	// Clientside only, nonsolid 	
enum Collision_Group_t
{
	COLLISION_GROUP_NONE = 0,
	COLLISION_GROUP_DEBRIS,				// Collides with nothing but world and static stuff
	COLLISION_GROUP_DEBRIS_TRIGGER,		// Same as debris, but hits triggers
	COLLISION_GROUP_INTERACTIVE_DEB,	// RIS, // Collides with everything except other interactive debris or debris
	COLLISION_GROUP_INTERACTIVE,		// Collides with everything except interactive debris or debris
	COLLISION_GROUP_PLAYER,
	COLLISION_GROUP_BREAKABLE_GLASS,
	COLLISION_GROUP_VEHICLE,
	COLLISION_GROUP_PLAYER_MOVEMENT,	// For HL2, same as Collision_Group_Player
	COLLISION_GROUP_NPC,				// Generic NPC group
	COLLISION_GROUP_IN_VEHICLE,			// for any entity inside a vehicle
	COLLISION_GROUP_WEAPON,				// for any weapons that need collision detection
	COLLISION_GROUP_VEHICLE_CLIP,		// vehicle clip brush to restrict vehicle movement
	COLLISION_GROUP_PROJECTILE,			// Projectiles!
	COLLISION_GROUP_DOOR_BLOCKER,		// Blocks entities not permitted to get near moving doors
	COLLISION_GROUP_PASSABLE_DOOR,		// Doors that the player shouldn't collide with
	COLLISION_GROUP_DISSOLVING,			// Things that are dissolving are in this group
	COLLISION_GROUP_PUSHAWAY,			// Nonsolid on client and server, pushaway in player code
	COLLISION_GROUP_NPC_ACTOR,			// Used so NPCs in scripts ignore the player.
	LAST_SHARED_COLLISION_GROUP
};
struct surfacephysicsparams_t
{
	float    friction;
	float    elasticity; // collision elasticity - used to compute coefficient of restitution
	float    density;    // physical density (in kg / m^3)
	float    thickness;    // material thickness if not solid (sheet materials) in inches
	float    dampening;
};

struct surfaceaudioparams_t
{
	float    reflectivity;             // like elasticity, but how much sound should be reflected by this surface
	float    hardnessFactor;           // like elasticity, but only affects impact sound choices
	float    roughnessFactor;          // like friction, but only affects scrape sound choices   
	float    roughThreshold;           // surface roughness > this causes "rough" scrapes, < this causes "smooth" scrapes
	float    hardThreshold;            // surface hardness > this causes "hard" impacts, < this causes "soft" impacts
	float    hardVelocityThreshold;    // collision velocity > this causes "hard" impacts, < this causes "soft" impacts   
	float    highPitchOcclusion;       //a value betweeen 0 and 100 where 0 is not occluded at all and 100 is silent (except for any additional reflected sound)
	float    midPitchOcclusion;
	float    lowPitchOcclusion;
};

struct surfacesoundnames_t
{
	unsigned short    stepleft;
	unsigned short    stepright;
	unsigned short    impactSoft;
	unsigned short    impactHard;
	unsigned short    scrapeSmooth;
	unsigned short    scrapeRough;
	unsigned short    bulletImpact;
	unsigned short    rolling;
	unsigned short    breakSound;
	unsigned short    strainSound;
};

struct surfacegameprops_t
{
public:
	float    maxSpeedFactor; //0x0000
	float    jumpFactor; //0x0004
	char    pad00[0x4]; //0x0008
	float    flPenetrationModifier; //0x000C
	float    flDamageModifier; //0x0010
	unsigned short    material; //0x0014
	char    pad01[0x3];

};//Size=0x0019

struct surfacedata_t
{
	surfacephysicsparams_t    physics;
	surfaceaudioparams_t    audio;
	surfacesoundnames_t        sounds;
	surfacegameprops_t        game;
};

class IPhysicsSurfaceProps
{
public:
	/*virtual ~IPhysicsSurfaceProps(void) {}
	virtual int				ParseSurfaceData(const char *pFilename, const char *pTextfile) = 0;
	virtual int				SurfacePropCount(void) const = 0;
	virtual int				GetSurfaceIndex(const char *pSurfacePropName) const = 0;
	virtual void			GetPhysicsProperties(int surfaceDataIndex, float *density, float *thickness, float *friction, float *elasticity) const = 0;
	virtual surfacedata_t*	GetSurfaceData(int surfaceDataIndex) = 0;
	virtual const char*		GetString(unsigned short stringTableIndex) const = 0;
	virtual const char*		GetPropName(int surfaceDataIndex) const = 0;
	virtual void			SetWorldMaterialIndexTable(int *pMapArray, int mapSize) = 0;
	virtual void			GetPhysicsParameters(int surfaceDataIndex, surfacephysicsparams_t *pParamsOut) const = 0;*/

	surfacedata_t	*GetSurfaceData(int surfaceDataIndex)
	{
		typedef surfacedata_t*(__thiscall* fnGetSurfaceData)(void*, int);
		return CallVFunction<fnGetSurfaceData>(this, 5)(this, surfaceDataIndex);
	}
};

class IMultiplayerPhysics
{
public:
	virtual int		GetMultiplayerPhysicsMode(void) = 0;
	virtual float	GetMass(void) = 0;
	virtual bool	IsAsleep(void) = 0;
};
enum mp_break_t
{
	MULTIPLAYER_BREAK_DEFAULT,
	MULTIPLAYER_BREAK_SERVERSIDE,
	MULTIPLAYER_BREAK_CLIENTSIDE,
	MULTIPLAYER_BREAK_BOTH
};
enum propdata_interactions_t
{
	PROPINTER_PHYSGUN_WORLD_STICK,		// "onworldimpact"	"stick"
	PROPINTER_PHYSGUN_FIRST_BREAK,		// "onfirstimpact"	"break"
	PROPINTER_PHYSGUN_FIRST_PAINT,		// "onfirstimpact"	"paintsplat"
	PROPINTER_PHYSGUN_FIRST_IMPALE,		// "onfirstimpact"	"impale"
	PROPINTER_PHYSGUN_LAUNCH_SPIN_NONE,	// "onlaunch"		"spin_none"
	PROPINTER_PHYSGUN_LAUNCH_SPIN_Z,	// "onlaunch"		"spin_zaxis"
	PROPINTER_PHYSGUN_BREAK_EXPLODE,	// "onbreak"		"explode_fire"
	PROPINTER_PHYSGUN_DAMAGE_NONE,		// "damage"			"none"
	PROPINTER_FIRE_FLAMMABLE,			// "flammable"			"yes"
	PROPINTER_FIRE_EXPLOSIVE_RESIST,	// "explosive_resist"	"yes"
	PROPINTER_FIRE_IGNITE_HALFHEALTH,	// "ignite"				"halfhealth"
	PROPINTER_PHYSGUN_CREATE_FLARE,		// "onpickup"		"create_flare"
	PROPINTER_PHYSGUN_ALLOW_OVERHEAD,	// "allow_overhead"	"yes"
	PROPINTER_WORLD_BLOODSPLAT,			// "onworldimpact", "bloodsplat"
	PROPINTER_PHYSGUN_NOTIFY_CHILDREN,	// "onfirstimpact" cause attached flechettes to explode
	PROPINTER_NUM_INTERACTIONS,			// If we get more than 32 of these, we'll need a different system
};
class IBreakableWithPropData
{
public:
	// Damage modifiers
	virtual void            SetDmgModBullet(float flDmgMod) = 0;
	virtual void            SetDmgModClub(float flDmgMod) = 0;
	virtual void            SetDmgModExplosive(float flDmgMod) = 0;
	virtual float           GetDmgModBullet(void) = 0;
	virtual float           GetDmgModClub(void) = 0;
	virtual float           GetDmgModExplosive(void) = 0;
	// Explosive
	virtual void            SetExplosiveRadius(float flRadius) = 0;
	virtual void            SetExplosiveDamage(float flDamage) = 0;
	virtual float           GetExplosiveRadius(void) = 0;
	virtual float           GetExplosiveDamage(void) = 0;
	// Physics damage tables
	virtual void            SetPhysicsDamageTable(char* iszTableName) = 0;
	virtual char*           GetPhysicsDamageTable(void) = 0;
	// Breakable chunks
	virtual void            SetBreakableModel(char* iszModel) = 0;
	virtual char*           GetBreakableModel(void) = 0;
	virtual void            SetBreakableSkin(int iSkin) = 0;
	virtual int                     GetBreakableSkin(void) = 0;
	virtual void            SetBreakableCount(int iCount) = 0;
	virtual int                     GetBreakableCount(void) = 0;
	virtual void            SetMaxBreakableSize(int iSize) = 0;
	virtual int                     GetMaxBreakableSize(void) = 0;
	// LOS blocking
	virtual void            SetPropDataBlocksLOS(bool bBlocksLOS) = 0;
	virtual void            SetPropDataIsAIWalkable(bool bBlocksLOS) = 0;
	// Interactions
	virtual void            SetInteraction(propdata_interactions_t Interaction) = 0;
	virtual bool            HasInteraction(propdata_interactions_t Interaction) = 0;
	// Multi player physics mode
	virtual void            SetPhysicsMode(int iMode) = 0;
	virtual int                     GetPhysicsMode() = 0;
	// Multi player breakable spawn behavior
	virtual void            SetMultiplayerBreakMode(mp_break_t mode) = 0;
	virtual mp_break_t      GetMultiplayerBreakMode(void) const = 0;
	// Used for debugging
	virtual void            SetBasePropData(char* iszBase) = 0;
	virtual char*           GetBasePropData(void) = 0;
};