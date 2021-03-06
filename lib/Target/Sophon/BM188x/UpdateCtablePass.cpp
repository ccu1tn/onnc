//===---------------------------------------------------------------------===//
//
//                             The ONNC Project
//
// Copyright(c) 2018, The ONNC Team
//
// This file is part of the ONNC Project and is distributed under
// 3-clause BSD license (https://opensource.org/licenses/BSD-3-Clause)
//
// See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//

#include "BM188xBackend.h"
#include "UpdateVisitor.h"
#include <onnc/Transforms/GraphBuildingPass.h>

using namespace onnc;

namespace {

class UpdateCtablePass : public GraphBuildingPass
{
public:
  static char ID;

public:
  UpdateCtablePass(BM1880Backend *pBackend)
      : GraphBuildingPass(ID), m_pBackend(pBackend)
  {
  }

  StringRef getPassName() const override { return "Update Pass"; }

  Pass::ReturnType runOnGraphs(xGraph &pTG, ComputeGraph &pCG) override;

private:
  BM1880Backend *m_pBackend; // NOLINT
};

} // namespace

Pass::ReturnType UpdateCtablePass::runOnGraphs(xGraph &pTG,
                                               ComputeGraph &pCG)
{
  onnc::BM188X::UpdateVisitor visitor(m_pBackend);
  auto nEnd = pCG.end();
  for (auto node = pCG.begin(); node != nEnd; ++node) {
    node->accept(visitor);
  }
  return Pass::kModuleChanged;
}

char UpdateCtablePass::ID = 0;

ModulePass *onnc::createUpdateCtablePass(BM1880Backend *pBackend)
{
  return new UpdateCtablePass(pBackend);
}
