/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qcomptree.h"

IDSTR("$Id: qcomptree.cpp,v 1.9 2006/08/02 04:08:07 bmiller Exp $");


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
  g_assert(maxNode > nodeNum);
  qComputationNode rootNode = nodeHeap.at(1);
  rootNode.parentNodeIdx = qComputationTreeNode_invalid;
  rootNode.mv = moveNull;
  rootNode.eval = NULL;
  rootNode.childNodes.resize(0);
  rootNode.childWithBestEval=0;
  rootNode.posInfo=NULL;
}

qComputationTreeNodeId qComputationTree::addNodeChild
(qComputationTreeNodeId     node,
 qMove                      mv,
 const qPositionEvaluation *eval)
{
  while (maxNode < nodeNum)
    if (!growNodeHeap())
      return qComputationTreeNode_invalid;
  qComputationNode &parentNode  = nodeHeap.at(node);
  qComputationNode &newNode = nodeHeap[nodeNum];
  g_assert(node < nodeNum);
  newNode.parentNodeIdx = node;
  newNode.childNodes.resize(0);
  newNode.childWithBestEval= 0;
  newNode.mv   = mv;
  newNode.eval = eval;
  newNode.posInfo = NULL;

  // Insert in sorted order by - eval.score - eval.complexity (per qcomptree.h)
  gint32 score = static_cast<gint32>(eval->score) - eval->complexity;
  std::list<qComputationTreeNodeId>::iterator itr(parentNode.childNodes.begin());
  while( itr != parentNode.childNodes.end() )
  {
    qComputationNode &tmpNode = nodeHeap.at(*itr);
    
    if (tmpNode.eval->score >= score + tmpNode.eval->complexity)
      break;
    itr++;
  }
  parentNode.childNodes.insert(itr, nodeNum);

  // Update parent's childWithBestEval record, if needed
  if (!parentNode.childWithBestEval)
    parentNode.childWithBestEval = nodeNum;
  else if (eval->score < getNodeEval(parentNode.childWithBestEval)->score)
    parentNode.childWithBestEval = nodeNum;

  return nodeNum++;
}

qComputationTreeNodeId qComputationTree::getRootNode() const
{ return 1; }

qComputationTreeNodeList *qComputationTree::getNodeChildList
(qComputationTreeNodeId node)
{
  return &(nodeHeap.at(node).childNodes);
}
bool qComputationTree::nodeHasChildList(qComputationTreeNodeId node)
{
  return (nodeHeap.at(node).childNodes.size() != 0);
}


qComputationTreeNodeId qComputationTree::getBestScoringChild
(qComputationTreeNodeId node) const
// Return best (i.e. lowest, since we're interested in opponent) score
{
  return nodeHeap.at(node).childWithBestEval;
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

void qComputationTree::resetBestChild
(qComputationNode &n)
{
  std::list<qComputationTreeNodeId>::iterator itr(n.childNodes.begin());
  if (itr == n.childNodes.end())
    n.childWithBestEval = 0;
  else {
    n.childWithBestEval = *itr;
    gint16 bestScore = nodeHeap.at(*itr).eval->score;

    while (itr != n.childNodes.end()) {
      if (nodeHeap.at(*itr).eval->score < bestScore) {
        n.childWithBestEval = *itr;
        bestScore = nodeHeap.at(*itr).eval->score;
      }
      /* Let's not try optimizing 'til the basic stuff works.  (???)
       * else if (bestScore >
       *     nodeHeap.at(*itr).eval->score + nodeHeap.at(*itr).eval->complexity)
       *   break; // Remaining nodes can't have a score higher than bestScore
       */
      ++itr;
    }
  }
}

void qComputationTree::setNodeEval
(qComputationTreeNodeId     node,
 const qPositionEvaluation *eval)
{
  if (!eval) {
    // Illegal move.  Remove it from tree.
    g_assert(node > 1);
    if (node == 1)
      return;

    qComputationNode &parent = nodeHeap.at(nodeHeap.at(node).parentNodeIdx);
    std::list<qComputationTreeNodeId>::iterator itr(parent.childNodes.begin());
    while (*itr != node)
      ++itr;

    parent.childNodes.erase(itr);
    if (parent.childWithBestEval == node)
      resetBestChild(parent);
    return;
  }

  // Adjust parent node's sorted child list & associated data as needed
  if (node != 1) {
    gint32 score = static_cast<gint32>(eval->score) - eval->complexity;

    // Modify best child, if it changed
    qComputationNode &parent = nodeHeap.at(nodeHeap.at(node).parentNodeIdx);
    if (!parent.childWithBestEval)
      parent.childWithBestEval = node;
    else if (parent.childWithBestEval == node) {
      // Note:  I was trying to examine the old value in nodeHeap[node] and
      // only reset if the score worsened, but can we trust that the contents
      // of the eval pointer didn't change?  In case not, better recompute.
      resetBestChild(parent);
    } else if (eval->score <
             getNodeEval(parent.childWithBestEval)->score)
      parent.childWithBestEval = node;

    { // Reorder node in parent's child list according to new score
      std::list<qComputationTreeNodeId>::iterator
        itr(parent.childNodes.begin());
      while (*itr != node)
        ++itr;

      itr = parent.childNodes.erase(itr);

      bool maybeImproved=TRUE;
      while( itr != parent.childNodes.end() )
        {
          qComputationNode &tmpNode = nodeHeap.at(*itr);
  
          if (tmpNode.eval->score >= score + tmpNode.eval->complexity)
            break;

          itr++;
          maybeImproved=FALSE;
        }

      if (maybeImproved)
        while( itr != parent.childNodes.begin() )
          {
            --itr;
            qComputationNode &tmpNode = nodeHeap.at(*itr);
  
            if (tmpNode.eval->score <= score + tmpNode.eval->complexity) {
              itr++;
              break;
            }
          }
      parent.childNodes.insert(itr, node);
    }
  }
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
