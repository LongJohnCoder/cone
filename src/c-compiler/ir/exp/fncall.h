/** Handling for expression nodes that might do copy/move
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#ifndef fncall_h
#define fncall_h

// Function or method call node. Also used for array indexing and field access.
// The parsed contents is lowered during type checking, potentially turning
// it into an ArrIndexTag or StrFieldTag node
typedef struct FnCallNode {
    ITypedNodeHdr;
    INode *objfn;        // Object (for method calls) or function to call
    NameUseNode *methprop;    // Name of method/property (or NULL)
    Nodes *args;           // List of function call arguments (or NULL)
} FnCallNode;

FnCallNode *newFnCallNode(INode *objfn, int nnodes);
// Create new fncall node, prefilling method, self, and creating room for nnodes args
FnCallNode *newFnCallOpname(INode *obj, Name *opname, int nnodes);
FnCallNode *newFnCallOp(INode *obj, char *op, int nnodes);
void fnCallPrint(FnCallNode *node);
void fnCallPass(PassState *pstate, FnCallNode **node);

// Do data flow analysis for fncall node (only real function calls)
void fnCallFlow(FlowState *fstate, FnCallNode **nodep);

#endif