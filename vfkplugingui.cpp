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
#include "vfkplugingui.h"
#include "qgscontexthelp.h"

//qt includes

//standard includes

VfkPluginGui::VfkPluginGui( QgisInterface *theQgisInterface, QWidget *parent, Qt::WFlags fl )
    : VfkMainWindow( theQgisInterface, parent )
{
}

VfkPluginGui::~VfkPluginGui()
{
}

void VfkPluginGui::on_buttonBox_accepted()
{
  //close the dialog
  //accept();
}

void VfkPluginGui::on_buttonBox_rejected()
{
  //reject();
}

void VfkPluginGui::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( context_id );
}

