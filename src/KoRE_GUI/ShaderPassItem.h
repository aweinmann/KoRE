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

/************************************************************************/
/* \author Dominik Ospelt                                               */
/************************************************************************/

#ifndef SHADERPASSITEM_H_
#define SHADERPASSITEM_H_

#include <QGraphicsItem>
#include "KoRE/ShaderProgram.h"
#include "KoRE_GUI/NodeItem.h"
#include "KoRE/Passes/ShaderProgramPass.h"
#include "KoRE/Passes/NodePass.h"

namespace koregui {
  class ShaderInputItem;
  class ShaderPassItem : public QGraphicsItem {
  public:
    ShaderPassItem(QGraphicsItem* parent = 0);
    ~ShaderPassItem(void);

    void refresh(void);
    inline int getHeight(void) {return _shaderheight;};
    inline int getWidth(void) {return _shaderwidth;};
    inline kore::ShaderProgramPass* getProgramPass(void) {return _programpass;}
    void setShaderProgram(kore::ShaderProgram* prog);

  protected:
    QRectF boundingRect() const;
    void paint(QPainter* painter,
      const QStyleOptionGraphicsItem* option,
      QWidget* widget);
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

  private:
    kore::ShaderProgramPass* _programpass;
    kore::ShaderProgram* _shader;
    std::vector<ShaderInputItem*> _attributes;
    std::vector<ShaderInputItem*> _uniforms;
    std::vector<ShaderInputItem*> _sampler;
    std::vector<NodeItem*> _connectedNodes;
    std::vector<kore::NodePass*> _nodePasses;
    uint _shaderwidth;
    uint _shaderheight;
    std::string _name;
  };
}
#endif  // SHADERPASSITEM_H_
