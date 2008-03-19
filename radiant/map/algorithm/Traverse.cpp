#include "Traverse.h"

#include "itraversable.h"
#include "scenelib.h"

namespace map {

class IncludeSelectedWalker :
	public scene::NodeVisitor
{
	scene::NodeVisitor& m_walker;
	mutable std::size_t m_selected;
	mutable bool m_skip;

	bool selectedParent() const {
		return m_selected != 0;
	}
public:
	IncludeSelectedWalker(scene::NodeVisitor& walker) :
		m_walker(walker),
		m_selected(0),
		m_skip(false)
	{}

	virtual bool pre(const scene::INodePtr& node) {
		// include node if:
		// node is not a 'root' AND ( node is selected OR any child of node is selected OR any parent of node is selected )
		if (!node->isRoot() && (Node_selectedDescendant(node) || selectedParent())) {
			if (Node_isSelected(node)) {
				++m_selected;
			}
			m_walker.pre(node);
			return true;
		}
		else {
			m_skip = true;
			return false;
		}
	}

	virtual void post(const scene::INodePtr& node) {
		if (m_skip) {
			m_skip = false;
		}
		else {
			if (Node_isSelected(node)) {
				--m_selected;
			}
			m_walker.post(node);
		}
	}
};

void traverseSelected(scene::INodePtr root, scene::NodeVisitor& walker) {
	IncludeSelectedWalker visitor(walker);
	root->traverse(visitor);
}

void traverse(scene::INodePtr root, scene::NodeVisitor& walker) {
	root->traverse(walker);
}

} // namespace map
