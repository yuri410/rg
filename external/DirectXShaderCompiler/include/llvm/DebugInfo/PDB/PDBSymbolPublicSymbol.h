//===- PDBSymbolPublicSymbol.h - public symbol info -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_PDB_PDBSYMBOLPUBLICSYMBOL_H
#define LLVM_DEBUGINFO_PDB_PDBSYMBOLPUBLICSYMBOL_H

#include "PDBSymbol.h"
#include "PDBTypes.h"

namespace llvm {

class raw_ostream;

class PDBSymbolPublicSymbol : public PDBSymbol {
public:
  PDBSymbolPublicSymbol(const IPDBSession &PDBSession,
                        std::unique_ptr<IPDBRawSymbol> PublicSymbol);

  DECLARE_PDB_SYMBOL_CONCRETE_TYPE(PDB_SymType::PublicSymbol)

  void dump(PDBSymDumper &Dumper) const override;

  FORWARD_SYMBOL_METHOD(getAddressOffset)
  FORWARD_SYMBOL_METHOD(getAddressSection)
  FORWARD_SYMBOL_METHOD(isCode)
  FORWARD_SYMBOL_METHOD(isFunction)
  FORWARD_SYMBOL_METHOD(getLength)
  FORWARD_SYMBOL_METHOD(getLexicalParentId)
  FORWARD_SYMBOL_METHOD(getLocationType)
  FORWARD_SYMBOL_METHOD(isManagedCode)
  FORWARD_SYMBOL_METHOD(isMSILCode)
  FORWARD_SYMBOL_METHOD(getName)
  FORWARD_SYMBOL_METHOD(getSymIndexId)
  FORWARD_SYMBOL_METHOD(getRelativeVirtualAddress)
  FORWARD_SYMBOL_METHOD(getVirtualAddress)
  FORWARD_SYMBOL_METHOD(getUndecoratedName)
};

} // namespace llvm

#endif // LLVM_DEBUGINFO_PDB_PDBSYMBOLPUBLICSYMBOL_H
