/*
  Copyright (c) 2012 The KoRE Project

  This file is part of KoRE.

  KoRE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  KoRE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with KoRE.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KORE_NODESTAGE_H_
#define KORE_NODESTAGE_H_

#include <vector>
#include "KoRE/SceneNode.h"
#include "KoRE/Operations/Operation.h"
#include "Kore/Operations/BindOperations/BindTexture.h"

namespace kore {
  class NodePass {
  public:
    NodePass(void);
    explicit NodePass(const SceneNode* node);
    ~NodePass(void);

    inline std::vector<Operation*>&
      getOperations() {return _operations;}
    inline const SceneNode* getSceneNode() {return _node;}

    inline std::vector<Operation*>&
      getStartupOperations() {return _startupOperations;}
    inline std::vector<Operation*>&
      getFinishOperations() {return _finishOperations;}

    void addOperation(Operation* op);
    void removeOperation(Operation* op);

    void addStartupOperation(Operation* op);
    void removeStartupOperation(Operation* op);

    void addFinishOperation(Operation* op);
    void removeFinishOperation(Operation* op);

    inline const EOperationExecutionType getExecutionType() const {return _executionType;}
    inline void setExecutionType(EOperationExecutionType exType) {_executionType = exType;}
    inline void setExecuted(bool executed) {_executed = executed;}
    inline const bool getExecuted() const {return _executed;}

  private:
    const SceneNode* _node;
    uint64 _id;
    std::vector<Operation*> _startupOperations;
    std::vector<Operation*> _finishOperations;
    std::vector<Operation*> _operations;

    EOperationExecutionType _executionType;
    bool _executed;
  };
}
#endif  // KORE_NODESTAGE_H_
