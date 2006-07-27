/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qcomptree.h"

IDSTR("$Id: qcomptree.cpp,v 1.5 2006/07/27 05:59:27 bmiller Exp $");


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
  /* guint16        childWithBestEval; */ // No need to touch this
  /* qPositionInfo *posInfo;                */ // No need to touch this???
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
  qComputationNode &newNode = nodeHeap.at(nodeNum);
  newNode.parentNodeIdx = node;
  g_assert(node < nodeNum);
  newNode.childNodes.resize(0);
  newNode.childWithBestEval= 0;
  newNode.mv   = mv;
  newNode.eval = eval;
  newNode.posInfo = NULL;

  // Insert in sorted order by - eval.score - eval.complexity (per qcomptree.h)
  gint32 score = static_cast<gint32>(eval->score) - eval->complexity;
  std::deque<qComputationTreeNodeId>::iterator itr(parentNode.childNodes.begin());
  while( itr != parentNode.childNodes.end() )
  {
    qComputationNode &tmpNode = nodeHeap.at(*itr);
    
    if (tmpNode.eval->score <= score + tmpNode.eval->complexity)
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

// Returns 0 if no such child;
// Children are ordered according to eval.score + eval.complexity
qComputationTreeNodeId qComputationTree::getNthChild
(qComputationTreeNodeId node, guint8 n) const
{
  if (nodeHeap.at(node).childNodes.empty())
    return qComputationTreeNode_invalid;
  if (n < nodeHeap.at(node).childNodes.size())
    return nodeHeap.at(node).childNodes.at(n);
  else
    return qComputationTreeNode_invalid;
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
  std::deque<qComputationTreeNodeId>::iterator itr(n.childNodes.begin());
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
    std::deque<qComputationTreeNodeId>::iterator itr(parent.childNodes.begin());
    while (*itr != node)
      ++itr;

    parent.childNodes.erase(itr);
    if (parent.childWithBestEval == node)
      resetBestChild(parent);
    return;
  }

  gint32 score = static_cast<gint32>(eval->score) - eval->complexity;

  bool improved = FALSE;
  const qPositionEvaluation *prevEval = nodeHeap.at(node).eval;
  if (prevEval &&  (score + prevEval->complexity < prevEval->score))
    improved = TRUE;

  nodeHeap.at(node).eval = eval;

  if (node != 1) {
    qComputationNode &parent = nodeHeap.at(nodeHeap.at(node).parentNodeIdx);
    if (!parent.childWithBestEval)
      parent.childWithBestEval = node;
    else if (parent.childWithBestEval == node) {
      if (!improved) // This was the best node but it worsened...recheck it
        resetBestChild(parent);
    } else if (eval->score <
             getNodeEval(parent.childWithBestEval)->score)
      parent.childWithBestEval = node;

    { // Reorder node in parent's child list according to new score
      std::deque<qComputationTreeNodeId>::iterator
        itr(parent.childNodes.begin());
      while (*itr != node)
        ++itr;

      itr = parent.childNodes.erase(itr);
      if (improved) {
        while( itr != parent.childNodes.begin() )
        {
          --itr;
          qComputationNode &tmpNode = nodeHeap.at(*itr);
  
          if (tmpNode.eval->score <= score + tmpNode.eval->complexity) {
            itr++;
            break;
          }
        }
      } else {
        while( itr != parent.childNodes.end() )
        {
          qComputationNode &tmpNode = nodeHeap.at(*itr);
  
          if (tmpNode.eval->score >= score + tmpNode.eval->complexity)
            break;
          itr++;
        }
      }
      parent.childNodes.insert(itr, node);
    }
  }
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
