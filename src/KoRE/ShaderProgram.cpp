/*
  Copyright Â© 2012 The KoRE Project

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

#include <vector>
#include <string>
#include "KoRE/ShaderProgram.h"
#include "KoRE/Log.h"
#include "KoRE/Operations/Operation.h"
#include "KoRE/ResourceManager.h"
#include "Kore/RenderManager.h"
#include "KoRE/IndexedBuffer.h"
#include "KoRE/IDManager.h"

const unsigned int BUFSIZE = 100;  // Buffer length for shader-element names

kore::ShaderProgram::ShaderProgram()
  : _name(""),
  _programHandle(KORE_GLUINT_HANDLE_INVALID),
  _vertex_prog(NULL),
  _geometry_prog(NULL),
  _fragment_prog(NULL),
  _tess_ctrl(NULL),
  _tess_eval(NULL),
  kore::BaseResource() {
}

kore::ShaderProgram::~ShaderProgram(void) {
  destroyProgram();
}

void kore::ShaderProgram::destroyProgram() {
  if(_programHandle != KORE_GLUINT_HANDLE_INVALID) {
    glDeleteProgram(_programHandle);
    _programHandle = KORE_GLUINT_HANDLE_INVALID;
  }


  _outputs.clear();
  _name = "";
  _uniforms.clear();
  _attributes.clear();
  _imgAccessParams.clear();
}

void kore::ShaderProgram::removeShaders() {
  _vertex_prog = NULL;
  _geometry_prog = NULL;
  _fragment_prog = NULL;
  _tess_ctrl = NULL;
  _tess_eval = NULL;
}


void kore::ShaderProgram::loadShader(const std::string& file,
                                     GLenum shadertype) {
  kore::Shader* shader = NULL;
  uint64 sid = kore::IDManager::getInstance()->getID(file);
  if(sid == 0) {  // Shader not found in cache.
    shader = new kore::Shader();
    shader->loadShaderCode(file, shadertype);
    kore::IDManager::getInstance()->registerURL(shader->getID(), file);
  } else {
    shader = kore::ResourceManager::getInstance()->getShader(sid);
  }

  switch (shadertype) {
  case GL_VERTEX_SHADER:
    _vertex_prog = shader;
    break;
  case GL_FRAGMENT_SHADER:
    _fragment_prog = shader;
    break;
  case GL_GEOMETRY_SHADER:
    _geometry_prog = shader;
    break;
  case GL_TESS_CONTROL_SHADER:
    _tess_ctrl = shader;
    break;
  case GL_TESS_EVALUATION_SHADER:
    _tess_eval = shader;
    break;
  }
}

kore::Shader* kore::ShaderProgram::getShader(GLenum shadertype) {
  switch (shadertype) {
  case GL_VERTEX_SHADER:
    return _vertex_prog;
    break;
  case GL_FRAGMENT_SHADER:
    return _fragment_prog;
    break;
  case GL_GEOMETRY_SHADER:
    return _geometry_prog;
    break;
  case GL_TESS_CONTROL_SHADER:
    return _tess_ctrl;
    break;
  case GL_TESS_EVALUATION_SHADER:
    return _tess_eval;
    break;
  default:
    return NULL;
  }
}

bool kore::ShaderProgram::init() {
  if (_programHandle != KORE_GLUINT_HANDLE_INVALID) {
    destroyProgram();
  }

  _programHandle = glCreateProgram();
  
  if (_vertex_prog) {
    glAttachShader(_programHandle, _vertex_prog->getHandle());
  }

  if (_fragment_prog) {
    glAttachShader(_programHandle, _fragment_prog->getHandle());
  }

  if (_geometry_prog) {
    glAttachShader(_programHandle, _geometry_prog->getHandle());
  }

  if (_tess_ctrl) {
    glAttachShader(_programHandle, _tess_ctrl->getHandle());
  }

  if (_tess_eval) {
    glAttachShader(_programHandle, _tess_eval->getHandle());
  }

  glLinkProgram(_programHandle);
    
  bool success = checkProgramLinkStatus(_programHandle, _name);
  if (!success) {
    destroyProgram();
    return false;
  }

  constructShaderInputInfo(GL_ACTIVE_ATTRIBUTES, _attributes);
  for (uint i = 0; i < _attributes.size(); i++) {
      kore::Log::getInstance()->write("\tAttribute '%s' at location %i\n",
          _attributes[i].name.c_str(),
          _attributes[i].location);
  }
  constructShaderInputInfo(GL_ACTIVE_UNIFORMS, _uniforms);
  for (uint j = 0; j < _uniforms.size(); j++) {
      kore::Log::getInstance()->write("\tUniform '%s' at location %i\n",
          _uniforms[j].name.c_str(),
          _uniforms[j].location);
  }

  /*
  /* OpenGL 4.3 or arb_program_interface_query needed
  /*
  constructShaderOutputInfo(_outputs);
  for (uint j = 0; j < _outputs.size(); j++) {
      kore::Log::getInstance()->write("\tOutput '%s'\n",
          _outputs[j].name.c_str());
  }
  */
  return success == GL_TRUE;
}

GLuint kore::ShaderProgram::getAttributeLocation(const std::string &name) {
  return 0;
}

GLuint kore::ShaderProgram::getUniformLocation(const std::string &name) {
  return glGetUniformLocation(_programHandle, name.c_str());
}

GLuint kore::ShaderProgram::getProgramLocation() const {
    return _programHandle;
}

const std::vector<kore::ShaderInput>& kore::ShaderProgram
  ::getAttributes() const {
    return _attributes;
}

const std::vector<kore::ShaderInput>& kore::ShaderProgram
  ::getUniforms() const {
    return _uniforms;
}

const std::vector<kore::ShaderOutput>& kore::ShaderProgram
  ::getOutputs() const {
    return _outputs;
}

void kore::ShaderProgram::constructShaderInputInfo(const GLenum activeType,
                                std::vector<kore::ShaderInput>& rInputVector) {
  GLint iNumActiveElements = 0;

  /*
  /* OpenGL 4.3 or arb_program_interface_query needed 
  /*
  glGetProgramInterfaceiv(_programHandle, activeType, GL_ACTIVE_RESOURCES, 
      &iNumActiveElements);

  const GLenum properties[3] = {GL_TYPE, GL_NAME_LENGTH, GL_LOCATION};
  */

  glGetProgramiv(_programHandle,
      activeType,
      &iNumActiveElements);

  for (int i = 0; i < iNumActiveElements; ++i) {
    GLchar szNameBuf[BUFSIZE];
    GLsizei iActualNameLength = 0;
    GLint iElementSize = 1;
    GLenum eElementType;
    GLint iElementLoc = -1;

    /*
    /* OpenGL 4.3 or arb_program_interface_query needed 
    /*
    GLint values[3];       
    glGetProgramResourceiv(_programHandle, activeType, i, 3, properties, 
        BUFSIZE, NULL, values);

    eElementType = values[0];
    iActualNameLength = values[1];
    iElementLoc = values[2];

    glGetProgramResourceName(_programHandle, activeType, i, BUFSIZE, 
        &iActualNameLength, szNameBuf);
    */

    if (activeType == GL_ACTIVE_ATTRIBUTES) {
        glGetActiveAttrib(_programHandle, i, BUFSIZE, &iActualNameLength,
            &iElementSize, &eElementType, szNameBuf);
        iElementLoc = glGetAttribLocation(_programHandle, szNameBuf);
    }
    if (activeType == GL_ACTIVE_UNIFORMS){
        glGetActiveUniform(_programHandle, i, BUFSIZE, &iActualNameLength,
            &iElementSize, &eElementType, szNameBuf);
        iElementLoc = glGetUniformLocation(_programHandle, szNameBuf);
    }

    std::string szName = std::string(szNameBuf);
    ShaderInput element;
    element.name = szName;
    element.type = eElementType;
    element.input_type = activeType;
    element.size = iElementSize;
    element.location = iElementLoc;
    element.programHandle = _programHandle;
    element.shader = this;

    rInputVector.push_back(element);
  }

  /*
  /* OpenGL 4.3 or arb_program_interface_query needed 
  /*
  // For Uniform texture-types: add the textureUnit-field
  if (activeType == GL_UNIFORM) {
    GLuint texUnit = 0;
    for (uint i = 0; i < rInputVector.size(); ++i) {
      if (isSamplerType(rInputVector[i].type)) {
        rInputVector[i].texUnit = texUnit;
        ++texUnit;
      }
    }
  }
  */

  // For Uniform texture-types: add the textureUnit-field
  if (activeType == GL_ACTIVE_UNIFORMS) {
    ResourceManager* resourceManager = ResourceManager::getInstance();
    GLuint texUnit = 0;
    GLuint imgUnit = 0;
    GLuint atomicCounterIndex = 0;  // e.g. for atomic counters.

    for (uint i = 0; i < rInputVector.size(); ++i) {
      if (isSamplerType(rInputVector[i].type)) {
        rInputVector[i].texUnit = texUnit;
        
        TexSamplerProperties samplerProperties;
        samplerProperties.type = rInputVector[i].type;
                     
        
        const TextureSampler* sampler =
          resourceManager->requestTextureSampler(samplerProperties);
        
        _vSamplers.push_back(sampler);
        
        ++texUnit;
      }

      // For Image-types: add the imgUnit-field,
      // but don't create a sampler.
      else if (isImageType(rInputVector[i].type)) {
        rInputVector[i].imgUnit = imgUnit;
        ++imgUnit;
      
        _imgAccessParams.push_back(GL_READ_WRITE);
      }
      
      else if(isAtomicCounterType(rInputVector[i].type)) {
        // First, get the bindingPoint (set with layout(binding = x) 
        // in GLSL.
        GLint bindingPoint;
        glGetActiveAtomicCounterBufferiv(_programHandle,
                                         atomicCounterIndex,
                                         GL_ATOMIC_COUNTER_BUFFER_BINDING,
                                         &bindingPoint);
                      
        rInputVector[i].atomicCounterBindingPoint = bindingPoint;
        ++atomicCounterIndex;
        
        ResourceManager* resMgr = ResourceManager::getInstance();
        
        if (atomicCounterIndex >= resMgr->getNumIndexedBuffers()) {
          // We need a new indexedBuffer
          IndexedBuffer* acBuffer = new IndexedBuffer;
          uint value = 0;
          acBuffer->create(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), GL_DYNAMIC_COPY, &value);
          rInputVector[i].additionalData = acBuffer;
          resMgr->addIndexedBuffer(acBuffer);
        } else {
          // We can reuse an existing buffer from the resourceManager
          IndexedBuffer* acBuffer = resMgr->getIndexedBufferByIndex(atomicCounterIndex);
          rInputVector[i].additionalData = acBuffer;
        }
      }
    }
  }
}

void kore::ShaderProgram::constructShaderOutputInfo(std::vector<ShaderOutput>& 
                                             rOutputVector) {
    GLint iNumActiveElements = 0;

    glGetProgramInterfaceiv(_programHandle, GL_PROGRAM_OUTPUT, 
        GL_ACTIVE_RESOURCES, &iNumActiveElements);

    const GLenum properties[3] = {GL_TYPE, GL_NAME_LENGTH, GL_LOCATION};

    for (int i = 0; i < iNumActiveElements; ++i) {
        GLchar szNameBuf[BUFSIZE];
        GLsizei iActualNameLength = 0;
        GLint iElementSize = 1;
        GLenum eElementType;
        GLint iElementLoc = -1;

        
        GLint values[3];       
        glGetProgramResourceiv(_programHandle, GL_PROGRAM_OUTPUT, i, 3, 
            properties, BUFSIZE, NULL, values);

        eElementType = values[0];
        iActualNameLength = values[1];
        iElementLoc = values[2];

        glGetProgramResourceName(_programHandle, GL_PROGRAM_OUTPUT, i, BUFSIZE, 
            &iActualNameLength, szNameBuf);

        std::string szName = std::string(szNameBuf);

        ShaderOutput element;
        element.name = szName;
        element.type = eElementType;
        element.size = iElementSize;

        rOutputVector.push_back(element);
    }
 }

bool kore::ShaderProgram::checkProgramLinkStatus(const GLuint programHandle,
                                                 const std::string& name) {
  GLint success;
  glGetProgramiv(programHandle, GL_LINK_STATUS, &success);

  int infologLen = 0;
  glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &infologLen);
  if (infologLen > 1) {
    GLchar * infoLog = new GLchar[infologLen];
    if (infoLog == NULL) {
      kore::Log::getInstance()->write(
        "[ERROR] Could not allocate ShaderInfoLog buffer from '%s'\n",
        name.c_str());
    }
    int charsWritten = 0;
    glGetProgramInfoLog(programHandle, infologLen, &charsWritten, infoLog);
    std::string shaderlog = infoLog;
    kore::Log::getInstance()->write(
      "[DEBUG] '%s' program Log %s\n", name.c_str(), shaderlog.c_str());
    KORE_SAFE_DELETE_ARR(infoLog);
  } else {
    kore::Log::getInstance()->write(
      "[DEBUG] Program '%s' compiled\n", name.c_str());
  }

  return success == GL_TRUE;
}

bool kore::ShaderProgram::isSamplerType(const GLuint uniformType) {
  switch (uniformType) {
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_CUBE_SHADOW:
    case GL_SAMPLER_1D_ARRAY:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_1D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_MULTISAMPLE:
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_SAMPLER_BUFFER:
    case GL_SAMPLER_2D_RECT:
    case GL_SAMPLER_2D_RECT_SHADOW:
    case GL_INT_SAMPLER_1D:
    case GL_INT_SAMPLER_2D:
    case GL_INT_SAMPLER_3D:
    case GL_INT_SAMPLER_CUBE:
    case GL_INT_SAMPLER_1D_ARRAY:
    case GL_INT_SAMPLER_2D_ARRAY:
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_INT_SAMPLER_BUFFER:
    case GL_INT_SAMPLER_2D_RECT:
    case GL_UNSIGNED_INT_SAMPLER_1D:
    case GL_UNSIGNED_INT_SAMPLER_2D:
    case GL_UNSIGNED_INT_SAMPLER_3D:
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
      return true;
  default:
    return false;
  }
}


bool kore::ShaderProgram::isImageType(const GLuint uniformType) {
  switch (uniformType) {
    case GL_IMAGE_1D: 
    case GL_IMAGE_2D: 
    case GL_IMAGE_3D: 
    case GL_IMAGE_2D_RECT: 
    case GL_IMAGE_CUBE: 
    case GL_IMAGE_BUFFER: 
    case GL_IMAGE_1D_ARRAY: 
    case GL_IMAGE_2D_ARRAY: 
    case GL_IMAGE_2D_MULTISAMPLE: 
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY: 
    case GL_INT_IMAGE_1D: 
    case GL_INT_IMAGE_2D: 
    case GL_INT_IMAGE_3D: 
    case GL_INT_IMAGE_2D_RECT: 
    case GL_INT_IMAGE_CUBE: 
    case GL_INT_IMAGE_BUFFER: 
    case GL_INT_IMAGE_1D_ARRAY: 
    case GL_INT_IMAGE_2D_ARRAY: 
    case GL_INT_IMAGE_2D_MULTISAMPLE: 
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY: 
    case GL_UNSIGNED_INT_IMAGE_1D: 
    case GL_UNSIGNED_INT_IMAGE_2D: 
    case GL_UNSIGNED_INT_IMAGE_3D: 
    case GL_UNSIGNED_INT_IMAGE_2D_RECT: 
    case GL_UNSIGNED_INT_IMAGE_CUBE: 
    case GL_UNSIGNED_INT_IMAGE_BUFFER: 
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY: 
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY: 
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE: 
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
      return true;
  default:
      return false;
  }
}

bool kore::ShaderProgram::isAtomicCounterType(const GLuint uniformType) {
  return uniformType == GL_UNSIGNED_INT_ATOMIC_COUNTER;
}


const kore::ShaderInput*
kore::ShaderProgram::getAttribute(const std::string& name) const {
  for (uint i = 0; i < _attributes.size(); ++i) {
    if (_attributes[i].name == name) {
      return &_attributes[i];
    }
  }

  Log::getInstance()->write("[ERROR] Attribute '%s' not found in shader '%s'\n",
                            name.c_str(), _name.c_str());
  return NULL;
}

const kore::ShaderInput*
kore::ShaderProgram::getUniform(const std::string& name) const {
  for (uint i = 0; i < _uniforms.size(); ++i) {
    if (_uniforms[i].name == name) {
      return &_uniforms[i];
    }
  }

  Log::getInstance()->write("[ERROR] Uniform '%s' not found in shader '%s'",
    name.c_str(), _name.c_str());
  return NULL;
}

void kore::ShaderProgram::setSamplerProperties(const uint idx,
                                   const TexSamplerProperties& properties) {
  if (idx > _vSamplers.size() - 1
      || properties == _vSamplers[idx]->getProperties()) {
        return;
  }

  _vSamplers[idx] = ResourceManager::getInstance()->
                                    requestTextureSampler(properties);
}

const GLuint kore::ShaderProgram::
  getImageAccessParam(const uint imgUnit) const {
    if (imgUnit < _imgAccessParams.size()) {
      return _imgAccessParams[imgUnit];
    }

    return KORE_GLUINT_HANDLE_INVALID;
}

void kore::ShaderProgram::setImageAccessParam(const uint imgUnit,
                                              const GLuint access) {
   if (imgUnit < _imgAccessParams.size()) {
     _imgAccessParams[imgUnit] = access;
   }
}
