/***************************************************************************
                               sp_fourier.h
                                ----------
    begin                : Sun May 17 2015
    copyright            : (C) 2015 by Vadim Kuznetsov
    email                : ra3xdh@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SP_FOURIER_H
#define SP_FOURIER_H

#include "components/simulation.h"


class SpiceFourier : public qucs::component::SimulationComponent {
public:
  SpiceFourier();
  ~SpiceFourier();
  Component* newOne();
  static Element* info(QString&, char* &, bool getNewOne=false);

protected:
  QString spice_netlist(spicecompat::SpiceDialect dialect = spicecompat::SPICEDefault);
  Qt::GlobalColor color() const override { return Qt::darkRed; }
};

#endif
