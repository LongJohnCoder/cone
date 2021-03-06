/** Handling for borrow expression nodes
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#ifndef borrrw_h
#define borrow_h

typedef struct BorrowNode {
    ITypedNodeHdr;
    INode *exp;
} BorrowNode;

BorrowNode *newBorrowNode();
void borrowPrint(BorrowNode *node);
void borrowPass(PassState *pstate, BorrowNode **node);

// Perform data flow analysis on addr node
void borrowFlow(FlowState *fstate, BorrowNode **nodep);

// Insert automatic ref, if node is a variable
void borrowAuto(INode **node, INode* reftype);

#endif