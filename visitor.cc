#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;
class Visitor : public clang::RecursiveASTVisitor<Visitor>
{
public:
  explicit Visitor(clang::ASTContext& context)
    : sourceManager_(context.getSourceManager()),
      langOpts_(context.getLangOpts())
  {
  }

#define PRINT printf("%*s%s %p\n", getIndent(p), "", __FUNCTION__, p)
#define NL printf("\n")

  bool VisitAttr(Attr *p) { NL; PRINT; return true; }
  bool VisitARMInterruptAttr(ARMInterruptAttr *p) { PRINT; return true; };
  bool VisitAcquireCapabilityAttr(AcquireCapabilityAttr *p) { PRINT; return true; };
  bool VisitAcquiredAfterAttr(AcquiredAfterAttr *p) { PRINT; return true; };
  bool VisitAcquiredBeforeAttr(AcquiredBeforeAttr *p) { PRINT; return true; };
  bool VisitAliasAttr(AliasAttr *p) { PRINT; return true; };
  bool VisitAlignMac68kAttr(AlignMac68kAttr *p) { PRINT; return true; };
  bool VisitAlignedAttr(AlignedAttr *p) { PRINT; return true; };
  bool VisitAlwaysInlineAttr(AlwaysInlineAttr *p) { PRINT; return true; };
  bool VisitAnalyzerNoReturnAttr(AnalyzerNoReturnAttr *p) { PRINT; return true; };
  bool VisitAnnotateAttr(AnnotateAttr *p) { PRINT; return true; };
  bool VisitArcWeakrefUnavailableAttr(ArcWeakrefUnavailableAttr *p) { PRINT; return true; };
  bool VisitArgumentWithTypeTagAttr(ArgumentWithTypeTagAttr *p) { PRINT; return true; };
  bool VisitAsmLabelAttr(AsmLabelAttr *p) { PRINT; return true; };
  bool VisitAssertCapabilityAttr(AssertCapabilityAttr *p) { PRINT; return true; };
  bool VisitAssertExclusiveLockAttr(AssertExclusiveLockAttr *p) { PRINT; return true; };
  bool VisitAssertSharedLockAttr(AssertSharedLockAttr *p) { PRINT; return true; };
  bool VisitAvailabilityAttr(AvailabilityAttr *p) { PRINT; return true; };
  bool VisitBlocksAttr(BlocksAttr *p) { PRINT; return true; };
  bool VisitC11NoReturnAttr(C11NoReturnAttr *p) { PRINT; return true; };
  bool VisitCDeclAttr(CDeclAttr *p) { PRINT; return true; };
  bool VisitCFAuditedTransferAttr(CFAuditedTransferAttr *p) { PRINT; return true; };
  bool VisitCFConsumedAttr(CFConsumedAttr *p) { PRINT; return true; };
  bool VisitCFReturnsNotRetainedAttr(CFReturnsNotRetainedAttr *p) { PRINT; return true; };
  bool VisitCFReturnsRetainedAttr(CFReturnsRetainedAttr *p) { PRINT; return true; };
  bool VisitCFUnknownTransferAttr(CFUnknownTransferAttr *p) { PRINT; return true; };
  bool VisitCUDAConstantAttr(CUDAConstantAttr *p) { PRINT; return true; };
  bool VisitCUDADeviceAttr(CUDADeviceAttr *p) { PRINT; return true; };
  bool VisitCUDAGlobalAttr(CUDAGlobalAttr *p) { PRINT; return true; };
  bool VisitCUDAHostAttr(CUDAHostAttr *p) { PRINT; return true; };
  bool VisitCUDALaunchBoundsAttr(CUDALaunchBoundsAttr *p) { PRINT; return true; };
  bool VisitCUDASharedAttr(CUDASharedAttr *p) { PRINT; return true; };
  bool VisitCXX11NoReturnAttr(CXX11NoReturnAttr *p) { PRINT; return true; };
  bool VisitCallableWhenAttr(CallableWhenAttr *p) { PRINT; return true; };
  bool VisitCapabilityAttr(CapabilityAttr *p) { PRINT; return true; };
  bool VisitCarriesDependencyAttr(CarriesDependencyAttr *p) { PRINT; return true; };
  bool VisitCleanupAttr(CleanupAttr *p) { PRINT; return true; };
  bool VisitColdAttr(ColdAttr *p) { PRINT; return true; };
  bool VisitCommonAttr(CommonAttr *p) { PRINT; return true; };
  bool VisitConstAttr(ConstAttr *p) { PRINT; return true; };
  bool VisitConstructorAttr(ConstructorAttr *p) { PRINT; return true; };
  bool VisitConsumableAttr(ConsumableAttr *p) { PRINT; return true; };
  bool VisitConsumableAutoCastAttr(ConsumableAutoCastAttr *p) { PRINT; return true; };
  bool VisitConsumableSetOnReadAttr(ConsumableSetOnReadAttr *p) { PRINT; return true; };
  bool VisitDLLExportAttr(DLLExportAttr *p) { PRINT; return true; };
  bool VisitDLLImportAttr(DLLImportAttr *p) { PRINT; return true; };
  bool VisitDeprecatedAttr(DeprecatedAttr *p) { PRINT; return true; };
  bool VisitDestructorAttr(DestructorAttr *p) { PRINT; return true; };
  bool VisitEnableIfAttr(EnableIfAttr *p) { PRINT; return true; };
  bool VisitExclusiveTrylockFunctionAttr(ExclusiveTrylockFunctionAttr *p) { PRINT; return true; };
  bool VisitFallThroughAttr(FallThroughAttr *p) { PRINT; return true; };
  bool VisitFastCallAttr(FastCallAttr *p) { PRINT; return true; };
  bool VisitFinalAttr(FinalAttr *p) { PRINT; return true; };
  bool VisitFlattenAttr(FlattenAttr *p) { PRINT; return true; };
  bool VisitFormatAttr(FormatAttr *p) { PRINT; return true; };
  bool VisitFormatArgAttr(FormatArgAttr *p) { PRINT; return true; };
  bool VisitGNUInlineAttr(GNUInlineAttr *p) { PRINT; return true; };
  bool VisitGuardedByAttr(GuardedByAttr *p) { PRINT; return true; };
  bool VisitGuardedVarAttr(GuardedVarAttr *p) { PRINT; return true; };
  bool VisitHotAttr(HotAttr *p) { PRINT; return true; };
  bool VisitIBActionAttr(IBActionAttr *p) { PRINT; return true; };
  bool VisitIBOutletAttr(IBOutletAttr *p) { PRINT; return true; };
  bool VisitIBOutletCollectionAttr(IBOutletCollectionAttr *p) { PRINT; return true; };
  bool VisitInitPriorityAttr(InitPriorityAttr *p) { PRINT; return true; };
  bool VisitInitSegAttr(InitSegAttr *p) { PRINT; return true; };
  bool VisitIntelOclBiccAttr(IntelOclBiccAttr *p) { PRINT; return true; };
  bool VisitLockReturnedAttr(LockReturnedAttr *p) { PRINT; return true; };
  bool VisitLocksExcludedAttr(LocksExcludedAttr *p) { PRINT; return true; };
  bool VisitLoopHintAttr(LoopHintAttr *p) { PRINT; return true; };
  bool VisitMSABIAttr(MSABIAttr *p) { PRINT; return true; };
  bool VisitMSInheritanceAttr(MSInheritanceAttr *p) { PRINT; return true; };
  bool VisitMSP430InterruptAttr(MSP430InterruptAttr *p) { PRINT; return true; };
  bool VisitMSVtorDispAttr(MSVtorDispAttr *p) { PRINT; return true; };
  bool VisitMallocAttr(MallocAttr *p) { PRINT; return true; };
  bool VisitMaxFieldAlignmentAttr(MaxFieldAlignmentAttr *p) { PRINT; return true; };
  bool VisitMayAliasAttr(MayAliasAttr *p) { PRINT; return true; };
  bool VisitMinSizeAttr(MinSizeAttr *p) { PRINT; return true; };
  bool VisitMips16Attr(Mips16Attr *p) { PRINT; return true; };
  bool VisitModeAttr(ModeAttr *p) { PRINT; return true; };
  bool VisitMsStructAttr(MsStructAttr *p) { PRINT; return true; };
  bool VisitNSConsumedAttr(NSConsumedAttr *p) { PRINT; return true; };
  bool VisitNSConsumesSelfAttr(NSConsumesSelfAttr *p) { PRINT; return true; };
  bool VisitNSReturnsAutoreleasedAttr(NSReturnsAutoreleasedAttr *p) { PRINT; return true; };
  bool VisitNSReturnsNotRetainedAttr(NSReturnsNotRetainedAttr *p) { PRINT; return true; };
  bool VisitNSReturnsRetainedAttr(NSReturnsRetainedAttr *p) { PRINT; return true; };
  bool VisitNakedAttr(NakedAttr *p) { PRINT; return true; };
  bool VisitNoCommonAttr(NoCommonAttr *p) { PRINT; return true; };
  bool VisitNoDebugAttr(NoDebugAttr *p) { PRINT; return true; };
  bool VisitNoDuplicateAttr(NoDuplicateAttr *p) { PRINT; return true; };
  bool VisitNoInlineAttr(NoInlineAttr *p) { PRINT; return true; };
  bool VisitNoInstrumentFunctionAttr(NoInstrumentFunctionAttr *p) { PRINT; return true; };
  bool VisitNoMips16Attr(NoMips16Attr *p) { PRINT; return true; };
  bool VisitNoReturnAttr(NoReturnAttr *p) { PRINT; return true; };
  bool VisitNoSanitizeAddressAttr(NoSanitizeAddressAttr *p) { PRINT; return true; };
  bool VisitNoSanitizeMemoryAttr(NoSanitizeMemoryAttr *p) { PRINT; return true; };
  bool VisitNoSanitizeThreadAttr(NoSanitizeThreadAttr *p) { PRINT; return true; };
  bool VisitNoSplitStackAttr(NoSplitStackAttr *p) { PRINT; return true; };
  bool VisitNoThreadSafetyAnalysisAttr(NoThreadSafetyAnalysisAttr *p) { PRINT; return true; };
  bool VisitNoThrowAttr(NoThrowAttr *p) { PRINT; return true; };
  bool VisitNonNullAttr(NonNullAttr *p) { PRINT; return true; };
  bool VisitObjCBridgeAttr(ObjCBridgeAttr *p) { PRINT; return true; };
  bool VisitObjCBridgeMutableAttr(ObjCBridgeMutableAttr *p) { PRINT; return true; };
  bool VisitObjCBridgeRelatedAttr(ObjCBridgeRelatedAttr *p) { PRINT; return true; };
  bool VisitObjCDesignatedInitializerAttr(ObjCDesignatedInitializerAttr *p) { PRINT; return true; };
  bool VisitObjCExceptionAttr(ObjCExceptionAttr *p) { PRINT; return true; };
  bool VisitObjCExplicitProtocolImplAttr(ObjCExplicitProtocolImplAttr *p) { PRINT; return true; };
  bool VisitObjCMethodFamilyAttr(ObjCMethodFamilyAttr *p) { PRINT; return true; };
  bool VisitObjCNSObjectAttr(ObjCNSObjectAttr *p) { PRINT; return true; };
  bool VisitObjCPreciseLifetimeAttr(ObjCPreciseLifetimeAttr *p) { PRINT; return true; };
  bool VisitObjCRequiresPropertyDefsAttr(ObjCRequiresPropertyDefsAttr *p) { PRINT; return true; };
  bool VisitObjCRequiresSuperAttr(ObjCRequiresSuperAttr *p) { PRINT; return true; };
  bool VisitObjCReturnsInnerPointerAttr(ObjCReturnsInnerPointerAttr *p) { PRINT; return true; };
  bool VisitObjCRootClassAttr(ObjCRootClassAttr *p) { PRINT; return true; };
  bool VisitObjCRuntimeNameAttr(ObjCRuntimeNameAttr *p) { PRINT; return true; };
  bool VisitOpenCLImageAccessAttr(OpenCLImageAccessAttr *p) { PRINT; return true; };
  bool VisitOpenCLKernelAttr(OpenCLKernelAttr *p) { PRINT; return true; };
  bool VisitOptimizeNoneAttr(OptimizeNoneAttr *p) { PRINT; return true; };
  bool VisitOverloadableAttr(OverloadableAttr *p) { PRINT; return true; };
  bool VisitOverrideAttr(OverrideAttr *p) { PRINT; return true; };
  bool VisitOwnershipAttr(OwnershipAttr *p) { PRINT; return true; };
  bool VisitPackedAttr(PackedAttr *p) { PRINT; return true; };
  bool VisitParamTypestateAttr(ParamTypestateAttr *p) { PRINT; return true; };
  bool VisitPascalAttr(PascalAttr *p) { PRINT; return true; };
  bool VisitPcsAttr(PcsAttr *p) { PRINT; return true; };
  bool VisitPnaclCallAttr(PnaclCallAttr *p) { PRINT; return true; };
  bool VisitPtGuardedByAttr(PtGuardedByAttr *p) { PRINT; return true; };
  bool VisitPtGuardedVarAttr(PtGuardedVarAttr *p) { PRINT; return true; };
  bool VisitPureAttr(PureAttr *p) { PRINT; return true; };
  bool VisitReleaseCapabilityAttr(ReleaseCapabilityAttr *p) { PRINT; return true; };
  bool VisitReqdWorkGroupSizeAttr(ReqdWorkGroupSizeAttr *p) { PRINT; return true; };
  bool VisitRequiresCapabilityAttr(RequiresCapabilityAttr *p) { PRINT; return true; };
  bool VisitReturnTypestateAttr(ReturnTypestateAttr *p) { PRINT; return true; };
  bool VisitReturnsNonNullAttr(ReturnsNonNullAttr *p) { PRINT; return true; };
  bool VisitReturnsTwiceAttr(ReturnsTwiceAttr *p) { PRINT; return true; };
  bool VisitScopedLockableAttr(ScopedLockableAttr *p) { PRINT; return true; };
  bool VisitSectionAttr(SectionAttr *p) { PRINT; return true; };
  bool VisitSelectAnyAttr(SelectAnyAttr *p) { PRINT; return true; };
  bool VisitSentinelAttr(SentinelAttr *p) { PRINT; return true; };
  bool VisitSetTypestateAttr(SetTypestateAttr *p) { PRINT; return true; };
  bool VisitSharedTrylockFunctionAttr(SharedTrylockFunctionAttr *p) { PRINT; return true; };
  bool VisitStdCallAttr(StdCallAttr *p) { PRINT; return true; };
  bool VisitSysVABIAttr(SysVABIAttr *p) { PRINT; return true; };
  bool VisitTLSModelAttr(TLSModelAttr *p) { PRINT; return true; };
  bool VisitTestTypestateAttr(TestTypestateAttr *p) { PRINT; return true; };
  bool VisitThisCallAttr(ThisCallAttr *p) { PRINT; return true; };
  bool VisitThreadAttr(ThreadAttr *p) { PRINT; return true; };
  bool VisitTransparentUnionAttr(TransparentUnionAttr *p) { PRINT; return true; };
  bool VisitTryAcquireCapabilityAttr(TryAcquireCapabilityAttr *p) { PRINT; return true; };
  bool VisitTypeTagForDatatypeAttr(TypeTagForDatatypeAttr *p) { PRINT; return true; };
  bool VisitTypeVisibilityAttr(TypeVisibilityAttr *p) { PRINT; return true; };
  bool VisitUnavailableAttr(UnavailableAttr *p) { PRINT; return true; };
  bool VisitUnusedAttr(UnusedAttr *p) { PRINT; return true; };
  bool VisitUsedAttr(UsedAttr *p) { PRINT; return true; };
  bool VisitUuidAttr(UuidAttr *p) { PRINT; return true; };
  bool VisitVecReturnAttr(VecReturnAttr *p) { PRINT; return true; };
  bool VisitVecTypeHintAttr(VecTypeHintAttr *p) { PRINT; return true; };
  bool VisitVisibilityAttr(VisibilityAttr *p) { PRINT; return true; };
  bool VisitWarnUnusedAttr(WarnUnusedAttr *p) { PRINT; return true; };
  bool VisitWarnUnusedResultAttr(WarnUnusedResultAttr *p) { PRINT; return true; };
  bool VisitWeakAttr(WeakAttr *p) { PRINT; return true; };
  bool VisitWeakImportAttr(WeakImportAttr *p) { PRINT; return true; };
  bool VisitWeakRefAttr(WeakRefAttr *p) { PRINT; return true; };
  bool VisitWorkGroupSizeHintAttr(WorkGroupSizeHintAttr *p) { PRINT; return true; };
  bool VisitX86ForceAlignArgPointerAttr(X86ForceAlignArgPointerAttr *p) { PRINT; return true; };

  bool VisitStmt(Stmt *p) { NL; PRINT; return true; }
  bool VisitAsmStmt(AsmStmt *p) { PRINT; return true; }
  bool VisitGCCAsmStmt(GCCAsmStmt *p) { PRINT; return true; }
  bool VisitMSAsmStmt(MSAsmStmt *p) { PRINT; return true; }
  bool VisitAttributedStmt(AttributedStmt *p) { PRINT; return true; }
  bool VisitBreakStmt(BreakStmt *p) { PRINT; return true; }
  bool VisitCXXCatchStmt(CXXCatchStmt *p) { PRINT; return true; }
  bool VisitCXXForRangeStmt(CXXForRangeStmt *p) { PRINT; return true; }
  bool VisitCXXTryStmt(CXXTryStmt *p) { PRINT; return true; }
  bool VisitCapturedStmt(CapturedStmt *p) { PRINT; return true; }
  bool VisitCompoundStmt(CompoundStmt *p) { PRINT; return true; }
  bool VisitContinueStmt(ContinueStmt *p) { PRINT; return true; }
  bool VisitDeclStmt(DeclStmt *p) { PRINT; return true; }
  bool VisitDoStmt(DoStmt *p) { PRINT; return true; }
  bool VisitExpr(Expr *p) { PRINT; return true; }
  bool VisitAbstractConditionalOperator(AbstractConditionalOperator *p) { PRINT; return true; }
  bool VisitBinaryConditionalOperator(BinaryConditionalOperator *p) { PRINT; return true; }
  bool VisitConditionalOperator(ConditionalOperator *p) { PRINT; return true; }
  bool VisitAddrLabelExpr(AddrLabelExpr *p) { PRINT; return true; }
  bool VisitArraySubscriptExpr(ArraySubscriptExpr *p) { PRINT; return true; }
  bool VisitArrayTypeTraitExpr(ArrayTypeTraitExpr *p) { PRINT; return true; }
  bool VisitAsTypeExpr(AsTypeExpr *p) { PRINT; return true; }
  bool VisitAtomicExpr(AtomicExpr *p) { PRINT; return true; }
  bool VisitBinaryOperator(BinaryOperator *p) { PRINT; return true; }
  bool VisitCompoundAssignOperator(CompoundAssignOperator *p) { PRINT; return true; }
  bool VisitBlockExpr(BlockExpr *p) { PRINT; return true; }
  bool VisitCXXBindTemporaryExpr(CXXBindTemporaryExpr *p) { PRINT; return true; }
  bool VisitCXXBoolLiteralExpr(CXXBoolLiteralExpr *p) { PRINT; return true; }
  bool VisitCXXConstructExpr(CXXConstructExpr *p) { PRINT; return true; }
  bool VisitCXXTemporaryObjectExpr(CXXTemporaryObjectExpr *p) { PRINT; return true; }
  bool VisitCXXDefaultArgExpr(CXXDefaultArgExpr *p) { PRINT; return true; }
  bool VisitCXXDefaultInitExpr(CXXDefaultInitExpr *p) { PRINT; return true; }
  bool VisitCXXDeleteExpr(CXXDeleteExpr *p) { PRINT; return true; }
  bool VisitCXXDependentScopeMemberExpr(CXXDependentScopeMemberExpr *p) { PRINT; return true; }
  bool VisitCXXNewExpr(CXXNewExpr *p) { PRINT; return true; }
  bool VisitCXXNoexceptExpr(CXXNoexceptExpr *p) { PRINT; return true; }
  bool VisitCXXNullPtrLiteralExpr(CXXNullPtrLiteralExpr *p) { PRINT; return true; }
  bool VisitCXXPseudoDestructorExpr(CXXPseudoDestructorExpr *p) { PRINT; return true; }
  bool VisitCXXScalarValueInitExpr(CXXScalarValueInitExpr *p) { PRINT; return true; }
  bool VisitCXXStdInitializerListExpr(CXXStdInitializerListExpr *p) { PRINT; return true; }
  bool VisitCXXThisExpr(CXXThisExpr *p) { PRINT; return true; }
  bool VisitCXXThrowExpr(CXXThrowExpr *p) { PRINT; return true; }
  bool VisitCXXTypeidExpr(CXXTypeidExpr *p) { PRINT; return true; }
  bool VisitCXXUnresolvedConstructExpr(CXXUnresolvedConstructExpr *p) { PRINT; return true; }
  bool VisitCXXUuidofExpr(CXXUuidofExpr *p) { PRINT; return true; }
  bool VisitCallExpr(CallExpr *p) { PRINT; return true; }
  bool VisitCUDAKernelCallExpr(CUDAKernelCallExpr *p) { PRINT; return true; }
  bool VisitCXXMemberCallExpr(CXXMemberCallExpr *p) { PRINT; return true; }
  bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr *p) { PRINT; return true; }
  bool VisitUserDefinedLiteral(UserDefinedLiteral *p) { PRINT; return true; }
  bool VisitCastExpr(CastExpr *p) { PRINT; return true; }
  bool VisitExplicitCastExpr(ExplicitCastExpr *p) { PRINT; return true; }
  bool VisitCStyleCastExpr(CStyleCastExpr *p) { PRINT; return true; }
  bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr *p) { PRINT; return true; }
  bool VisitCXXNamedCastExpr(CXXNamedCastExpr *p) { PRINT; return true; }
  bool VisitCXXConstCastExpr(CXXConstCastExpr *p) { PRINT; return true; }
  bool VisitCXXDynamicCastExpr(CXXDynamicCastExpr *p) { PRINT; return true; }
  bool VisitCXXReinterpretCastExpr(CXXReinterpretCastExpr *p) { PRINT; return true; }
  bool VisitCXXStaticCastExpr(CXXStaticCastExpr *p) { PRINT; return true; }
  bool VisitObjCBridgedCastExpr(ObjCBridgedCastExpr *p) { PRINT; return true; }
  bool VisitImplicitCastExpr(ImplicitCastExpr *p) { PRINT; return true; }
  bool VisitCharacterLiteral(CharacterLiteral *p) { PRINT; return true; }
  bool VisitChooseExpr(ChooseExpr *p) { PRINT; return true; }
  bool VisitCompoundLiteralExpr(CompoundLiteralExpr *p) { PRINT; return true; }
  bool VisitConvertVectorExpr(ConvertVectorExpr *p) { PRINT; return true; }
  bool VisitDeclRefExpr(DeclRefExpr *p) { PRINT; return true; }
  bool VisitDependentScopeDeclRefExpr(DependentScopeDeclRefExpr *p) { PRINT; return true; }
  bool VisitDesignatedInitExpr(DesignatedInitExpr *p) { PRINT; return true; }
  bool VisitExprWithCleanups(ExprWithCleanups *p) { PRINT; return true; }
  bool VisitExpressionTraitExpr(ExpressionTraitExpr *p) { PRINT; return true; }
  bool VisitExtVectorElementExpr(ExtVectorElementExpr *p) { PRINT; return true; }
  bool VisitFloatingLiteral(FloatingLiteral *p) { PRINT; return true; }
  bool VisitFunctionParmPackExpr(FunctionParmPackExpr *p) { PRINT; return true; }
  bool VisitGNUNullExpr(GNUNullExpr *p) { PRINT; return true; }
  bool VisitGenericSelectionExpr(GenericSelectionExpr *p) { PRINT; return true; }
  bool VisitImaginaryLiteral(ImaginaryLiteral *p) { PRINT; return true; }
  bool VisitImplicitValueInitExpr(ImplicitValueInitExpr *p) { PRINT; return true; }
  bool VisitInitListExpr(InitListExpr *p) { PRINT; return true; }
  bool VisitIntegerLiteral(IntegerLiteral *p) { PRINT; return true; }
  bool VisitLambdaExpr(LambdaExpr *p) { PRINT; return true; }
  bool VisitMSPropertyRefExpr(MSPropertyRefExpr *p) { PRINT; return true; }
  bool VisitMaterializeTemporaryExpr(MaterializeTemporaryExpr *p) { PRINT; return true; }
  bool VisitMemberExpr(MemberExpr *p) { PRINT; return true; }
  bool VisitObjCArrayLiteral(ObjCArrayLiteral *p) { PRINT; return true; }
  bool VisitObjCBoolLiteralExpr(ObjCBoolLiteralExpr *p) { PRINT; return true; }
  bool VisitObjCBoxedExpr(ObjCBoxedExpr *p) { PRINT; return true; }
  bool VisitObjCDictionaryLiteral(ObjCDictionaryLiteral *p) { PRINT; return true; }
  bool VisitObjCEncodeExpr(ObjCEncodeExpr *p) { PRINT; return true; }
  bool VisitObjCIndirectCopyRestoreExpr(ObjCIndirectCopyRestoreExpr *p) { PRINT; return true; }
  bool VisitObjCIsaExpr(ObjCIsaExpr *p) { PRINT; return true; }
  bool VisitObjCIvarRefExpr(ObjCIvarRefExpr *p) { PRINT; return true; }
  bool VisitObjCMessageExpr(ObjCMessageExpr *p) { PRINT; return true; }
  bool VisitObjCPropertyRefExpr(ObjCPropertyRefExpr *p) { PRINT; return true; }
  bool VisitObjCProtocolExpr(ObjCProtocolExpr *p) { PRINT; return true; }
  bool VisitObjCSelectorExpr(ObjCSelectorExpr *p) { PRINT; return true; }
  bool VisitObjCStringLiteral(ObjCStringLiteral *p) { PRINT; return true; }
  bool VisitObjCSubscriptRefExpr(ObjCSubscriptRefExpr *p) { PRINT; return true; }
  bool VisitOffsetOfExpr(OffsetOfExpr *p) { PRINT; return true; }
  bool VisitOpaqueValueExpr(OpaqueValueExpr *p) { PRINT; return true; }
  bool VisitOverloadExpr(OverloadExpr *p) { PRINT; return true; }
  bool VisitUnresolvedLookupExpr(UnresolvedLookupExpr *p) { PRINT; return true; }
  bool VisitUnresolvedMemberExpr(UnresolvedMemberExpr *p) { PRINT; return true; }
  bool VisitPackExpansionExpr(PackExpansionExpr *p) { PRINT; return true; }
  bool VisitParenExpr(ParenExpr *p) { PRINT; return true; }
  bool VisitParenListExpr(ParenListExpr *p) { PRINT; return true; }
  bool VisitPredefinedExpr(PredefinedExpr *p) { PRINT; return true; }
  bool VisitPseudoObjectExpr(PseudoObjectExpr *p) { PRINT; return true; }
  bool VisitShuffleVectorExpr(ShuffleVectorExpr *p) { PRINT; return true; }
  bool VisitSizeOfPackExpr(SizeOfPackExpr *p) { PRINT; return true; }
  bool VisitStmtExpr(StmtExpr *p) { PRINT; return true; }
  bool VisitStringLiteral(StringLiteral *p) { PRINT; return true; }
  bool VisitSubstNonTypeTemplateParmExpr(SubstNonTypeTemplateParmExpr *p) { PRINT; return true; }
  bool VisitSubstNonTypeTemplateParmPackExpr(SubstNonTypeTemplateParmPackExpr *p) { PRINT; return true; }
  bool VisitTypeTraitExpr(TypeTraitExpr *p) { PRINT; return true; }
  bool VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *p) { PRINT; return true; }
  bool VisitUnaryOperator(UnaryOperator *p) { PRINT; return true; }
  bool VisitVAArgExpr(VAArgExpr *p) { PRINT; return true; }

  bool VisitForStmt(ForStmt *p) { PRINT; return true; }
  bool VisitGotoStmt(GotoStmt *p) { PRINT; return true; }
  bool VisitIfStmt(IfStmt *p) { PRINT; return true; }
  bool VisitIndirectGotoStmt(IndirectGotoStmt *p) { PRINT; return true; }
  bool VisitLabelStmt(LabelStmt *p) { PRINT; return true; }
  bool VisitMSDependentExistsStmt(MSDependentExistsStmt *p) { PRINT; return true; }
  bool VisitNullStmt(NullStmt *p) { PRINT; return true; }
  bool VisitOMPExecutableDirective(OMPExecutableDirective *p) { PRINT; return true; }
  bool VisitOMPBarrierDirective(OMPBarrierDirective *p) { PRINT; return true; }
  bool VisitOMPCriticalDirective(OMPCriticalDirective *p) { PRINT; return true; }
  bool VisitOMPFlushDirective(OMPFlushDirective *p) { PRINT; return true; }
  bool VisitOMPForDirective(OMPForDirective *p) { PRINT; return true; }
  bool VisitOMPMasterDirective(OMPMasterDirective *p) { PRINT; return true; }
  bool VisitOMPParallelDirective(OMPParallelDirective *p) { PRINT; return true; }
  bool VisitOMPParallelForDirective(OMPParallelForDirective *p) { PRINT; return true; }
  bool VisitOMPParallelSectionsDirective(OMPParallelSectionsDirective *p) { PRINT; return true; }
  bool VisitOMPSectionDirective(OMPSectionDirective *p) { PRINT; return true; }
  bool VisitOMPSectionsDirective(OMPSectionsDirective *p) { PRINT; return true; }
  bool VisitOMPSimdDirective(OMPSimdDirective *p) { PRINT; return true; }
  bool VisitOMPSingleDirective(OMPSingleDirective *p) { PRINT; return true; }
  bool VisitOMPTaskDirective(OMPTaskDirective *p) { PRINT; return true; }
  bool VisitOMPTaskwaitDirective(OMPTaskwaitDirective *p) { PRINT; return true; }
  bool VisitOMPTaskyieldDirective(OMPTaskyieldDirective *p) { PRINT; return true; }
  bool VisitObjCAtCatchStmt(ObjCAtCatchStmt *p) { PRINT; return true; }
  bool VisitObjCAtFinallyStmt(ObjCAtFinallyStmt *p) { PRINT; return true; }
  bool VisitObjCAtSynchronizedStmt(ObjCAtSynchronizedStmt *p) { PRINT; return true; }
  bool VisitObjCAtThrowStmt(ObjCAtThrowStmt *p) { PRINT; return true; }
  bool VisitObjCAtTryStmt(ObjCAtTryStmt *p) { PRINT; return true; }
  bool VisitObjCAutoreleasePoolStmt(ObjCAutoreleasePoolStmt *p) { PRINT; return true; }
  bool VisitObjCForCollectionStmt(ObjCForCollectionStmt *p) { PRINT; return true; }
  bool VisitReturnStmt(ReturnStmt *p) { PRINT; return true; }
  bool VisitSEHExceptStmt(SEHExceptStmt *p) { PRINT; return true; }
  bool VisitSEHFinallyStmt(SEHFinallyStmt *p) { PRINT; return true; }
  bool VisitSEHLeaveStmt(SEHLeaveStmt *p) { PRINT; return true; }
  bool VisitSEHTryStmt(SEHTryStmt *p) { PRINT; return true; }
  bool VisitSwitchCase(SwitchCase *p) { PRINT; return true; }
  bool VisitCaseStmt(CaseStmt *p) { PRINT; return true; }
  bool VisitDefaultStmt(DefaultStmt *p) { PRINT; return true; }
  bool VisitSwitchStmt(SwitchStmt *p) { PRINT; return true; }
  bool VisitWhileStmt(WhileStmt *p) { PRINT; return true; }
  bool VisitUnaryPostInc(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryPostDec(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryPreInc(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryPreDec(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryAddrOf(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryDeref(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryPlus(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryMinus(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryNot(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryLNot(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryReal(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryImag(UnaryOperator *p) { PRINT; return true; }
  bool VisitUnaryExtension(UnaryOperator *p) { PRINT; return true; }
  bool VisitBinPtrMemD(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinPtrMemI(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinMul(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinDiv(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinRem(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinAdd(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinSub(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinShl(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinShr(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinLT(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinGT(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinLE(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinGE(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinEQ(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinNE(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinAnd(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinXor(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinOr(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinLAnd(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinLOr(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinAssign(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinComma(BinaryOperator *p) { PRINT; return true; }
  bool VisitBinMulAssign(CompoundAssignOperator *p) { PRINT; return true; }
  bool VisitBinDivAssign(CompoundAssignOperator *p) { PRINT; return true; }
  bool VisitBinRemAssign(CompoundAssignOperator *p) { PRINT; return true; }
  bool VisitBinAddAssign(CompoundAssignOperator *p) { PRINT; return true; }
  bool VisitBinSubAssign(CompoundAssignOperator *p) { PRINT; return true; }
  bool VisitBinShlAssign(CompoundAssignOperator *p) { PRINT; return true; }
  bool VisitBinShrAssign(CompoundAssignOperator *p) { PRINT; return true; }
  bool VisitBinAndAssign(CompoundAssignOperator *p) { PRINT; return true; }
  bool VisitBinOrAssign(CompoundAssignOperator *p) { PRINT; return true; }
  bool VisitBinXorAssign(CompoundAssignOperator *p) { PRINT; return true; }

  bool VisitType(Type *p) { NL; PRINT; return true; }
  bool VisitBuiltinType(BuiltinType *p) { PRINT; return true; }
  bool VisitComplexType(ComplexType *p) { PRINT; return true; }
  bool VisitPointerType(PointerType *p) { PRINT; return true; }
  bool VisitBlockPointerType(BlockPointerType *p) { PRINT; return true; }
  bool VisitReferenceType(ReferenceType *p) { PRINT; return true; }
  bool VisitLValueReferenceType(LValueReferenceType *p) { PRINT; return true; }
  bool VisitRValueReferenceType(RValueReferenceType *p) { PRINT; return true; }
  bool VisitMemberPointerType(MemberPointerType *p) { PRINT; return true; }
  bool VisitArrayType(ArrayType *p) { PRINT; return true; }
  bool VisitConstantArrayType(ConstantArrayType *p) { PRINT; return true; }
  bool VisitIncompleteArrayType(IncompleteArrayType *p) { PRINT; return true; }
  bool VisitVariableArrayType(VariableArrayType *p) { PRINT; return true; }
  bool VisitDependentSizedArrayType(DependentSizedArrayType *p) { PRINT; return true; }
  bool VisitDependentSizedExtVectorType(DependentSizedExtVectorType *p) { PRINT; return true; }
  bool VisitVectorType(VectorType *p) { PRINT; return true; }
  bool VisitExtVectorType(ExtVectorType *p) { PRINT; return true; }
  bool VisitFunctionType(FunctionType *p) { PRINT; return true; }
  bool VisitFunctionProtoType(FunctionProtoType *p) { PRINT; return true; }
  bool VisitFunctionNoProtoType(FunctionNoProtoType *p) { PRINT; return true; }
  bool VisitUnresolvedUsingType(UnresolvedUsingType *p) { PRINT; return true; }
  bool VisitParenType(ParenType *p) { PRINT; return true; }
  bool VisitTypedefType(TypedefType *p) { PRINT; return true; }
  bool VisitAdjustedType(AdjustedType *p) { PRINT; return true; }
  bool VisitDecayedType(DecayedType *p) { PRINT; return true; }
  bool VisitTypeOfExprType(TypeOfExprType *p) { PRINT; return true; }
  bool VisitTypeOfType(TypeOfType *p) { PRINT; return true; }
  bool VisitDecltypeType(DecltypeType *p) { PRINT; return true; }
  bool VisitUnaryTransformType(UnaryTransformType *p) { PRINT; return true; }
  bool VisitTagType(TagType *p) { PRINT; return true; }
  bool VisitRecordType(RecordType *p) { PRINT; return true; }
  bool VisitEnumType(EnumType *p) { PRINT; return true; }
  bool VisitElaboratedType(ElaboratedType *p) { PRINT; return true; }
  bool VisitAttributedType(AttributedType *p) { PRINT; return true; }
  bool VisitTemplateTypeParmType(TemplateTypeParmType *p) { PRINT; return true; }
  bool VisitSubstTemplateTypeParmType(SubstTemplateTypeParmType *p) { PRINT; return true; }
  bool VisitSubstTemplateTypeParmPackType(SubstTemplateTypeParmPackType *p) { PRINT; return true; }
  bool VisitTemplateSpecializationType(TemplateSpecializationType *p) { PRINT; return true; }
  bool VisitAutoType(AutoType *p) { PRINT; return true; }
  bool VisitInjectedClassNameType(InjectedClassNameType *p) { PRINT; return true; }
  bool VisitDependentNameType(DependentNameType *p) { PRINT; return true; }
  bool VisitDependentTemplateSpecializationType(DependentTemplateSpecializationType *p) { PRINT; return true; }
  bool VisitPackExpansionType(PackExpansionType *p) { PRINT; return true; }
  bool VisitObjCObjectType(ObjCObjectType *p) { PRINT; return true; }
  bool VisitObjCInterfaceType(ObjCInterfaceType *p) { PRINT; return true; }
  bool VisitObjCObjectPointerType(ObjCObjectPointerType *p) { PRINT; return true; }
  bool VisitAtomicType(AtomicType *p) { PRINT; return true; }

  bool VisitDecl(Decl *p) { NL; PRINT; return true; }
  bool VisitAccessSpecDecl(AccessSpecDecl *p) { PRINT; return true; }
  bool VisitBlockDecl(BlockDecl *p) { PRINT; return true; }
  bool VisitCapturedDecl(CapturedDecl *p) { PRINT; return true; }
  bool VisitClassScopeFunctionSpecializationDecl(ClassScopeFunctionSpecializationDecl *p) { PRINT; return true; }
  bool VisitEmptyDecl(EmptyDecl *p) { PRINT; return true; }
  bool VisitFileScopeAsmDecl(FileScopeAsmDecl *p) { PRINT; return true; }
  bool VisitFriendDecl(FriendDecl *p) { PRINT; return true; }
  bool VisitFriendTemplateDecl(FriendTemplateDecl *p) { PRINT; return true; }
  bool VisitImportDecl(ImportDecl *p) { PRINT; return true; }
  bool VisitLinkageSpecDecl(LinkageSpecDecl *p) { PRINT; return true; }
  bool VisitNamedDecl(NamedDecl *p)
  {
    printf("%*s%s %s\n", getIndent(p), "", __FUNCTION__, p->getNameAsString().c_str());
    return true;
  }
  bool VisitLabelDecl(LabelDecl *p) { PRINT; return true; }
  bool VisitNamespaceDecl(NamespaceDecl *p) { PRINT; return true; }
  bool VisitNamespaceAliasDecl(NamespaceAliasDecl *p) { PRINT; return true; }
  bool VisitObjCCompatibleAliasDecl(ObjCCompatibleAliasDecl *p) { PRINT; return true; }
  bool VisitObjCContainerDecl(ObjCContainerDecl *p) { PRINT; return true; }
  bool VisitObjCCategoryDecl(ObjCCategoryDecl *p) { PRINT; return true; }
  bool VisitObjCImplDecl(ObjCImplDecl *p) { PRINT; return true; }
  bool VisitObjCCategoryImplDecl(ObjCCategoryImplDecl *p) { PRINT; return true; }
  bool VisitObjCImplementationDecl(ObjCImplementationDecl *p) { PRINT; return true; }
  bool VisitObjCInterfaceDecl(ObjCInterfaceDecl *p) { PRINT; return true; }
  bool VisitObjCProtocolDecl(ObjCProtocolDecl *p) { PRINT; return true; }
  bool VisitObjCMethodDecl(ObjCMethodDecl *p) { PRINT; return true; }
  bool VisitObjCPropertyDecl(ObjCPropertyDecl *p) { PRINT; return true; }
  bool VisitTemplateDecl(TemplateDecl *p) { PRINT; return true; }
  bool VisitRedeclarableTemplateDecl(RedeclarableTemplateDecl *p) { PRINT; return true; }
  bool VisitClassTemplateDecl(ClassTemplateDecl *p) { PRINT; return true; }
  bool VisitFunctionTemplateDecl(FunctionTemplateDecl *p) { PRINT; return true; }
  bool VisitTypeAliasTemplateDecl(TypeAliasTemplateDecl *p) { PRINT; return true; }
  bool VisitVarTemplateDecl(VarTemplateDecl *p) { PRINT; return true; }
  bool VisitTemplateTemplateParmDecl(TemplateTemplateParmDecl *p) { PRINT; return true; }
  bool VisitTypeDecl(TypeDecl *p) { PRINT; return true; }
  bool VisitTagDecl(TagDecl *p) { PRINT; return true; }
  bool VisitEnumDecl(EnumDecl *p) { PRINT; return true; }
  bool VisitRecordDecl(RecordDecl *p) { PRINT; return true; }
  bool VisitCXXRecordDecl(CXXRecordDecl *p) { PRINT; return true; }
  bool VisitClassTemplateSpecializationDecl(ClassTemplateSpecializationDecl *p) { PRINT; return true; }
  bool VisitClassTemplatePartialSpecializationDecl(ClassTemplatePartialSpecializationDecl *p) { PRINT; return true; }
  bool VisitTemplateTypeParmDecl(TemplateTypeParmDecl *p) { PRINT; return true; }
  bool VisitTypedefNameDecl(TypedefNameDecl *p) { PRINT; return true; }
  bool VisitTypeAliasDecl(TypeAliasDecl *p) { PRINT; return true; }
  bool VisitTypedefDecl(TypedefDecl *p) { PRINT; return true; }
  bool VisitUnresolvedUsingTypenameDecl(UnresolvedUsingTypenameDecl *p) { PRINT; return true; }
  bool VisitUsingDecl(UsingDecl *p) { PRINT; return true; }
  bool VisitUsingDirectiveDecl(UsingDirectiveDecl *p) { PRINT; return true; }
  bool VisitUsingShadowDecl(UsingShadowDecl *p) { PRINT; return true; }
  bool VisitValueDecl(ValueDecl *p) { PRINT; return true; }
  bool VisitDeclaratorDecl(DeclaratorDecl *p) { PRINT; return true; }
  bool VisitFieldDecl(FieldDecl *p) { PRINT; return true; }
  bool VisitObjCAtDefsFieldDecl(ObjCAtDefsFieldDecl *p) { PRINT; return true; }
  bool VisitObjCIvarDecl(ObjCIvarDecl *p) { PRINT; return true; }
  bool VisitFunctionDecl(FunctionDecl *p) { PRINT; return true; }
  bool VisitCXXMethodDecl(CXXMethodDecl *p) { PRINT; return true; }
  bool VisitCXXConstructorDecl(CXXConstructorDecl *p) { PRINT; return true; }
  bool VisitCXXConversionDecl(CXXConversionDecl *p) { PRINT; return true; }
  bool VisitCXXDestructorDecl(CXXDestructorDecl *p) { PRINT; return true; }
  bool VisitMSPropertyDecl(MSPropertyDecl *p) { PRINT; return true; }
  bool VisitNonTypeTemplateParmDecl(NonTypeTemplateParmDecl *p) { PRINT; return true; }
  bool VisitVarDecl(VarDecl *p) { PRINT; return true; }
  bool VisitImplicitParamDecl(ImplicitParamDecl *p) { PRINT; return true; }
  bool VisitParmVarDecl(ParmVarDecl *p) { PRINT; return true; }
  bool VisitVarTemplateSpecializationDecl(VarTemplateSpecializationDecl *p) { PRINT; return true; }
  bool VisitVarTemplatePartialSpecializationDecl(VarTemplatePartialSpecializationDecl *p) { PRINT; return true; }
  bool VisitEnumConstantDecl(EnumConstantDecl *p) { PRINT; return true; }
  bool VisitIndirectFieldDecl(IndirectFieldDecl *p) { PRINT; return true; }
  bool VisitUnresolvedUsingValueDecl(UnresolvedUsingValueDecl *p) { PRINT; return true; }
  bool VisitOMPThreadPrivateDecl(OMPThreadPrivateDecl *p) { PRINT; return true; }
  bool VisitObjCPropertyImplDecl(ObjCPropertyImplDecl *p) { PRINT; return true; }
  bool VisitStaticAssertDecl(StaticAssertDecl *p) { PRINT; return true; }
  bool VisitTranslationUnitDecl(TranslationUnitDecl *p) { PRINT; return true; }

#undef PRINT
#define PRINT printf("%s\n", __FUNCTION__)
  bool VisitTypeLoc(TypeLoc TL) { NL; PRINT; return true; }
  bool VisitQualifiedTypeLoc(QualifiedTypeLoc TL) { PRINT; return true; }
  bool VisitUnqualTypeLoc(UnqualTypeLoc TL) { PRINT; return true; }
  bool VisitBuiltinTypeLoc(BuiltinTypeLoc TL) { PRINT; return true; }
  bool VisitComplexTypeLoc(ComplexTypeLoc TL) { PRINT; return true; }
  bool VisitPointerTypeLoc(PointerTypeLoc TL) { PRINT; return true; }
  bool VisitBlockPointerTypeLoc(BlockPointerTypeLoc TL) { PRINT; return true; }
  bool VisitReferenceTypeLoc(ReferenceTypeLoc TL) { PRINT; return true; }
  bool VisitLValueReferenceTypeLoc(LValueReferenceTypeLoc TL) { PRINT; return true; }
  bool VisitRValueReferenceTypeLoc(RValueReferenceTypeLoc TL) { PRINT; return true; }
  bool VisitMemberPointerTypeLoc(MemberPointerTypeLoc TL) { PRINT; return true; }
  bool VisitArrayTypeLoc(ArrayTypeLoc TL) { PRINT; return true; }
  bool VisitConstantArrayTypeLoc(ConstantArrayTypeLoc TL) { PRINT; return true; }
  bool VisitIncompleteArrayTypeLoc(IncompleteArrayTypeLoc TL) { PRINT; return true; }
  bool VisitVariableArrayTypeLoc(VariableArrayTypeLoc TL) { PRINT; return true; }
  bool VisitDependentSizedArrayTypeLoc(DependentSizedArrayTypeLoc TL) { PRINT; return true; }
  bool VisitDependentSizedExtVectorTypeLoc(DependentSizedExtVectorTypeLoc TL) { PRINT; return true; }
  bool VisitVectorTypeLoc(VectorTypeLoc TL) { PRINT; return true; }
  bool VisitExtVectorTypeLoc(ExtVectorTypeLoc TL) { PRINT; return true; }
  bool VisitFunctionTypeLoc(FunctionTypeLoc TL) { PRINT; return true; }
  bool VisitFunctionProtoTypeLoc(FunctionProtoTypeLoc TL) { PRINT; return true; }
  bool VisitFunctionNoProtoTypeLoc(FunctionNoProtoTypeLoc TL) { PRINT; return true; }
  bool VisitUnresolvedUsingTypeLoc(UnresolvedUsingTypeLoc TL) { PRINT; return true; }
  bool VisitParenTypeLoc(ParenTypeLoc TL) { PRINT; return true; }
  bool VisitTypedefTypeLoc(TypedefTypeLoc TL) { PRINT; return true; }
  bool VisitAdjustedTypeLoc(AdjustedTypeLoc TL) { PRINT; return true; }
  bool VisitDecayedTypeLoc(DecayedTypeLoc TL) { PRINT; return true; }
  bool VisitTypeOfExprTypeLoc(TypeOfExprTypeLoc TL) { PRINT; return true; }
  bool VisitTypeOfTypeLoc(TypeOfTypeLoc TL) { PRINT; return true; }
  bool VisitDecltypeTypeLoc(DecltypeTypeLoc TL) { PRINT; return true; }
  bool VisitUnaryTransformTypeLoc(UnaryTransformTypeLoc TL) { PRINT; return true; }
  bool VisitTagTypeLoc(TagTypeLoc TL) { PRINT; return true; }
  bool VisitRecordTypeLoc(RecordTypeLoc TL) { PRINT; return true; }
  bool VisitEnumTypeLoc(EnumTypeLoc TL) { PRINT; return true; }
  bool VisitElaboratedTypeLoc(ElaboratedTypeLoc TL) { PRINT; return true; }
  bool VisitAttributedTypeLoc(AttributedTypeLoc TL) { PRINT; return true; }
  bool VisitTemplateTypeParmTypeLoc(TemplateTypeParmTypeLoc TL) { PRINT; return true; }
  bool VisitSubstTemplateTypeParmTypeLoc(SubstTemplateTypeParmTypeLoc TL) { PRINT; return true; }
  bool VisitSubstTemplateTypeParmPackTypeLoc(SubstTemplateTypeParmPackTypeLoc TL) { PRINT; return true; }
  bool VisitTemplateSpecializationTypeLoc(TemplateSpecializationTypeLoc TL) { PRINT; return true; }
  bool VisitAutoTypeLoc(AutoTypeLoc TL) { PRINT; return true; }
  bool VisitInjectedClassNameTypeLoc(InjectedClassNameTypeLoc TL) { PRINT; return true; }
  bool VisitDependentNameTypeLoc(DependentNameTypeLoc TL) { PRINT; return true; }
  bool VisitDependentTemplateSpecializationTypeLoc(DependentTemplateSpecializationTypeLoc TL) { PRINT; return true; }
  bool VisitPackExpansionTypeLoc(PackExpansionTypeLoc TL) { PRINT; return true; }
  bool VisitObjCObjectTypeLoc(ObjCObjectTypeLoc TL) { PRINT; return true; }
  bool VisitObjCInterfaceTypeLoc(ObjCInterfaceTypeLoc TL) { PRINT; return true; }
  bool VisitObjCObjectPointerTypeLoc(ObjCObjectPointerTypeLoc TL) { PRINT; return true; }
  bool VisitAtomicTypeLoc(AtomicTypeLoc TL) { PRINT; return true; }

private:
  int getIndent(void* p)
  {
    if (p == last_)
    {
      ++indent_;
    }
    else
    {
      indent_ = 0;
    }
    last_ = p;
    return 2*indent_;
  }

  void* last_ = nullptr;
  int indent_ = 0;
  clang::SourceManager& sourceManager_;
  const clang::LangOptions& langOpts_;
};

class Consumer : public clang::ASTConsumer
{
 public:
  Consumer(clang::CompilerInstance& compiler)
    : preprocessor_(compiler.getPreprocessor()),
      sourceManager_(compiler.getSourceManager())
  {
  }

  void HandleTranslationUnit(clang::ASTContext& context) override
  {
    assert(&sourceManager_ == &context.getSourceManager());
    if (preprocessor_.getDiagnostics().hasErrorOccurred())
    {
      return;
    }

    Visitor visitor(context);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

 private:
  const clang::Preprocessor& preprocessor_;
  clang::SourceManager& sourceManager_;
};

class Action : public clang::ASTFrontendAction
{
 protected:
  clang::ASTConsumer *CreateASTConsumer(clang::CompilerInstance& compiler,
                                        clang::StringRef inputFile) override
  {
    return new Consumer(compiler);
  }
};

int main(int argc, char* argv[])
{
  std::vector<std::string> commands;
  for (int i = 0; i < argc; ++i)
    commands.push_back(argv[i]);
  llvm::IntrusiveRefCntPtr<clang::FileManager> files(
      new clang::FileManager(clang::FileSystemOptions()));
  clang::tooling::ToolInvocation tool(commands, new Action, files.get());

  bool succeed = tool.run();
  return succeed ? 0 : -1;
}

