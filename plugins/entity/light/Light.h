#ifndef LIGHTCLASS_H_
#define LIGHTCLASS_H_

#include "igl.h"
#include "irender.h"
#include "cullable.h"
#include "scenelib.h"
#include "editable.h"
#include "render.h"
#include "irenderable.h"
#include "math/frustum.h"

#include "../keyobservers.h"
#include "../origin.h"
#include "../rotation.h"
#include "../colour.h"
#include "../namedentity.h"
#include "../entity.h"
#include "../Doom3Entity.h"

#include "Renderables.h"
#include "LightShader.h"
#include "RenderableVertices.h"
#include "Doom3LightRadius.h"

namespace entity {

/* greebo: This is the actual light class. It contains the information about the geometry
 * of the light and the actual render functions.
 * 
 * This class owns all the keyObserver callbacks, that get invoked as soon as the entity key/values get
 * changed by the user.
 * 
 * The subclass Doom3LightRadius contains some variables like the light radius and light center coordinates,
 * and there are some "onChanged" callbacks for the light radius and light center.
 * 
 * Note: All the selection stuff is handled by the LightInstance class. This is just the bare bone light.
 */
 
void light_vertices(const AABB& aabb_light, Vector3 points[6]);
void light_draw(const AABB& aabb_light, RenderStateFlags state);

inline const BasicVector4<double>& plane3_to_vector4(const Plane3& self)
{
  return reinterpret_cast<const BasicVector4<double>&>(self);
}

inline BasicVector4<double>& plane3_to_vector4(Plane3& self)
{
  return reinterpret_cast<BasicVector4<double>&>(self);
}

inline Matrix4 matrix4_from_planes(const Plane3& left, const Plane3& right, const Plane3& bottom, const Plane3& top, const Plane3& front, const Plane3& back)
{
  return Matrix4(
    (right.a - left.a) / 2,
    (top.a - bottom.a) / 2,
    (back.a - front.a) / 2,
    right.a - (right.a - left.a) / 2,
    (right.b - left.b) / 2,
    (top.b - bottom.b) / 2,
    (back.b - front.b) / 2,
    right.b - (right.b - left.b) / 2,
    (right.c - left.c) / 2,
    (top.c - bottom.c) / 2,
    (back.c - front.c) / 2,
    right.c - (right.c - left.c) / 2,
    (right.d - left.d) / 2,
    (top.d - bottom.d) / 2,
    (back.d - front.d) / 2,
    right.d - (right.d - left.d) / 2
  );
}

inline void default_extents(Vector3& extents) {
	extents = Vector3(8,8,8);
}

class LightNode;

class Light :
	public OpenGLRenderable,
	public Cullable,
	public Bounded,
	public Editable,
	public Snappable
{
    // The parent entity object that uses this light
	Doom3Entity& _entity;

  KeyObserverMap m_keyObservers;
  IdentityTransform m_transform;

  OriginKey m_originKey;
  RotationKey m_rotationKey;
  Float9 m_rotation;
  Colour m_colour;

  NamedEntity m_named;

	Doom3LightRadius m_doom3Radius;

	// Renderable components of this light
	RenderLightRadiiBox _renderableRadius;
    RenderLightProjection _renderableFrustum;

	RenderableLightTarget _rCentre;
	RenderableLightTarget _rTarget;
	
	RenderableLightRelative _rUp;
	RenderableLightRelative _rRight;
	
	RenderableLightTarget _rStart;
	RenderableLightTarget _rEnd;
	RenderableNamedEntity m_renderName;

  Float9 m_lightRotation;
  bool m_useLightRotation;

	// These are the vectors that define a projected light
	Vector3 _lightTarget;
	Vector3 _lightUp;
	Vector3 _lightRight;
	Vector3 _lightStart;
	Vector3 _lightEnd;
	
	// The "temporary" vectors, that get changed during a transform operation
	Vector3 _lightTargetTransformed;
	Vector3 _lightUpTransformed;
	Vector3 _lightRightTransformed;
	Vector3 _lightStartTransformed;
	Vector3 _lightEndTransformed;
	
	Vector3 _colourLightTarget;
	Vector3 _colourLightUp;
	Vector3 _colourLightRight;
	Vector3 _colourLightStart;
	Vector3 _colourLightEnd;
	
	bool m_useLightTarget;
	bool m_useLightUp;
	bool m_useLightRight;
	bool m_useLightStart;
	bool m_useLightEnd;

  mutable AABB m_doom3AABB;
  mutable AABB _lightAABB;
  mutable Matrix4 m_doom3Rotation;

    // Projection matrix for projected light
    mutable Matrix4 _projection;

    // Frustum for projected light (used for rendering the light volume)
    mutable Frustum _frustum;

  mutable bool m_doom3ProjectionChanged;

  LightShader m_shader;

    // The 8x8 box representing the light object itself
    AABB _lightBox;

  Callback m_transformChanged;
  Callback m_boundsChanged;
  Callback m_evaluateTransform;

	void construct();

private:

    // Ensure the start and end points are set to sensible values
	void checkStartEnd();

    // Update the bounds of the renderable radius box
	void updateRenderableRadius() const;

	void updateOrigin();

public:

	void originChanged();
	typedef MemberCaller<Light, &Light::originChanged> OriginChangedCaller;

	void lightTargetChanged(const std::string& value);
	typedef MemberCaller1<Light, const std::string&, &Light::lightTargetChanged> LightTargetChangedCaller;
	void lightUpChanged(const std::string& value);
	typedef MemberCaller1<Light, const std::string&, &Light::lightUpChanged> LightUpChangedCaller;
	void lightRightChanged(const std::string& value);
	typedef MemberCaller1<Light, const std::string&, &Light::lightRightChanged> LightRightChangedCaller;
	void lightStartChanged(const std::string& value);
	typedef MemberCaller1<Light, const std::string&, &Light::lightStartChanged> LightStartChangedCaller;
	void lightEndChanged(const std::string& value);
	typedef MemberCaller1<Light, const std::string&, &Light::lightEndChanged> LightEndChangedCaller;

	void writeLightOrigin();

	void rotationChanged();
	typedef MemberCaller<Light, &Light::rotationChanged> RotationChangedCaller;

	void lightRotationChanged(const std::string& value);
	typedef MemberCaller1<Light, const std::string&, &Light::lightRotationChanged> LightRotationChangedCaller;

	/**
     * \brief
     * Main constructor.
     */
	Light(Doom3Entity& entity,
          const Callback& transformChanged,
          const Callback& boundsChanged,
          const Callback& evaluateTransform);
	
	/**
     * \brief
     * Copy constructor.
     */
	Light(const Light& other,
          Doom3Entity& entity,
          const Callback& transformChanged,
          const Callback& boundsChanged,
          const Callback& evaluateTransform);
	
	InstanceCounter m_instanceCounter;
	void instanceAttach(const scene::Path& path);
	void instanceDetach(const scene::Path& path);

	Doom3Entity& getEntity();
	const Doom3Entity& getEntity() const;

	//Namespaced& getNamespaced();
	NamedEntity& getNameable();
	const NamedEntity& getNameable() const;
	TransformNode& getTransformNode();
	const TransformNode& getTransformNode() const;

	void render(const RenderInfo& info) const;

	VolumeIntersectionValue intersectVolume(const VolumeTest& volume, const Matrix4& localToWorld) const;
	const AABB& localAABB() const;
	AABB lightAABB() const;

	// Note: move this upwards
	mutable Matrix4 m_projectionOrientation;

	// Renderable submission functions
	void renderWireframe(RenderableCollector& collector, 
						 const VolumeTest& volume, 
						 const Matrix4& localToWorld, 
						 bool selected) const;

	// Adds the light centre renderable to the given collector
	void renderLightCentre(RenderableCollector& collector, const VolumeTest& volume, const Matrix4& localToWorld) const;
	void renderProjectionPoints(RenderableCollector& collector, const VolumeTest& volume, const Matrix4& localToWorld) const;

	// Returns a reference to the member class Doom3LightRadius (used to set colours)
	Doom3LightRadius& getDoom3Radius();

	void testSelect(Selector& selector, SelectionTest& test, const Matrix4& localToWorld);
  
	void translate(const Vector3& translation);
	void translateLightTarget(const Vector3& translation);
	void translateLightStart(const Vector3& translation);
	
	void rotate(const Quaternion& rotation);
	
	// This snaps the light as a whole to the grid (basically the light origin)
	void snapto(float snap);
	void setLightRadius(const AABB& aabb);
	void transformLightRadius(const Matrix4& transform);
	void revertTransform();
	void freezeTransform();
	void transformChanged();
	typedef MemberCaller<Light, &Light::transformChanged> TransformChangedCaller;

	// note: move this
	mutable Matrix4 m_localPivot;
	const Matrix4& getLocalPivot() const;

	void setLightChangedCallback(const Callback& callback);

    // Is this light projected or omni?
    bool isProjected() const;

    // Set the projection-changed flag
	void projectionChanged();

    // Update and return the projection matrix
	const Matrix4& projection() const;

    // RendererLight implementation
    Vector3 worldOrigin() const
    {
        return _lightBox.origin;
    }

    Matrix4 getLightTextureTransformation() const;
  	bool testAABB(const AABB& other) const;
	const Matrix4& rotation() const;
	Vector3 getLightOrigin() const;
	const Vector3& colour() const;
	ShaderPtr getShader() const;
	
	Vector3& target();
	Vector3& targetTransformed();
	Vector3& up();
	Vector3& upTransformed();
	Vector3& right();
	Vector3& rightTransformed();
	Vector3& start();
	Vector3& startTransformed();
	Vector3& end();
	Vector3& endTransformed();
	
	Vector3& colourLightTarget();
	Vector3& colourLightRight();
	Vector3& colourLightUp();
	Vector3& colourLightStart();
	Vector3& colourLightEnd();
	
	bool useStartEnd() const;
	
}; // class Light

} // namespace entity

#endif /*LIGHTCLASS_H_*/
