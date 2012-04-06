/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef VfkPluginGUI_H
#define VfkPluginGUI_H

#include "vfkmainwindow.h"

#include <qgisinterface.h>

#include <QDialog>


/**
@author
*/
class VfkPluginGui : public VfkMainWindow
{
    Q_OBJECT
  public:
    VfkPluginGui( QgisInterface *theQgisInterface, QWidget* parent = 0 );
    ~VfkPluginGui();
};

#endif
