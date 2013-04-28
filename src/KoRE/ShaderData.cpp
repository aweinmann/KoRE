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

#include "KoRE/ShaderData.h"

kore::ShaderData::ShaderData(void)
  : type(GL_NONE),
  size(1),
  name("UNDEFINED"),
  data(NULL),
  component(NULL) {
}

kore::ShaderData::ShaderData(const GLenum _type, const GLuint _size,
                             const std::string& _name, void* _data,
                             SceneNodeComponent* _comp) :
  type(_type),
  size(_size),
  name(_name),
  data(_data),
  component(_comp) {
}
