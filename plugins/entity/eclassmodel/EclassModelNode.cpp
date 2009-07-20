#include "EclassModelNode.h"

namespace entity {

EclassModelNode::EclassModelNode(const IEntityClassConstPtr& eclass) :
	EntityNode(eclass),
	TransformModifier(EclassModel::TransformChangedCaller(m_contained), 
					  ApplyTransformCaller(*this)),
	m_contained(*this, // pass <self> as scene::INode&
				Node::TransformChangedCaller(*this), 
				EvaluateTransformCaller(*this)),
	_updateSkin(true)
{
	construct();
}

EclassModelNode::EclassModelNode(const EclassModelNode& other) :
	EntityNode(other),
	scene::Cloneable(other),
	Nameable(other),
	Snappable(other),
	TransformNode(other),
	TransformModifier(EclassModel::TransformChangedCaller(m_contained), 
					  ApplyTransformCaller(*this)),
	m_contained(other.m_contained, 
				*this, // pass <self> as scene::INode&
				Node::TransformChangedCaller(*this), 
				EvaluateTransformCaller(*this)),
	_updateSkin(true)
{
	construct();
}

EclassModelNode::~EclassModelNode() {
	destroy();
}

void EclassModelNode::construct() {
	// Attach the InstanceSet as Traversable::Observer to the nodeset
	Node::attachTraverseObserver(this);
	m_contained.addKeyObserver("skin", SkinChangedCaller(*this));
}

void EclassModelNode::destroy() {
	m_contained.removeKeyObserver("skin", SkinChangedCaller(*this));
	Node::detachTraverseObserver(this);
}

// Snappable implementation
void EclassModelNode::snapto(float snap) {
	m_contained.snapto(snap);
}

// TransformNode implementation
const Matrix4& EclassModelNode::localToParent() const {
	return m_contained.getTransformNode().localToParent();
}

// EntityNode implementation
Entity& EclassModelNode::getEntity() {
	return m_contained.getEntity();
}

void EclassModelNode::refreshModel() {
	// Simulate a "model" key change
	m_contained.modelChanged(m_contained.getEntity().getKeyValue("model"));

	// Trigger a skin change
	skinChanged(m_contained.getEntity().getKeyValue("skin"));
}

void EclassModelNode::renderSolid(RenderableCollector& collector, const VolumeTest& volume) const
{
	EntityNode::renderSolid(collector, volume);

	// greebo: Check if the skin needs updating before rendering.
	if (_updateSkin) {
		// Instantiate a walker class equipped with the new value
		SkinChangedWalker walker(m_contained.getEntity().getKeyValue("skin"));
		// Update all children
		Node_traverseSubgraph(Node::getSelf(), walker);

		_updateSkin = false;
	}

	m_contained.renderSolid(collector, volume, localToWorld(), isSelected());
}

void EclassModelNode::renderWireframe(RenderableCollector& collector, const VolumeTest& volume) const
{
	EntityNode::renderWireframe(collector, volume);

	m_contained.renderWireframe(collector, volume, localToWorld(), isSelected());
}

scene::INodePtr EclassModelNode::clone() const {
	scene::INodePtr clone(new EclassModelNode(*this));
	clone->setSelf(clone);
	return clone;
}

void EclassModelNode::instantiate(const scene::Path& path) {
	m_contained.instanceAttach(path);
	Node::instantiate(path);
}

void EclassModelNode::uninstantiate(const scene::Path& path) {
	m_contained.instanceDetach(path);
	Node::uninstantiate(path);
}

// Nameable implementation
std::string EclassModelNode::name() const {
	return m_contained.getNameable().name();
}

void EclassModelNode::evaluateTransform() {
	if (getType() == TRANSFORM_PRIMITIVE) {
		m_contained.translate(getTranslation());
		m_contained.rotate(getRotation());
	}
}

void EclassModelNode::applyTransform() {
	m_contained.revertTransform();
	evaluateTransform();
	m_contained.freezeTransform();
}

void EclassModelNode::skinChanged(const std::string& value) {
	// Instantiate a walker class equipped with the new value
	SkinChangedWalker walker(value);
	// Update all children
	Node_traverseSubgraph(Node::getSelf(), walker);
}

} // namespace entity
