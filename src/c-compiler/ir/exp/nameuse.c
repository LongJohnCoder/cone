/** Name and Member Use nodes.
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#include "../ir.h"
#include "../../shared/memory.h"
#include "../../parser/lexer.h"
#include "../nametbl.h"
#include "../../shared/error.h"

#include <string.h>
#include <assert.h>

// A list of module names (qualifiers)
typedef struct NameList {
    uint16_t avail;     // Max. number of names allocated for
    uint16_t used;      // Number of names stored in list
    ModuleAstNode *basemod;  // base module (root or current) holding qualifiers
    // Name* pointers for qualifiers follow, starting here
} NameList;

// Create a new name use node
NameUseAstNode *newNameUseNode(Name *namesym) {
	NameUseAstNode *name;
	newNode(name, NameUseAstNode, NameUseTag);
	name->qualNames = NULL;
	name->dclnode = NULL;
    name->namesym = namesym;
	return name;
}

NameUseAstNode *newMemberUseNode(Name *namesym) {
	NameUseAstNode *name;
	newNode(name, NameUseAstNode, MbrNameUseTag);
    name->qualNames = NULL;
    name->dclnode = NULL;
    name->namesym = namesym;
	return name;
}

// If a NameUseAstNode has module name qualifiers, it will first set basemod
// (either root module or the current module scope). This allocates an area
// for qualifiers to be added.
void nameUseBaseMod(NameUseAstNode *node, ModuleAstNode *basemod) {
    node->qualNames = (NameList *)memAllocBlk(sizeof(NameList) + 4 * sizeof(Name*));
    node->qualNames->avail = 4;
    node->qualNames->used = 0;
    node->qualNames->basemod = basemod;
}

// Add a module name qualifier to the end of the list
void nameUseAddQual(NameUseAstNode *node, Name *name) {
    int16_t used = node->qualNames->used;
    if (used + 1 >= node->qualNames->avail) {
        NameList *oldlist = node->qualNames;
        uint16_t newavail = oldlist->avail << 1;
        node->qualNames = (NameList *)memAllocBlk(sizeof(NameList) + newavail * sizeof(Name*));
        node->qualNames->avail = newavail;
        node->qualNames->used = used;
        node->qualNames->basemod = oldlist->basemod;
        Name **oldp = (Name**)(oldlist + 1);
        Name **newp = (Name**)(node->qualNames + 1);
        int16_t cnt = used;
        while (cnt--)
            *newp++ = *oldp++;
    }
    Name **namep = (Name**)&(node->qualNames + 1)[used];
    *namep = name;
    ++node->qualNames->used;
}

// Serialize the AST for a name use
void nameUsePrint(NameUseAstNode *name) {
    if (name->qualNames) {
        // if root: inodeFprint("::");
        int16_t cnt = name->qualNames->used;
        Name **namep = (Name**)(name->qualNames + 1);
        while (cnt--)
            inodeFprint("%s::", &(*namep++)->namestr);
    }
	inodeFprint("%s", &name->namesym->namestr);
}

// Handle name resolution and type check for name use references
void nameUseWalk(PassState *pstate, NameUseAstNode **namep) {
    NameUseAstNode *name = *namep;
	// During name resolution, point to name declaration and copy over needed properties
	switch (pstate->pass) {
	case NameResolution:
        if (name->qualNames) {
            // Do iterative look ups of module qualifiers beginning with basemod
            ModuleAstNode *mod = name->qualNames->basemod;
            int16_t cnt = name->qualNames->used;
            Name **namep = (Name**)(name->qualNames + 1);
            while (cnt--) {
                mod = (ModuleAstNode*)namespaceFind(&mod->namednodes, *namep++);
                if (mod==NULL || mod->asttype!=ModuleNode) {
                    errorMsgNode((INode*)name, ErrorUnkName, "Module %s does not exist", &(*--namep)->namestr);
                    return;
                }
            }
            name->dclnode = namespaceFind(&mod->namednodes, name->namesym);
        }
        else
            // For current module, should already be hooked in global name table
			name->dclnode = (NamedAstNode*)name->namesym->node;

        // If variable is actually an instance property, rewrite it to 'self.property'
        if (name->dclnode->asttype == VarDclTag && name->dclnode->flags & FlagMethProp) {
            // Doing this rewrite ensures we reuse existing type check and gen code for
            // properly handling property access
            NameUseAstNode *selfnode = newNameUseNode(selfName);
            FnCallAstNode *fncall = newFnCallAstNode((INode *)selfnode, 0);
            fncall->methprop = name;
            copyNodeLex(fncall, name); // Copy lexer info into injected node in case it has errors
            *((FnCallAstNode**)namep) = fncall;
            inodeWalk(pstate, (INode **)namep);
        }
        // Make it easy to distinguish whether a name is for a variable/function name vs. type
        else if (name->dclnode) {
            if (name->dclnode->asttype == VarDclTag || name->dclnode->asttype == FnDclTag)
                name->asttype = VarNameUseTag;
            else
                name->asttype = TypeNameUseTag;
        }
        else
			errorMsgNode((INode*)name, ErrorUnkName, "The name %s does not refer to a declared name", &name->namesym->namestr);
		break;
	case TypeCheck:
		name->vtype = name->dclnode->vtype;
		break;
	}
}
