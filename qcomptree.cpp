/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qtypes.h"

IDSTR("$Id: qcomptree.cpp,v 1.1 2006/07/18 02:32:16 bmiller Exp $");


/****/

const qComputationNode emptyNode(edge.mv = 0,
                                 edge.eval = NULL,
                                 parentNodeIdx = qComputationTree::qComputationTreeNode_invalid,
                                 childNodes(0),
                                 childWithBestEvalScore=0,
                                 posInfo = NULL);

/**************************
 * class qComputationTree *
 **************************/
// Root node is always 1.  Makes life simple and good.
void qComputationTree::qComputationTree()
:nodeHeap(COMPTREE_INITIAL_SIZE)
{
  nodeNum = 2;
  maxNode = nodeHeap.size() - 1;
  qComputationNode rootNode = nodeHeap.at(1);
  rootNode.parentNodeIdx = qComputationTree::qComputationTreeNode_invalid;
  rootNode.edge.mv = moveNull;
  rootNode.edge.eval = NULL;
}

qComputationTree::~qComputationTree()
{ ; }

void qComputationTree::initializeTree()
{
  nodeNum = 2; // lowest free node
  while (maxNode < maxNode)
    growNodeHeap();
  qComputationNode rootNode = nodeHeap.at(1);
  rootNode.parentNodeIdx = qComputationTree::qComputationTreeNode_invalid;
  rootNode.edge.mv = moveNull;
  rootNode.edge.eval = NULL;
  childNodes.resize(0);
  /* guint16        childWithBestEvalScore; */ // No need to touch this
  /* qPositionInfo *posInfo;                */ // No need to touch this???
}

qComputationTreeNodeId qComputationTree::getRootNode() const
{ return 1; }

qComputationTreeNodeId qComputationTree::addNodeChild
(qComputationTreeNodeId node,
 qMove mv,
 const qPositionEvaluation *eval)


// Returns 0 if no such child;
// Children are ordered according to eval.score + eval.complexity
qComputationTreeNodeId qComputationTree::getNthChild
(qComputationTreeNodeId node, guint8 n) const
{
  qComputationNode myNode = nodeHeap.at(node);
  return myNode.childNodes.at(n);
}

qComputationTreeNodeId qComputationTree::getTopScoringChild
(qComputationTreeNodeId node) const
// Does this return best score+complexity, or just best score???

qComputationTreeNodeId qComputationTree::getNodeParent
(qComputationTreeNodeId node) const
{
  return nodeHeap.at(node).parentNodeIdx;
}

qPositionInfo *qComputationTree::getNodePosInfo
(qComputationTreeNodeId node) const
{
  return nodeHeap.at(node).posInfo;
}

void qComputationTree::setNodePosInfo
(qComputationTreeNodeId node, qPositionInfo *posInfo)
{
  nodeHeap.at(node).posInfo = posInfo;
}

void qComputationTree::setNodeEval
(qComputationTreeNodeId node,
 qPositionEvaluation const *eval)
{
  nodeHeap.at(node).edge.eval = eval;
}

qPositionEvaluation *qComputationTree::getNodeEval
(qComputationTreeNodeId node) const
{
  return nodeHeap.at(node).edge.eval;
}

qMove qComputationTree::getNodePrecedingMove
(qComputationTreeNodeId node) const
{
  return nodeHeap.at(node).edge.mv;
}


/**************************
 * class qComputationNode *
 **************************/

qComputationNode::qComputationNode()
:parentNodeIdx(0),
 posInfo(NULL)
{;};

qComputationNode::~qComputationNode()
{;};
