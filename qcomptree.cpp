/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qcomptree.h"

IDSTR("$Id: qcomptree.cpp,v 1.2 2006/07/18 06:55:33 bmiller Exp $");


/****/

const qComputationNode emptyNode; // Default constructor is empty node

/**************************
 * class qComputationTree *
 **************************/
// Root node is always 1.  Makes life simple and good.
qComputationTree::qComputationTree()
:nodeHeap(COMPTREE_INITIAL_SIZE)
{
  nodeNum = 2;
  maxNode = nodeHeap.size() - 1;
  qComputationNode rootNode = nodeHeap.at(1);
  rootNode.parentNodeIdx = qComputationTreeNode_invalid;
  rootNode.mv = moveNull;
  rootNode.eval = NULL;
  rootNode.childNodes.resize(0);
}

qComputationTree::~qComputationTree()
{ ; }

void qComputationTree::initializeTree()
{
  nodeNum = 2; // lowest free node
  while (maxNode < nodeNum)
    growNodeHeap();
  qComputationNode rootNode = nodeHeap.at(1);
  rootNode.parentNodeIdx = qComputationTreeNode_invalid;
  rootNode.mv = moveNull;
  rootNode.eval = NULL;
  rootNode.childNodes.resize(0);
  /* guint16        childWithBestEvalScore; */ // No need to touch this
  /* qPositionInfo *posInfo;                */ // No need to touch this???
}

qComputationTreeNodeId qComputationTree::addNodeChild
(qComputationTreeNodeId     node,
 qMove                      mv,
 const qPositionEvaluation *eval)
{
  while (maxNode < nodeNum)
    growNodeHeap();
  qComputationNode &parentNode  = nodeHeap.at(node);
  qComputationNode &newNode = nodeHeap.at(nodeNum);
  newNode.parentNodeIdx = node;
  newNode.childNodes.resize(0);
  newNode.childWithBestEvalScore = 0;
  newNode.mv   = mv;
  newNode.eval = eval;
  newNode.posInfo = NULL;

  // Insert in sorted order by eval.score + eval.complexity (per qcomptree.h)
  gint32 score = eval->score + eval->complexity;
  std::vector<qComputationTreeNodeId>::iterator itr(parentNode.childNodes.begin());
  while( itr != parentNode.childNodes.end() )
  {
    qComputationNode &tmpNode = nodeHeap.at(*itr);
    
    if (tmpNode.eval->score + tmpNode.eval->complexity <= score )
      break;
    itr++;
  }
  parentNode.childNodes.insert(itr, nodeNum);
  ++nodeNum;
}

qComputationTreeNodeId qComputationTree::getRootNode() const
{ return 1; }

// Returns 0 if no such child;
// Children are ordered according to eval.score + eval.complexity
qComputationTreeNodeId qComputationTree::getNthChild
(qComputationTreeNodeId node, guint8 n) const
{
  return nodeHeap.at(node).childNodes.at(n);
}

qComputationTreeNodeId qComputationTree::getTopScoringChild
(qComputationTreeNodeId node) const
// Does this return best score+complexity, or just best score???
{
  if (nodeHeap.empty())
    return qComputationTreeNode_invalid;
  return nodeHeap.at(0).parentNodeIdx;
}

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
(qComputationTreeNodeId     node,
 const qPositionEvaluation *eval)
{
  nodeHeap.at(node).eval = eval;
}

const qPositionEvaluation *qComputationTree::getNodeEval
(qComputationTreeNodeId node) const
{
  return nodeHeap.at(node).eval;
}

qMove qComputationTree::getNodePrecedingMove
(qComputationTreeNodeId node) const
{
  return nodeHeap.at(node).mv;
}


/**************************
 * class qComputationNode *
 **************************/
// Constructor & destructor moved to qcomptree.h
