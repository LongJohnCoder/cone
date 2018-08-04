/** AST handling for structs
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#include "../ir.h"
#include "../../shared/memory.h"
#include "../../parser/lexer.h"
#include "../nametbl.h"

// Create a new array type whose info will be filled in afterwards
ArrayAstNode *newArrayNode() {
	ArrayAstNode *anode;
	newNode(anode, ArrayAstNode, ArrayType);
    anode->vtype = NULL;
    anode->owner = NULL;
    anode->namesym = anonName;
    anode->llvmtype = NULL;
    methnodesInit(&anode->methprops, 0);
    anode->subtypes = newNodes(0);
    return anode;
}

// Serialize an array type
void arrayPrint(ArrayAstNode *node) {
	inodeFprint("[%d]", (int)node->size);
	inodePrintNode(node->elemtype);
}

// Semantically analyze an array type
void arrayPass(PassState *pstate, ArrayAstNode *node) {
	inodeWalk(pstate, &node->elemtype);
}

// Compare two struct signatures to see if they are equivalent
int arrayEqual(ArrayAstNode *node1, ArrayAstNode *node2) {
	return (node1->size == node2->size
		/* && typeEqual(node1->elemtype, node2->elemtype*/);
}
