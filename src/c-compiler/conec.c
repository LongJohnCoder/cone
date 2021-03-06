/** Main program file
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#include "conec.h"
#include "coneopts.h"
#include "shared/fileio.h"
#include "ir/nametbl.h"
#include "ir/ir.h"
#include "shared/error.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "genllvm/genllvm.h"

#include <stdio.h>
#include <time.h>
#include <assert.h>

clock_t startTime;

// Run all semantic analysis passes against the AST/IR (after parse and before gen)
void doAnalysis(ModuleNode **mod) {
    PassState pstate;
    pstate.mod = *mod;
    pstate.typenode = NULL;
    pstate.fnsig = NULL;
    pstate.scope = 0;
    pstate.flags = 0;

    // Resolve all name uses to their appropriate declaration
    // Note: Some nodes may be replaced (e.g., 'a' to 'self.a')
    pstate.pass = NameResolution;
    inodeWalk(&pstate, (INode**)mod);
    if (errors)
        return;

    // Apply syntactic sugar, and perform type inference/check
    // Note: Some nodes may be lowered, injected or replaced
    pstate.pass = TypeCheck;
    inodeWalk(&pstate, (INode**)mod);
}

int main(int argc, char **argv) {
    ConeOptions coneopt;
    GenState gen;
    ModuleNode *modnode;
    int ok;

    // Start measuring processing time for compilation
    startTime = clock();

    // Get compiler's options from passed arguments
    ok = coneOptSet(&coneopt, &argc, argv);
    if (ok <= 0)
        exit(ok == 0 ? 0 : ExitOpts);
    if (argc < 2)
        errorExit(ExitOpts, "Specify a Cone program to compile.");
    coneopt.srcpath = argv[1];
    coneopt.srcname = fileName(coneopt.srcpath);

    // We set up generation early because we need target info, e.g.: pointer size
    genSetup(&gen, &coneopt);

    // Parse source file, do semantic analysis, and generate code
    modnode = parsePgm(&coneopt);
    if (errors == 0) {
        doAnalysis(&modnode);
        if (errors == 0) {
            if (coneopt.print_ir)
                inodePrint(coneopt.output, coneopt.srcpath, (INode*)modnode);
            genmod(&gen, modnode);
            genClose(&gen);
        }
    }

    // Close up everything necessary
    errorSummary();
#ifdef _DEBUG
    getchar();    // Hack for VS debugging
#endif
}
