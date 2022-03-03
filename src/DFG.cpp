// Declares clang::SyntaxOnlyAction.
#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

// using namespace clang::tooling;
// using namespace llvm;

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;


// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

int numFunctions = 0;

class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {
private:
    ASTContext *astContext; // used for getting additional AST info

public:
    explicit ExampleVisitor(CompilerInstance *CI) 
      : astContext(&(CI->getASTContext())) // initialize private members
    {
        // rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());
    }

    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        numFunctions++;
        string funcName = func->getNameInfo().getName().getAsString();
        // if (funcName == "do_math") {
        //     rewriter.ReplaceText(func->getNameInfo().getLoc(), funcName.length(), "add5");
        //     errs() << "** Rewrote function def: " << funcName << "\n";
        // }    
        errs() << "Hello: " << funcName << "\n";
        return true;
    }

    // virtual bool VisitStmt(Stmt *st) {
    //     if (ReturnStmt *ret = dyn_cast<ReturnStmt>(st)) {
    //         rewriter.ReplaceText(ret->getRetValue()->getExprLoc(), 6, "val");
    //         errs() << "** Rewrote ReturnStmt\n";
    //     }        
    //     if (CallExpr *call = dyn_cast<CallExpr>(st)) {
    //         rewriter.ReplaceText(call->getBeginLoc(), 7, "add5");
    //         errs() << "** Rewrote function call\n";
    //     }
    //     return true;
    // }

/*
    virtual bool VisitReturnStmt(ReturnStmt *ret) {
        rewriter.ReplaceText(ret->getRetValue()->getLocStart(), 6, "val");
        errs() << "** Rewrote ReturnStmt\n";
        return true;
    }

    virtual bool VisitCallExpr(CallExpr *call) {
        rewriter.ReplaceText(call->getLocStart(), 7, "add5");
        errs() << "** Rewrote function call\n";
        return true;
    }
*/
};



class ExampleASTConsumer : public ASTConsumer {
private:
    ExampleVisitor *visitor; // doesn't have to be private

public:
    // override the constructor in order to pass CI
    explicit ExampleASTConsumer(CompilerInstance *CI)
        : visitor(new ExampleVisitor(CI)) // initialize the visitor
    { }

    // override this to call our ExampleVisitor on the entire source file
    virtual void HandleTranslationUnit(ASTContext &Context) {
        /* we can use ASTContext to get the TranslationUnitDecl, which is
             a single Decl that collectively represents the entire source file */
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
    }

/*
    // override this to call our ExampleVisitor on each top-level Decl
    virtual bool HandleTopLevelDecl(DeclGroupRef DG) {
        // a DeclGroupRef may have multiple Decls, so we iterate through each one
        for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; i++) {
            Decl *D = *i;    
            visitor->TraverseDecl(D); // recursively visit each AST node in Decl "D"
        }
        return true;
    }
*/
};



class ExampleFrontendAction : public ASTFrontendAction {
public:
    virtual unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) {
        return unique_ptr<ASTConsumer>(new ExampleASTConsumer(&CI)); // pass CI pointer to ASTConsumer
    }
};

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    // Fail gracefully for unsupported options.
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser& OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  return Tool.run(newFrontendActionFactory<ExampleFrontendAction>().get());
}