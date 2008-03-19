#ifndef GRAPHTREEMODELPOPULATOR_H_
#define GRAPHTREEMODELPOPULATOR_H_

#include "iscenegraph.h"
#include "scenelib.h"
#include "GraphTreeModel.h"

namespace ui {

/**
 * greebo: The purpose of this class is to traverse the entire scenegraph and
 *         push all the found nodes into the given GraphTreeModel.
 * 
 * This is used by the GraphTreeModel itself to update its status on show.
 */
class GraphTreeModelPopulator :
	public scene::Graph::Walker
{
	// The model to be populated
	GraphTreeModel& _model;

public:
	GraphTreeModelPopulator(GraphTreeModel& model) :
		_model(model)
	{
		// Clear out the model before traversal
		_model.clear();
	}
	
	// Graph::Walker implementation
	bool pre(const scene::Path& path, const scene::INodePtr& node) const {
		// Insert this node into the GraphTreeModel
		_model.insert(node);
		
		return true; // traverse children
	}
};

} // namespace ui

#endif /*GRAPHTREEMODELPOPULATOR_H_*/
