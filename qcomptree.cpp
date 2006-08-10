/*
 * Copyright (c) 2005-2006
 *    Brent Miller and Charles Morrey.  All rights reserved.
 *
 * See the COPYRIGHT_NOTICE file for terms.
 */


#include "qcomptree.h"

IDSTR("$Id: qcomptree.cpp,v 1.10 2006/08/10 07:40:02 bmiller Exp $");


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

  return nodeNum++;
}

qComputationTreeNodeId qComputationTree::getRootNode() const
{ return 1; }

const qComputationTreeNodeList *qComputationTree::getNodeChildList
(qComputationTreeNodeId node) const
{
  return &(nodeHeap.at(node).childNodes);
}
bool qComputationTree::nodeHasChildList(qComputationTreeNodeId node) const
{
  return (nodeHeap.at(node).childNodes.size() != 0);
}

qComputationTreeNodeId qComputationTree::sortNodeChildList
(qComputationTreeNodeId node)
{
  qComputationTreeNodeList &childList = nodeHeap.at(node).childNodes;

  // Bubblicious sort (OPTIMIZE???  Bubble may be ok since lists are nearly sorted)
  qComputationTreeNodeListIterator itr(childList.begin());
  if (itr==childList.end())
    return qComputationTreeNode_invalid;

  register const qPositionEvaluation *eval = getNodeEval(*itr);
  gint32 prev_score = static_cast<gint32>(eval->score) - eval->complexity;
  gint32 new_score = 0;

  qComputationTreeNodeId childNodeWithBestEval = *itr;
  gint16 bestScore = eval->score;

  ++itr;

  // Find out-of order nodes, and immediately bubble up the better node until it's in order.
  // This is quasi-bubble sort/quasi-insert sort.  Since we bubble each elt as far as needed to
  // put it in order (instead of one position per swap), we sort in one pass.
  while (itr != childList.end()) {
    eval = getNodeEval(*itr);

    if (eval->score < bestScore) {
      childNodeWithBestEval = *itr;
      bestScore = eval->score;
    }

    new_score = static_cast<gint32>(eval->score) - eval->complexity;
    if (new_score >= prev_score) {
      prev_score = new_score;
      itr++;
      continue;
    }

    // Found an out-of-order pair; move the better elt up until it's in order
    qComputationTreeNodeId jettsonNode /* It's movin on up */ = *itr;
    itr = childList.erase(itr);
    
    // OPTIMIZATION:  can we use an iterator to remember the location where we left off???
    --itr;
    while( itr != childList.begin() )
      {
        eval = getNodeEval(*itr);
        if (new_score + eval->complexity >= eval->score) {
          itr++;
          break;
        }
        --itr;
      }
    itr = childList.insert(itr, jettsonNode);
    prev_score = new_score; // OPTIMIZATION: move forward all the way to where we left off.
    ++itr;
  }
  return childNodeWithBestEval;
}

qComputationTreeNodeId qComputationTree::getBestScoringChild
(qComputationTreeNodeId node) const
// Return best (i.e. lowest, since we're interested in opponent) score
{
  if (!node)
    return qComputationTreeNode_invalid;

  const qComputationTreeNodeList* childList = this->getNodeChildList(node);

  qComputationTreeNodeListConstIterator itr(childList->begin());
  qComputationTreeNodeListConstIterator end(childList->end());

  if (itr == end)
    return qComputationTreeNode_invalid;

  qComputationTreeNodeId childWithBestEval = *itr;
  gint16                         bestScore = nodeHeap.at(childWithBestEval).eval->score;

  while (itr != end) {
    if (nodeHeap.at(*itr).eval->score < bestScore) {
      childWithBestEval = *itr;
      bestScore = nodeHeap.at(childWithBestEval).eval->score;
    }
    ++itr;
  }
  g_assert(nodeHeap.at(childWithBestEval).eval->score <= nodeHeap.at(getNodeChildList(node)->front()).eval->score);
  g_assert(nodeHeap.at(childWithBestEval).eval->score <= nodeHeap.at(getNodeChildList(node)->back()).eval->score);
  return childWithBestEval;
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
    return;
  }

  // Adjust parent node's sorted child list & associated data as needed
  if (node == 1)
    nodeHeap.at(node).eval = eval; // Don't need to do anything else for the root node
  else {
    qComputationNode &parent = nodeHeap.at(nodeHeap.at(node).parentNodeIdx);
    gint32 score = static_cast<gint32>(eval->score) - eval->complexity;

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

    nodeHeap.at(node).eval = eval;
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

#ifdef DEBUG
qComputationTreeNodeId qComputationTree::getNodeNthChild
(qComputationTreeNodeId node, int n)
{
  if ((n >= nodeHeap.at(node).childNodes.size()) ||
      (n < 0))
    return qComputationTreeNode_invalid;

  const qComputationTreeNodeList* childList = this->getNodeChildList(node);

  qComputationTreeNodeListConstIterator itor = childList->begin();
  qComputationTreeNodeListConstIterator end  = childList->end();

  while (n--) {
    itor++;
    if (itor == end)
      return qComputationTreeNode_invalid;
  }

  return *itor;
}
#endif


/**************************
 * class qComputationNode *
 **************************/
// Constructor & destructor moved to qcomptree.h
