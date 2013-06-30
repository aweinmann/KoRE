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

#include "KoRE/Passes/FrameBufferStage.h"
#include "KoRE/Operations/UseFBO.h"
#include "KoRE/ResourceManager.h"
#include "KoRE/Log.h"
#include <algorithm>

kore::FrameBufferStage::FrameBufferStage(void)
  : _frameBuffer(NULL),
    _executionType(EXECUTE_REPEATING),
    _executed(false)
{
  _activeBuffers.push_back(GL_COLOR_ATTACHMENT0);
}
kore::FrameBufferStage::~FrameBufferStage(void) {
  for (uint i = 0; i < _startupOperations.size(); ++i) {
    KORE_SAFE_DELETE(_startupOperations[i]);
  }
  for (uint i = 0; i < _internalStartup.size(); ++i) {
    KORE_SAFE_DELETE(_internalStartup[i]);
  }
  for (uint i = 0; i < _finishOperations.size(); ++i) {
    KORE_SAFE_DELETE(_finishOperations[i]);
  }
  for (uint i = 0; i < _internalFinish.size(); ++i) {
    KORE_SAFE_DELETE(_internalFinish[i]);
  }
  for (uint i = 0; i < _programPasses.size(); ++i) {
    KORE_SAFE_DELETE(_programPasses[i]);
  }
}

void kore::FrameBufferStage::setActiveAttachments(const std::vector<GLenum>& attachments) {
  _activeBuffers.clear();
  for (uint i = 0; i < attachments.size(); i++) {
    if(attachments[i] != GL_DEPTH_ATTACHMENT
        || attachments[i] != GL_DEPTH_STENCIL_ATTACHMENT
        || attachments[i] != GL_STENCIL_ATTACHMENT)
    _activeBuffers.push_back(attachments[i]);
  }
  _activeBuffers = attachments;
  
  for (uint i = 0; i < _internalStartup.size(); i++) {
    if (_internalStartup[i]->getType() == OP_USEFBO) {
      UseFBO* ufbo = static_cast<UseFBO*>(_internalStartup[i]);
      if(_activeBuffers.size() > 0) {
        ufbo->connect(_frameBuffer,
                      GL_FRAMEBUFFER,
                      &_activeBuffers[0],
                      _activeBuffers.size());
      }
      return;
    }
  }
}

void kore::FrameBufferStage::
  addProgramPass(ShaderProgramPass* progPass) {
    if (std::find(_programPasses.begin(), _programPasses.end(), progPass)
        != _programPasses.end()) {
          return;
    }

    _programPasses.push_back(progPass);
}

void kore::FrameBufferStage::
  setFrameBuffer(const FrameBuffer* frameBuffer) {

  for (uint i = 0; i < _internalStartup.size(); ++i) {
    KORE_SAFE_DELETE(_internalStartup[i]);
  }
  for (uint i = 0; i < _internalFinish.size(); ++i) {
    KORE_SAFE_DELETE(_internalFinish[i]);
  }

  _internalStartup.clear();
  _internalFinish.clear();

  _frameBuffer = frameBuffer;

  if (_frameBuffer == kore::FrameBuffer::BACKBUFFER) {
    _activeBuffers.clear();
    _activeBuffers.push_back(GL_BACK_LEFT);
  }

  UseFBO* pUseFBO = new UseFBO;
  if(_activeBuffers.size()>0) {
    pUseFBO->connect(_frameBuffer, GL_FRAMEBUFFER,
      &_activeBuffers[0],
      _activeBuffers.size());
  }
  _internalStartup.push_back(pUseFBO);
}

void kore::FrameBufferStage::removeProgramPass(ShaderProgramPass* progPass) {
  auto it = std::find(_programPasses.begin(), _programPasses.end(), progPass);
  if (it != _programPasses.end()) {
    _programPasses.erase(it);
  }
}

void kore::FrameBufferStage::swapPasses(ShaderProgramPass* which,
                                        ShaderProgramPass* towhere) {
  auto it = std::find(_programPasses.begin(), _programPasses.end(), which);
  auto it2 = std::find(_programPasses.begin(), _programPasses.end(), towhere);

  if(it != _programPasses.end() && it2 != _programPasses.end()) {
   std::iter_swap(it,it2);
  }
}

void kore::FrameBufferStage::addStartupOperation(Operation* op) {
  if (std::find(_startupOperations.begin(), _startupOperations.end(), op)
    != _startupOperations.end()) {
      return;
  }
  _startupOperations.push_back(op);
}

void kore::FrameBufferStage::removeStartupOperation(Operation* op) {
  auto it = std::find(_startupOperations.begin(), _startupOperations.end(), op);
  if(it != _startupOperations.end()) {
    _startupOperations.erase(it);
  }
}

void kore::FrameBufferStage::addFinishOperation(Operation* op) {
  if (std::find(_finishOperations.begin(), _finishOperations.end(), op)
    != _finishOperations.end()) {
      return;
  }
  _finishOperations.push_back(op);
}

void kore::FrameBufferStage::removeFinishOperation(Operation* op) {
  auto it = std::find(_finishOperations.begin(), _finishOperations.end(), op);
  if(it != _finishOperations.end()) {
    _finishOperations.erase(it);
  }
}
