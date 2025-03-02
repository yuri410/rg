//===- llvm/unittest/IR/ValueTest.cpp - Value unit tests ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/SourceMgr.h"
#include "gtest/gtest.h"
using namespace llvm;

namespace {

TEST(ValueTest, UsedInBasicBlock) {
  LLVMContext C;

  const char *ModuleString = "define void @f(i32 %x, i32 %y) {\n"
                             "bb0:\n"
                             "  %y1 = add i32 %y, 1\n"
                             "  %y2 = add i32 %y, 1\n"
                             "  %y3 = add i32 %y, 1\n"
                             "  %y4 = add i32 %y, 1\n"
                             "  %y5 = add i32 %y, 1\n"
                             "  %y6 = add i32 %y, 1\n"
                             "  %y7 = add i32 %y, 1\n"
                             "  %y8 = add i32 %x, 1\n"
                             "  ret void\n"
                             "}\n";
  SMDiagnostic Err;
  std::unique_ptr<Module> M = parseAssemblyString(ModuleString, Err, C);

  Function *F = M->getFunction("f");

  EXPECT_FALSE(F->isUsedInBasicBlock(F->begin()));
  EXPECT_TRUE((++F->arg_begin())->isUsedInBasicBlock(F->begin()));
  EXPECT_TRUE(F->arg_begin()->isUsedInBasicBlock(F->begin()));
}

TEST(GlobalTest, CreateAddressSpace) {
  LLVMContext &Ctx = getGlobalContext();
  std::unique_ptr<Module> M(new Module("TestModule", Ctx));
  Type *Int8Ty = Type::getInt8Ty(Ctx);
  Type *Int32Ty = Type::getInt32Ty(Ctx);

  GlobalVariable *Dummy0
    = new GlobalVariable(*M,
                         Int32Ty,
                         true,
                         GlobalValue::ExternalLinkage,
                         Constant::getAllOnesValue(Int32Ty),
                         "dummy",
                         nullptr,
                         GlobalVariable::NotThreadLocal,
                         1);

  EXPECT_TRUE(Value::MaximumAlignment == 536870912U);
  Dummy0->setAlignment(536870912U);
  EXPECT_EQ(Dummy0->getAlignment(), 536870912U);

  // Make sure the address space isn't dropped when returning this.
  Constant *Dummy1 = M->getOrInsertGlobal("dummy", Int32Ty);
  EXPECT_EQ(Dummy0, Dummy1);
  EXPECT_EQ(1u, Dummy1->getType()->getPointerAddressSpace());


  // This one requires a bitcast, but the address space must also stay the same.
  GlobalVariable *DummyCast0
    = new GlobalVariable(*M,
                         Int32Ty,
                         true,
                         GlobalValue::ExternalLinkage,
                         Constant::getAllOnesValue(Int32Ty),
                         "dummy_cast",
                         nullptr,
                         GlobalVariable::NotThreadLocal,
                         1);

  // Make sure the address space isn't dropped when returning this.
  Constant *DummyCast1 = M->getOrInsertGlobal("dummy_cast", Int8Ty);
  EXPECT_EQ(1u, DummyCast1->getType()->getPointerAddressSpace());
  EXPECT_NE(DummyCast0, DummyCast1) << *DummyCast1;
}

#ifdef GTEST_HAS_DEATH_TEST
#ifndef NDEBUG
TEST(GlobalTest, AlignDeath) {
  LLVMContext &Ctx = getGlobalContext();
  std::unique_ptr<Module> M(new Module("TestModule", Ctx));
  Type *Int32Ty = Type::getInt32Ty(Ctx);
  GlobalVariable *Var =
      new GlobalVariable(*M, Int32Ty, true, GlobalValue::ExternalLinkage,
                         Constant::getAllOnesValue(Int32Ty), "var", nullptr,
                         GlobalVariable::NotThreadLocal, 1);

  EXPECT_DEATH(Var->setAlignment(536870913U), "Alignment is not a power of 2");
  EXPECT_DEATH(Var->setAlignment(1073741824U),
               "Alignment is greater than MaximumAlignment");
}
#endif
#endif

TEST(ValueTest, printSlots) {
  // Check that Value::print() and Value::printAsOperand() work with and
  // without a slot tracker.
  LLVMContext C;

  const char *ModuleString = "define void @f(i32 %x, i32 %y) {\n"
                             "entry:\n"
                             "  %0 = add i32 %y, 1\n"
                             "  %1 = add i32 %y, 1\n"
                             "  ret void\n"
                             "}\n";
  SMDiagnostic Err;
  std::unique_ptr<Module> M = parseAssemblyString(ModuleString, Err, C);

  Function *F = M->getFunction("f");
  ASSERT_TRUE(F);
  ASSERT_FALSE(F->empty());
  BasicBlock &BB = F->getEntryBlock();
  ASSERT_EQ(3u, BB.size());

  Instruction *I0 = BB.begin();
  ASSERT_TRUE(I0);
  Instruction *I1 = ++BB.begin();
  ASSERT_TRUE(I1);

  ModuleSlotTracker MST(M.get());

#define CHECK_PRINT(INST, STR)                                                 \
  do {                                                                         \
    {                                                                          \
      std::string S;                                                           \
      raw_string_ostream OS(S);                                                \
      INST->print(OS);                                                         \
      EXPECT_EQ(STR, OS.str());                                                \
    }                                                                          \
    {                                                                          \
      std::string S;                                                           \
      raw_string_ostream OS(S);                                                \
      INST->print(OS, MST);                                                    \
      EXPECT_EQ(STR, OS.str());                                                \
    }                                                                          \
  } while (false)
  CHECK_PRINT(I0, "  %0 = add i32 %y, 1");
  CHECK_PRINT(I1, "  %1 = add i32 %y, 1");
#undef CHECK_PRINT

#define CHECK_PRINT_AS_OPERAND(INST, TYPE, STR)                                \
  do {                                                                         \
    {                                                                          \
      std::string S;                                                           \
      raw_string_ostream OS(S);                                                \
      INST->printAsOperand(OS, TYPE);                                          \
      EXPECT_EQ(StringRef(STR), StringRef(OS.str()));                          \
    }                                                                          \
    {                                                                          \
      std::string S;                                                           \
      raw_string_ostream OS(S);                                                \
      INST->printAsOperand(OS, TYPE, MST);                                     \
      EXPECT_EQ(StringRef(STR), StringRef(OS.str()));                          \
    }                                                                          \
  } while (false)
  CHECK_PRINT_AS_OPERAND(I0, false, "%0");
  CHECK_PRINT_AS_OPERAND(I1, false, "%1");
  CHECK_PRINT_AS_OPERAND(I0, true, "i32 %0");
  CHECK_PRINT_AS_OPERAND(I1, true, "i32 %1");
#undef CHECK_PRINT_AS_OPERAND
}

} // end anonymous namespace
